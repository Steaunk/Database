#include "ix.h"
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
using namespace std;

int findpage(char *data, void *pData, AttrType type, int length){
    int l = 0, r = getsize(data);
    data += DATA_HEADER_LENGTH;
    while(l < r - 1){
        int mid = l + r >> 1;
        char *qData;
        int pos = mid * (length + 4);
        qData = data + pos;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) <= *((int*)pData)){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) <= *((float*)pData)){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) <= 0){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            
            default:
                break;
        }
    }
    return *((int*)(data + l * (length + 4) + length));
}

RID findrid(char *data, void *pData, AttrType type, int length){
    int l = 0, r = getsize(data);
    data += DATA_HEADER_LENGTH;
    while(l < r - 1){
        int mid = l + r >> 1;
        char *qData;
        int pos = mid * (length + 8);
        qData = (char*)data + pos;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) <= *((int*)pData)){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) <= *((float*)pData)){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) <= 0){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            
            default:
                break;
        }
    }
    int a = *((int*)(data + l * (length + 8) + length)),
    b = *((int*)(data + l * (length + 8) + length + 4));
    return RID(a,b);
}

int lower_bound_pos(char *data, void *pData, AttrType type, int length){
    int l = -1, r = getsize(data);
    int datalen = 4 + 4*(data[IS_BOTTOM] - '0');
    data += DATA_HEADER_LENGTH;
    while(l < r - 1){
        int mid = l + r >> 1;
        char *qData;
        int pos = mid * (length + datalen);
        qData = (char*)data + pos;
        switch (type)
        {
            case INT:
                /* code */
                if(*((int*)qData) < *((int*)pData)){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) < *((float*)pData)){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) < 0){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            
            default:
                break;
        }
    }
    return l+1;
}

int upper_bound_pos(char *data, void *pData, AttrType type, int length){
    int l = -1, r = getsize(data);
    int datalen = 4 + 4*(data[IS_BOTTOM] - '0');
    data += DATA_HEADER_LENGTH;
    cout << "l=" << l << ",r=" << r << endl;
    while(l < r - 1){
        int mid = l + r >> 1;
        char *qData;
        int pos = mid * (length + datalen);
        qData = (char*)data + pos;
        switch (type)
        {
            case INT:
                /* code */
                cout << *(int*) qData << " " << *(int*)pData << endl;
                if(*((int*)qData) <= *((int*)pData)){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            case FLOAT:
                /* code */
                if(*((float*)qData) <= *((float*)pData)){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            case STRING:
                /* code */
                if(strcmp((char*)qData,(char*)pData) <= 0){
                    l = mid;
                }else{
                    r = mid;
                }
                break;
            
            default:
                break;
        }
    }
    cout << l << endl;
    return l+1;
}

void setsize(char *data, int size){
    for(int i = HEADER_LENGTH - 1; i >= SIZE_START; --i){
        data[i] = size % 10 + '0';
        size /= 10;
    }
}
int getsize(char *data){
    int ans = 0;
    for(int i = SIZE_START; i < HEADER_LENGTH; ++i){
        ans = ans * 10 + data[i] - '0';
    }
    return ans;
}

void setnext(char *data, int pagenum){
    for(int i = SIZE_START - 1; i >= NEXT_START; --i){
        data[i] = pagenum % 10 + '0';
        pagenum /= 10;
    }
}
int getnext(char *data){
    int ans = 0;
    for(int i = NEXT_START; i < SIZE_START; ++i){
        ans = ans * 10 + data[i] - '0';
    }
    return ans;
}


void setroot(char *data, int root){
    for(int i = HEADER_LENGTH - 1; i >= ROOT_POS_START; --i){
        data[i] = root % 10 + '0';
        root /= 10;
    }
}
int getroot(char *data){
    int ans = 0;
    for(int i = ROOT_POS_START; i < HEADER_LENGTH; ++i){
        ans = ans * 10 + data[i] - '0';
    }
    return ans;
}

