#include "ix.h"
#include <string>
#include <cstring>

// class IX_IndexScan {
//   public:
//        IX_IndexScan  ();                                 // Constructor
//        ~IX_IndexScan ();                                 // Destructor
//     RC OpenScan      (const IX_IndexHandle &indexHandle, // Initialize index scan
//                       CompOp      compOp,
//                       void        *value,
//                       ClientHint  pinHint = NO_HINT);           
//     RC GetNextEntry  (RID &rid);                         // Get next matching entry
//     RC CloseScan     ();                                 // Terminate index scan
// };

IX_IndexScan::IX_IndexScan();
IX_IndexScan::~IX_IndexScan();

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle, // Initialize index scan
                      CompOp      compOp,
                      void        *value,
                      ClientHint  pinHint = NO_HINT){
    op = compOp;
    value = value;
    type = indexHandle.type;
    length = indexHandle.length;
    if(compOp == EQ_OP || compOp == GE_OP){
        PF_PageHandle page;
        TRY(file.GetFirstPage(page));
        char *data;
        TRY(page.GetData(data));
        int root = data[ROOT_POS];
        std::vector<int> nodes;
        int cur = root;
        while(true){
            nodes.push_back(cur);
            TRY(file.GetThisPage(cur,page));
            TRY(page.GetData(data));
            if(data[IS_BOTTOM] == '1'){
                break;
            }else{
                cur = lower_bound_pos(data,value,type,length);
                cur = *((int*)(data + cur * (length + 4) + length + DATA_HEADER_LENGTH));
            }
            file.UnpinPage(page.GetPageNum());
        }
        curpage = page.GetPageNum();
        curslot = lower_bound_pos(data,value,type,length);
    }
    if(compOp == GT_OP){
        PF_PageHandle page;
        TRY(file.GetFirstPage(page));
        char *data;
        TRY(page.GetData(data));
        int root = data[ROOT_POS];
        std::vector<int> nodes;
        int cur = root;
        while(true){
            nodes.push_back(cur);
            TRY(file.GetThisPage(cur,page));
            TRY(page.GetData(data));
            if(data[IS_BOTTOM] == '1'){
                break;
            }else{
                cur = upper_bound_pos(data,value,type,length);
                cur = *((int*)(data + (cur-1) * (length + 4) + length + DATA_HEADER_LENGTH));
            }
            file.UnpinPage(page.GetPageNum());
        }
        curpage = page.GetPageNum();
        curslot = upper_bound_pos(data,value,type,length) - 1;
    }
    return OK_RC;                      
}

RC GetNextEntry (RID &rid){
    if(curpage == lastpage && curslot == lastslot){
        return IX_EOF;
    }
    if((op == LE_OP || op == LT_OP || op == EQ_OP) && notacc(curpage,curslot)){
        return IX_EOF;
    }
    rid = getrid(curpage, curslot);
    next(curpage,curslot);
    while(op == NE_OP && notacc(curpage,curslot)){
        next(curpage,curslot);
    }
    return OK_RC;
}