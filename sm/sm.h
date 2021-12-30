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
    RC OpenDb      (const char *dbName);                // Open database
    RC CloseDb     ();                                  // Close database
    RC CreateDb    (const char *dbName);
    RC DropDb      ();
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
#define SM_DB_NOT_OPEN (START_DB_WARN + 0) // haven't use any database
#define SM_CREATE_DB_FAIL (START_DB_WARN + 1) // fail to craete new database
#define SM_DB_OPEN_ERR (START_DB_WARN + 2) // can't open database
#define SM_DB_EXISTS (START_DB_WARN + 3) // database has already existed
//#define RM_EOF (START_RM_WARN + 2)

// RM ERR
//#define RM_RECORD_SIZE_TOO_LARGE (START_RM_ERR - 0) //record size is too large
//#define RM_INVALID_RID (START_RM_ERR - 1)
//#define RM_INVALID_SCAN (START_RM_ERR - 2)