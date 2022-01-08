#pragma once

#define MAX_NAME_LENGTH 50
#define MAX_COLUMN_NUM 30
#define MAX_TABLE_NUM 30
#define MAX_FOREIGN_KEY_NUM 30
#define MAX_REFERENCE_KEY_NUM 30
#define MAX_INDEX_NUM 30

//外键约束
struct ForeignKey {
    char name[MAX_NAME_LENGTH];
    int keyNum = 0; //联合主键
    char relName[MAX_NAME_LENGTH]; //
    int columnID[MAX_COLUMN_NUM];
    char refRelName[MAX_NAME_LENGTH];
    int refColumnID[MAX_COLUMN_NUM];
};

struct PrimaryKey {
    char name[MAX_NAME_LENGTH];
    int keyNum = 0;
    int columnID[MAX_COLUMN_NUM];
    int referenceNum;
    ForeignKey references[MAX_REFERENCE_KEY_NUM];
};

struct Index {
    char name[MAX_NAME_LENGTH];
    //没有联合索引！
    //int keyNum;
    int columnID;
};

struct ColumnAttr{
    char name[MAX_NAME_LENGTH];
    AttrType attrType;            // Type of attribute
    int attrLength = 0;
    bool isPrimaryKey = false;
    //bool isForeignKey = false;  //是否为外键
    bool hasDefaultValue = false;
};

struct TableInfo{
    int size = 0;

    char name[MAX_NAME_LENGTH];

    int columnNum = 0;
    ColumnAttr columnAttr[MAX_COLUMN_NUM];

    PrimaryKey primaryKey;

    int foreignNum = 0;
    ForeignKey foreignKey[MAX_FOREIGN_KEY_NUM];

    int indexNum = 0; 
    Index index[MAX_INDEX_NUM];

};