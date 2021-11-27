#include "rm_rid.h"

RID::RID(){}

RID::RID(PageNum pageNum, SlotNum SlotNum): pageNum(pageNum), slotNum(slotNum){
}

RID::~RID(){}

RC RID::GetPageNum(PageNum &pageNum) const {
    pageNum = this->pageNum;
    return OK_RC;
}

RC RID::GetSlotNum(SlotNum &slotNum) const {
    slotNum = this->slotNum;
    return OK_RC;
}