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
    RID rid;
    RM_FileHandle rmfh;
    rmm->OpenFile(smm->RMName(relName).c_str(), rmfh);
    rmfh.InsertRec(data, rid);
    rmm->CloseFile(rmfh);
    free(data);
    return OK_RC;

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
                std::cout << "'" << tableInfo.columnAttr[lID].name << "' should be " << tableInfo.columnAttr[lID].attrType << "\n";
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
                std::cout << "'" << tableInfo.columnAttr[ID].name << "' should be " << tableInfo.columnAttr[lID].attrType << "\n";
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
            if(!rmfs.Comp(attrType[i], attrLength[i], c.op, 
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
                rmfh.DeleteRec(rid);
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