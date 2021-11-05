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

	int RecordEndPos(char *b, int num);
public:
	RecordHandler ();                     // 构造函数
	~RecordHandler ();                     // 析构函数
	RC CreateFile(const char *fileName); // 创建文件
	RC DestroyFile(const char *fileName); // 删除文件
	RC OpenFile(const char *fileName); // 通过缓存管理模块打开文件，并获取其句柄
	RC CloseFile(); // 关闭fileID对应文件
	RC GetSlotNum(const int &pageID);
	RC GetRecordBySlotID(const RID &rid, char *&pData);
	RC GetRecordGreaterThan(const RID &rid, const int &key, char *&pData);
	RC GetRecordLessThan(const RID &rid, const int &key, char *&pData);
				      // 通过页号和槽号访问记录后，相应字节序列可以通过pData访问
	RC DeleteRecord(const RID &rid);       // 删除特定记录
	RC InsertRecord(const RID &rid, const char *pData);
	RC UpdateRecord(const RID &rid, const char *pData);
                                              // 将特定位置记录更新为字节序列pData
};
