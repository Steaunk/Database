#include "ix.h"
#include <string>
#include <cstring>
#include <vector>

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
            cur = find_data(data,pData,type,length);
        }
    }
    for(int i = 0; i < nodes.size(); ++i){
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
            for (int i = nodes.size()-2; i; --i){
                cur = nodes[i];
                if(!is_full(cur)){
                    add_page(cur,son.first,son.second);
                    break;
                }
                else{
                    if(cur != root)
                        son = split_add_page(cur,son.first,son.second)
                    else
                        split_add_root_page(cur,son.first,son.second);
                }
            }
        }
    }
    return OK_RC;
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid){
    PF_PageHandle page;
    TRY(file.GetFirstPage(page));
    char *data;
    TRY(page.GetData(data));
    int root = data[ROOT_POS];
    int cur = root;
    while(true){
        nodes.push_back(cur);
        TRY(file.GetThisPage(cur,page));
        TRY(page.GetData(data));
        if(data[IS_BOTTOM] == '1'){
            break;
        }else{
            cur = find_data(data,pData,type,length);
        }
        file.UnpinPage(page.GetPageNum());
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
            int a = *((int*)(pos + length)),
            b = *((int*)(pos + length + 4));
            if(RID(a,b) == rid){
                f = 1;
                for(int j = 0; j < length + 8; ++j){
                    data[pos+j] = data[pos+length+8+j];
                }
            }
        }
    }
    setsize(data,len-1);
    file.MarkDirty(page.GetPageNum());
    file.UnpinPage(page.GetPageNum());
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
    bool ans = strlen(data) + length + 10 < PF_PAGE_SIZE;
    file.UnpinPage(page.GetPageNum());
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
                if(strcmp(qData,pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 4)] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 4) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 4; ++j){
            data[pos + j + length + 4] = data[pos + j];
        }
    }
    pos = k*(length+4) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&son),sizeof(PageNum));
    setsize(data,len+1);
    file.MarkDirty(page.GetPageNum());
    file.UnpinPage(page.GetPageNum());
}
std::pair<void *,PageNum> IX_IndexHandle::split_add_page(PageNum cur, void *pData, PageNum son){
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data1;
    page.GetData(data1);
    char *data = new char[2*PF_PAGE_SIZE];
    strcpy(data,data1);
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
                if(strcmp(qData,pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 4)] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 4) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 4; ++j){
            data[pos + j + length + 4] = data[pos + j];
        }
    }
    pos = k*(length+4) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&son),sizeof(PageNum));

    PF_PageHandle newpage;
    file.AllocatePage(newpage);
    char* data2;
    newpage.GetData(data2);
    data2[IS_BOTTOM] = data1[IS_BOTTOM];
    data2[NEXT] = data1[NEXT];
    data1[NEXT] = (char)(newpage.GetPageNum());
    memcpy(data1 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH, (size_t)(len/2 * (length + 4)));
    memcpy(data2 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH + len/2 * (length + 4), (size_t)((len+1-len/2) * (length + 4)));
    std::pair<void*,PageNum>a;
    a.first = data2 + DATA_HEADER_LENGTH;
    a.second = newpage.GetPageNum();
    setsize(data1,len/2);
    setsize(data2,len+1-len/2);
    file.MarkDirty(page.GetPageNum());
    file.MarkDirty(newpage.GetPageNum());
    file.UnpinPage(page.GetPageNum());
    file.UnpinPage(newpage.GetPageNum());
    return a;
}
void IX_IndexHandle::split_add_root_page(PageNum cur, void *pData, PageNum son){
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data1;
    page.GetData(data1);
    char *data = new char[2*PF_PAGE_SIZE];
    strcpy(data,data1);
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
                if(strcmp(qData,pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 4)] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 4) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 4; ++j){
            data[pos + j + length + 4] = data[pos + j];
        }
    }
    pos = k*(length+4) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&son),sizeof(PageNum));

    PF_PageHandle newpage;
    file.AllocatePage(newpage);
    char* data2;
    newpage.GetData(data2);
    data2[IS_BOTTOM] = data1[IS_BOTTOM];
    data2[NEXT] = data1[NEXT];
    data1[NEXT] = (char)(newpage.GetPageNum());
    memcpy(data1 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH, (size_t)(len/2 * (length + 4)));
    memcpy(data2 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH + len/2 * (length + 4), (size_t)((len+1-len/2) * (length + 4)));
    setsize(data1,len/2);
    setsize(data2,len+1-len/2);

    PF_PageHandle newrootpage;
    file.AllocatePage(newrootpage);
    char *newrootdata;
    newrootpage.GetData(newrootdata);
    newrootdata[IS_BOTTOM] = '0';
    newrootdata[NEXT] = (char)255;
    setsize(newrootdata,2);
    memcpy(newrootdata+DATA_HEADER_LENGTH,data1+DATA_HEADER_LENGTH,(size_t)length);
    memcpy(newrootdata+DATA_HEADER_LENGTH+length,(void*)(&(page.GetPageNum())),sizeof(PageNum));
    memcpy(newrootdata+DATA_HEADER_LENGTH+length+4,data2+DATA_HEADER_LENGTH,(size_t)length);
    memcpy(newrootdata+DATA_HEADER_LENGTH+length+4+length,(void*)(&(newpage.GetPageNum())),sizeof(PageNum));

    PF_PageHandle firstpage;
    file.GetFirstPage(firstpage);
    char *sysdata;
    firstpage.GetData(sysdata);
    sysdata[ROOT_POS] = (char) newrootpage.GetPageNum();

    file.MarkDirty(page.GetPageNum());
    file.MarkDirty(newpage.GetPageNum());
    file.MarkDirty(newrootpage.GetPageNum());
    file.MarkDirty(firstpage.GetPageNum());
    file.UnpinPage(page.GetPageNum());
    file.UnpinPage(newpage.GetPageNum());
    file.UnpinPage(newrootpage.GetPageNum());
    file.UnpinPage(firstpage.GetPageNum());

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
                if(strcmp(qData,pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 8)] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 8) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 8; ++j){
            data[pos + j + length + 8] = data[pos + j];
        }
    }
    pos = k*(length+8) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&rid),sizeof(RID));
    setsize(data,len+1);
    file.MarkDirty(page.GetPageNum());
    file.UnpinPage(page.GetPageNum());
}
std::pair<void *,PageNum> IX_IndexHandle::split_add_rid(PageNum cur, void *pData, const RID &rid){
    file.MarkDirty(cur);
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data1;
    page.GetData(data1);
    char *data;
    strcpy(data,data1);
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
                if(strcmp(qData,pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 8)] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 8) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 8; ++j){
            data[pos + j + length + 8] = data[pos + j];
        }
    }
    pos = k*(length+8) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&rid),sizeof(RID));

    PF_PageHandle newpage;
    file.AllocatePage(newpage);
    char* data2;
    newpage.GetData(data2);
    data2[IS_BOTTOM] = data1[IS_BOTTOM];
    data2[NEXT] = data1[NEXT];
    data1[NEXT] = (char)(newpage.GetPageNum());
    memcpy(data1 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH, (size_t)(len/2 * (length + 8)));
    memcpy(data2 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH + len/2 * (length + 8), (size_t)((len+1-len/2) * (length + 8)));
    std::pair<void*,PageNum>a;
    a.first = data2 + DATA_HEADER_LENGTH;
    a.second = newpage.GetPageNum();
    setsize(data1,len/2);
    setsize(data2,len+1-len/2);
    file.MarkDirty(page.GetPageNum());
    file.MarkDirty(newpage.GetPageNum());
    file.UnpinPage(page.GetPageNum());
    file.UnpinPage(newpage.GetPageNum());
    return a;

}
void IX_IndexHandle::split_add_root_rid(PageNum cur, void *pData, const RID &rid){
    file.MarkDirty(cur);
    PF_PageHandle page;
    file.GetThisPage(cur,page);
    char *data1;
    page.GetData(data1);
    char *data;
    strcpy(data,data1);
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
                if(strcmp(qData,pData) > 0){
                   f=1;
                }
                break;
            
            default:
                break;
        }
        if(f)break;
    }
    data[(len + 1)*(length + 8)] = 0;
    for(int i = len - 1; i >= k; --i){
        int pos = i*(length + 8) + DATA_HEADER_LENGTH;
        for(int j = 0; j < length + 8; ++j){
            data[pos + j + length + 8] = data[pos + j];
        }
    }
    pos = k*(length+8) + DATA_HEADER_LENGTH;
    memcpy(data+pos,pData,(size_t)length);
    memcpy(data+pos+length,(void*)(&rid),sizeof(RID));

    PF_PageHandle newpage;
    file.AllocatePage(newpage);
    char* data2;
    newpage.GetData(data2);
    data2[IS_BOTTOM] = data1[IS_BOTTOM];
    data2[NEXT] = data1[NEXT];
    data1[NEXT] = (char)(newpage.GetPageNum());
    memcpy(data1 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH, (size_t)(len/2 * (length + 8)));
    memcpy(data2 + DATA_HEADER_LENGTH, data + DATA_HEADER_LENGTH + len/2 * (length + 8), (size_t)((len+1-len/2) * (length + 8)));
    setsize(data1,len/2);
    setsize(data2,len+1-len/2);

    PF_PageHandle newrootpage;
    file.AllocatePage(newrootpage);
    char *newrootdata;
    newrootpage.GetData(newrootdata);
    newrootdata[IS_BOTTOM] = '0';
    newrootdata[NEXT] = (char)255;
    setsize(newrootdata,2);
    memcpy(newrootdata+DATA_HEADER_LENGTH,data1+DATA_HEADER_LENGTH,(size_t)length);
    memcpy(newrootdata+DATA_HEADER_LENGTH+length,(void*)(&(page.GetPageNum())),sizeof(PageNum));
    memcpy(newrootdata+DATA_HEADER_LENGTH+length+4,data2+DATA_HEADER_LENGTH,(size_t)length);
    memcpy(newrootdata+DATA_HEADER_LENGTH+length+4+length,(void*)(&(newpage.GetPageNum())),sizeof(PageNum));

    PF_PageHandle firstpage;
    file.GetFirstPage(firstpage);
    char *sysdata;
    firstpage.GetData(sysdata);
    sysdata[ROOT_POS] = (char) newrootpage.GetPageNum();

    file.MarkDirty(page.GetPageNum());
    file.MarkDirty(newpage.GetPageNum());
    file.MarkDirty(newrootpage.GetPageNum());
    file.MarkDirty(firstpage.GetPageNum());
    file.UnpinPage(page.GetPageNum());
    file.UnpinPage(newpage.GetPageNum());
    file.UnpinPage(newrootpage.GetPageNum());
    file.UnpinPage(firstpage.GetPageNum());


    file.MarkDirty(page.GetPageNum());
    file.MarkDirty(newpage.GetPageNum());
    file.UnpinPage(page.GetPageNum());
    file.UnpinPage(newpage.GetPageNum());

}