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
	for(int i = 0; i <= 10; ++i){
		t1.a = i;
		t1.p = RID(i,i);
		ixh.InsertEntry(&t1.a,t1.p);
	}
	int t = 5;
	ixs.OpenScan(ixh,EQ_OP,&t);
	RID ans;
	ixs.GetNextEntry(ans);
	PageNum p;
	SlotNum s;
	ans.GetPageNum(p);
	ans.GetSlotNum(s);
	cout << p << " " << s << endl;
	assert(ans==RID(5,5));
	assert(ixs.GetNextEntry(ans) == -1);
	ixm.CloseIndex(ixh);
	ixm.DestroyIndex("test_index",0);
	return 0;
}
