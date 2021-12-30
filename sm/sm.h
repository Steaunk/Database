#pragma once

#include "../base.h"
#include "../rm/rm.h"
#include "../ix/ix.h"

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
    bool isOpenDb;
  public:
       SM_Manager  (IX_Manager &ixm, RM_Manager &rmm);  // Constructor
       ~SM_Manager ();                                  // Destructor
       SM_Manager  ();                                  // Constructor for test
    RC OpenDb      (const char *dbName);                // Open database
    RC CloseDb     ();                                  // Close database
    RC CreateDb    (const char *dbName);
    RC DropDb      (const char *dbName);
    RC CreateTable (const char *relName,                // Create relation
                    int        attrCount,
                    AttrInfo   *attributes);
    RC DropTable   (const char *relName);               // Destroy relation
    RC CreateIndex (const char *relName,                // Create index
                    const char *attrName);
    RC DropIndex   (const char *relName,                // Destroy index
                    const char *attrName);
    RC Load        (const char *relName,                // Load utility
                    const char *fileName);
    RC Help        ();                                  // Help for database
    RC Help        (const char *relName);               // Help for relation
    RC Print       (const char *relName);               // Print relation
    RC Set         (const char *paramName,              // Set system parameter
                    const char *value);
};


// SM WARN
#define SM_DB_NOT_OPEN (START_SM_WARN + 0) // haven't use any database
#define SM_DB_EXISTS (START_SM_WARN + 1) // database has already existed
#define SM_DB_NOT_EXISTS (START_SM_WARN + 2)
//#define RM_EOF (START_RM_WARN + 2)

// SM ERR
#define SM_CREATE_DB_FAIL (START_SM_ERR - 0) // fail to craete new database
#define SM_DB_OPEN_ERR (START_SM_ERR - 1) // can't open database
#define SM_DB_CLOSE_ERR (START_SM_ERR - 2) // can't close database
