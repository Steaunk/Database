#include <cstdio>
#include <cassert>
#include <iostream>
using namespace std;
#include "ix.h"
#include "../pf/pf.h"
#include "../base.h"
PF_Manager pfm;
IX_Manager ixm(pfm);
IX_IndexHandle ixh;
IX_IndexScan ixs;
struct Data{
	int a;
	RID p;
};
#define RECORD_SIZE sizeof(Data) 
#define FILE1 "test.sql"
#define FILE2 "test1.sql" 
int main(){
    PF_Manager pfm;
    IX_Manager rmm(pfm);
    IX_IndexHandle rmfh;

    cout << "Create File" << endl;
    TRY(rmm.CreateIndex(FILE1,0,STRING, RECORD_SIZE));
    cout << "Open File" << endl;
    TRY(rmm.OpenIndex(FILE1,0, rmfh));
    
    // int n = 100000;
    // Data *arr = new Data[n];
    // for(int i = 0; i < n; ++i){
    // }

    // for(int i = 0; i < n / 2; ++i){
        
    // }

    // for(int i = 0; i < n * 10; ++i){
    //     bool check = true;
    //     do{
    //         int opt = rand() % 3;
    //         int c = rand() % n;
    //         switch (opt)
    //         {
    //         case 0:
    //             break;
                
    //         case 1:
    //             break;
            
    //         case 2:
    //             break;
    //         }
    //     }
    //     while(check);
    // }



    cout << "Close File" << endl;
    TRY(rmm.CloseIndex(rmfh));
    cout << "Destroy File" << endl;
    TRY(rmm.DestroyIndex(FILE1,0));

    return OK_RC;
}