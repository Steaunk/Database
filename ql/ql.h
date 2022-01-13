#include <iostream>
#include "../base.h"
#include "../sm/sm.h"
#include "../ix/ix.h"
#include "../rm/rm.h"

using namespace std;

struct RelAttr {
  char *relName;     // relation name (may be NULL) 
  char *attrName;    // attribute name              
  friend ostream &operator<<(ostream &s, const RelAttr &ra);
};

struct Value {
  AttrType type;     // type of value               
  void     *data;    // value                       
  friend ostream &operator<<(ostream &s, const Value &v);
};

struct Condition {
  RelAttr lhsAttr;      // left-hand side attribute                     
  CompOp  op;           // comparison operator                          
  int     bRhsIsAttr;   // TRUE if right-hand side is an attribute
                        //   and not a value
  RelAttr rhsAttr;      // right-hand side attribute if bRhsIsAttr = TRUE
  Value   rhsValue;     // right-hand side value if bRhsIsAttr = FALSE
  friend ostream &operator<<(ostream &s, const Condition &c);
};

class QL_Manager {
  SM_Manager *smm;
  IX_Manager *ixm;
  RM_Manager *rmm;
  int CntAttrOffset(TableInfo *tableInfo, int id);
  RC InsertAll(TableInfo &tableInfo, char *data);
  RC DeleteAll(TableInfo &tableInfo, char *data, RID rid);
  RC UpdateAll(TableInfo &tableInfo, char *new_data, char *old_data, RID rid);
  RC CheckColumn(int           nSelAttrs,
                 const RelAttr selAttrs[],
                 int           nRelations,       // # relations in From clause
                 const char * const relations[], // relations in From clause
                 int           nConditions,      // # conditions in Where clause
                 const Condition conditions[]);
  RC SelectChecked(int nSelAttrs,
                   const RelAttr selAttrs[],
                   const char *relName,
                   int nConditions,
                   const Condition conditions[],
                   int nAggregator,
                   const Aggregator aggregators[],
                   int limit,
                   int offset);
  RC CheckForeignKeyInsert(const char *relName, char *data);
  RC CheckForeignKeyDelete(const char *relName, char *data);
  bool CheckForeignKeyEqual(const char *relNameA, const char *relNameB, int keyNum, int *columnIDA, int *columnIDB, char *dataA);
             
  
  bool UseIndex(TableInfo &tableInfo, int nConditions, const Condition conditions[], int &indexNo, int &cid);
  /*RC FindRelName(const char *attrName,
                 int nRelations,
                 const char * const relations[]);*/
  RC GetConditionInfo(TableInfo &tableInfo, int nConditions, const Condition conditions[], 
                      AttrType *attrType, int *attrLength, int *lAttrOffset, int *rAttrOffset);
  void PrintData(TableInfo &tableInfo, int nSelAttrs, const RelAttr selAttrs[], char *data);
  void PrintTable(TableInfo &tableInfo, int nSelAttrs, const RelAttr selAttrs[]);
 public:
                                              // Constructor
      QL_Manager (SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
      ~QL_Manager ();                         // Destructor
   RC Select (int           nSelAttrs,        // # attrs in Select clause
              const RelAttr selAttrs[],       // attrs in Select clause
              int           nRelations,       // # relations in From clause
              const char * const relations[], // relations in From clause
              int           nConditions,      // # conditions in Where clause
              const Condition conditions[],
              int           nAggregator,
              const Aggregator aggregators[],
              int limit = -1,
              int offset = 0);  // conditions in Where clause
   RC Insert (const char  *relName,           // relation to insert into
              int         nValues,            // # values to insert
              const Value values[]);          // values to insert
   RC Delete (const char *relName,            // relation to delete from
              int        nConditions,         // # conditions in Where clause
              const Condition conditions[]);  // conditions in Where clause
   RC Update (const char *relName,            // relation to update
              int   nUpdAttr, 
              const RelAttr updAttr[],         // attribute to update
              const Value rhsValue[],          // value on RHS of =
              int   nConditions,              // # conditions in Where clause
              const Condition conditions[]);  // conditions in Where clause

   RC InsertIndex(const char *relName, const char *attrName);
   RC Join(const char *relNameA, const char *relNameB);
   RC Load(const char *fileName, const char *relName);
   RC Store(const char *fileName, const char *relName);
   
 private:
  RC DeleteOrUpdate (const char *relName,            // relation to delete from
              int        nConditions,         // # conditions in Where clause
              const Condition conditions[],
              int nUpdAttr,
              const RelAttr updAttr[],
              const Value rhsValue[]);
  RC CheckPrimaryKey (const char *relName, char *data, int *offset = nullptr, RID rid = RID(-1, -1));
};

#define QL_DATA_NOT_MATCH (START_QL_WARN + 0) // haven't use any database
#define QL_UNKNOW_COLUMN (START_QL_WARN + 1)
#define QL_DUPLICATE_ENTRY (START_QL_WARN + 2)
#define QL_NOT_UNIQUE_TABLE (START_QL_WARN + 3)
#define QL_UNKNOW_INDEX (START_QL_WARN + 4)
#define QL_FOREIGNKEY (START_QL_WARN + 5)
//#define RM_EOF (START_RM_WARN + 2)

// QL ERR
//#define SM_CREATE_DB_FAIL (START_SM_ERR - 0) // fail to craete new database