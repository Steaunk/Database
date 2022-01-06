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
	for(int i = 10; i >= 0; --i){
		t1.a = i;
		t1.p = RID(i,i);
		ixh.DeleteEntry(&t1.a,t1.p);
	}
	for(int i = 20; i >= 18; --i){
		t1.a = i;
		t1.p = RID(i,i);
		ixh.InsertEntry(&t1.a,t1.p);
	}
	for(int i = 0; i <= 4; ++i){
		t1.a = i;
		t1.p = RID(i,i);
		ixh.InsertEntry(&t1.a,t1.p);
	}
	for(int i = 17; i >= 5; --i){
		t1.a = i;
		t1.p = RID(i,i);
		ixh.InsertEntry(&t1.a,t1.p);
	}
	for(int i = 0; i <= 12; ++i){
		t1.a = i;
		t1.p = RID(i,i);
		ixh.DeleteEntry(&t1.a,t1.p);
	}
	for(int i = 100; i >= 71; --i){
		t1.a = i;
		t1.p = RID(i,i);
		ixh.InsertEntry(&t1.a,t1.p);
	}
	for(int i = 21; i <= 70; ++i){
		t1.a = i;
		t1.p = RID(i,i);
		ixh.InsertEntry(&t1.a,t1.p);
	}
	for(int i = 54; i <= 98; ++i){
		t1.a = i;
		t1.p = RID(i,i);
		ixh.DeleteEntry(&t1.a,t1.p);
	}
	int t = 50;
	ixh.DeleteEntry(&t, RID(50,50));
	ixs.OpenScan(ixh,GE_OP,&t);
	RID ans;
	ixs.GetNextEntry(ans);
	PageNum p;
	SlotNum s;
	ans.GetPageNum(p);
	ans.GetSlotNum(s);
	cout << p << " " << s << endl;
	assert(ans==RID(51,51));
	assert(ixs.GetNextEntry(ans) != -1);
	assert(ixh.DeleteEntry(&t, RID(5,5)) == -3);
	assert(ans==RID(52,52));
	assert(ixs.GetNextEntry(ans) != -1);
	assert(ans==RID(53,53));
	assert(ixs.GetNextEntry(ans) != -1);
	assert(ans==RID(99,99));
	assert(ixs.GetNextEntry(ans) != -1);
	assert(ans==RID(100,100));
	assert(ixs.GetNextEntry(ans) == -1);
	ixs.OpenScan(ixh,LE_OP,&t);
	assert(ixs.GetNextEntry(ans) != -1);
	assert(ans==RID(13,13));
	cout << "OK" << endl;
	ixm.CloseIndex(ixh);
	ixm.DestroyIndex("test_index",0);
	return 0;
}
