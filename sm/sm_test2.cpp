#include "sm.h"
#include "../ix/ix.h"
#include "../rm/rm.h"
#include <cstdio>
#include <iostream>
#include <filesystem>
#include <cstring>
#include <unistd.h>
using namespace std;
namespace fs = std::filesystem;

#define FILE1 "test.sql"
#define FILE2 "test1.sql" 

AttrInfo attr[3] = {
    (AttrInfo){(char *)"good", INT, 4},
    (AttrInfo){(char *)"morning", FLOAT, 4},
    (AttrInfo){(char *)"ohno", STRING, 20}
};

AttrInfo tc1 = (AttrInfo){(char *)"steaunk", STRING, 44};

const int attrCount = 3;

static RC test1(){
    fs::path p = fs::current_path();
    fs::path fdb = "/first_db";
    p += fdb;
    cout << p << endl;
    cout << fs::current_path() << endl;

    SM_Manager sm;
    cout << "Create Database" << endl;
    TRY(sm.CreateDb("first_db"));
    cout << "Open Database" << endl;
    TRY(sm.OpenDb("first_db"));
    cout << fs::current_path() << endl;

    TRY(sm.CreateTable("table", attrCount, attr));
    TRY(sm.DropTable("table"));
    TRY(sm.CreateTable("table2", attrCount - 1, attr));
    TRY(sm.AddColumn("table2", &tc1));
    TRY(sm.DropColumn("table2", "good"));

    FILE *file = fopen("table2", "r");
    TableInfo tableInfo;
    fread(&tableInfo, sizeof(TableInfo), 1, file);

    assert(tableInfo.columnNum == 2);
    assert(strcmp(tableInfo.columnAttr[1].name, "steaunk") == 0);
    assert(tableInfo.columnAttr[1].attrType == STRING);
    assert(tableInfo.columnAttr[1].attrLength == 44);

    assert(fs::current_path() == p);
    cout << "Close Database" << endl;
    TRY(sm.CloseDb());
    cout << "Drop Database" << endl;
    TRY(sm.DropDb("first_db"));
    return OK_RC;
}

/*static RC test2(){
    fs::path p = fs::current_path();
    fs::path db1 = p;
    db1 += "/db1";
    fs::path db2 = p;
    db2 += "/db2";
    cout << db1 << endl << db2 << endl;
    cout << fs::current_path() << endl;
    SM_Manager sm;
    cout << "Create Database" << endl;
    TRY(sm.CreateDb("db1"));
    TRY(sm.CreateDb("db2"));
    cout << "Open Database" << endl;
    TRY(sm.OpenDb("db1"));
    cout << fs::current_path() << endl;
    assert(fs::current_path() == db1);
    cout << "Close Database" << endl;
    TRY(sm.CloseDb());

    TRY(sm.OpenDb("db2"));
    assert(fs::current_path() == db2);
    TRY(sm.CloseDb());
    TRY(sm.DropDb("db1"));
    TRY(sm.DropDb("db2"));
    return OK_RC;
}*/

int main(){
    fs::create_directories("sm_test");
    chdir("sm_test");
    RC rc;
    if((rc = test1()) != OK_RC){
        cout << "RC : " << rc << endl;
        assert(false);
    }
    chdir("..");
    fs::remove_all("sm_test");
    cout << "ok" << endl;
    return 0;

}