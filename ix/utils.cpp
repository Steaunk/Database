#include "ix.h"
#include <string>
#include <cstring>
#include <vector>

int findpage(char *data, void *pData, AttrType type, int length){
    data += DATA_HEADER_LENGTH;
    int l = 0, r = getsize(data);
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
    data += DATA_HEADER_LENGTH;
    int l = 0, r = getsize(data);
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
    int datalen = 4 + 4*(data[IS_BOTTOM] - '0');
    data += DATA_HEADER_LENGTH;
    int l = -1, r = getsize(data);
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
    int datalen = 4 + 4*(data[IS_BOTTOM] - '0');
    data += DATA_HEADER_LENGTH;
    int l = 1, r = getsize(data);
    while(l < r - 1){
        int mid = l + r >> 1;
        char *qData;
        int pos = mid * (length + datalen);
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
    return l+1;
}

void setsize(char *data, int size){
    data[2] = size/1000 + '0', data[3] = size/100 % 10 + '0', data[4] = size/10 %10 + '0', data[5] = size%10 + '0';
}
int getsize(char *data){
    return (data[2]-'0')*1000+(data[3]-'0')*100+(data[4]-'0')*10+data[5]-'0';
}
