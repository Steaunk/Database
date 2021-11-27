#include "rm.h"
#include "rm_internal.h"

RM_Manager::RM_Manager(PF_Manager &pfm){
    pfManager = pfm;
}

RM_Manager::~RM_Manager(){}

RC RM_Manager::CreateFile(const char *fileName, int recordSize){
    TRY(pfManager.CreateFile(fileName));
    PF_FileHandle fileHandle;
    TRY(pfManager.OpenFile(fileName, fileHandle));
    PF_PageHandle pageHandle;
    TRY(fileHandle.AllocatePage(pageHandle));
    PageNum pageNum;
    TRY(pageHandle.GetPageNum(pageNum));
    char *data;
    TRY(pageHandle.GetData(data));
    RM_FileHeader fileHeader;
}

RC RM_Manager::DestroyFile(const char *fileName){
    return pfManager.DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle){
}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle){
}
