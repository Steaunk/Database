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
  /*case SM_COLUMN_NOT_EXSITS:
    cerr << "Column '" << msg << "' doesn't exist\n";
    break; */
  case SM_DUPLICATE_KEY:
    cerr << "Multiple primary key defined\n";
    break;
  /*case SM_UNKNOW_COLUMN:
    cerr << "Unknown column '" << msg << "'\n";
    break;*/
  case OK_RC: 
    cerr << "SM : I'm healthy!\n";
    break;
  default:
    break;
  }
}
