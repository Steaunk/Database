#include <cstdio>
#include "ix.h"
#include "../pf/pf.h"
#include "../base.h"
PF_Manager pfm;
IX_Manager ixm(pfm);
IX_IndexHandle ixh;
IX_IndexScan ixs;
int main(){
	puts("NULL");
	ixm.CreateIndex("test_index",0,INT,4);
	ixm.CreateIndex("test_index",1,FLOAT,4);
	ixm.CreateIndex("test_index_2",2,STRING,4);
	ixm.OpenIndex("test_index",0,ixh);
	ixm.CloseIndex(ixh);
	ixm.OpenIndex("test_index", 1, ixh);
	ixm.CloseIndex(ixh);
	ixm.OpenIndex("test_index_2", 2, ixh);
	ixm.CloseIndex(ixh);
	ixm.DestroyIndex("test_index",0);
	ixm.DestroyIndex("test_index",1);
	ixm.DestroyIndex("test_index_2",2);
	return 0;
}
