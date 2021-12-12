#include "ix.h"
#include <string>
#include <cstring>

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
  indexFileName += ".";
  indexFileName += indexNo;
  TRY(pfmp->CreateFile(indexFileName.c_str()));
  PF_FileHandle file;
  TRY(pfmp->OpenFile(indexFileName.c_str(),file));
  PF_PageHandle page;

  TRY(file.AllocatePage(page));
  char *data;
  TRY(page.GetData(data));
  std::string data_s = "";
  data_s += (char)attrType;
  data_s += (char)attrLength;
  data_s += (char)1;
  strcpy(data,data_s.c_str());
  PageNum pn;
  TRY(page.GetPageNum(pn))
  TRY(file.MarkDirty(pn));
  TRY(file.UnpinPage(pn));

  TRY(file.AllocatePage(page))
  TRY(page.GetData(data))
  data_s = "1";
  data_s += (char)(255);
  data_s += "0000";
  strcpy(data,data_s.c_str());
  TRY(page.GetPageNum(pn))
  TRY(file.MarkDirty(pn));
  TRY(file.UnpinPage(pn));
  
  TRY(pfmp->CloseFile(file));
  return OK_RC;
}

RC IX_Manager::DestroyIndex  (const char *fileName,          // Destroy index
                             int        indexNo){
  std::string indexFileName = fileName;
  indexFileName += ".";
  indexFileName += indexNo;
  TRY(pfmp->DestroyFile(indexFileName.c_str()));
  return OK_RC;

}

RC IX_Manager::OpenIndex(const char *fileName,          // Open index
                          int        indexNo,
                          IX_IndexHandle &indexHandle){
  std::string indexFileName = fileName;
  indexFileName += ".";
  indexFileName += indexNo;
  PF_FileHandle file;
  TRY(pfmp->OpenFile(indexFileName.c_str(),file));
  indexHandle.file = file;

  PF_PageHandle page;
  TRY(file.GetFirstPage(page));
  char *data;
  TRY(page.GetData(data))
  indexHandle.type = (AttrType)((int)data[TYPE_POS]);
  indexHandle.length = data[LEN_POS];
  return OK_RC;
}

RC IX_Manager::CloseIndex   (IX_IndexHandle &indexHandle){
  return pfmp->CloseFile(indexHandle.file);
}