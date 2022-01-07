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
 public:
                                              // Constructor
      QL_Manager (SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
      ~QL_Manager ();                         // Destructor
   RC Select (int           nSelAttrs,        // # attrs in Select clause
              const RelAttr selAttrs[],       // attrs in Select clause
              int           nRelations,       // # relations in From clause
              const char * const relations[], // relations in From clause
              int           nConditions,      // # conditions in Where clause
              const Condition conditions[]);  // conditions in Where clause
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
   
 private:
  RC DeleteOrUpdate (const char *relName,            // relation to delete from
              int        nConditions,         // # conditions in Where clause
              const Condition conditions[],
              int nUpdAttr,
              const RelAttr updAttr[],
              const Value rhsValue[]);
};

#define QL_DATA_NOT_MATCH (START_QL_WARN + 0) // haven't use any database
#define QL_UNKNOW_COLUMN (START_QL_WARN + 1)
#define QL_UNKNOW_INDEX (START_QL_WARN + 2)
//#define RM_EOF (START_RM_WARN + 2)

// QL ERR
//#define SM_CREATE_DB_FAIL (START_SM_ERR - 0) // fail to craete new database