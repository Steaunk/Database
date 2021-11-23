#include "recordhandler.h"

RecordHandler::RecordHandler(){
    fm = new FileManager();
    bpm = new BufPageManager(fm);
}

RecordHandler::~RecordHandler(){
    delete bpm;
    delete fm;
}

RC RecordHandler::createFile(const char *filename){
    bool result = fm->createFile(filename);
    if(result == true)
        return SUCCESS;
    return ERROR;
}

RC RecordHandler::destroyFile(const char *filename){
    //bool result 
    assert(false);
    return ERROR;
}

RC RecordHandler::openFile(const char *filename){
    bool result = fm->openFile(filename, curFileID);
    if(result == true)
        return SUCCESS;
    return ERROR;
}

RC RecordHandler::closeFile(){
    int result = fm->closeFile(curFileID);
    if(result == 0)
        return SUCCESS;
    return ERROR;
}
//4 PageIndex|4 NextPage|4 PrevPage|2 SlotNum|
RC RecordHandler::getRecordPos(char *src, int slotID, char *dest){
    //int slotNum = *((short*)(src + SLOT_NUM_OFFSET));
    //if(slotID >= slotNum) return SLOT_ID_EXCEED;
    dest = src + *((short *)(src + PAGE_SIZE - 2 * slotID));
    return SUCCESS;
}

RC RecordHandler::getRecordBySlotID(const RID &rid, char *&pData){
    int index;
    char* b = (char *)(bpm->getPage(curFileID, rid.getPageID(), index));
    RC rc = getRecordPos(b, rid.getSlotID(), pData);
    if(rc != SUCCESS) return rc;
    return SUCCESS;
    //b += PAGE_HEADER_SIZE;
    //int s = new 
}

RC RecordHandler::getSlotNum(const int &pageID, int &slotNum){
    int index;
    char* b = (char *)(bpm->getPage(curFileID, pageID, index));
    slotNum = *((short*)(b + SLOT_NUM_OFFSET));
    return SUCCESS;
}


//遍历
RC RecordHandler::getRecordGreaterThan(const RID &rid, char *&pData){
    int index;
    char* b = (char *)(bpm->getPage(curFileID, rid.getPageID(), index));

    return SUCCESS;
}

RC RecordHandler::deleteRecord(const RID &rid){

}

RC RecordHandler::insertRecord(const RID &rid, const char *pData){

}

RC RecordHandler::updateRecord(const RID &rid, const char *pData){

}



