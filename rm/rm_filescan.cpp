#include "rm.h"

RM_FileScan::RM_FileScan(){}
RM_FileScan::~RM_FileScan (){}

RC RM_FileScan::OpenScan(const RM_FileHandle &fileHandle,
                  AttrType   attrType,
                  int        attrLength,
                  int        attrOffset,
                  CompOp     compOp,
                  void       *value,
                  ClientHint pinHint = NO_HINT){

}
RC RM_FileScan::GetNextRec(RM_Record &rec){

}

RC RM_FileScan::CloseScan(){
    
}