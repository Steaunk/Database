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
    TableInfo tableInfo;
    TRY(smm->GetTableInfo(relName, tableInfo));
    if(nValues != tableInfo.columnNum) return QL_DATA_NOT_MATCH;
    int length = 0;
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
        length += tableInfo.columnAttr[i].attrLength;
    }
    char *data = (char *)malloc(length);
    RID rid;
    rmfh.InsertRec(data, rid);
    return OK_RC;

}          // values to insert

RC QL_Manager::Delete  (const char *relName,            // relation to delete from
            int        nConditions,         // # conditions in Where clause
            const Condition conditions[]){
    
    return OK_RC;
}  // conditions in Where clause
RC QL_Manager::Update  (const char *relName,            // relation to update
            const RelAttr &updAttr,         // attribute to update
            const int bIsValue,             // 0/1 if RHS of = is attribute/value
            const RelAttr &rhsRelAttr,      // attr on RHS of =
            const Value &rhsValue,          // value on RHS of =
            int   nConditions,              // # conditions in Where clause
            const Condition conditions[]){
    
    return OK_RC;
}  // conditions in Where clause

