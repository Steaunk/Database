#pragma once
#include "rm.h"
#include <cassert>

struct RM_FileHeader{
    int recordSize;
    int recordNumPerPage; //per page
    PageNum nextFreePage;
    int pageNum;
    int bitmapOffset;
    int bitmapSize;
};

struct RM_PageHeader{
    int recordNum;
    PageNum nextFreePage;
};

/*struct RM_Slot{
    char 
}*/

typedef bool RM_Slot;

#define RM_FILE_HEADER_SIZE sizeof(RM_FileHeader)
#define RM_PAGE_HEADER_SIZE sizeof(RM_PageHeader)

#define RM_SLOT_SIZE 1 //sizeof(RM_Slot)

#define NO_FREE_PAGE -1

#define ASSERT(condition) assert(condition)