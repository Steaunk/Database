#include "rm.h"
//#include "rm_internal.h"

RM_Record::RM_Record(){
    pData = nullptr;
}

RM_Record::RM_Record(RID &rid, char *&pData): rid(rid), pData(pData){}

RM_Record::~RM_Record(){

}

RC RM_Record::GetData(char *&pData) const {
    ASSERT(this->pData == nullptr);
    pData = this->pData;
    return OK_RC;
}

RC RM_Record::GetRid(RID &rid) const {
    rid = this->rid;
    return OK_RC;
}

RC RM_Record::SetData(char * const &pData){
    this->pData = pData;
    return OK_RC;
}

RC RM_Record::SetRid(const RID &rid){
    this->rid = rid;
    return OK_RC;
}
