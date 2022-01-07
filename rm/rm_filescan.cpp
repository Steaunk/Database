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
            notFound = !Comp(attrType, attrLength, compOp, data + attrOffset, value);
        }
    }
    while(notFound);
    
    debug("GetNextRec *data : %d\n", *((int *)(data+4)));
    rec.SetData(data);
    rec.SetRid(RID(curPageNum, curSlotNum));
	return OK_RC;
}

RC RM_FileScan::CloseScan(){
    this->isOpen = false;
	return OK_RC;
}

bool RM_FileScan::Comp(AttrType attrType, 
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
        debug("COMP flag = %d (%d %d)\n", flag, *((int *)lvalue), *((int *)rvalue));
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

    default:
        break;
    }
    return false;
}