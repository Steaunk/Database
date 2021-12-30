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
struct data_{
	int a;
	RID p;
};
int main(){
	puts("NULL");
	RC code;
	if(code = ixm.CreateIndex("test_index",0,INT,4)){
		cout << "Create failed" << endl;
		PF_PrintError(code);
		assert(false);
	}
	if(code = ixm.OpenIndex("test_index",0,ixh)){
		cout << "Open failed" << endl;
		PF_PrintError(code);
		assert(false);
	}
	cout << "open success" << endl;
	data_ t1;
	t1.a = 3;
	t1.p = RID(2,3);
	ixh.InsertEntry((void*)(&t1.a),(t1.p));
	cout << "success 1" << endl;
	t1.a = 4;
	t1.p = RID(1,3);
	ixh.InsertEntry((void*)(&t1.a),(t1.p));
	cout << "success 2" << endl;
	t1.a = 2;
	t1.p = RID(1,2);
	ixh.InsertEntry((void*)(&t1.a),(t1.p));
	cout << "success 3" << endl;
	int t = 2;
	ixs.OpenScan(ixh,GT_OP,&t);
	RID trid;
	ixs.GetNextEntry(trid);
	PageNum pn;
	SlotNum sn;
	cout << trid.GetPageNum(pn) << " " << trid.GetSlotNum(sn) << endl;
	cout << pn << " " << sn << endl;
	assert(trid==RID(2,3));
	ixm.CloseIndex(ixh);
	ixm.DestroyIndex("test_index",0);
	return 0;
}
