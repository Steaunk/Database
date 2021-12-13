#include "rm.h"
#include <cstdio>
#include <iostream>
using namespace std;

#define RECORD_SIZE 10
#define FILE1 "test.sql"
#define FILE2 "test1.sql" 

static RC test1(){
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    RM_FileHandle rmfh;

    cout << "Create File" << endl;
    TRY(rmm.CreateFile(FILE1, RECORD_SIZE));
    cout << "Open File" << endl;
    TRY(rmm.OpenFile(FILE1, rmfh));
    cout << "Close File" << endl;
    TRY(rmm.CloseFile(rmfh));
    cout << "Destory File" << endl;
    TRY(rmm.DestroyFile(FILE1));

    return OK_RC;
}

static RC test2(){
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    RM_FileHandle rmfh;

    cout << "Create two files" << endl;
    TRY(rmm.CreateFile(FILE1, RECORD_SIZE));
    TRY(rmm.CreateFile(FILE2, RECORD_SIZE * 2));
    cout << "Open File 1" << endl;
    TRY(rmm.OpenFile(FILE1, rmfh));
    cout << "Close File 1" << endl;
    TRY(rmm.CloseFile(rmfh));
    cout << "Open File 2" << endl;
    TRY(rmm.OpenFile(FILE2, rmfh));
    cout << "Destory File 1" << endl;
    TRY(rmm.DestroyFile(FILE1));
    cout << "Close File 2" << endl;
    TRY(rmm.CloseFile(rmfh));
    cout << "Destory File 2" << endl;
    TRY(rmm.DestroyFile(FILE2));

    return OK_RC;
}



int main(){
    cout << "rm_test1" << endl;
    cout << "test RM_Manager" << endl;
    RC rc;
    if((rc = test1())){
        cout << "error found : " << rc << endl;
        ASSERT(false);
        return 0;
    }
    if((rc = test2())){
        cout << "error found : " << rc << endl;
        ASSERT(false);
        return 0;
    }
    cout << "ok" << endl;
    return 0;

}