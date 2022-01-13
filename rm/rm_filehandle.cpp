#include "rm.h"
#include "rm_internal.h"
#include <cstring>
#include <cstdio>

RM_FileHandle::RM_FileHandle(){

}

RM_FileHandle::~RM_FileHandle(){

}

/*RM_FileHeader* RM_FileHandle::GetFileHeaderPointer(){
    return &rmFileHeader;
}*/

RM_FileHeader RM_FileHandle::GetFileHeader() const{
    return rmFileHeader;
}

RC RM_FileHandle::AllocatePage(PF_PageHandle &pageHandle, PageNum &pageNum){
    TRY(pfFileHandle.AllocatePage(pageHandle));
    TRY(pageHandle.GetPageNum(pageNum));
    rmFileHeader.nextFreePage = pageNum;

    rmFileHeader.pageNum++;

    RM_PageHeader *pageHeader;
    char *data;
    TRY(GetPageHeaderAndData(pageHandle, pageHeader, data));
    pageHeader = (RM_PageHeader *) data;
    pageHeader->nextFreePage = NO_FREE_PAGE;
    pageHeader->recordNum = 0;

    data += rmFileHeader.bitmapOffset;
    memset(data, 0, rmFileHeader.bitmapSize);

    TRY(pfFileHandle.MarkDirty(pageNum));
    TRY(pfFileHandle.UnpinPage(pageNum));

    return OK_RC;
}

RC RM_FileHandle::GetPageHeaderAndData(const PF_PageHandle &pageHandle, RM_PageHeader *&pageHeader, char *&data){
    TRY(pageHandle.GetData(data));
    pageHeader = (RM_PageHeader *) data;
    return OK_RC;
}

RC RM_FileHandle::GetPageHeader(const PF_PageHandle & pageHandle, RM_PageHeader *&pageHeader) const{
    char *data; 
    TRY(pageHandle.GetData(data));
    pageHeader = (RM_PageHeader *) data;

    return OK_RC;
}

int RM_FileHandle::FindZero(char value) const{
    if((int)value == 255) return -1;
    return __builtin_ffs(~value) - 1;
}

RC RM_FileHandle::GetFreeSlot(const PF_PageHandle &pageHandle, SlotNum &slotNum, char *&data) const {
    char *cur_data;
    TRY(pageHandle.GetData(cur_data));
    cur_data += rmFileHeader.bitmapOffset;

    for(int i = 0; i < rmFileHeader.bitmapSize; ++i){
        if(*(cur_data + i) == 0){
            TRY(GetDataBySlotNum(pageHandle, slotNum = i, data));
            return OK_RC;
        }

    }

    return RM_NO_FREE_SLOT;
}

RC RM_FileHandle::FindNextSlot(SlotNum &slotNum, PageNum pageNum, char *&data){
    //very strange bug?
    /*static*/ PF_PageHandle pageHandle;
    RC rc = OK_RC;
    SAFE_TRY(pfFileHandle.GetThisPage(pageNum, pageHandle));
        
    //}
    RM_Slot slot;
    for(int i = slotNum; i < rmFileHeader.recordNumPerPage; ++i){
        SAFE_TRY(GetSlot(pageHandle, i, slot));
        //debug("(%d %d %d) ?\n", pageNum, i, slot);
        if(slot == true){
            slotNum = i;
            GetDataBySlotNum(pageHandle, slotNum, data);
            if(rc == OK_RC) goto safe_exit;
        }
    }
    slotNum = -1; 
safe_exit:
    TRY(pfFileHandle.UnpinPage(pageNum));
    return OK_RC;
}

RC RM_FileHandle::GetDataBySlotNum(const PF_PageHandle &pageHandle, const SlotNum &slotNum, char *&data) const{
    TRY(pageHandle.GetData(data));
    data += slotNum * rmFileHeader.recordSize + rmFileHeader.bitmapOffset + rmFileHeader.bitmapSize;
    return OK_RC;
}

RC RM_FileHandle::GetSlot(const PF_PageHandle &pageHandle, const SlotNum &slotNum, RM_Slot &slot) const {
    if(slotNum >= rmFileHeader.recordNumPerPage) return RM_INVALID_RID;
    char *data;
    TRY(pageHandle.GetData(data));
    data += rmFileHeader.bitmapOffset + slotNum;
    slot = *(data);
    
    return OK_RC;
}

RC RM_FileHandle::SetSlot(const PF_PageHandle &pageHandle, const SlotNum &slotNum, const RM_Slot &slot) {
    if(slotNum >= rmFileHeader.recordNumPerPage) return RM_INVALID_RID;
    char *data;
    TRY(pageHandle.GetData(data));
    data += rmFileHeader.bitmapOffset + slotNum;
    if(slot == true) *(data) = 1;
    else *(data) = 0;
    
    return OK_RC;
}



void RM_FileHandle::CopyToFileHeader(const RM_FileHeader *fileHeader){
    memcpy(&rmFileHeader, fileHeader, RM_FILE_HEADER_SIZE);
}

void RM_FileHandle::CopyFromFileHeader(RM_FileHeader *fileHeader){
    memcpy(fileHeader, &rmFileHeader, RM_FILE_HEADER_SIZE);
}

PF_FileHandle RM_FileHandle::GetFileHandle() const {
    return pfFileHandle;
}

/*bool RM_FileHandle::IsHeaderModified() const {
    return isHeaderModified;
}*/

void RM_FileHandle::InitSetting(){
    //isHeaderModified = false;
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
    SAFE_TRY(GetDataBySlotNum(pageHandle, slotNum, data));
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
    SlotNum slotNum;
    //TRY(rid.GetPageNum(pageNum));

    RM_PageHeader *pageHeader;

    if(rmFileHeader.nextFreePage == NO_FREE_PAGE){
        TRY(AllocatePage(pageHandle, pageNum));
    }
    else {
        pageNum = rmFileHeader.nextFreePage;
        TRY(pfFileHandle.GetThisPage(pageNum, pageHandle));
    }
    SAFE_TRY(GetPageHeader(pageHandle, pageHeader));
    char *data;
    SAFE_TRY(GetFreeSlot(pageHandle, slotNum, data));
    memcpy(data, pData, rmFileHeader.recordSize);
    SAFE_TRY(SetSlot(pageHandle, slotNum, true));

    pageHeader->recordNum += 1;
    if(pageHeader->recordNum == rmFileHeader.recordNumPerPage){
        rmFileHeader.nextFreePage = pageHeader->nextFreePage;
        //isHeaderModified = true;
    }

    rid = RID(pageNum, slotNum);

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
    RM_PageHeader *pageHeader;
    SAFE_TRY(GetPageHeader(pageHandle, pageHeader));

    RM_Slot slot;
    SAFE_TRY(GetSlot(pageHandle, slotNum, slot));

    if(slot == false){
        rc = RM_RECORD_DELETED;
        goto safe_exit;
    }
    
    debug("DeleteRec [%d %d]\n", pageNum, slotNum);
    SAFE_TRY(SetSlot(pageHandle, slotNum, false))
    
    //-------- deleted
        GetSlot(pageHandle, slotNum, slot);
        debug("DeleteRec after SetSlot slot = %d\n", slot);



    if(pageHeader->recordNum == rmFileHeader.recordNumPerPage){
        pageHeader->nextFreePage = rmFileHeader.nextFreePage;
        rmFileHeader.nextFreePage = pageNum;
        //isHeaderModified = true;
    }
    pageHeader->recordNum -= 1;

safe_exit:
    TRY(pfFileHandle.MarkDirty(pageNum));
    TRY(pfFileHandle.UnpinPage(pageNum));
    return rc;
}



RC RM_FileHandle::ForcePages(PageNum pageNum){
    //if(pageNum == ALL_PAGES) ASSERT(false); //TO-DO

    TRY(pfFileHandle.ForcePages(pageNum));    

    return OK_RC;
}

bool RM_FileHandle::Comp(AttrType attrType, 
                    int attrLength, 
                    CompOp compOp,
                    void *lvalue,
                    void *rvalue){
    bool flag;
    switch (compOp)
    {
    case NO_OP: return true;

    case EQ_OP:
    case NE_OP:
        if(attrType == INT) flag = (*((int *)lvalue) == *((int *)rvalue));
        else if(attrType == FLOAT) flag = (*((float *)lvalue) == *((float *)rvalue));
        else flag = strncmp((char *)lvalue, (char *)rvalue, attrLength) == 0; 
        return compOp == NE_OP ? !flag : flag;

    case LT_OP:
    case GE_OP:
        if(attrType == INT) flag = (*((int *)lvalue) < *((int *)rvalue));
        else if(attrType == FLOAT) flag = (*((float *)lvalue) < *((float *)rvalue));
        else flag = strncmp((char *)lvalue, (char *)rvalue, attrLength) < 0;
        return compOp == GE_OP ? !flag : flag;

    case GT_OP:
    case LE_OP:
        if(attrType == INT) flag = (*((int *)lvalue) > *((int *)rvalue));
        else if(attrType == FLOAT) flag = (*((float *)lvalue) > *((float *)rvalue));
        else flag = strncmp((char *)lvalue, (char *)rvalue, attrLength) > 0;
        return compOp == LE_OP ? !flag : flag;
    case LIKE:
        if(attrType != STRING) return false;
        else return LikeComp((char *)lvalue, (char *)rvalue);
        break;

    default:
        break;
    }
    return false;
}

bool RM_FileHandle::LikeComp(char *lvalue, char *rvalue){
    //debug("LikeComp Start [%s]\n", rvalue);
    int lLen = strlen(lvalue);
    int rLen = strlen(rvalue);
    bool vis[lLen + 1][rLen + 1];
    for(int i = 0; i < lLen; ++i)
        for(int j = 0; j < rLen; ++j)
            vis[i][j] = 0; 
    vis[0][0] = 1;
    for(int i = 0; i < lLen; ++i){
        for(int j = 0; j < rLen; ++j){
            if(lvalue[i] == rvalue[j] || rvalue[j] == '_')
                vis[i + 1][j + 1] |= vis[i][j];
            if(rvalue[j] == '%'){
                vis[i + 1][j] |= vis[i][j];
                vis[i][j + 1] |= vis[i][j];
            }
        } 
    }
    //debug("vis[%d][%d] = %d\n", lLen, rLen, vis[lLen][rLen]);
    return vis[lLen][rLen];
}