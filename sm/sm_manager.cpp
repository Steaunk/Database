#include "sm.h"
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <iostream>

namespace fs = std::filesystem;

SM_Manager::SM_Manager(IX_Manager &ixm, RM_Manager &rmm): ixm(&ixm), rmm(&rmm){
    isOpenDb = false;
    this->ixm = &ixm;
    this->rmm = &rmm;
}

SM_Manager::~SM_Manager(){
}

RC SM_Manager::OpenDb(const char *dbName){
    if(isOpenDb == true) CloseDb();
    if(!fs::exists(dbName)) return SM_DB_NOT_EXISTS;
    if(chdir(dbName) < 0) return SM_DB_OPEN_ERR;
    isOpenDb = true;
    std::cout << "Database changed" << std::endl;
    return OK_RC;
}

RC SM_Manager::CloseDb(){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(chdir("..") < 0) return SM_DB_CLOSE_ERR;
    isOpenDb = false;
    return OK_RC;
}

RC SM_Manager::DropDb(const char *dbName){
    fs::path path = fs::current_path();
    if(isOpenDb == true){
        chdir("..");
    }
    if(!fs::exists(dbName)) return SM_DB_NOT_EXISTS;
    //if(isOpenDb == true) return SM_DB_NOT_OPEN;
    fs::remove_all(dbName);
    if(isOpenDb){
        if(fs::exists(path)) chdir(path.c_str());
        else isOpenDb = false;
    }
    std::cout << "Database '" << dbName << "' dropped\n";
    return OK_RC;
}

RC SM_Manager::CreateDb(const char *dbName){
    //printf("START CREATE\n");
    fs::path path = fs::current_path();
    if(isOpenDb == true) chdir("..");
    if(fs::exists(dbName)) return SM_DB_EXISTS;
    //printf("MIDDLE\n");
    if(fs::create_directory(dbName) == 0) return SM_CREATE_DB_FAIL;
    //printf("END CREATE\n");
    if(isOpenDb == true) chdir(path.c_str());
    std::cout << "Database '" << dbName << "' created\n";
    return OK_RC;
}

RC SM_Manager::ShowDbs(){
    std::string path = isOpenDb ? "../." : ".";
    std::cout << "# 数据库 \n\n";
    int cnt = 0;
    for (const auto &entry : fs::directory_iterator(path)){
	    std::cout << "- " << entry.path().filename() << std::endl;
        cnt++;
	}
    std::cout << "\n总计：" << cnt << std::endl;
    return OK_RC;
}

RC SM_Manager::ShowTables(){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    std::string path = ".";

    std::cout << "## 数据表 \n\n";

    int cnt = 0;
    for (const auto &entry : fs::directory_iterator(path)) if(!entry.path().has_extension()){
	    std::cout << "- " << entry.path().filename() << std::endl;
        cnt++;
	}
    std::cout << "\n总计：" << cnt << std::endl;

    return OK_RC;
}


RC SM_Manager::CreateTable(const char *relName,
                           int        attrCount,
                           AttrInfo   *attributes){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(fs::exists(relName)) return SM_TABLE_EXISTS;
    TableInfo tableInfo;
    strcpy(tableInfo.name, relName);
    tableInfo.columnNum = attrCount;
    int recordSize = 0;
    for(int i = 0; i < attrCount; ++i){
        tableInfo.columnAttr[i].attrLength = attributes[i].attrLength;
        recordSize += attributes[i].attrLength;
        tableInfo.columnAttr[i].attrType = attributes[i].attrType;
        strcpy(tableInfo.columnAttr[i].name, attributes[i].attrName);
    }
    tableInfo.size = recordSize;
    tableInfo.indexNum = 0;
    for(int i = 0; i < MAX_COLUMN_NUM; ++i){
        tableInfo.index[i].columnID = -1;
    }
    TRY(WriteData(relName, &tableInfo));
    rmm->CreateFile(RMName(relName).c_str(), recordSize);
    std::cout << "Table '" << relName << "' created\n";
    return OK_RC;
}

RC SM_Manager::DropTable   (const char *relName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    fs::remove(relName);
    rmm->DestroyFile(RMName(relName).c_str());
    std::cout << "Table '" << relName << "' dropped\n";
    return OK_RC;
}               // Destroy relation


RC SM_Manager::DescTable(const char *relName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;

    TableInfo tableInfo;
    ReadData(relName, &tableInfo);

    std::cout << "### 数据表 " << relName << std::endl;

    for(int i = 0; i < tableInfo.columnNum; ++i){
        std::cout << tableInfo.columnAttr[i].name << " " << tableInfo.columnAttr[i].attrLength << " ";
        switch (tableInfo.columnAttr[i].attrType)
        {
        case INT:
            std::cout << "INT" << std::endl;
            break;
        case FLOAT:
            std::cout << "FLOAT" << std::endl;
            break;
        case STRING:
            std::cout << "CHAR(" << tableInfo.columnAttr[i].attrLength << ")" << std::endl;
            break;
        default:
            break;
        }
    }

    std::cout << "总计：" << tableInfo.columnNum << std::endl;
    return OK_RC;

}

RC SM_Manager::AddColumn   (const char *relName,
                            AttrInfo *attrInfo){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    TableInfo tableInfo;
    ReadData(relName, &tableInfo);
    tableInfo.columnAttr[tableInfo.columnNum].attrLength = attrInfo->attrLength;
    tableInfo.columnAttr[tableInfo.columnNum].attrType = attrInfo->attrType;
    strcpy(tableInfo.columnAttr[tableInfo.columnNum].name, attrInfo->attrName);
    tableInfo.columnNum++;
    tableInfo.size += attrInfo->attrLength;
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
    return SM_UNKNOW_COLUMN;
}

RC SM_Manager::DropColumn   (const char *relName,
                             const char *attrName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    TableInfo tableInfo; 
    ReadData(relName, &tableInfo);
    int id;
    TRY(GetColumnIDByName(attrName, &tableInfo, id));
    tableInfo.size -= tableInfo.columnAttr[id].attrLength;
    for(int i = id; i < tableInfo.columnNum - 1; ++i){
        tableInfo.columnAttr[i] = tableInfo.columnAttr[i+1];
    }
    tableInfo.columnNum--;
    WriteData(relName, &tableInfo);
    
    return OK_RC;
}

RC SM_Manager::CreateIndex (const char *relName,                // Create index
                const char *attrName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    TableInfo table;
    ReadData(relName, &table);
    int columnID = -1;
    for(int i = 0; i < MAX_COLUMN_NUM; ++i){
        if(strcmp(attrName, table.index[i].name) == 0 && table.index[i].columnID != -1){
            return SM_DB_DUPLICATE_INDEX;
        }
        if(strcmp(attrName,table.columnAttr[i].name) == 0){
            columnID = i;
        }
    }
    if(columnID == -1){
        return SM_DB_WRONG_INDEX;
    }
    for(int i = 0; i < MAX_COLUMN_NUM; ++i){
        if(table.index[i].columnID == -1){
            ++table.indexNum;
            ixm->CreateIndex(relName,i,table.columnAttr[columnID].attrType,table.columnAttr[columnID].attrLength);
            table.index[i].columnID = columnID;
            strcpy(table.index[i].name,attrName);
            WriteData(relName,&table);
            return OK_RC;
        }
    }
    return SM_DB_INDEX_FULL;
                }
RC SM_Manager::DropIndex   (const char *relName,                // Destroy index
                const char *attrName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    TableInfo table;
    ReadData(relName, &table);
    for(int i = 0; i < MAX_COLUMN_NUM; ++i){
        if(strcmp(attrName, table.index[i].name) == 0 && table.index[i].columnID != -1){
            memset(table.index[i].name,0,sizeof(table.index[i].name));
            table.index[i].columnID = -1;
            ixm->DestroyIndex(relName,i);
            WriteData(relName, &table);
            return OK_RC;
        }
    }
    return SM_DB_NO_INDEX;
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
RC SM_Manager::Set         (const char *paramName,              // Set system parameter
                const char *value){

    return OK_RC;
                }

RC SM_Manager::WriteData(const char *relName, TableInfo *data){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    FILE *file = fopen(relName, "w");
    fwrite(data, sizeof(TableInfo), 1, file);
    fclose(file);
    return OK_RC;
}

RC SM_Manager::ReadData(const char *relName, TableInfo *data){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    FILE *file = fopen(relName, "r");
    fread(data, sizeof(TableInfo), 1, file);
    fclose(file);
    return OK_RC;
}

RC SM_Manager::GetTableInfo(const char *relName, TableInfo &tableInfo){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    ReadData(relName, &tableInfo);
    return OK_RC;
}

std::string SM_Manager::RMName(const char *relName){
    std::string s(relName, relName + strlen(relName));
    s += ".rec";
    return s;
}

std::string SM_Manager::RelNameCat(const char *relNameA, const char *relNameB){
    std::string s = relNameA;
    return s + "." + relNameB;
} //数据表名字拼接
std::string SM_Manager::AttrNameCat(const char *relName, const char *attrName){
    std::string s = attrName;
    if(s.find(".") != -1)return attrName;
    return relName + ("." + s);
} //数据表与字段名字拼接
RC SM_Manager::InnerJoin(const char *relNameA, const char *relNameB){
    TableInfo tableA,tableB;
    ReadData(relNameA,&tableA);
    ReadData(relNameB,&tableB);
    int count = tableA.columnNum + tableB.columnNum;
    AttrInfo *attrs = new AttrInfo[count + 5];
    for(int i = 0; i < tableA.columnNum; ++i){
        strcpy(attrs[i].attrName, (this->AttrNameCat(relNameA,tableA.columnAttr[i].name)).c_str());
        attrs[i].attrType = tableA.columnAttr[i].attrType;
        attrs[i].attrLength = tableA.columnAttr[i].attrLength;
    }
    for(int i = 0; i < tableB.columnNum; ++i){
        strcpy(attrs[i+tableA.columnNum].attrName, (this->AttrNameCat(relNameA,tableB.columnAttr[i].name)).c_str());
        attrs[i+tableA.columnNum].attrType = tableB.columnAttr[i].attrType;
        attrs[i+tableA.columnNum].attrLength = tableB.columnAttr[i].attrLength;
    }
    this->CreateTable(this->RelNameCat(relNameA,relNameB).c_str(),count,attrs);
    delete[] attrs;
}

/*RC SM_Manager::CheckColumn(cnost char *relName, const char *relName_t, const char *attrName_t){

    return OK_RC;
}*/

