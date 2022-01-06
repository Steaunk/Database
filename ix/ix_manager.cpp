#include "ix.h"
#include <string>
#include <cstring>
#include<iostream>
using namespace std;

// class IX_Manager {
//   public:
//        IX_Manager   (PF_Manager &pfm);              // Constructor
//        ~IX_Manager  ();                             // Destructor
//     RC CreateIndex  (const char *fileName,          // Create new index
//                      int        indexNo,
//                      AttrType   attrType,
//                      int        attrLength);
//     RC DestroyIndex (const char *fileName,          // Destroy index
//                      int        indexNo);
//     RC OpenIndex    (const char *fileName,          // Open index
//                      int        indexNo,
//                      IX_IndexHandle &indexHandle);
//     RC CloseIndex   (IX_IndexHandle &indexHandle);  // Close index
// };

IX_Manager::IX_Manager(PF_Manager &pfm):pfmp(&pfm){
}

IX_Manager::~IX_Manager(){
}

RC IX_Manager::CreateIndex  (const char *fileName,          // Create new index
                             int        indexNo,
                             AttrType   attrType,
                             int        attrLength){
  std::string indexFileName = fileName;
  indexFileName += '.';
  indexFileName += indexNo+'0';
  TRY(pfmp->CreateFile(indexFileName.c_str()));
  PF_FileHandle file;
  TRY(pfmp->OpenFile(indexFileName.c_str(),file));
  PF_PageHandle page;

  TRY(file.AllocatePage(page));
  char *data;
  TRY(page.GetData(data));
  data[TYPE_POS] = (char)((int)attrType);
  setnext(data,1);
  setprev(data,CHAIN_EOF);
  setlen(data,attrLength);
  setroot(data,1);

  PageNum pn;
  TRY(page.GetPageNum(pn))
  TRY(file.MarkDirty(pn));
  TRY(file.UnpinPage(pn));

  TRY(file.AllocatePage(page))
  TRY(page.GetData(data))
  data[IS_BOTTOM] = '1';
  setnext(data,CHAIN_EOF);
  setprev(data,CHAIN_EOF);
  setsize(data,0);

  TRY(page.GetPageNum(pn))
  TRY(file.MarkDirty(pn));
  TRY(file.UnpinPage(pn));
  
  TRY(pfmp->CloseFile(file));
  return OK_RC;
}

RC IX_Manager::DestroyIndex  (const char *fileName,          // Destroy index
                             int        indexNo){
  std::string indexFileName = fileName;
  indexFileName += '.';
  indexFileName += indexNo+'0';
  TRY(pfmp->DestroyFile(indexFileName.c_str()));
  return OK_RC;

}

RC IX_Manager::OpenIndex(const char *fileName,          // Open index
                          int        indexNo,
                          IX_IndexHandle &indexHandle){
  std::string indexFileName = fileName;
  indexFileName += ".";
  indexFileName += indexNo+"0";
  PF_FileHandle file;
  TRY(pfmp->OpenFile(indexFileName.c_str(),file));
  indexHandle.file = file;

  PF_PageHandle page;
  TRY(file.GetFirstPage(page));
  char *data;
  TRY(page.GetData(data))
  indexHandle.type = (AttrType)((int)data[TYPE_POS]);
  indexHandle.length = getlen(data);
  return OK_RC;
}

RC IX_Manager::CloseIndex   (IX_IndexHandle &indexHandle){
  return pfmp->CloseFile(indexHandle.file);
}