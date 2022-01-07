#include "ql.h"
#include "../sm/sm.h"
#include "../rm/rm.h"
#include "../ix/ix.h"
#include <cstring>

QL_Manager::QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm):smm(&smm), ixm(&ixm), rmm(&rmm){}

QL_Manager::~QL_Manager(){}

RC QL_Manager::Select  (int           nSelAttrs,        // # attrs in Select clause
            const RelAttr selAttrs[],       // attrs in Select clause
            int           nRelations,       // # relations in From clause
            const char * const relations[], // relations in From clause
            int           nConditions,      // # conditions in Where clause
            const Condition conditions[]){
    return OK_RC;
            }  // conditions in Where clause
RC QL_Manager::Insert  (const char  *relName,           // relation to insert into
            int         nValues,            // # values to insert
            const Value values[]){
    //主键约束 TODO
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
    TRY(DeleteOrUpdate(relName, nConditions, conditions, nUpdAttr, updAttr, rhsValue));
    return OK_RC;
}  

int QL_Manager::CntAttrOffset(TableInfo *tableInfo, int id){
    int offset = 0;
    for(int i = 0; i < id; ++i){
        offset += tableInfo->columnAttr[i].attrLength;
    }
    return offset;
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
                conditions[i].lhsAttr.attrName << "' in 'where clause\n";
            return QL_UNKNOW_COLUMN;
        }
        else if(conditions[i].bRhsIsAttr){
            if(strcmp(conditions[i].rhsAttr.relName, relName) ||
             (rc = smm->GetColumnIDByName(conditions[i].rhsAttr.attrName, &tableInfo, rID))
            ) {
                std::cout << "Unknown column '" << conditions[i].rhsAttr.relName << "." << 
                    conditions[i].rhsAttr.attrName << "' in 'where clause\n";
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
                 std::cout << "Unknown column '" << updAttr[i].attrName << "' in 'where clause\n";
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

    if(useIndex){
        assert(false);
    }


    RM_FileScan rmfs;
    RM_FileHandle rmfh;
    rmm->OpenFile(smm->RMName(relName).c_str(), rmfh);
    rmfs.OpenScan(rmfh, attrType[0], attrLength[0], 0, NO_OP, nullptr);
    RM_Record rec;
    int cnt = 0;
    printf("<%d>\n", RM_EOF);
    char *data;
    while((rc = rmfs.GetNextRec(rec)) != RM_EOF){
        if(rc != OK_RC){
            std::cout << "RM : " << rc << endl;
            return rc;
        }
        bool flag = true;
        
        rec.GetData(data);
        std::cout << "(" << *((int *)data) << ")" << std::endl;
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
            if(nUpdAttr == 0){
                PageNum pageNum;
                SlotNum slotNum;
                rid.GetPageNum(pageNum);
                rid.GetSlotNum(slotNum);
                debug("Delete [%d %d]\n", pageNum, slotNum);
                rmfh.DeleteRec(rid);
                RC rc = rmfh.DeleteRec(rid);
                debug("Delete rc = %d\n", rc);
            }
            else {
                for(int i = 0; i < nUpdAttr; ++i){
                    memcpy(data + updAttrOffset[i], rhsValue[i].data, updAttrLength[i]);
                }
                rmfh.UpdateRec(rec);
            }
            cnt++;
        }
    }
    rmm->CloseFile(rmfh);
    if(nUpdAttr == 0) std::cout << "Query OK, " << cnt << " rows deleted\n";
    else std::cout << "Query OK, " << cnt << " rows updated\n";
    return OK_RC;
}

RC QL_Manager::CheckPrimaryKey(const char *relName, char *data, int *offset){
    TableInfo tableInfo;
    TRY(smm->GetTableInfo(relName, tableInfo));

    if(tableInfo.primaryKey.keyNum == 0) return OK_RC;

    bool remember2delete = false;
    if(offset == nullptr){
        offset = new int[tableInfo.primaryKey.keyNum];
        for(int i = 0; i  < tableInfo.primaryKey.keyNum; ++i)
            CntAttrOffset(&tableInfo, offset[i]);
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
    debug("WHST?\n");
    while((rc = ixis.GetNextEntry(rid)) != IX_EOF){
        debug("HERE");
        rc = rmfh.GetRec(rid, trec);
        if(rc != OK_RC) break;
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
    return rc;

}

RC QL_Manager::InsertAll(TableInfo &tableInfo, char *data){
    RM_FileHandle rmfh;
    rmm->OpenFile(tableInfo.name, rmfh);
    RID rid;
    rmfh.InsertRec(data, rid);
    rmm->CloseFile(rmfh);

    debug("InsertAll Middle\n");
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