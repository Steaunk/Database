#include "ql.h"
#include "../sm/sm.h"
#include "../rm/rm.h"
#include "../ix/ix.h"
#include <cstring>
#include <fstream>
#include <iostream>


ostream &operator<<(ostream &s, const RelAttr &ra){
    s << ra.relName << "." << ra.attrName;
    return s;
}

QL_Manager::QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm):smm(&smm), ixm(&ixm), rmm(&rmm){}

QL_Manager::~QL_Manager(){}

void QL_Manager::PrintTable(TableInfo &tableInfo, int nSelAttrs, const RelAttr selAttrs[]){
    if(nSelAttrs == 0){
        for(int i = 0; i < tableInfo.columnNum; ++i)
            std::cout << "\t" << tableInfo.name << "." << tableInfo.columnAttr[i].name;
    }
    else {
        for(int i = 0; i < nSelAttrs; ++i)
            std::cout << "\t" << selAttrs[i];
    }
    std::cout << std::endl; 
}

void QL_Manager::PrintData(TableInfo &tableInfo, int nSelAttrs, const RelAttr selAttrs[], char *data){

    if(nSelAttrs == 0){
        for(int i = 0; i < tableInfo.columnNum; ++i){
            std::cout << "\t";
            switch (tableInfo.columnAttr[i].attrType)
            {
            case INT:
                std::cout << *((int*)data);
                break;
            case FLOAT:
                std::cout << *((float*)data);
                break;
            case STRING:
                std::cout << *((char*)data);
                break;

            default:
                assert(false);
                break;
            }
            data += tableInfo.columnAttr[i].attrLength;
        }
    }
    else {
        for(int i = 0; i < nSelAttrs; ++i){
            std::cout << "\t";
            int ID;
            smm->GetColumnIDByName(selAttrs[i].attrName, &tableInfo, ID);
            int offset = CntAttrOffset(&tableInfo, ID);
            switch (tableInfo.columnAttr[ID].attrType)
            {
            case INT:
                std::cout << *((int*)(data+offset));
                break;
            case FLOAT:
                std::cout << *((float*)(data+offset));
                break;
            case STRING:
                std::cout << *((char*)(data+offset));
                break;

            default:
                assert(false);
                break;
            }
        }
    }
    std::cout << std::endl; 
}

bool QL_Manager::UseIndex(TableInfo &tableInfo, int nConditions, const Condition conditions[], int &indexNo, int &cid){
    /*if(tableInfo.primaryKey.keyNum > 0){
        indexNo = -1;
        return true;
    }*/
    for(int i = 0; i < nConditions; ++i){
        if(conditions[i].bRhsIsAttr == false){
            int ID;
            if(smm->GetColumnIDByName(conditions[i].lhsAttr.attrName, &tableInfo, ID) != OK_RC) assert(false); //Only use after check columns
            if(tableInfo.primaryKey.keyNum > 0 && ID == tableInfo.primaryKey.columnID[0]){
                indexNo = -1;
                cid = i;
                return true;
            }
            for(int j = 0; j < tableInfo.indexNum; ++j) if(tableInfo.index[j].columnID == ID){
                indexNo = j;
                cid = i;
                return true; 
            }
        }
    }
    return false;
}

RC QL_Manager::GetConditionInfo(TableInfo &tableInfo, int nConditions, const Condition conditions[], 
                    AttrType *attrType, int *attrLength, int *lAttrOffset, int *rAttrOffset){
    for(int i = 0; i < nConditions; ++i){
        int ID;
        smm->GetColumnIDByName(conditions[i].lhsAttr.attrName, &tableInfo, ID);
        attrType[i] = tableInfo.columnAttr[ID].attrType;
        attrLength[i] = tableInfo.columnAttr[ID].attrLength;
        lAttrOffset[i] = CntAttrOffset(&tableInfo, ID);
        if(conditions[i].bRhsIsAttr){
            smm->GetColumnIDByName(conditions[i].rhsAttr.attrName, &tableInfo, ID);
            rAttrOffset[i] = CntAttrOffset(&tableInfo, ID);
        }
    }
    return OK_RC;
}
 

RC QL_Manager::SelectChecked(int nSelAttrs,
                   const RelAttr selAttrs[],
                   const char *relName,
                   int nConditions,
                   const Condition conditions[],
                   int limit,
                   int offset){
    RC rc = OK_RC;

    TableInfo tableInfo;
    TRY(smm->GetTableInfo(relName, tableInfo));

    int indexNo, cid;
    bool useIndex = UseIndex(tableInfo, nConditions, conditions, indexNo, cid);

    debug(useIndex ? "Index Speed Up\n" : "Nothing happened\n");

    RM_FileHandle rmfh;
    AttrType attrType[nConditions];
    int attrLength[nConditions];
    int lAttrOffset[nConditions];
    int rAttrOffset[nConditions];
    GetConditionInfo(tableInfo, nConditions, conditions, attrType, attrLength, lAttrOffset, rAttrOffset);

    TRY(rmm->OpenFile(smm->RMName(relName).c_str(), rmfh));

    int cnt = 0;

    PrintTable(tableInfo, nSelAttrs, selAttrs);

    auto func = [&](char *data, bool &flag){
        for(int i = 0; i < nConditions; ++i){
            auto &c = conditions[i];
            if(!RM_FileHandle::Comp(attrType[i], attrLength[i], c.op, 
                                    (void *)(data + lAttrOffset[i]),
                                    (void *)(c.bRhsIsAttr ? (data + rAttrOffset[i]) : c.rhsValue.data))){
                flag = false;
                break;
            } 
        }
        
    };

    if(useIndex == false){
        RM_FileScan rmfs;
        rmfs.OpenScan(rmfh, AttrType(0), 0, 0, NO_OP, nullptr);
        RM_Record rec;
        int num = 0;
        while((rc = rmfs.GetNextRec(rec)) != RM_EOF){
            if(rc != OK_RC) goto safe_exit;
            char *data;
            rec.GetData(data);
            bool flag = true;
            func(data, flag);
            if(flag == true){
                if(limit == -1 ||(num >= offset && num < offset + limit)){
                    PrintData(tableInfo, nSelAttrs, selAttrs, data);
                    cnt++;
                }
                if(limit != -1 && num >= offset + limit){
                    break;
                }
                ++num;
            }
        }
        rmfs.CloseScan();
    }
    else{
        IX_IndexHandle ixih;
        ixm->OpenIndex(relName, indexNo, ixih);
        IX_IndexScan ixis;
        ixis.OpenScan(ixih, conditions[cid].op, conditions[cid].rhsValue.data);//rmfh, AttrType(0), 0, 0, NO_OP, nullptr);
        RM_Record rec;
        RID rid;
        int num = 0;
        while((rc = ixis.GetNextEntry(rid)) != IX_EOF){
            debug("RC middle rc = %d\n", rc);
            if(rc != OK_RC) goto safe_exit;
            debug("RC middle rc = %d\n", rc);
            rmfh.GetRec(rid, rec);
            char *data;
            rec.GetData(data);
            bool flag = true;
            func(data, flag);
            if(flag == true){
                if(limit == -1 ||(num >= offset && num < offset + limit)){
                    PrintData(tableInfo, nSelAttrs, selAttrs, data);
                    cnt++;
                }
                if(limit != -1 && num >= offset + limit){
                    break;
                }
                ++num;
            }
        }
        //ixis.CloseScan();
        debug("RC next rc = %d\n", rc);
    }
    std::cout << "\nQuery OK, " << cnt << " rows in set\n";

safe_exit:
    if(rc == IX_EOF) rc = OK_RC;
    TRY(rmm->CloseFile(rmfh));
    debug("RC rc = %d\n", rc);
    return rc; 
}

RC QL_Manager::Select  (int           nSelAttrs,        // # attrs in Select clause
            const RelAttr selAttrs[],       // attrs in Select clause
            int           nRelations,       // # relations in From clause
            const char * const relations[], // relations in From clause
            int           nConditions,      // # conditions in Where clause
            const Condition conditions[],
            int limit,
            int offset){

    debug("CheckColumn Pass!\n");
    
    for(int i = 0; i < nSelAttrs; ++i){
        debug("%s.%s\n", selAttrs[i].relName, selAttrs[i].attrName);
    }
    for(int i = 0; i < nRelations; ++i){
        debug("%s\n", relations[i]);
    }
    for(int i = 0; i < nConditions; ++i){
        debug("[%s]\n", conditions[i].lhsAttr.attrName);
    }


    TRY(CheckColumn(nSelAttrs, selAttrs, nRelations, relations, nConditions, conditions));

    if(nRelations == 1){
        TRY(SelectChecked(nSelAttrs, selAttrs, relations[0], nConditions, conditions,limit,offset));
        //TableInfo tableInfo;
    }
    else{
        debug("BEGIN\n");
        string name = relations[0];
        for(int i = 1; i < nRelations; ++i){
            debug("test");
            smm->InnerJoin(name.c_str(), relations[i+1]);
            name = smm->RelNameCat(name.c_str(), relations[i+1]);
        }
        smm->DescTable(name.c_str());
    }
        //TRY(smm->GetTableInfo())
        //bool useIndex = false;

    debug("Select END\n");

    return OK_RC;
}  // conditions in Where clause

RC QL_Manager::Insert  (const char  *relName,           // relation to insert into
            int         nValues,            // # values to insert
            const Value values[]){
    //主键约束 TODO
    TRY(SM_Manager::TableExist(relName));
    std::cout << nValues << std::endl;
    std::cout << *((int*)values[3].data) << std::endl;
    RC rc = OK_RC;
    TableInfo tableInfo;
    TRY(smm->GetTableInfo(relName, tableInfo));
    if(nValues != tableInfo.columnNum) return QL_DATA_NOT_MATCH;
    char *data = (char *)malloc(tableInfo.size);
    for(int i = 0; i < nValues; ++i){
        if(values[i].type != tableInfo.columnAttr[i].attrType){
            std::cout << "'" << tableInfo.columnAttr[i].name << "' should be " << AttrTypeMsg[tableInfo.columnAttr[i].attrType] << "\n";
            return QL_DATA_NOT_MATCH;
        }
        if(values[i].type == STRING){
            int length = strlen((char *)values[i].data);
            if(length > tableInfo.columnAttr[i].attrLength){
                std::cout << tableInfo.columnAttr[i].name << "' should be less than " << tableInfo.columnAttr[i].attrLength << "\n";
                return QL_DATA_NOT_MATCH;
            }
        }
        memcpy(data, values[i].data, tableInfo.columnAttr[i].attrLength);
        data += tableInfo.columnAttr[i].attrLength;
    }
    data -= tableInfo.size;
    for(int i = 0; i < nValues; ++i){

    }
    //memcpy(data, &values)
    rc = CheckPrimaryKey(relName, data);
    if(rc == OK_RC) rc = CheckForeignKeyInsert(relName, data);
    if(rc == OK_RC) rc = InsertAll(tableInfo, data);
    free(data);
    return rc;

}          // values to insert

RC QL_Manager::Delete  (const char *relName,            // relation to delete from
            int        nConditions,         // # conditions in Where clause
            const Condition conditions[]){
    TRY(DeleteOrUpdate(relName, nConditions, conditions, 0, nullptr, nullptr));
    return OK_RC;
}  

RC QL_Manager::Update  (const char *relName,            // relation to update
            int nUpdAttr,
            const RelAttr updAttr[],
            const Value rhsValue[],          // value on RHS of =
            int   nConditions,              // # conditions in Where clause
            const Condition conditions[]){
    TRY(SM_Manager::TableExist(relName));
    TRY(DeleteOrUpdate(relName, nConditions, conditions, nUpdAttr, updAttr, rhsValue));
    return OK_RC;
}  

RC QL_Manager::InsertIndex(const char *relName, const char *attrName){
    int indexid = -1;
    TableInfo table;
    smm->ReadData(relName,&table);
    for(int i = 0; i < MAX_COLUMN_NUM; ++i){
        if(strcmp(attrName, table.index[i].name) == 0){
            indexid = i;
            break;
        }
    }
    if(indexid == -1){
        return QL_UNKNOW_INDEX;
    }
    int columnid = table.index[indexid].columnID;
    IX_IndexHandle ixh;
    (ixm->OpenIndex(relName,indexid,ixh));
    RM_FileHandle rmh;
    rmm->OpenFile(smm->RMName(relName).c_str(),rmh);
    RM_FileScan rms;
    rms.OpenScan(rmh,INT,0,0,NO_OP,nullptr);
    RM_Record record;
    int offset = CntAttrOffset(&table,columnid),
    length = table.columnAttr[columnid].attrLength;
    RC rc;
    while((rc = rms.GetNextRec(record)) != RM_EOF){
        if(rc!=0){
            rms.CloseScan();
            rmm->CloseFile(rmh);
            ixm->CloseIndex(ixh);
            smm->DropIndex(relName,attrName);
            return rc;
        }
        char *data;
        record.GetData(data);
        RID rid;
        record.GetRid(rid);
        ixh.InsertEntry(data+offset,rid);
    }
    rms.CloseScan();
    rmm->CloseFile(rmh);
    ixm->CloseIndex(ixh);
    return OK_RC;
}

int QL_Manager::CntAttrOffset(TableInfo *tableInfo, int id){
    int offset = 0;
    for(int i = 0; i < id; ++i){
        offset += tableInfo->columnAttr[i].attrLength;
    }
    return offset;
}

RC QL_Manager::CheckForeignKeyInsert(const char *relName, char *data){
    TableInfo tableInfo;
    TRY(smm->GetTableInfo(relName, tableInfo));

    if(tableInfo.foreignNum == 0) return OK_RC;

    for(int i = 0; i < tableInfo.foreignNum; ++i){
        auto info = tableInfo.foreignKey[i];
        TableInfo refTableInfo;
        smm->GetTableInfo(info.refRelName, refTableInfo);

        bool result = CheckForeignKeyEqual(relName, info.refRelName, info.keyNum, info.columnID, info.refColumnID, data);

        if(result == false) return QL_FOREIGNKEY;
    }
    return OK_RC;
}

RC QL_Manager::CheckForeignKeyDelete(const char *relName, char *data){
    debug("Check Foreign Key Delete\n");
    TableInfo refTableInfo;
    TRY(smm->GetTableInfo(relName, refTableInfo));

    if(refTableInfo.primaryKey.keyNum == 0) return OK_RC;
    if(refTableInfo.primaryKey.referenceNum == 0) return OK_RC;

    for(int i = 0; i < refTableInfo.primaryKey.referenceNum; ++i){
        auto info = refTableInfo.primaryKey.references[i];
        TableInfo tableInfo;
        smm->GetTableInfo(info.relName, tableInfo);

        bool result = CheckForeignKeyEqual(relName, info.relName, info.keyNum, info.refColumnID, info.columnID, data);
        debug("Result = %d\n", result);
        if(result == true) return QL_FOREIGNKEY;
    }
    return OK_RC;
}

bool QL_Manager::CheckForeignKeyEqual(const char *relNameA, const char *relNameB, int keyNum, int *columnIDA, int *columnIDB, char *dataA){
    TableInfo tableInfoA;
    smm->GetTableInfo(relNameA, tableInfoA);
    TableInfo tableInfoB;
    smm->GetTableInfo(relNameB, tableInfoB);

    if(keyNum == 0) return OK_RC;

    RM_FileHandle rmfh;
    rmm->OpenFile(smm->RMName(relNameB).c_str(), rmfh);
    RM_FileScan rmfs;
    rmfs.OpenScan(rmfh, AttrType(0), 0, 0, NO_OP, nullptr);
    RM_Record trec;
    char *dataB;
    RC rc;
    while((rc = rmfs.GetNextRec(trec)) != RM_EOF){
        if(rc != OK_RC) break;
        bool flag = true;
        trec.GetData(dataB);
        //debug("dataB <%d %d %d>\n", *((int *)dataB), *((int *)(dataB + 4)), *((int *)(dataB + 8)));
        for(int i = 0; i < keyNum; ++i){
            auto &t = tableInfoA.columnAttr[columnIDA[i]];
            if(RM_FileHandle::Comp(t.attrType, t.attrLength, EQ_OP, 
                                dataA + CntAttrOffset(&tableInfoA, columnIDA[i]), 
                                dataB + CntAttrOffset(&tableInfoB, columnIDB[i])) == 0){
                flag = false;
                break;
            }
        }

       
        if(flag == true){
            rmfs.CloseScan();
            rmm->CloseFile(rmfh);
            return true;
        }
    } 
    rmfs.CloseScan();
    rmm->CloseFile(rmfh);
    return false;
}

RC QL_Manager::CheckColumn(int  nSelAttrs,
                 const RelAttr selAttrs[],
                 int           nRelations,       // # relations in From clause
                 const char * const relations[], // relations in From clause
                 int           nConditions,      // # conditions in Where clause
                 const Condition conditions[]){
    
    for(int i = 0; i < nRelations; ++i){
        for(int j = i + 1; j < nRelations; ++j){
            if(strcmp(relations[i], relations[j]) == 0){
                std::cout << "Not unique table\n";
                return QL_NOT_UNIQUE_TABLE;
            }
        }
    }
    debug("CheckColumn Middle\n");
    RC rc = OK_RC;
    TableInfo *tableInfo = new TableInfo[nRelations];
    for(int i = 0; i < nRelations; ++i){
        debug("CheckColumn relations[%d] = %s\n", i, relations[i]);
        SAFE_TRY(smm->GetTableInfo(relations[i], tableInfo[i]));
    }

    for(int i = 0; i < nSelAttrs; ++i){
        bool flag = false;
        int ID;
        for(int j = 0; j < nRelations; ++j){
            if(strcmp(selAttrs[i].relName, relations[j]) == 0 && 
                (smm->GetColumnIDByName(selAttrs[i].attrName, &tableInfo[j], ID) == OK_RC)
            ){
                //debug("1st for [i = %d, j = %d; ID = %d]\n", i, j, ID);
                flag = true;
                break;
           }
        }
        if(flag == false){
            std::cout << "Unknown column '" << selAttrs[i].relName << "." << 
                selAttrs[i].attrName << "' in selectors\n";
            return QL_UNKNOW_COLUMN;
        }
    }
    debug("CheckColumn Where\n");

    for(int i = 0; i < nConditions; ++i){
        bool flag = false;
        int lID, rID, j = 0;

        for(; j < nRelations; ++j){
            if(strcmp(conditions[i].lhsAttr.relName, relations[j]) == 0 && 
                (smm->GetColumnIDByName(conditions[i].lhsAttr.attrName, &tableInfo[j], lID) == OK_RC)
            ){
                flag = true;
                break;
           }
        }
        if(flag == false){
            std::cout << "Unknown column '" << conditions[i].lhsAttr.relName << "." << 
                conditions[i].lhsAttr.attrName << "' in where clause\n";
            return QL_UNKNOW_COLUMN;
        }
        debug("Middle\n");
        flag = true;
        if(conditions[i].bRhsIsAttr == true){
            int k = 0;
            for(; k < nRelations; ++k){
                if(strcmp(conditions[i].rhsAttr.relName, relations[k]) == 0 && 
                    (smm->GetColumnIDByName(conditions[i].rhsAttr.attrName, &tableInfo[k], rID) == OK_RC)
                ){
                    flag = true;
                    break;
                }
            }
            if(flag == false){
                std::cout << "Unknown column '" << conditions[i].lhsAttr.relName << "." << 
                    conditions[i].lhsAttr.attrName << "' in where clause\n";
                return QL_UNKNOW_COLUMN;
            }
            else if(tableInfo[j].columnAttr[lID].attrType != tableInfo[k].columnAttr[rID].attrType){
                std::cout << "Values don't have same type\n";
                return QL_DATA_NOT_MATCH;
            }
        }
        else if(conditions[i].rhsValue.type != tableInfo[j].columnAttr[lID].attrType){
                std::cout << "'" << tableInfo[j].columnAttr[lID].name << "' should be " << AttrTypeMsg[tableInfo[j].columnAttr[lID].attrType] << "\n";
                return QL_DATA_NOT_MATCH;
        }
    }

safe_exit:
    delete[] tableInfo;
    return rc;
}

RC QL_Manager::DeleteOrUpdate (const char *relName,            // relation to delete from
            int        nConditions,         // # conditions in Where clause
            const Condition conditions[],
            int nUpdAttr,
            const RelAttr updAttr[],
            const Value rhsValue[]
){

    debug("Delete nUpdAttr = %d\n", nUpdAttr);
    RC rc;
    TableInfo tableInfo;
    TRY(smm->ReadData(relName, &tableInfo));

    /*if(nConditions == 0){
        rmm->DestroyFile(smm->RMName(relName).c_str());
        rmm->CreateFile(smm->RMName(relName).c_str(), tableInfo.size);
        std::cout << "Query OK, all deleted\n";
        return OK_RC;
    }*/
    //检查是否 where cause 中是否存在该列

    int lAttrOffset[nConditions];
    int rAttrOffset[nConditions];
    AttrType attrType[nConditions];
    int attrLength[nConditions];
    int lID, rID;
    bool useIndex = false;
    CompOp compOp;
    void *value;

    for(int i = 0; i < nConditions; ++i){
        if(strcmp(conditions[i].lhsAttr.relName, relName) ||
            (rc = smm->GetColumnIDByName(conditions[i].lhsAttr.attrName, &tableInfo, lID))
        ){
            std::cout << "Unknown column '" << conditions[i].lhsAttr.relName << "." << 
                conditions[i].lhsAttr.attrName << "' in where clause\n";
            return QL_UNKNOW_COLUMN;
        }
        else if(conditions[i].bRhsIsAttr){
            if(strcmp(conditions[i].rhsAttr.relName, relName) ||
             (rc = smm->GetColumnIDByName(conditions[i].rhsAttr.attrName, &tableInfo, rID))
            ) {
                std::cout << "Unknown column '" << conditions[i].rhsAttr.relName << "." << 
                    conditions[i].rhsAttr.attrName << "' in where clause\n";
                return QL_UNKNOW_COLUMN;
            }
            if(tableInfo.columnAttr[lID].attrType != tableInfo.columnAttr[rID].attrType){
                std::cout << "Values don't have same type\n";
                return QL_DATA_NOT_MATCH;
            }
            rAttrOffset[i] = CntAttrOffset(&tableInfo, rID);
        }
        else {
            if(conditions[i].rhsValue.type != tableInfo.columnAttr[lID].attrType){
                std::cout << "'" << tableInfo.columnAttr[lID].name << "' should be " << AttrTypeMsg[tableInfo.columnAttr[lID].attrType] << "\n";
                return QL_DATA_NOT_MATCH;
            }
            if(!useIndex){
                debug("tableInfo.indexNum = %d\n", tableInfo.indexNum);
                for(int i = 0; i < tableInfo.indexNum; ++i){
                    if(tableInfo.index[i].columnID == lID){
                        useIndex = true;
                        compOp = conditions[i].op;
                        value = conditions[i].rhsValue.data;
                    }
                }
            }
        }
        lAttrOffset[i] = CntAttrOffset(&tableInfo, lID);
        attrType[i] = tableInfo.columnAttr[lID].attrType;
        attrLength[i] = tableInfo.columnAttr[lID].attrLength;
    }
    int updAttrOffset[nUpdAttr + 1];
    int updAttrLength[nUpdAttr + 1];
    if(nUpdAttr > 0){
        int ID;
        for(int i = 0; i < nUpdAttr; ++i){
            if((rc = smm->GetColumnIDByName(updAttr[i].attrName, &tableInfo, ID))){
                 std::cout << "Unknown column '" << updAttr[i].attrName << "' in where clause\n";
                return QL_UNKNOW_COLUMN;
            }
            updAttrOffset[i] = CntAttrOffset(&tableInfo, ID);
            if(rhsValue[i].type != tableInfo.columnAttr[ID].attrType){
                std::cout << "'" << tableInfo.columnAttr[ID].name << "' should be " << AttrTypeMsg[tableInfo.columnAttr[lID].attrType] << "\n";
                return QL_DATA_NOT_MATCH;
            }
            updAttrLength[i] = tableInfo.columnAttr[ID].attrLength;
        }
    }

    useIndex = false;
    if(useIndex){
        assert(false);
    }


    RM_FileScan rmfs;
    RM_FileHandle rmfh;
    rmm->OpenFile(smm->RMName(relName).c_str(), rmfh);
    rmfs.OpenScan(rmfh, attrType[0], 0, 0, NO_OP, nullptr);
    RM_Record rec;
    int cnt = 0;
    char *data;
    int cnt_f = 0;
    while((rc = rmfs.GetNextRec(rec)) != RM_EOF){
        if(rc != OK_RC){
            std::cout << "RM : " << rc << endl;
            return rc;
        }
        bool flag = true;
        rec.GetData(data);
        std::cout << "(" << *((int *)data) << ", " << *((int *)(data + 4)) << ")" << std::endl;
        for(int i = 0; i < nConditions; ++i){
            auto &c = conditions[i];
            if(!RM_FileHandle::Comp(attrType[i], attrLength[i], c.op, 
                            (void *)(data + lAttrOffset[i]),
                            (void *)(c.bRhsIsAttr ? (data + rAttrOffset[i]) : c.rhsValue.data)
                        )
            ){
                flag = false;
                break;
            } 
        }
        if(flag){
            RID rid; rec.GetRid(rid);
            RC rc = OK_RC;
            if(nUpdAttr == 0){
                rmm->CloseFile(rmfh);
                rc = CheckForeignKeyDelete(relName, data);
                if(rc == OK_RC) rc = DeleteAll(tableInfo, data, rid);//rmfh.DeleteRec(rid);
                if(rc != OK_RC) cnt_f++;
                debug("Delete rc = %d\n", rc);
                rmm->OpenFile(smm->RMName(relName).c_str(), rmfh);
            }
            else {
                rmm->CloseFile(rmfh);
                rc = CheckForeignKeyDelete(relName, data);
                if(rc == OK_RC){
                    char *new_data = (char *) malloc(tableInfo.size);
                    memcpy(new_data, data, tableInfo.size);
                    for(int i = 0; i < nUpdAttr; ++i){
                        memcpy(new_data + updAttrOffset[i], rhsValue[i].data, updAttrLength[i]);
                    }
                    rc = CheckPrimaryKey(relName, data, nullptr, rid);
                    if(rc == OK_RC) rc = CheckForeignKeyInsert(relName, new_data);
                    if(rc != OK_RC) cnt_f++;
                    else UpdateAll(tableInfo, new_data, data, rid);
                    free(new_data);
                }
                else cnt_f++;
                rmm->OpenFile(smm->RMName(relName).c_str(), rmfh);
            }
            if(rc == OK_RC) cnt++;
            debug("CNT %d\n", cnt);
        }
    }
    rmm->CloseFile(rmfh);
    if(nUpdAttr == 0) std::cout << "Query OK, " << cnt << " rows deleted\n";
    else std::cout << "Query OK, " << cnt << " rows updated\n";
    return OK_RC;
}

RC QL_Manager::CheckPrimaryKey(const char *relName, char *data, int *offset, RID Rid){
    TableInfo tableInfo;
    TRY(smm->GetTableInfo(relName, tableInfo));

    if(tableInfo.primaryKey.keyNum == 0) return OK_RC;

    bool remember2delete = false;
    if(offset == nullptr){
        offset = new int[tableInfo.primaryKey.keyNum];
        for(int i = 0; i  < tableInfo.primaryKey.keyNum; ++i)
            offset[i] = CntAttrOffset(&tableInfo, tableInfo.primaryKey.columnID[i]);
        remember2delete = true;
    }

    IX_IndexHandle ixih;
    ixm->OpenIndex(relName, -1, ixih);
    IX_IndexScan ixis;
    RM_FileHandle rmfh;
    rmm->OpenFile(smm->RMName(relName).c_str(), rmfh);
    //int offset = CntAttrOffset(&tableInfo, tableInfo.primaryKey.columnID[0]);
    ixis.OpenScan(ixih, EQ_OP, data + offset[0]);
    RID rid;
    RM_Record trec;
    char *tdata;
    RC rc;
    while((rc = ixis.GetNextEntry(rid)) != IX_EOF){
        PageNum pageNum;
        SlotNum slotNum;
        rid.GetPageNum(pageNum);
        rid.GetSlotNum(slotNum);
        debug("HERE %d %d\n", pageNum, slotNum);
        rc = rmfh.GetRec(rid, trec);
        if(rc != OK_RC) break;
        if(rid == Rid) continue;
        bool flag = false;
        trec.GetData(tdata);
        for(int i = 1; i < tableInfo.primaryKey.keyNum; ++i){
            auto &t = tableInfo.columnAttr[tableInfo.primaryKey.columnID[i]];
            debug("CheckPrimaryKey [%d %d] [%d %d]\n", *((int *)(data)), *((int *)(data + 4)), *((int *)(tdata)), *((int *)(tdata + 4)));
            if(RM_FileHandle::Comp(t.attrType, t.attrLength, EQ_OP, data + offset[i], tdata + offset[i]) == false){
                flag = true;
                break;
            }
        }
        if(flag == false){
            rc = QL_DUPLICATE_ENTRY;
            break;
        }
    }

    if(rc == IX_EOF) rc = OK_RC;

    ixm->CloseIndex(ixih);
    rmm->CloseFile(rmfh);
    if(remember2delete) delete offset;
    debug("CheckPrimaryKey rc = %d\n", rc);
    return rc;

}

RC QL_Manager::InsertAll(TableInfo &tableInfo, char *data){
    RM_FileHandle rmfh;
    //debug("InsertAll Start, tab")
    TRY(rmm->OpenFile(smm->RMName(tableInfo.name).c_str(), rmfh));
    debug("InsertAll OpenFile\n");
    RID rid;
    TRY(rmfh.InsertRec(data, rid));
    TRY(rmm->CloseFile(rmfh));

    IX_IndexHandle ixih;
    for(int i = 0; i < MAX_INDEX_NUM; ++i) if(tableInfo.index[i].columnID != -1){
        debug("InsertAll i = %d\n", i);
        ixm->OpenIndex(tableInfo.name, i, ixih);
        ixih.InsertEntry(data + CntAttrOffset(&tableInfo, tableInfo.index[i].columnID), rid);
        ixm->CloseIndex(ixih);
    }

    if(tableInfo.primaryKey.keyNum > 0){
        ixm->OpenIndex(tableInfo.name, -1, ixih);
        ixih.InsertEntry(data + CntAttrOffset(&tableInfo, tableInfo.primaryKey.columnID[0]), rid);
        ixm->CloseIndex(ixih);
    }

    return OK_RC;
}

RC QL_Manager::DeleteAll(TableInfo &tableInfo, char *data, RID rid){
    RM_FileHandle rmfh;
    TRY(rmm->OpenFile(smm->RMName(tableInfo.name).c_str(), rmfh));
    TRY(rmfh.DeleteRec(rid));
    TRY(rmm->CloseFile(rmfh));
    IX_IndexHandle ixih;
    for(int i = 0; i < MAX_INDEX_NUM; ++i) if(tableInfo.index[i].columnID != -1){
        debug("InsertAll i = %d\n", i);
        ixm->OpenIndex(tableInfo.name, i, ixih);
        ixih.DeleteEntry(data + CntAttrOffset(&tableInfo, tableInfo.index[i].columnID), rid);
        ixm->CloseIndex(ixih);
    }

    if(tableInfo.primaryKey.keyNum > 0){
        ixm->OpenIndex(tableInfo.name, -1, ixih);
        ixih.DeleteEntry(data + CntAttrOffset(&tableInfo, tableInfo.primaryKey.columnID[0]), rid);
        ixm->CloseIndex(ixih);
    }
    return OK_RC;
}

RC QL_Manager::UpdateAll(TableInfo &tableInfo, char *new_data, char *old_data, RID rid){
    RM_FileHandle rmfh;
    TRY(rmm->OpenFile(smm->RMName(tableInfo.name).c_str(), rmfh));
    RM_Record rec; rec.SetData(new_data); rec.SetRid(rid);
    TRY(rmfh.UpdateRec(rec));
    TRY(rmm->CloseFile(rmfh));
    IX_IndexHandle ixih;
    for(int i = 0; i < MAX_INDEX_NUM; ++i) if(tableInfo.index[i].columnID != -1){
        ixm->OpenIndex(tableInfo.name, i, ixih);
        ixih.DeleteEntry(old_data + CntAttrOffset(&tableInfo, tableInfo.index[i].columnID), rid);
        ixih.InsertEntry(new_data + CntAttrOffset(&tableInfo, tableInfo.index[i].columnID), rid);
        ixm->CloseIndex(ixih);
    }

    if(tableInfo.primaryKey.keyNum > 0){
        ixm->OpenIndex(tableInfo.name, -1, ixih);
        ixih.DeleteEntry(old_data + CntAttrOffset(&tableInfo, tableInfo.primaryKey.columnID[0]), rid);
        ixih.InsertEntry(new_data + CntAttrOffset(&tableInfo, tableInfo.primaryKey.columnID[0]), rid);
        ixm->CloseIndex(ixih);
    }
    return OK_RC;
}

RC QL_Manager::Join(const char *relNameA, const char *relNameB){
    smm->InnerJoin(relNameA, relNameB);
    RM_FileHandle rmh,rmha,rmhb;
    rmm->OpenFile(smm->RMName(smm->RelNameCat(relNameA,relNameB).c_str()).c_str(), rmh);
    rmm->OpenFile(smm->RMName(relNameA).c_str(),rmha);
    rmm->OpenFile(smm->RMName(relNameB).c_str(),rmhb);
    RM_FileScan sa,sb;
    sa.OpenScan(rmha,INT,0,0,NO_OP,nullptr);
    sb.OpenScan(rmhb,INT,0,0,NO_OP,nullptr);
    RC rca,rcb;
    RM_Record ra,rb;
    int lena, lenb;
    TableInfo tablea,tableb;
    smm->ReadData(relNameA,&tablea);
    smm->ReadData(relNameB,&tableb);
    lena = tablea.size;
    lenb = tableb.size;
    while((rca = sa.GetNextRec(ra)) != RM_EOF){
        while((rcb = sb.GetNextRec(rb)) != RM_EOF){
            if(rca != 0 || rcb != 0){
                sa.CloseScan();
                sb.CloseScan();
                rmm->CloseFile(rmh);
                rmm->CloseFile(rmha);
                rmm->CloseFile(rmhb);
                return rca == 0?rcb:rca;
            }
            RID temp;
            char *dataa,*datab;
            ra.GetData(dataa);
            rb.GetData(datab);
            char *data = new char [lena+lenb+5];
            memcpy(data,dataa,sizeof(char) * lena);
            memcpy(data+lena,datab,sizeof(char) * lenb);
            rmh.InsertRec(data,temp);
        }
    }
    sa.CloseScan();
    sb.CloseScan();
    rmm->CloseFile(rmh);
    rmm->CloseFile(rmha);
    rmm->CloseFile(rmhb);
    return OK_RC;
}

RC QL_Manager::Load(const char *fileName, const char *relName){
    string fs = fileName;
    fs = fs.substr(1,fs.length()-2);
    // fs = "../.."+fs;
    std::ifstream fin(fs);
    string s;
    TableInfo table;
    smm->ReadData(relName,&table);
    int len = table.columnNum;
    Value *data = new Value [len + 5];
    for(int i = 0; i < len; ++i){
        data[i].type = table.columnAttr[i].attrType;
        data[i].data = new char [table.columnAttr[i].attrLength + 5];
    }
    int total = 0;
    while(getline(fin,s)){
        string curs = "";
        int cnt = 0;
        for(int i = 0; i < s.length(); ++i){
            if(s[i] == ','){
                int x;
                float f;
                if(cnt < len){
                    switch (table.columnAttr[cnt].attrType)
                    {
                        case INT:
                            /* code */
                            x = atoi(curs.c_str());
                            memcpy(data[cnt].data,&x,sizeof(int));
                            break;
                        case FLOAT:
                            /* code */
                            f = atof(curs.c_str());
                            memcpy(data[cnt].data,&f,sizeof(float));
                            break;
                        case STRING:
                            /* code */
                            memcpy(data[cnt].data,curs.c_str(),sizeof(char) * (curs.length() + 1));
                            break;
                        
                        default:
                            break;
                    }
                }
                ++cnt;
                curs = "";
            }
            else curs += s[i];
        }
        int x;
        float f;
        if(cnt < len){
            switch (table.columnAttr[cnt].attrType)
            {
                case INT:
                    /* code */
                    x = atoi(curs.c_str());
                    memcpy(data[cnt].data,&x,sizeof(int));
                    break;
                case FLOAT:
                    /* code */
                    f = atof(curs.c_str());
                    memcpy(data[cnt].data,&f,sizeof(float));
                    break;
                case STRING:
                    /* code */
                    memcpy(data[cnt].data,curs.c_str(),sizeof(char) * (curs.length() + 1));
                    break;
                
                default:
                    break;
            }
        }
        RC rc;
        rc = Insert(relName,len,data);
        if(rc){
            cout << "Incorrect data '" << s << "'\n";
        }else{
            ++total;
        }
    }
    for(int i = 0; i < len; ++i){
        delete[] data[i].data;
    }
    delete[] data;
    cout << "Successfully add " << total << " lines\n";
    return OK_RC;
}

RC QL_Manager::Store(const char *fileName, const char *relName){
    string fs = fileName;
    fs = fs.substr(1,fs.length()-2);
    // fs = "../.."+fs;
    std::ofstream fout(fs);
    TableInfo table;
    smm->ReadData(relName,&table);
    int len = table.columnNum;
    RM_FileHandle rmh;
    rmm->OpenFile(smm->RMName(relName).c_str(),rmh);
    RM_FileScan rms;
    rms.OpenScan(rmh,INT,0,0,NO_OP,nullptr);
    RC rc;
    RM_Record record;
    while((rc = rms.GetNextRec(record)) != RM_EOF){
        if(rc!=0){
            rms.CloseScan();
            rmm->CloseFile(rmh);
            return rc;
        }
        char *data;
        record.GetData(data);
        for(int i = 0; i < len; ++i){
            if(i)fout << ",";
            int x;
            float f;
            switch (table.columnAttr[i].attrType)
            {
                case INT:
                    /* code */
                    fout << *((int*)data);
                    break;
                case FLOAT:
                    /* code */
                    fout << *((float*)data);
                    break;
                case STRING:
                    /* code */
                    fout << data;
                    break;
                
                default:
                    break;
            }
            data += table.columnAttr[i].attrLength;
        }
        fout << "\n";
    }
    rms.CloseScan();
    rmm->CloseFile(rmh);
    return OK_RC;
}