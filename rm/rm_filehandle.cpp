#include "rm.h"

RM_FileHandle::RM_FileHandle(){

}

RM_FileHandle::~RM_FileHandle(){

}

RC RM_FileHandle::GetRec(const RID &rid, RM_Record &rec) const {
    
}

RC RM_FileHandle::InsertRec(const char *pData, RID &rid){

}

RC RM_FileHandle::UpdateRec(const RM_Record &rec){

}

RC RM_FileHandle::DeleteRec(const RID &rid){

}

RC RM_FileHandle::ForcePages(PageNum pageNum = ALL_PAGES){

}