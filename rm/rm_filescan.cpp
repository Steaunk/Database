#include "rm.h"
#include <cstring>

RM_FileScan::RM_FileScan(){}
RM_FileScan::~RM_FileScan (){}

RC RM_FileScan::OpenScan(const RM_FileHandle &fileHandle,
                  AttrType   attrType,
                  int        attrLength,
                  int        attrOffset,
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
    this->curPageNum = 0;
    this->curSlotNum = -1;
	return OK_RC;
}
RC RM_FileScan::GetNextRec(RM_Record &rec){
    if(!isOpen) return RM_INVALID_SCAN;

    /*bool notFound = true;

    RC rc;*/

    /*do{
        curSlotNum += 1;
        if(curSlotNum == rmFileHandle.GetFileHeader().recordSize){
            curSlotNum = 0;
            curPageNum++;
        }
        RID rid(curPageNum, curSlotNum);
        RM_Record 
        if((rc = rmFileHandle.GetNextRec(rid)))

    }
    while(notFound);
    */

	return OK_RC;
}

RC RM_FileScan::CloseScan(){
    this->isOpen = false;
	return OK_RC;
}
