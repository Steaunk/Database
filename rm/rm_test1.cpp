#include "rm.h"
#include <cstdio>
#include <iostream>
using namespace std;

#define RECORD_SIZE 10

static RC test(){
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    RM_FileHandle rmfh;

    cout << "Create File" << endl;
    TRY(rmm.CreateFile("test.sql", RECORD_SIZE));
    //TRY(rmm.CreateFile("test1.sql", RECORD_SIZE * 2));
    cout << "Open File" << endl;
    TRY(rmm.OpenFile("test.sql", rmfh));
    cout << "Close File" << endl;
    TRY(rmm.CloseFile(rmfh));
    //TRY(rmm.OpenFile("test1.sql", rmfh));
    cout << "Destory File" << endl;
    TRY(rmm.DestroyFile("test.sql"));
    //TRY(rmm.CloseFile(rmfh));
    //TRY(rmm.DestroyFile("test.sql"));

    return OK_RC;
}

int main(){
    cout << "rm_test1" << endl;
    cout << "test RM_Manager" << endl;
    RC rc;
    if((rc = test())){
        cout << "error found : " << rc << endl;
        ASSERT(false);
        return 0;
    }
    cout << "ok" << endl;
    return 0;

}