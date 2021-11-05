#pragma once

#include <cassert>

#define ASSERT(x) (assert(x))

class RID{
	int pageID;
	int slotID; //槽号暂未启用
	int keyStart; //主键起始位置
	int keyEnd; //主键结束位置
public:
	Rid(int pageID, int keyStart, int keyEnd){
		this->pageID = pageId;
		this->keyStart = keyStart;
		this->keyEnd = keyEnd;
	}
	Rid(int pageID, int slotID, int keyStart, int keyEnd){
		this->pageID = pageId;
		this->slotID = slotId;
		this->keyStart = keyStart;
		this->keyEnd = keyEnd;
	}
	int getPageID(){
		return pageID;
	}
	int getSlotID(){
		ASSERT(false);
		return slotID;
	}
	int getKeyStart(){
		return keyStart;
	}	
	int getKeyEnd(){
		return keyEnd;
	}
};
