#include "rm.h"
#include "rm_internal.h"
#include <cstring>

RM_FileHandle::RM_FileHandle(){

}

RM_FileHandle::~RM_FileHandle(){

}

/*RM_FileHeader* RM_FileHandle::GetFileHeaderPointer(){
    return &rmFileHeader;
}*/

void RM_FileHandle::CopyToFileHeader(const RM_FileHeader *fileHeader){
    memcpy(&rmFileHeader, fileHeader, RM_FILE_HEADER_SIZE);
}

void RM_FileHandle::CopyFromFileHeader(RM_FileHeader *fileHeader){
    memcpy(fileHeader, &rmFileHeader, RM_FILE_HEADER_SIZE);
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
    RC rc = OK_RC;
    PageNum pageNum;
    TRY(rid.GetPageNum(pageNum));
    SlotNum slotNum;
    TRY(rid.GetSlotNum(slotNum));

    PF_PageHandle pageHandle;
    TRY(pfFileHandle.GetThisPage(pageNum, pageHandle));

    //RM_PageHeader pageHeader;
    char *data;
    SAFE_TRY(GetDataBySlot(pageHandle, slotNum, data));
    rec.SetData(data);
    rec.SetRid(rid);

safe_exit:
    TRY(pfFileHandle.UnpinPage(pageNum));
    return rc;
}

RC RM_FileHandle::InsertRec(const char *pData, RID &rid){
    RC rc = OK_RC;
    PageNum pageNum;
    PF_PageHandle pageHandle;
    //TRY(rid.GetPageNum(pageNum));
    SlotNum slotNum;

    RM_PageHeader pageHeader;

    if(rmFileHeader.nextFreePage == NO_FREE_PAGE){
        TRY(AllocatePage(pfFileHandle, pageNum));
    }
    else {
        pageNum = rmFileHeader.nextFreePage;
        TRY(pfFileHandle.GetThisPage(pageNum, pageHandle));
    }

    char *data;
    SAFE_TRY(GetFreeSlot(pageHandle, data));
    memcpy(data, pData, rmFileHeader.recordSize);

    SAFE_TRY(GetPageHeader(pageHandle, pageHeader));
    pageHeader.recordNum += 1;
    if(pageHeader.recordNum == rmFileHeader.recordNumPerPage){
        rmFileHeader.nextFreePage = pageHeader.nextFreePage;
    }

safe_exit:
    TRY(pfFileHandle.MarkDirty(pageNum));
    TRY(pfFileHandle.UnpinPage(pageNum));
    return rc;
}

RC RM_FileHandle::UpdateRec(const RM_Record &rec){
    RC rc = OK_RC;

    RID rid;
    rec.GetRid(rid);
    char *new_data;
    rec.GetData(new_data);
    PageNum pageNum;
    rid.GetPageNum(pageNum);
    SlotNum slotNum;
    rid.GetSlotNum(slotNum);
    PF_PageHandle pageHandle;
    TRY(pfFileHandle.GetThisPage(pageNum, pageHandle));

    char *old_data;
    SAFE_TRY(GetDataBySlotNum(pageHandle, slotNum, old_data));

    memcpy(old_data, new_data, rmFileHeader.recordSize);

safe_exit:
    TRY(pfFileHandle.MarkDirty(pageNum));
    TRY(pfFileHandle.UnpinPage(pageNum));
    return OK_RC;
}

RC RM_FileHandle::DeleteRec(const RID &rid){
    RC rc = OK_RC;
    PageNum pageNum;
    TRY(rid.GetPageNum(pageNum));
    SlotNum slotNum;
    TRY(rid.GetSlotNum(slotNum));
    PF_PageHandle pageHandle;
    TRY(pfFileHandle.GetThisPage(pageNum, pageHandle));
    RM_PageHeader pageHeader;
    SAFE_TRY(GetPageHeader(pageHandle, pageHeader));

    RM_Slot slot;
    SAFE_TRY(GetSlot(pageHandle, slotNum, slot));

    if(slot == false){
        rc = RM_RECORD_DELETED;
        goto safe_exit;
    }
    
    SAFE_TRY(SetSlot(pageHandle, slotNum, false)){

    if(pageHeader.recordNum == rmFileHeader.recordNumPerPage){
        pageHeader.nextFreePage = rmFileHeader.nextFreePage;
        rmFileHeader.nextFreePage = pageNum;
        isHeaderModified = true;
    }
    pageHeader.recordNum -= 1;

safe_exit:
    TRY(pfFileHandle.MarkDirty(pageNum));
    TRY(pfFileHandle.UnpinPage(pageNum));
    return rc;
}



RC RM_FileHandle::ForcePages(PageNum pageNum = ALL_PAGES){
    if(pageNum == ALL_PAGES) ASSERT(false); //TO-DO

    TRY(pfFileHandle.ForcePages(pageNum));    

    return OK_RC;
}