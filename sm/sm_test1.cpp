#include "sm.h"
#include "../ix/ix.h"
#include "../rm/rm.h"
#include <cstdio>
#include <iostream>
#include <filesystem>
#include <unistd.h>
using namespace std;
namespace fs = std::filesystem;

#define FILE1 "test.sql"
#define FILE2 "test1.sql" 


static RC test1(){
    fs::path p = fs::current_path();
    fs::path fdb = "/first_db";
    p += fdb;
    cout << p << endl;
    cout << fs::current_path() << endl;

    //PF_Manager pfm;
    //RM_Manager rm(pfm);
    //IX_Manager ix(pfm);
    //SM_Manager sm(ix, rm);
    SM_Manager sm;
    cout << "Create Database" << endl;
    TRY(sm.CreateDb("first_db"));
    cout << "Open Database" << endl;
    TRY(sm.OpenDb("first_db"));
    cout << fs::current_path() << endl;
    assert(fs::current_path() == p);
    cout << "Close Database" << endl;
    TRY(sm.CloseDb());
    cout << "Drop Database" << endl;
    TRY(sm.DropDb("first_db"));
    return OK_RC;
}

static RC test2(){
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
}

int main(){
    fs::create_directories("sm_test");
    chdir("sm_test");
    RC rc;
    if((rc = test1()) != OK_RC || (rc = test2()) != OK_RC){
        cout << "RC : " << rc << endl;
        assert(false);
    }
    fs::remove_all("sm_test");
    cout << "ok" << endl;
    return 0;

}