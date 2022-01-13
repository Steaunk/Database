#include "rm_rid.h"

RID::RID(){}

RID::RID(const PageNum &pageNum, const SlotNum &slotNum): pageNum(pageNum), slotNum(slotNum){
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

bool RID::operator == (const RID& rid) const {
	PageNum pageNum; rid.GetPageNum(pageNum);
	SlotNum slotNum; rid.GetSlotNum(slotNum);
	return pageNum == this->pageNum && slotNum == this->slotNum;
}
