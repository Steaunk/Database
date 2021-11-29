#pragma once
#include "rm.h"

struct RM_FileHeader{
    int recordSize;
    int recordNumPerPage; //per page
    PageNum nextFreePage;
    int pageNum;
};

struct RM_PageHeader{
    int recordNum;
    PageNum nextPage;
};

#define RM_FILE_HEADER_SIZE sizeof(RM_FileHeader)
#define RM_PAGE_HEADER_SIZE sizeof(RM_PageHeader)

#define RM_SLOT_SIZE 2