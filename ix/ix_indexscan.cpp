#include "ix.h"
#include <string>
#include <cstring>
#include <vector>

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

IX_IndexScan::IX_IndexScan(){}
IX_IndexScan::~IX_IndexScan(){}

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle, // Initialize index scan
                      CompOp      compOp,
                      void        *value,
                      ClientHint  pinHint){
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
            PageNum pn;
            page.GetPageNum(pn);
            file.UnpinPage(pn);
        }
        page.GetPageNum(curpage);
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
            PageNum pn;
            page.GetPageNum(pn);
            file.UnpinPage(pn);
        }
        page.GetPageNum(curpage);
        curslot = upper_bound_pos(data,value,type,length) - 1;
    }
    return OK_RC;                      
}

RC IX_IndexScan::GetNextEntry (RID &rid){
    if(curpage == 255){
        return IX_EOF;
    }
    if((op == LE_OP || op == LT_OP || op == EQ_OP) && notacc(curpage,curslot)){
        return IX_EOF;
    }
    getrid(curpage, curslot, rid);
    next(curpage,curslot);
    while(op == NE_OP && notacc(curpage,curslot)){
        next(curpage,curslot);
        if(curpage == 255)break;
    }
    return OK_RC;
}

RC IX_IndexScan::getrid(const PageNum &PageNum, const SlotNum &slotnum, RID &rid){
    PF_PageHandle page;
    TRY(file.GetThisPage(PageNum,page));
    char *data;
    TRY(page.GetData(data));
    int a = *((int*)(data + slotnum * (length + 8) + length)),
    b = *((int*)(data + slotnum * (length + 8) + length + 4));
    rid = RID(a,b);
}

void IX_IndexScan::next(PageNum &pagenum, SlotNum &slotnum){
    curslot += 1;
    PF_PageHandle page;
    file.GetThisPage(pagenum,page);
    char *data;
    page.GetData(data);
    if(getsize(data) == slotnum){
        pagenum = data[NEXT];
        slotnum = 0;
    }
}

bool IX_IndexScan::notacc(const PageNum &pagenum,const SlotNum &slotnum){
    PF_PageHandle page;
    file.GetThisPage(pagenum,page);
    char *data;
    page.GetData(data);
    void *qData;
    int pos = slotnum * (length + 4);
    qData = (void*)data + pos;
    switch (type)
    {
        case INT:
            /* code */
            switch (op)
            {
                case EQ_OP:
                    return *((int*)qData) != *((int*)value);
                    break;
                case NE_OP:
                    return *((int*)qData) == *((int*)value);
                    break;
                case LE_OP:
                    return *((int*)qData) > *((int*)value);
                    break;
                case LT_OP:
                    return *((int*)qData) >= *((int*)value);
                    break;
                case GE_OP:
                    return *((int*)qData) < *((int*)value);
                    break;
                case GT_OP:
                    return *((int*)qData) <= *((int*)value);
                    break;
                
                default:
                    break;
            }
            break;
        case FLOAT:
            /* code */
            switch (op)
            {
                case EQ_OP:
                    return *((float*)qData) != *((float*)value);
                    break;
                case NE_OP:
                    return *((float*)qData) == *((float*)value);
                    break;
                case LE_OP:
                    return *((float*)qData) > *((float*)value);
                    break;
                case LT_OP:
                    return *((float*)qData) >= *((float*)value);
                    break;
                case GE_OP:
                    return *((float*)qData) < *((float*)value);
                    break;
                case GT_OP:
                    return *((float*)qData) <= *((float*)value);
                    break;
                
                default:
                    break;
            }
            break;
        case STRING:
            /* code */
            switch (op)
            {
                case EQ_OP:
                    return strcmp((char*)qData,(char*)value) != 0;
                    break;
                case NE_OP:
                    return strcmp((char*)qData,(char*)value) == 0;
                    break;
                case LE_OP:
                    return strcmp((char*)qData,(char*)value) > 0;
                    break;
                case LT_OP:
                    return strcmp((char*)qData,(char*)value) >= 0;
                    break;
                case GE_OP:
                    return strcmp((char*)qData,(char*)value) < 0;
                    break;
                case GT_OP:
                    return strcmp((char*)qData,(char*)value) <= 0;
                    break;
                
                default:
                    break;
            }
            break;
        
        default:
            break;
    }
}