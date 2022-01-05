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
    int keyNum; //联合主键
    char father[MAX_NAME_LENGTH];
    int fatherColumn[MAX_COLUMN_NUM];
    char son[MAX_NAME_LENGTH];
    int sonColumn[MAX_COLUMN_NUM];
};

struct PrimaryKey {
    int keyNum;
    int columnID[MAX_COLUMN_NUM];
    int referenceNum;
    ForeignKey references[MAX_REFERENCE_KEY_NUM];
};

struct Index {
    char name[MAX_NAME_LENGTH];
    int keyNum;
    int columnID[MAX_COLUMN_NUM];
};

struct ColumnAttr{
    char name[MAX_NAME_LENGTH];
    AttrType attrType;            // Type of attribute
    int attrLength;
    bool isPrimaryKey;
    bool isForeignKey;  //是否为外键
    bool hasDefaultValue;
};

struct TableInfo{
    char name[MAX_NAME_LENGTH];

    int columnNum;
    ColumnAttr columnAttr[MAX_COLUMN_NUM];

    PrimaryKey primaryKey;

    ForeignKey foreignKey[MAX_FOREIGN_KEY_NUM];
    
    Index index[MAX_INDEX_NUM];

};