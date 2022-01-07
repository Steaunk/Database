#include "ix.h"
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
using namespace std;

// class IX_IndexHandle {
//   public:
//        IX_IndexHandle  ();                             // Constructor
//        ~IX_IndexHandle ();                             // Destructor
//     RC InsertEntry     (void *pData, const RID &rid);  // Insert new index entry
//     RC DeleteEntry     (void *pData, const RID &rid);  // Delete index entry
//     RC ForcePages      ();                             // Copy index to disk
// };

IX_IndexHandle::IX_IndexHandle(){}
IX_IndexHandle::~IX_IndexHandle(){}

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid){
    PF_PageHandle page;
    TRY(file.GetFirstPage(page));
    char *data;
    TRY(page.GetData(data));
    int root = getroot(data);
    std::vector<int> nodes;
    std::vector<int> poses;
    int cur = root;
    PageNum fpn;
    TRY(page.GetPageNum(fpn));
    TRY(file.UnpinPage(fpn));
    while(true){
        nodes.push_back(cur);
        TRY(file.GetThisPage(cur,page));
        TRY(page.GetData(data));
        if(data[IS_BOTTOM] == '1'){
            break;
        }else{
            cur = findpagepos(data,pData,type,length);
            poses.push_back(cur);
            cur = *((int*)(data + DATA_HEADER_LENGTH + cur * (length + 4) + length));
        }
    }
    for(size_t i = 0; i < nodes.size(); ++i){
        file.UnpinPage(nodes[i]);
    }
    if(!is_full(cur)){
        add_rid(cur,pData,rid);
    }
    else {
        if(cur == root)split_add_root_rid(cur,pData,rid);
        else{
            std::pair<void*, PageNum>son;
            son = split_add_rid(cur,pData,rid);
            for (int i = (int)nodes.size()-2; i >= 0; --i){
                cur = nodes[i];
                if(!is_full(cur)){
                    add_page(cur,son.first,son.second);
                    break;
                }
                else{
                    if(cur != root)
                        son = split_add_page(cur,son.first,son.second);
                    else
                        split_add_root_page(cur,son.first,son.second);
                }
            }
        }
    }
    for (int i = (int)nodes.size()-2; i >= 0; --i){
        update(nodes[i],nodes[i+1],poses[i]);
    }
    return OK_RC;
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid){
    IX_IndexScan temp;
    temp.OpenScan(*this,EQ_OP,pData);
    PageNum sp;
    SlotNum ss;
    sp = temp.GetPageNum(), ss = temp.GetSlotNum();
    RID ans;
    while(true){
        if(temp.GetNextEntry(ans) == IX_EOF){
            return IX_NOT_FOUND;
        }
        if(ans == rid){
            break;
        }
        sp = temp.GetPageNum(), ss = temp.GetSlotNum();
    }
    return _DeleteEntry(pData, rid, sp, ss);
}

RC IX_IndexHandle::_DeleteEntry(void *pData, const RID &rid, PageNum PN, SlotNum SN){
    PF_PageHandle page;
    TRY(file.GetFirstPage(page));
    char *data;
    TRY(page.GetData(data));
    int root = getroot(data);
    int cur = root;
    std::vector<int> nodes;
    std::vector<int> poses;
    PageNum fpn;
    TRY(page.GetPageNum(fpn));
    TRY(file.UnpinPage(fpn));
    while(true){
        nodes.push_back(cur);
        TRY(file.GetThisPage(cur,page));
        TRY(page.GetData(data));
        if(data[IS_BOTTOM] == '1'){
            break;
        }else{
            cur = findpagepos(data,pData,type,length);
            poses.push_back(cur);
            cur = *((int*)(data + DATA_HEADER_LENGTH + cur * (length + 4) + length));
        }
        PageNum pn;
        page.GetPageNum(pn);
        file.UnpinPage(pn);
    }
    
    int len = getsize(data);
    bool f = 0;
    for(int i = 0; i < len; ++i){
        int pos = i*(length + 8) + DATA_HEADER_LENGTH;
        if(f){
            for(int j = 0; j < length + 8; ++j){
                data[pos+j] = data[pos+length+8+j];
            }
        }else{
            char *ipos = data + pos;
            int a = *((int*)(ipos + length)),
            b = *((int*)(ipos + length + 4));
            bool equ = 0;
            switch (type)
            {
                case INT:
                    /* code */
                    if(*((int*)ipos) == *((int*)pData)){
                        equ=1;
                    }
                    break;
                case FLOAT:
                    /* code */
                    if(*((float*)ipos) == *((float*)pData)){
                        equ=1;
                    }
                    break;
                case STRING:
                    /* code */
                    if(strcmp((char*)ipos,(char*)pData) == 0){
                        equ=1;
                    }
                    break;
                
                default:
                    break;
            }
            if(RID(a,b) == rid || (equ && PN != cur)){
                f = 1;
                if(PN != cur){
                    PF_PageHandle temppage;
                    file.GetThisPage(PN,temppage);
                    char *tempdata;
                    temppage.GetData(tempdata);
                    RID temp = RID(a,b);
                    memcpy(tempdata + SN*(length + 8) + DATA_HEADER_LENGTH + length
                    , &temp, sizeof(rid));
                    file.MarkDirty(PN);
                    file.UnpinPage(PN);
                }
                for(int j = 0; j < length + 8; ++j){
                    data[pos+j] = data[pos+length+8+j];
                }
            }
        }
    }
    if(f == 0){
        PageNum pn;
        page.GetPageNum(pn);
        file.MarkDirty(pn);
        file.UnpinPage(pn);
        return IX_NOT_FOUND;
    }
    setsize(data,len-1);
    PageNum pn;
    page.GetPageNum(pn);
    file.MarkDirty(pn);
    file.UnpinPage(pn);
    for (int i = (int)nodes.size()-2; i >= 0; --i){
        update(nodes[i],nodes[i+1],poses[i]);
    }
    file.GetThisPage(root,page);
    page.GetData(data);
    if(getsize(data) == 0){
        data[IS_BOTTOM] = '1';
        file.GetFirstPage(page);
        page.GetData(data);
        setnext(data,root);
        setroot(data,root);
        PageNum fpn;
        page.GetPageNum(fpn);
        file.MarkDirty(fpn);
        file.UnpinPage(fpn);
    }
    file.MarkDirty(root);
    file.UnpinPage(root);
    return OK_RC;
}

RC IX_IndexHandle::ForcePages(){
    return file.ForcePages();
}

bool IX_IndexHandle::is_full(PageNum cur){
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data;
    page.GetData(data);
    //bool ans = getsize(data) * (length+8) + DATA_HEADER_LENGTH + (length+8) >= PF_PAGE_SIZE - 4;
    bool ans = getsize(data) >= MAX_INDEX_SIZE;
    PageNum pn;
    page.GetPageNum(pn);
    file.UnpinPage(pn);
    return ans;
}

void IX_IndexHandle::add_page(PageNum cur, void *pData, PageNum son){
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data;
    page.GetData(data);
    int len = getsize(data);
    int k = 0;
    for(; k < len;++k){
        void *qData = data + k*(length + 4) + DATA_HEADER_LENGTH;
        bool f = 0;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) > *((int*)pData)){
                    f=1;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) > *((float*)pData)){
                    f=1;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 4) + DATA_HEADER_LENGTH] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 4) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 4; ++j){
            data[pos + j + length + 4] = data[pos + j];
        }
    }
    int pos = k*(length+4) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&son),sizeof(PageNum));
    setsize(data,len+1);
    PageNum pn;
    page.GetPageNum(pn);
    file.MarkDirty(pn);
    file.UnpinPage(pn);
}
std::pair<void *,PageNum> IX_IndexHandle::split_add_page(PageNum cur, void *pData, PageNum son){
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data1;
    page.GetData(data1);
    char *data = new char[2*PF_PAGE_SIZE];
    memcpy(data,data1,sizeof(char) * PF_PAGE_SIZE);
    int len = getsize(data);
    int k = 0;
    for(; k < len;++k){
        void *qData = data + k*(length + 4) + DATA_HEADER_LENGTH;
        bool f = 0;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) > *((int*)pData)){
                    f=1;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) > *((float*)pData)){
                    f=1;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 4) + DATA_HEADER_LENGTH] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 4) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 4; ++j){
            data[pos + j + length + 4] = data[pos + j];
        }
    }
    int pos = k*(length+4) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&son),sizeof(PageNum));

    PF_PageHandle newpage;
    file.AllocatePage(newpage);
    char* data2;
    newpage.GetData(data2);
    data2[IS_BOTTOM] = data1[IS_BOTTOM];
    int next1 = getnext(data1);
    PageNum pn,npn;
    page.GetPageNum(pn);
    newpage.GetPageNum(npn);
    setnext(data2,next1);
    setnext(data1,npn);
    setprev(data2,pn);
    if(next1 != CHAIN_EOF){
        PF_PageHandle nextpage;
        file.GetThisPage(next1,nextpage);
        char *datanext;
        nextpage.GetData(datanext);
        setprev(datanext,npn);
        file.MarkDirty(next1);
        file.UnpinPage(next1);
    }
    len = len+1;
    memcpy(data1 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH, (size_t)(len/2 * (length + 4)));
    memcpy(data2 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH + len/2 * (length + 4), (size_t)((len-len/2) * (length + 4)));
    std::pair<void*,PageNum>a;
    a.first = data2 + DATA_HEADER_LENGTH;
    newpage.GetPageNum(a.second);
    setsize(data1,len/2);
    setsize(data2,len-len/2);
    file.MarkDirty(pn);
    file.MarkDirty(npn);
    file.UnpinPage(pn);
    file.UnpinPage(npn);
    return a;
}
void IX_IndexHandle::split_add_root_page(PageNum cur, void *pData, PageNum son){
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data1;
    page.GetData(data1);
    char *data = new char[2*PF_PAGE_SIZE];
    memcpy(data,data1,sizeof(char) * PF_PAGE_SIZE);
    int len = getsize(data);
    int k = 0;
    for(; k < len;++k){
        void *qData = data + k*(length + 4) + DATA_HEADER_LENGTH;
        bool f = 0;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) > *((int*)pData)){
                    f=1;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) > *((float*)pData)){
                    f=1;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 4) + DATA_HEADER_LENGTH] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 4) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 4; ++j){
            data[pos + j + length + 4] = data[pos + j];
        }
    }
    int pos = k*(length+4) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&son),sizeof(PageNum));

    PF_PageHandle newpage;
    file.AllocatePage(newpage);
    char* data2;
    newpage.GetData(data2);
    data2[IS_BOTTOM] = data1[IS_BOTTOM];
    int next1 = getnext(data1);
    PageNum pn,npn;
    page.GetPageNum(pn);
    newpage.GetPageNum(npn);
    setnext(data2,next1);
    setnext(data1,npn);
    setprev(data2,pn);
    if(next1 != CHAIN_EOF){
        PF_PageHandle nextpage;
        file.GetThisPage(next1,nextpage);
        char *datanext;
        nextpage.GetData(datanext);
        setprev(datanext,npn);
        file.MarkDirty(next1);
        file.UnpinPage(next1);
    }
    len = len+1;
    memcpy(data1 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH, (size_t)(len/2 * (length + 4)));
    memcpy(data2 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH + len/2 * (length + 4), (size_t)((len-len/2) * (length + 4)));
    setsize(data1,len/2);
    setsize(data2,len-len/2);

    PF_PageHandle newrootpage;
    file.AllocatePage(newrootpage);
    char *newrootdata;
    newrootpage.GetData(newrootdata);
    newrootdata[IS_BOTTOM] = '0';
    setnext(newrootdata,CHAIN_EOF);
    setprev(newrootdata,CHAIN_EOF);
    setsize(newrootdata,2);
    PageNum nrpn,fpn;
    memcpy(newrootdata+DATA_HEADER_LENGTH,data1+DATA_HEADER_LENGTH,(size_t)length);
    memcpy(newrootdata+DATA_HEADER_LENGTH+length,(void*)(&(pn)),sizeof(PageNum));
    memcpy(newrootdata+DATA_HEADER_LENGTH+length+4,data2+DATA_HEADER_LENGTH,(size_t)length);
    memcpy(newrootdata+DATA_HEADER_LENGTH+length+4+length,(void*)(&(npn)),sizeof(PageNum));

    PF_PageHandle firstpage;
    file.GetFirstPage(firstpage);
    newrootpage.GetPageNum(nrpn);
    firstpage.GetPageNum(fpn);
    char *sysdata;
    firstpage.GetData(sysdata);
    setroot(sysdata, nrpn);

    file.MarkDirty(pn);
    file.MarkDirty(npn);
    file.MarkDirty(nrpn);
    file.MarkDirty(fpn);
    file.UnpinPage(pn);
    file.UnpinPage(npn);
    file.UnpinPage(nrpn);
    file.UnpinPage(fpn);

}
void IX_IndexHandle::add_rid(PageNum cur, void *pData, const RID &rid){
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data;
    page.GetData(data);
    int len = getsize(data);
    int k = 0;
    for(; k < len;++k){
        void *qData = data + k*(length + 8) + DATA_HEADER_LENGTH;
        bool f = 0;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) > *((int*)pData)){
                    f=1;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) > *((float*)pData)){
                    f=1;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 8) + DATA_HEADER_LENGTH] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 8) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 8; ++j){
            data[pos + j + length + 8] = data[pos + j];
        }
    }
    int pos = k*(length+8) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&rid),sizeof(RID));
    setsize(data,len+1);
    PageNum pn;
    page.GetPageNum(pn);
    file.MarkDirty(pn);
    file.UnpinPage(pn);
}
std::pair<void *,PageNum> IX_IndexHandle::split_add_rid(PageNum cur, void *pData, const RID &rid){
    file.MarkDirty(cur);
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data1;
    page.GetData(data1);
    char *data = new char[2*PF_PAGE_SIZE];
    memcpy(data,data1,sizeof(char) * PF_PAGE_SIZE);
    int len = getsize(data);
    int k = 0;
    for(; k < len;++k){
        void *qData = data + k*(length + 8) + DATA_HEADER_LENGTH;
        bool f = 0;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) > *((int*)pData)){
                    f=1;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) > *((float*)pData)){
                    f=1;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 8) + DATA_HEADER_LENGTH] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 8) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 8; ++j){
            data[pos + j + length + 8] = data[pos + j];
        }
    }
    int pos = k*(length+8) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&rid),sizeof(RID));
    PF_PageHandle newpage;
    file.AllocatePage(newpage);
    char* data2;
    newpage.GetData(data2);
    data2[IS_BOTTOM] = data1[IS_BOTTOM];
    int next1 = getnext(data1);
    PageNum pn,npn;
    page.GetPageNum(pn);
    newpage.GetPageNum(npn);
    setnext(data2,next1);
    setnext(data1,npn);
    setprev(data2,pn);
    if(next1 != CHAIN_EOF){
        PF_PageHandle nextpage;
        file.GetThisPage(next1,nextpage);
        char *datanext;
        nextpage.GetData(datanext);
        setprev(datanext,npn);
        file.MarkDirty(next1);
        file.UnpinPage(next1);
    }
    len = len+1;
    memcpy(data1 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH, (size_t)(len/2 * (length + 8)));
    memcpy(data2 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH + len/2 * (length + 8), (size_t)((len-len/2) * (length + 8)));
    std::pair<void*,PageNum>a;
    a.first = data2 + DATA_HEADER_LENGTH;
    newpage.GetPageNum(a.second);
    setsize(data1,len/2);
    setsize(data2,len-len/2);
    file.MarkDirty(pn);
    file.MarkDirty(npn);
    file.UnpinPage(pn);
    file.UnpinPage(npn);
    return a;

}
void IX_IndexHandle::split_add_root_rid(PageNum cur, void *pData, const RID &rid){
    file.MarkDirty(cur);
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data1;
    page.GetData(data1);
    char *data = new char[2*PF_PAGE_SIZE];
    memcpy(data,data1,sizeof(char) * PF_PAGE_SIZE);
    int len = getsize(data);
    int k = 0;
    for(; k < len;++k){
        void *qData = data + k*(length + 8) + DATA_HEADER_LENGTH;
        bool f = 0;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) > *((int*)pData)){
                    f=1;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) > *((float*)pData)){
                    f=1;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 8) + DATA_HEADER_LENGTH] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 8) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 8; ++j){
            data[pos + j + length + 8] = data[pos + j];
        }
    }
    int pos = k*(length+8) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&rid),sizeof(RID));

    PF_PageHandle newpage;
    file.AllocatePage(newpage);
    char* data2;
    newpage.GetData(data2);
    data2[IS_BOTTOM] = data1[IS_BOTTOM];
    int next1 = getnext(data1);
    PageNum pn,npn;
    page.GetPageNum(pn);
    newpage.GetPageNum(npn);
    setnext(data2,next1);
    setnext(data1,npn);
    setprev(data2,pn);
    if(next1 != CHAIN_EOF){
        PF_PageHandle nextpage;
        file.GetThisPage(next1,nextpage);
        char *datanext;
        nextpage.GetData(datanext);
        setprev(datanext,npn);
        file.MarkDirty(next1);
        file.UnpinPage(next1);
    }
    len = len + 1;
    memcpy(data1 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH, (size_t)(len/2 * (length + 8)));
    memcpy(data2 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH + len/2 * (length + 8), (size_t)((len-len/2) * (length + 8)));
    setsize(data1,len/2);
    setsize(data2,len-len/2);

    PF_PageHandle newrootpage;
    file.AllocatePage(newrootpage);
    char *newrootdata;
    newrootpage.GetData(newrootdata);
    newrootdata[IS_BOTTOM] = '0';
    setnext(newrootdata,CHAIN_EOF);
    setprev(newrootdata,CHAIN_EOF);
    setsize(newrootdata,2);
    PageNum nrpn,fpn;
    memcpy(newrootdata+DATA_HEADER_LENGTH,data1+DATA_HEADER_LENGTH,(size_t)length);
    memcpy(newrootdata+DATA_HEADER_LENGTH+length,(void*)(&(pn)),sizeof(PageNum));
    memcpy(newrootdata+DATA_HEADER_LENGTH+length+4,data2+DATA_HEADER_LENGTH,(size_t)length);
    memcpy(newrootdata+DATA_HEADER_LENGTH+length+4+length,(void*)(&(npn)),sizeof(PageNum));
    

    PF_PageHandle firstpage;
    file.GetFirstPage(firstpage);
    newrootpage.GetPageNum(nrpn);
    firstpage.GetPageNum(fpn);
    char *sysdata;
    firstpage.GetData(sysdata);
    setroot(sysdata, nrpn);

    file.MarkDirty(pn);
    file.MarkDirty(npn);
    file.MarkDirty(nrpn);
    file.MarkDirty(fpn);
    file.UnpinPage(pn);
    file.UnpinPage(npn);
    file.UnpinPage(nrpn);
    file.UnpinPage(fpn);

}

void IX_IndexHandle::update(PageNum fa, PageNum son, int pos){
    PF_PageHandle pf,ps;
    file.GetThisPage(fa,pf);
    file.GetThisPage(son,ps);
    char *dataf, *datas;
    pf.GetData(dataf);
    ps.GetData(datas);
    if(getsize(datas) != 0){
        void *qData = dataf + DATA_HEADER_LENGTH + pos * (length+4);
        void *pData = datas + DATA_HEADER_LENGTH;
        bool f = 0;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) > *((int*)pData)){
                    f=1;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) > *((float*)pData)){
                    f=1;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f){
            memcpy(qData,pData,(size_t)length);
        }
        file.MarkDirty(fa);
        file.MarkDirty(son);
        file.UnpinPage(fa);
        file.UnpinPage(son);
    }else{
        int prev,next;
        prev = getprev(datas);
        next = getnext(datas);
        PF_PageHandle pprev, pnext;
        char *datap, *datan;
        file.GetThisPage(prev,pprev);
        file.GetThisPage(next,pnext);
        pprev.GetData(datap);
        pnext.GetData(datan);
        if(next != CHAIN_EOF)setprev(datan, prev);
        if(prev != CHAIN_EOF || datas[IS_BOTTOM] == '1')setnext(datap, next);
        int len = getsize(dataf);
        for(int i = pos; i < len; ++i){
            int posf = i*(length + 4) + DATA_HEADER_LENGTH;
            for(int j = 0; j < length + 4; ++j){
                dataf[posf+j] = dataf[posf+length+4+j];
            }
        }
        setsize(dataf,len-1);
        file.MarkDirty(fa);
        file.MarkDirty(son);
        file.MarkDirty(prev);
        file.MarkDirty(next);
        file.UnpinPage(fa);
        file.UnpinPage(son);
        file.UnpinPage(prev);
        file.UnpinPage(next);
        file.DisposePage(son);
    }
}