#include "rm.h"
#include "rm_internal.h"
#include <cstring>

RM_Manager::RM_Manager(PF_Manager &pfm){
    pfManager = &pfm;
}

RM_Manager::~RM_Manager(){}

RC RM_Manager::CreateFile(const char *fileName, int recordSize){
    TRY(pfManager->CreateFile(fileName));
    PF_FileHandle fileHandle;
    TRY(pfManager->OpenFile(fileName, fileHandle));
    PF_PageHandle pageHandle;
    TRY(fileHandle.AllocatePage(pageHandle));
    PageNum pageNum;
    TRY(pageHandle.GetPageNum(pageNum));
    char *data;
    TRY(pageHandle.GetData(data));
    RM_FileHeader *fileHeader = (RM_FileHeader *) data;
    fileHeader->recordSize = recordSize;
    fileHeader->pageNum = 1;
    fileHeader->nextFreePage = -1;
    TRY(RM_FileHandle::GetRecordNumPerPage(fileHeader->recordNumPerPage, recordSize));
    TRY(fileHandle.MarkDirty(pageNum));
    TRY(fileHandle.UnpinPage(pageNum));
    TRY(pfManager->CloseFile(fileHandle));
    return OK_RC;
}

RC RM_Manager::DestroyFile(const char *fileName){
    return pfManager->DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &rmFileHandle){
    PF_FileHandle pfFileHandle;
    TRY(pfManager->OpenFile(fileName, pfFileHandle));
    PF_PageHandle pageHandle;
    TRY(pfFileHandle.GetFirstPage(pageHandle));
    char *data;
    TRY(pageHandle.GetData(data));
    PageNum pageNum;
    TRY(pageHandle.GetPageNum(pageNum));
    RM_FileHeader *fileHeader = (RM_FileHeader *) data;
    memcpy(rmFileHandle.GetFileHeaderPointer(), fileHeader, RM_FILE_HEADER_SIZE);
    rmFileHandle.SetFileHandle(pfFileHandle);

    //可能还需要对 RM_FileHandle 进行一些配置
    rmFileHandle.InitSetting();
    //TO-DO

    TRY(pfFileHandle.UnpinPage(pageNum));
    return OK_RC;
}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle){
    PF_FileHandle pfFileHandle = fileHandle.GetFileHandle();
    if(fileHandle.IsHeaderModified()){
        PF_PageHandle pageHandle;
        TRY(pfFileHandle.GetFirstPage(pageHandle));
        PageNum pageNum;
        TRY(pageHandle.GetPageNum(pageNum));
        char *data;
        TRY(pageHandle.GetData(data));
        RM_FileHeader *fileHeader = (RM_FileHeader *) data;
        memcpy(fileHeader, fileHandle.GetFileHeaderPointer(), RM_FILE_HEADER_SIZE);
        pfFileHandle.MarkDirty(pageNum);
        pfFileHandle.UnpinPage(pageNum);
    }    

    TRY(pfManager->CloseFile(pfFileHandle));

    return OK_RC;
}
