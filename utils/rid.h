#pragma once

#include <cassert>

#define ASSERT(x) (assert(x))

class RID{
	int pageID;
	int slotID;
	int keyStart; //主键起始位置
	int keyEnd; //主键结束位置
	int key; //主键值
public:
	RID(int pageID, int keyStart, int keyEnd){
		this->pageID = pageID;
		this->keyStart = keyStart;
		this->keyEnd = keyEnd;
	}
	RID(int pageID, int slotID, int keyStart, int keyEnd){
		this->pageID = pageID;
		this->slotID = slotID;
		this->keyStart = keyStart;
		this->keyEnd = keyEnd;
	}
	int getPageID() const {
		return pageID;
	}
	int getSlotID() const {
		//ASSERT(false);
		return slotID;
	}
	int getKeyStart() const {
		return keyStart;
	}	
	int getKeyEnd() const {
		return keyEnd;
	}
};
