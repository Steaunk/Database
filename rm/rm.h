//
// rm.h
//
//   Record Manager component interface
//
// This file does not include the interface for the RID class.  This is
// found in rm_rid.h
//

#ifndef RM_H
#define RM_H

// Please DO NOT include any files other than redbase.h and pf.h in this
// file.  When you submit your code, the test program will be compiled
// with your rm.h and your redbase.h, along with the standard pf.h that
// was given to you.  Your rm.h, your redbase.h, and the standard pf.h
// should therefore be self-contained (i.e., should not depend upon
// declarations in any other file).

// Do not change the following includes
#include "../base.h"
#include "rm_rid.h"
#include "rm_internal.h"
#include "../pf/pf.h"

//
// RM_Record: RM Record interface
//
class RM_Record {
    RID rid;
    char *pData;
public:
    RM_Record ();
    RM_Record (RID &rid, char *&pData);
    ~RM_Record();

    // Return the data corresponding to the record.  Sets *pData to the
    // record contents.
    RC GetData(char *&pData) const;

    // Return the RID associated with the record
    RC GetRid (RID &rid) const;

    RC SetData(char * const &pData);

    RC SetRid(const RID &rid);
};

//
// RM_FileHandle: RM File interface
//
class RM_FileHandle {

    friend class RM_Manager;

    RM_FileHeader rmFileHeader;
    PF_FileHandle pfFileHandle;
    //bool isHeaderModified;


    int FindZero(char value) const; //找到一个字节中的第一个0所在位置，其中-1位无。

    RC GetDataBySlotNum(const PF_PageHandle &, const SlotNum &, char *&) const;

    RC AllocatePage(PF_PageHandle &, PageNum &);

    RC GetFreeSlot(const PF_PageHandle &, SlotNum &, char *&) const;

    RC GetPageHeaderAndData(const PF_PageHandle &, RM_PageHeader *&, char *&);

    RC GetPageHeader(const PF_PageHandle &, RM_PageHeader *&) const;

    RC GetSlot(const PF_PageHandle &, const SlotNum &, RM_Slot &) const;

    RC SetSlot(const PF_PageHandle &, const SlotNum &, const RM_Slot &);

    void CopyToFileHeader(const RM_FileHeader *fileHeader);
    
    void CopyFromFileHeader(RM_FileHeader *fileHeader);

    //bool IsHeaderModified() const;

    void InitSetting();

    void SetFileHandle(const PF_FileHandle &pfFileHandle);   //Set PF_FileHandle

    static RC GetRecordNumPerPage(int &recordNumPerPage, int recordSize);

public:
    RM_FileHandle ();
    ~RM_FileHandle();

    PF_FileHandle GetFileHandle() const;

    RM_FileHeader GetFileHeader() const;

    //RM_FileHeader* GetFileHeaderPointer();
    RC GetNextRec(const RID &, RM_Record &) const;

       // Given a RID, return the record
    RC GetRec     (const RID &rid, RM_Record &rec) const;

    RC InsertRec  (const char *pData, RID &rid);       // Insert a new record

    RC DeleteRec  (const RID &rid);                    // Delete a record
    RC UpdateRec  (const RM_Record &rec);              // Update a record

    // Forces a page (along with any contents stored in this class)
    // from the buffer pool to disk.  Default value forces all pages.
    RC ForcePages (PageNum pageNum = ALL_PAGES);

    RC FindNextSlot(SlotNum &slotNum, PageNum pageNum, char *&data);

    static bool Comp(AttrType attrType, 
                    int attrLength, 
                    CompOp compOp,
                    void *lvalue,
                    void *rvalue);
    static bool LikeComp(char *lvalue, char *rvalue);
};

//
// RM_FileScan: condition-based scan of records in the file
//
class RM_FileScan {
    bool isOpen;
    RM_FileHandle rmFileHandle;
    CompOp compOp;
    AttrType attrType;
    int attrLength;
    int attrOffset;
    void *value;
    PageNum curPageNum;
    SlotNum curSlotNum;
public:
    RM_FileScan  ();
    ~RM_FileScan ();

    RC OpenScan  (const RM_FileHandle &fileHandle,
                  AttrType   attrType,
                  int        attrLength,
                  int        attrOffset,
                  CompOp     compOp,
                  void       *value,
                  ClientHint pinHint = NO_HINT); // Initialize a file scan
    RC GetNextRec(RM_Record &rec);               // Get next matching record
    RC CloseScan ();                             // Close the scan
};

//
// RM_Manager: provides RM file management
//
class RM_Manager {
    PF_Manager *pfManager;
public:
    RM_Manager    (PF_Manager &pfm);
    ~RM_Manager   ();

    RC CreateFile (const char *fileName, int recordSize);
    RC DestroyFile(const char *fileName);
    RC OpenFile   (const char *fileName, RM_FileHandle &fileHandle);

    RC CloseFile  (RM_FileHandle &fileHandle);
};

//
// Print-error function
//
void RM_PrintError(RC rc);

// RM WARN
#define RM_RECORD_DELETED (START_RM_WARN + 0) //record has already deleted
#define RM_NO_FREE_SLOT (START_RM_WARN + 1) //record has already deleted
#define RM_EOF (START_RM_WARN + 2)

// RM ERR
#define RM_RECORD_SIZE_TOO_LARGE (START_RM_ERR - 0) //record size is too large
#define RM_INVALID_RID (START_RM_ERR - 1)
#define RM_INVALID_SCAN (START_RM_ERR - 2)

#endif
