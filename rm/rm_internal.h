#pragma once
#include "rm.h"

struct RM_FileHeader{
    int recordSize;
    int recordNum; //per page
    PageNum nextFreePage;
    int pageNum;
};