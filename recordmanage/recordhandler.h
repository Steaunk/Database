#pragma once
#include "../utils/returncode.h"
#include "../bufmanager/BufPageManager.h"
#include "../utils/rid.h"

#define PAGE_HEADER_SIZE 96 //byte

#define SLOT_NUM_OFFSET 12

class RecordHandler {
	FileManager *fm;
	BufPageManager *bpm;
	int curFileID;

	RC getRecordPos(char *src, int slotID, char *dest);
public:
	RecordHandler ();                     // 构造函数
	~RecordHandler ();                     // 析构函数
	RC createFile(const char *fileName); // 创建文件
	RC destroyFile(const char *fileName); // 删除文件
	RC openFile(const char *fileName); // 通过缓存管理模块打开文件，并获取其句柄
	RC closeFile(); // 关闭fileID对应文件
	RC getRecord(const RID &rid, char *&pData);
				      // 通过页号和槽号访问记录后，相应字节序列可以通过pData访问
	RC deleteRecord(const RID &rid);       // 删除特定记录
	RC insertRecord(RID &rid, const char *pData);
	RC updateRecord(const RID &rid, const char *pData);
                                              // 将特定位置记录更新为字节序列pData
};
