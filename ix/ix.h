#pragma once
#include "../pf/pf.h"
#include "../rm/rm_rid.h"
#include "../base.h"
#include <utility>
#define TYPE_POS 0
#define LEN_POS 1
#define ROOT_POS_START 2
#define HEADER_LENGTH 11
#define IS_BOTTOM 0
#define NEXT_START 1
#define SIZE_START 10
#define DATA_HEADER_LENGTH 13
#define IX_EOF -1
#define MAX_INDEX_SIZE 200
#define CHAIN_EOF 0

class IX_Manager;
class IX_IndexHandle;
class IX_IndexScan;


/**
 * 管理索引的创建、销毁、使用等。
 * indexNo可以认为一个file中有多个索引，也可以用fileName.indexNo来创建不同文件。
*/
class IX_Manager {
  friend class IX_IndexHandle;
  private:
      PF_Manager *pfmp;
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
  friend class IX_Manager;
  friend class IX_IndexScan;
  private:
    PF_FileHandle file;
    AttrType type;
    int length;
    bool is_full(PageNum);
    void add_rid(PageNum,void*,const RID&);
    std::pair<void *,PageNum> split_add_rid(PageNum,void*,const RID&);
    void add_page(PageNum,void*,PageNum);
    std::pair<void *,PageNum> split_add_page(PageNum,void*,PageNum);
    void split_add_root_page(PageNum,void*,PageNum);
    void split_add_root_rid(PageNum,void*,const RID&);
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
  private:
    PF_FileHandle file;
    AttrType type;
    int length;
    PageNum curpage;
    SlotNum curslot;
    CompOp op;
    void *value;
    bool notacc(const PageNum&,const SlotNum&);
    void next(PageNum&,SlotNum&);
    RC getrid(const PageNum&, const SlotNum&, RID&);
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

int findpage(char *data, void *pData, AttrType type, int length);
RID findrid(char *data, void *pData, AttrType type, int length);
int findpos(char *data, void *pData, AttrType type, int length);
int lower_bound_pos(char *data, void *pData, AttrType type, int length);
int upper_bound_pos(char *data, void *pData, AttrType type, int length);
void setsize(char *data, int size);
int getsize(char *data);
void setnext(char *data, int pagenum);
int getnext(char *data);
void setroot(char *data, int root);
int getroot(char *data);