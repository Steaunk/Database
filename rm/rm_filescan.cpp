#include "rm.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

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
    this->curSlotNum = 0;
    
	return OK_RC;
}
RC RM_FileScan::GetNextRec(RM_Record &rec){
    if(!isOpen) return RM_INVALID_SCAN;

    char *data;
    bool notFound = true;
    do{
        TRY(rmFileHandle.FindNextSlot(curSlotNum, curPageNum, data));
        if(curSlotNum == -1){
            curPageNum++;
            if(curPageNum == rmFileHandle.GetFileHeader().pageNum)
                return RM_EOF;
            curSlotNum = 0;
        }
        else{
            data += attrOffset;
            switch (compOp)
            {
            case NO_OP:
                notFound = false;
                break;
            case EQ_OP:
            case NE_OP:
                if(attrType == INT) notFound = !((int *)data == (int *)value);
                else if(attrType == FLOAT) notFound = !((float *)data == (float *)value);
                else notFound = strncmp(data, (char *)value, attrLength); 
                if(compOp == NE_OP) notFound = !notFound;
                break;
            
            case LT_OP:
            case GE_OP:
                if(attrType == INT) notFound = !((int *)data < (int *)value);
                else if(attrType == FLOAT) notFound = !((float *)data < (float *)value);
                else notFound = strncmp(data, (char *)value, attrLength) >= 0;
                if(compOp == GE_OP) notFound = !notFound;
                break;

            case GT_OP:
            case LE_OP:
                if(attrType == INT) notFound = !((int *)data > (int *)value);
                else if(attrType == FLOAT) notFound = !((float *)data > (float *)value);
                else notFound = strncmp(data, (char *)value, attrLength) <= 0;
                if(compOp == LE_OP) notFound = !notFound;
                break;

            default:
                assert(false);
            }
        }
    }
    while(notFound);
    rec.SetData(data);
    rec.SetRid(RID(curPageNum, curSlotNum));

	return OK_RC;
}

RC RM_FileScan::CloseScan(){
    this->isOpen = false;
	return OK_RC;
}
