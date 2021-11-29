#include "rm.h"
#include "rm_internal.h"

RM_FileHandle::RM_FileHandle(){

}

RM_FileHandle::~RM_FileHandle(){

}

RM_FileHeader* RM_FileHandle::GetFileHeaderPointer(){
    return &rmFileHeader;
}

PF_FileHandle RM_FileHandle::GetFileHandle() const {
    return pfFileHandle;
}

bool RM_FileHandle::IsHeaderModified() const {
    return isHeaderModified;
}

void RM_FileHandle::InitSetting(){
    isHeaderModified = false;
}

void RM_FileHandle::SetFileHandle(const PF_FileHandle &pfFileHandle){
    this->pfFileHandle = pfFileHandle;
}

RC RM_FileHandle::GetRecordNumPerPage(int &recordNumPerPage, int recordSize){
    int res = PF_PAGE_SIZE - RM_PAGE_HEADER_SIZE;
    int cnt = res / (recordSize + RM_SLOT_SIZE);
    if(cnt <= 0) return RM_RECORD_SIZE_TOO_LARGE;
    recordNumPerPage = cnt;
    return OK_RC;
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