#include "sm.h"
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <cstring>

namespace fs = std::filesystem;

SM_Manager::SM_Manager(IX_Manager &ixm, RM_Manager &rmm){
    isOpenDb = false;
}

SM_Manager::SM_Manager(){
    isOpenDb = false;
}

SM_Manager::~SM_Manager(){

}

RC SM_Manager::OpenDb(const char *dbName){
    if(!fs::exists(dbName)) return SM_DB_NOT_EXISTS;
    if(isOpenDb == true) CloseDb();
    if(chdir(dbName) < 0) return SM_DB_OPEN_ERR;
    isOpenDb = true;
    return OK_RC;
}

RC SM_Manager::CloseDb(){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(chdir("..") < 0) return SM_DB_CLOSE_ERR;
    isOpenDb = false;
    return OK_RC;
}

RC SM_Manager::DropDb(const char *dbName){
    if(!fs::exists(dbName)) return SM_DB_NOT_EXISTS;
    ASSERT(isOpenDb == false);
    //if(isOpenDb == true) return SM_DB_NOT_OPEN;
    fs::remove_all(dbName);
    return OK_RC;
}

RC SM_Manager::CreateDb(const char *dbName){
    //printf("START CREATE\n");
    ASSERT(isOpenDb == false);
    if(fs::exists(dbName)) return SM_DB_EXISTS;
    //printf("MIDDLE\n");
    if(fs::create_directory(dbName) == 0) return SM_CREATE_DB_FAIL;
    //printf("END CREATE\n");
    return OK_RC;
}

RC SM_Manager::CreateTable(const char *relName,
                           int        attrCount,
                           AttrInfo   *attributes){
    if(fs::exists(relName)) return SM_TABLE_EXISTS;
    TableInfo tableInfo;
    strcpy(tableInfo.name, relName);
    tableInfo.columnNum = attrCount;
    for(int i = 0; i < attrCount; ++i){
        tableInfo.columnAttr[i].attrLength = attributes[i].attrLength;
        tableInfo.columnAttr[i].attrType = attributes[i].attrType;
        strcpy(tableInfo.columnAttr[i].name, attributes[i].attrName);
    }
    WriteData(relName, &tableInfo);
    return OK_RC;
}

RC SM_Manager::DropTable   (const char *relName){
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    fs::remove(relName);
    return OK_RC;
}               // Destroy relation

RC SM_Manager::AddColumn   (const char *relName,
                            AttrInfo *attrInfo){
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    TableInfo tableInfo;
    ReadData(relName, &tableInfo);
    tableInfo.columnAttr[tableInfo.columnNum].attrLength = attrInfo->attrLength;
    tableInfo.columnAttr[tableInfo.columnNum].attrType = attrInfo->attrType;
    strcpy(tableInfo.columnAttr[tableInfo.columnNum].name, attrInfo->attrName);
    tableInfo.columnNum++;
    WriteData(relName, &tableInfo);

    return OK_RC;
}

RC SM_Manager::GetColumnIDByName(const char *attrName, TableInfo *tableInfo, int &ID){
    for(int i = 0; i < tableInfo->columnNum; ++i){
        if(strcmp(attrName, tableInfo->columnAttr[i].name) == 0){
            ID = i;
            return OK_RC;
        }
    }
    return SM_COLUMN_NOT_EXSITS;
}

RC SM_Manager::DropColumn   (const char *relName,
                             const char *attrName){
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    TableInfo tableInfo; 
    ReadData(relName, &tableInfo);
    int id;
    TRY(GetColumnIDByName(attrName, &tableInfo, id));

    for(int i = id; i < tableInfo.columnNum - 1; ++i){
        tableInfo.columnAttr[i] = tableInfo.columnAttr[i+1];
    }
    tableInfo.columnNum--;
    WriteData(relName, &tableInfo);
    
    return OK_RC;
}

RC SM_Manager::CreateIndex (const char *relName,                // Create index
                const char *attrName){
    return OK_RC;
                }
RC SM_Manager::DropIndex   (const char *relName,                // Destroy index
                const char *attrName){
    return OK_RC;
                }
RC SM_Manager::Load        (const char *relName,                // Load utility
                const char *fileName){

    return OK_RC;
                }
RC SM_Manager::Help        (){

    return OK_RC;
}                         // Help for database
RC SM_Manager::Help        (const char *relName){

    return OK_RC;
}               // Help for relation
RC SM_Manager::Print       (const char *relName){

    return OK_RC;
}               // Print relation
RC SM_Manager::Set         (const char *paramName,              // Set system parameter
                const char *value){

    return OK_RC;
                }

void SM_Manager::WriteData(const char *relName, TableInfo *data){
    FILE *file = fopen(relName, "w");
    fwrite(data, sizeof(TableInfo), 1, file);
    fclose(file);
}

void SM_Manager::ReadData(const char *relName, TableInfo *data){
    FILE *file = fopen(relName, "r");
    fread(data, sizeof(TableInfo), 1, file);
    fclose(file);
}