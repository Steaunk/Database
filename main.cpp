#include <cstdio>
#include "ix/ix.h"
#include "pf/pf.h"
#include "base.h"
PF_Manager pfm;
IX_Manager ixm(pfm);
IX_IndexHandle ixh;
IX_IndexScan ixs;
int main(){
	puts("NULL");
	ixm.CreateIndex("test_index",0,INT,4);
	ixm.OpenIndex("test_index",0,ixh);
	return 0;
}
