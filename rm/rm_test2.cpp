#include "rm.h"
#include "rm_rid.h"
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

struct Data{
    int i;
    char c;
    short s;
    long long ll;
    bool operator == (const Data &tmp) const{
        return i == tmp.i && c == tmp.c && s == tmp.s && ll == tmp.ll;
    }
};

#define RECORD_SIZE sizeof(Data) 
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

    Data *w = new (Data){0, 1, 2, 3};
    Data *u = new (Data){1, 3, 5, 7};
    Data *v = new (Data){2, 4, 6, 8};
    RID rid;
    RID rid1;
    RID rid2;
    PageNum pageNum;
    SlotNum slotNum;
    TRY(rmfh.InsertRec((char *)u, rid1));
    TRY(rmfh.InsertRec((char *)w, rid));
    TRY(rmfh.InsertRec((char *)u, rid2));
    TRY(rmfh.DeleteRec(rid1));
    TRY(rmfh.InsertRec((char *)w, rid1));
    rid.GetPageNum(pageNum);
    rid.GetSlotNum(slotNum);
    printf("pageNum = %d, slotNum = %d\n", pageNum, slotNum);
    assert(slotNum == 1);
    rid1.GetPageNum(pageNum);
    rid1.GetSlotNum(slotNum);
    printf("pageNum = %d, slotNum = %d\n", pageNum, slotNum);
    assert(slotNum == 0);
    RM_Record rec;
    rec.SetData((char *)v);
    rec.SetRid(rid);
    TRY(rmfh.UpdateRec(rec));
    TRY(rmfh.GetRec(rid, rec));
    char *t;
    rec.GetData(t);
    Data *wt = (Data *)t;
    cout << "w : " << wt->i << ", " << (int)wt->c << ", " << wt->s << ", " << wt->ll << endl;
    assert(*wt == *v);
    cout << "Close File" << endl;
    TRY(rmm.CloseFile(rmfh));
    cout << "Destory File" << endl;
    TRY(rmm.DestroyFile(FILE1));

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
    /*if((rc = test2())){
        cout << "error found : " << rc << endl;
        ASSERT(false);
        return 0;
    }*/
    cout << "ok" << endl;
    return 0;

}