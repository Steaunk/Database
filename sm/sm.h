#pragma once

#include <cstring>
#include "../base.h"
#include "../rm/rm.h"
#include "../ix/ix.h"
#include "../pf/pf.h"
#include "sm_internal.h"
#include <string>

// Used by SM_Manager::CreateTable
struct AttrInfo {
   char     *attrName;           // Attribute name
   AttrType attrType;            // Type of attribute
   int      attrLength;          // Length of attribute
};

// Used by Printer class
struct DataAttrInfo {
   char     relName[MAXNAME+1];  // Relation name
   char     attrName[MAXNAME+1]; // Attribute name
   int      offset;              // Offset of attribute
   AttrType attrType;            // Type of attribute
   int      attrLength;          // Length of attribute
   int      indexNo;             // Attribute index number
};


class SM_Manager {
  private:
    IX_Manager *ixm;
    RM_Manager *rmm;
    bool isOpenDb;
    RC WriteData(const char *relName, TableInfo *data);
    RC ShowIndexes   (const char *relName);
  public:
    int CntAttrOffset(TableInfo *tableInfo, const int &id);
    std::string RMName(const char *relName);
       SM_Manager  (IX_Manager &ixm, RM_Manager &rmm);  // Constructor
       ~SM_Manager ();                                  // Destructor
       SM_Manager  () = delete;                                  // Constructor for test
    RC GetColumnIDByName(const char *attrName, TableInfo *tableInfo, int &ID);
    RC ReadData(const char *relName, TableInfo *data);
    RC OpenDb      (const char *dbName);                // Open database
    RC CloseDb     ();                                  // Close database
    RC CreateDb    (const char *dbName);
    RC DropDb      (const char *dbName);
    RC CreateTable (const char *relName,                // Create relation
                    int        attrCount,
                    AttrInfo   *attributes);
    RC DropTable   (const char *relName);               // Destroy relation
    RC AddColumn   (const char *relName,
                    AttrInfo   *attribute);
    RC DropColumn  (const char *relName,
                    const char *attrName);
    RC CreateIndex (const char *relName,                // Create index
                    const char *attrName);
    RC DropIndex   (const char *relName,                // Destroy index
                    const char *attrName);
    RC AddPrimaryKey (const char *relName,
                      int keyNum,
                      const AttrInfo *attributes);

    RC DropPrimaryKey (const char *relName);
    RC Load        (const char *relName,                // Load utility
                    const char *fileName);
    RC Help        ();                                  // Help for database
    RC Help        (const char *relName);               // Help for relation
    RC ShowDbs       ();              // Show Databases
    RC ShowTables   ();
    RC ShowIndexes   ();
    RC DescTable   (const char *relName);
    RC Set         (const char *paramName,              // Set system parameter
                    const char *value);
    RC GetTableInfo (const char *relName, TableInfo &tableInfo);

    std::string RelNameCat(const char *relNameA, const char *relNameB); //数据表名字拼接
    std::string AttrNameCat(const char *relName, const char *attrName); //数据表与字段名字拼接
    RC InnerJoin(const char *relNameA, const char *relNameB); //Inner Join
};

void SM_PrintError(RC rc, std::string msg);

// SM WARN
#define SM_DB_NOT_OPEN (START_SM_WARN + 0) // haven't use any database
#define SM_DB_EXISTS (START_SM_WARN + 1) // database has already existed
#define SM_DB_NOT_EXISTS (START_SM_WARN + 2)
#define SM_TABLE_EXISTS (START_SM_WARN + 3)
#define SM_TABLE_NOT_EXISTS (START_SM_WARN + 4)
#define SM_UNKNOW_COLUMN (START_SM_WARN + 5)
#define SM_DUPLICATE_KEY (START_SM_WARN + 6)
#define SM_DUPLICATE_ENTRY (START_SM_WARN + 7)
//#define RM_EOF (START_RM_WARN + 2)

// SM ERR
#define SM_CREATE_DB_FAIL (START_SM_ERR - 0) // fail to craete new database
#define SM_DB_OPEN_ERR (START_SM_ERR - 1) // can't open database
#define SM_DB_CLOSE_ERR (START_SM_ERR - 2) // can't close database
#define SM_DB_DUPLICATE_INDEX (START_SM_ERR - 3) //duplicate index
#define SM_DB_WRONG_INDEX (START_SM_ERR - 4) //create a index which is not in column
#define SM_DB_INDEX_FULL (START_SM_ERR - 5) //index overflow
#define SM_DB_NO_INDEX (START_SM_ERR - 6) //delete wrong index