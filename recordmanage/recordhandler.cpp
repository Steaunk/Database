#include "recordhandler.h"

RecordHandler::RecordHandler(){
    fm = new FileManager();
    bpm = new BufPageManager(fm);
}

RecordHandler::~RecordHandler(){
    delete bpm;
    delete fm;
}

RC RecordHandler::CreateFile(const char *filename){
    bool result = fm->createFile(filename);
    if(result == true)
        return SUCCESS;
    return ERROR;
}

RC RecordHandler::OpenFile(const char *filename){
    bool result = fm->openFile(filename, curFileID);
    if(result == true)
        return SUCCESS;
    return ERROR;
}

RC RecordHandler::CloseFile(){
    int result = fm->closeFile(curFileID);
    if(result == 0)
        return SUCCESS;
    return ERROR;
}

RC RecordHandler::GetRecord(const RID &rid, char *&pData){
    
}

RC RecordHandler::DeleteRecord(const RID &rid){

}

RC RecordHandler::InsertRecord(const RID &rid, const char *pData){

}

RC RecordHandler::UpdateRecord(const RID &rid, const char *pData){

}



