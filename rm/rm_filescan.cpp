#include "rm.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>

RM_FileScan::RM_FileScan(){}
RM_FileScan::~RM_FileScan (){}

RC RM_FileScan::OpenScan(const RM_FileHandle &fileHandle,
                  AttrType   attrType,  //类型
                  int        attrLength,    //长度（字节）
                  int        attrOffset,    //
                  CompOp     compOp,
                  void       *value,
                  ClientHint pinHint){
    ASSERT(pinHint == NO_HINT);
    this->isOpen = true;
    this->rmFileHandle = fileHandle;
    this->compOp = compOp;
    this->attrType = attrType;
    if(this->compOp != NO_OP){
        if((attrOffset + attrLength) > this->rmFileHandle.GetFileHeader().recordSize)
            return RM_INVALID_SCAN;
        this->attrLength = attrLength;
        this->attrOffset = attrOffset;
        ASSERT(attrLength == 4 || attrType == STRING);
        this->value = (void *) malloc(attrLength);
        memcpy(this->value, value, attrLength);
    }
    this->curPageNum = 1;
    this->curSlotNum = -1;
    
	return OK_RC;
}
RC RM_FileScan::GetNextRec(RM_Record &rec){
    if(!isOpen) return RM_INVALID_SCAN;
    if(curPageNum == rmFileHandle.GetFileHeader().pageNum) return RM_EOF;

    char *data;
    bool notFound = true;
    do{
        curSlotNum += 1;
        TRY(rmFileHandle.FindNextSlot(curSlotNum, curPageNum, data));
        if(curSlotNum == -1){
            curPageNum++;
            if(curPageNum == rmFileHandle.GetFileHeader().pageNum)
                return RM_EOF;
        }
        else{
            //debug("GetNextRec while data : %d\n", *((int *)data+4));
            notFound = !RM_FileHandle::Comp(attrType, attrLength, compOp, data + attrOffset, value);
        }
    }
    while(notFound);
    
    debug("GetNextRec *data : %d\n", *((int *)(data+34)));
    rec.SetData(data);
    rec.SetRid(RID(curPageNum, curSlotNum));
	return OK_RC;
}

RC RM_FileScan::CloseScan(){
    this->isOpen = false;
	return OK_RC;
}