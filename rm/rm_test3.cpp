#include "rm.h"
#include "rm_rid.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <cstdlib>
using namespace std;

struct Data{
    int i;
    int c;
    int s;
    int ll;
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
    RM_FileScan rms;

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
    //PageNum pageNum;
    //SlotNum slotNum;
    TRY(rmfh.InsertRec((char *)u, rid1));
    TRY(rmfh.InsertRec((char *)w, rid));
    TRY(rmfh.InsertRec((char *)v, rid2));
    TRY(rmfh.InsertRec((char *)w, rid1));
    int value = 3;
    TRY(rms.OpenScan(rmfh, INT, 4, 4, EQ_OP, &value));
    RM_Record rec;
    RC rc = rms.GetNextRec(rec);
    assert(rc == OK_RC);
    char *data;
    rec.GetData(data);
    Data *ans = (Data *)data;
    cout << "ans.c = " << ans->c << endl;
    assert(ans->c == u->c);
    rc = rms.GetNextRec(rec);
    assert(rc == RM_EOF);
    cout << "Close File" << endl;
    TRY(rmm.CloseFile(rmfh));
    cout << "Destory File" << endl;
    TRY(rmm.DestroyFile(FILE1));

    return OK_RC;
}

//test for huge data
/*
static RC test2(){
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    RM_FileHandle rmfh;

    cout << "Create File" << endl;
    TRY(rmm.CreateFile(FILE1, RECORD_SIZE));
    cout << "Open File" << endl;
    TRY(rmm.OpenFile(FILE1, rmfh));
    
    int n = 100000;
    Data *arr = new Data[n];
    for(int i = 0; i < n; ++i){
        arr[i].i = rand();
        arr[i].c = rand() % 256;
        arr[i].s = rand() % 65536;
        arr[i].ll = 1ll * rand() << 32 | rand();
    }

    RID *rid = new RID[n];

    for(int i = 0; i < n; ++i){
        //cout << "InsertRec : " << arr[i].i << ", " << (int)arr[i].c << ", " << arr[i].s << ", " << arr[i].ll << endl;
        TRY(rmfh.InsertRec((char *)(arr + i), rid[i]));
    }

    for(int i = 0; i < n / 5; ++i){
        int c = rand() % n;
        RM_Record rec;
        TRY(rmfh.GetRec(rid[c], rec));
        char *data;
        rec.GetData(data);
        Data *d = (Data *) data;
        assert(*d == arr[c]);
    }

    cout << "Close File" << endl;
    TRY(rmm.CloseFile(rmfh));
    cout << "Destroy File" << endl;
    TRY(rmm.DestroyFile(FILE1));

    return OK_RC;
}

static RC test3(){
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    RM_FileHandle rmfh;

    cout << "Create File" << endl;
    TRY(rmm.CreateFile(FILE1, RECORD_SIZE));
    cout << "Open File" << endl;
    TRY(rmm.OpenFile(FILE1, rmfh));
    
    int n = 100000;
    Data *arr = new Data[n];
    for(int i = 0; i < n; ++i){
        arr[i].i = rand();
        arr[i].c = rand() % 256;
        arr[i].s = rand() % 65536;
        arr[i].ll = 1ll * rand() << 32 | rand();
    }

    RID *rid = new RID[n];
    bool *vis = new bool[n];
    for(int i = 0; i < n; ++i) vis[i] = 0;

    for(int i = 0; i < n / 2; ++i){
        //cout << "InsertRec : " << arr[i].i << ", " << (int)arr[i].c << ", " << arr[i].s << ", " << arr[i].ll << endl;
        TRY(rmfh.InsertRec((char *)(arr + i), rid[i]));
        vis[i] = true;
    }

    for(int i = 0; i < n * 10; ++i){
        bool check = true;
        do{
            int opt = rand() % 4;
            int c = rand() % n;
            switch (opt)
            {
            case 0:
                if(vis[c] == 0){
                    //cout << "InsertRec : " << arr[c].i << ", " << (int)arr[c].c << ", " << arr[c].s << ", " << arr[c].ll << endl;
                    TRY(rmfh.InsertRec((char *)(arr + c), rid[c]));
                    vis[c] = 1;
                    check = false;
                }
                break;
                
            case 1:
                if(vis[c] == 1){
                    //cout << "DeleteRec " << c << ": " << arr[c].i << ", " << (int)arr[c].c << ", " << arr[c].s << ", " << arr[c].ll << endl;
                    PageNum pageNum;
                    SlotNum slotNum;
                    rid[c].GetPageNum(pageNum);
                    rid[c].GetSlotNum(slotNum);
                    //printf("pageNum = %d, slotNum = %d\n", pageNum, slotNum);
                    TRY(rmfh.DeleteRec(rid[c]));
                    vis[c] = 0;
                    check = false;
                }
                break;
            
            case 2:
                if(vis[c] == 1){
                    //cout << "GetRec : " << arr[c].i << ", " << (int)arr[c].c << ", " << arr[c].s << ", " << arr[c].ll << endl;
                    RM_Record rec;
                    TRY(rmfh.GetRec(rid[c], rec));
                    char *data;
                    rec.GetData(data);
                    Data *d = (Data *)data;
                    assert(*d == arr[c]);
                    check = false;
                }
                break;
            case 3:
                if(vis[c] == 1){
                    arr[c].i = rand();
                    arr[c].c = rand() % 256;
                    arr[c].s = rand() % 65536;
                    arr[c].ll = 1ll * rand() << 32 | rand();
   
                    //cout << "UpdateRec : " << arr[c].i << ", " << (int)arr[c].c << ", " << arr[c].s << ", " << arr[c].ll << endl;
                    RM_Record rec;
                    rec.SetData((char *)(arr + c));
                    rec.SetRid(rid[c]);

                    TRY(rmfh.UpdateRec(rec));
                    check = false;

                }
                break;
            }
        }
        while(check);
    }



    cout << "Close File" << endl;
    TRY(rmm.CloseFile(rmfh));
    cout << "Destroy File" << endl;
    TRY(rmm.DestroyFile(FILE1));

    return OK_RC;
}
*/

int main(){
    RC rc;
    cout << "test1 : manual test" << endl;
    if((rc = test1())){
        cout << "error found : " << rc << endl;
        ASSERT(false);
        return 0;
    }
    cout << endl;

    /*cout << "test2 : insert data, then get data" << endl;
    if((rc = test2())){
        cout << "error found : " << rc << endl;
        ASSERT(false);
        return 0;
    }
    cout << endl;

    cout << "test3 : random operations" << endl;
    if((rc = test3())){
        cout << "error found : " << rc << endl;
        ASSERT(false);
        return 0;
    }
    */

    cout << "ok" << endl;
    return 0;

}