#pragma once
#include "../pf/pf.h"
#include "../rm/rm_rid.h"
#include "../base.h"


/**
 * 管理索引的创建、销毁、使用等。
 * indexNo可以认为一个file中有多个索引，也可以用fileName.indexNo来创建不同文件。
*/
class IX_Manager {
  public:
       IX_Manager   (PF_Manager &pfm);              // Constructor
       ~IX_Manager  ();                             // Destructor
    RC CreateIndex  (const char *fileName,          // Create new index
                     int        indexNo,
                     AttrType   attrType,
                     int        attrLength);
    RC DestroyIndex (const char *fileName,          // Destroy index
                     int        indexNo);
    RC OpenIndex    (const char *fileName,          // Open index
                     int        indexNo,
                     IX_IndexHandle &indexHandle);
    RC CloseIndex   (IX_IndexHandle &indexHandle);  // Close index
};

/**
 * 对索引中的元素进行插入删除
*/
class IX_IndexHandle {
  public:
       IX_IndexHandle  ();                             // Constructor
       ~IX_IndexHandle ();                             // Destructor
    RC InsertEntry     (void *pData, const RID &rid);  // Insert new index entry
    RC DeleteEntry     (void *pData, const RID &rid);  // Delete index entry
    RC ForcePages      ();                             // Copy index to disk
};

/**
 * 多个元素的遍历
 * compOp在base.h中定义了，有多种比较方式
*/
class IX_IndexScan {
  public:
       IX_IndexScan  ();                                 // Constructor
       ~IX_IndexScan ();                                 // Destructor
    RC OpenScan      (const IX_IndexHandle &indexHandle, // Initialize index scan
                      CompOp      compOp,
                      void        *value,
                      ClientHint  pinHint = NO_HINT);           
    RC GetNextEntry  (RID &rid);                         // Get next matching entry
    RC CloseScan     ();                                 // Terminate index scan
};