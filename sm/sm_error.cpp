#include <cerrno>
#include <cstdio>
#include <iostream>
#include <cstring>
#include "sm.h"

using namespace std;

void SM_PrintError(RC rc, std::string msg = "") 
{
  switch (rc)
  {
  case SM_DB_NOT_OPEN: 
    cerr << "No database selected\n";
    break;
  case SM_DB_EXISTS:
    cerr << "Database '" << msg << "' already exists\n";
    break;
  case SM_DB_NOT_EXISTS:
    cerr << "Database '" << msg << "' doesn't exist\n";
    break;
  case SM_TABLE_EXISTS:
    cerr << "Table '" << msg << "' already exists\n";
    break;
  case SM_TABLE_NOT_EXISTS:
    cerr << "Table '" << msg << "' doesn't exist\n";
    break;
  case SM_DB_DUPLICATE_INDEX:
    cerr << "Index already exists\n";
    break;
  case SM_DB_NO_INDEX:
    cerr << "Index doesn't exist\n";
    break;
  /*case SM_COLUMN_NOT_EXSITS:
    cerr << "Column '" << msg << "' doesn't exist\n";
    break; */
  case SM_DUPLICATE_KEY:
    cerr << "Multiple primary key defined\n";
    break;
  /*case SM_UNKNOW_COLUMN:
    cerr << "Unknown column '" << msg << "'\n";
    break;*/
  case SM_DUPLICATE_ENTRY:
    cerr << "Duplicated entry for key\n";
    break;

  case SM_REF_TABLE_NOT_EXISTS:
    cerr << "Reference Table doesn't exist\n";
    break;
  
  case SM_DUPLICATE_NAME:
    cerr << "Duplicated name exist\n";
    break;

  case SM_COLUMN_NOT_UNIQUE:  
    cerr << "Reference table isn't unique\n";
    break;
  
  case SM_COLUMN_TYPE_NOT_MATCH:
    cerr << "Column type doesn't match\n";
    break;

  case SM_ENTRY_NOT_MATCH:
    cerr << "Entry doesn't match\n";
    break;
  
  case OK_RC: 
    cerr << "SM : I'm healthy!\n";
    break;
  default:
    cerr << "RC = " << rc << endl;
    break;
  }
}
