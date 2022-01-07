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
    return OK_RC;
                }
RC SM_Manager::DropIndex   (const char *relName,                // Destroy index
                const char *attrName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
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

RC SM_Manager::AddPrimaryKey(const char *relName, int keyNum, const AttrInfo *attributes){
    TableInfo tableInfo;
    TRY(ReadData(relName, &tableInfo));
    if(tableInfo.primaryKey.keyNum > 0) return SM_DUPLICATE_KEY;
    tableInfo.primaryKey.keyNum = keyNum;
    for(int i = 0; i < keyNum; ++i){
        RC rc = GetColumnIDByName(attributes[i].attrName, &tableInfo, tableInfo.primaryKey.columnID[i]);
        if(rc != OK_RC){
            if(rc == SM_UNKNOW_COLUMN){
                std::cout << "Unknown column '" << attributes[i].attrName << "'\n";
            }
            return rc;
        }
    }

    int pkOffset[keyNum];
    //----- check duplicate --- 
    for(int i = 0; i < keyNum; ++i){
        if(tableInfo.columnAttr[tableInfo.primaryKey.columnID[i]].isPrimaryKey == true)
            return SM_DUPLICATE_KEY;
        tableInfo.columnAttr[tableInfo.primaryKey.columnID[i]].isPrimaryKey = true;
        pkOffset[i] = CntAttrOffset(&tableInfo, tableInfo.primaryKey.columnID[i]);
    }
    //----- check end ---

    int first_key_id = tableInfo.primaryKey.columnID[0];
    auto &first_key = tableInfo.columnAttr[first_key_id];
    ixm->CreateIndex(relName, tableInfo.indexNum, first_key.attrType, first_key.attrLength);
    IX_IndexHandle ixih;
    ixm->OpenIndex(relName, tableInfo.indexNum, ixih);
    RM_FileHandle rmfh;
    rmm->OpenFile(RMName(relName).c_str(), rmfh);
    RM_FileScan rmfs;
    rmfs.OpenScan(rmfh, AttrType(0), 0, 0, NO_OP, nullptr);
    RC rc;
    RM_Record rec;
    bool flag = true;
    while((rc = rmfs.GetNextRec(rec)) != RM_EOF){
        flag = false;
        if(rc != OK_RC) break;    
        IX_IndexScan ixis;
        char *data;
        RID rid;
        rec.GetData(data);
        ixis.OpenScan(ixih, EQ_OP, data + pkOffset[0]);  
        rc = ixis.GetNextEntry(rid);
        if(rc == IX_EOF) flag = true;
        if(flag == false){
            RM_Record trec;
            char *tdata;
            
            rc = rmfh.GetRec(rid, trec);
            if(rc != OK_RC) break;

            trec.GetData(tdata);
            for(int i = 1; i < keyNum; ++i){
                auto &t = tableInfo.columnAttr[tableInfo.primaryKey.columnID[i]];
                if(rmfs.Comp(t.attrType, t.attrLength, EQ_OP, data + pkOffset[i], tdata + pkOffset[i]) == false){
                    flag = true;
                    break;
                }
            }
        }
        if(flag == false) break;
        rec.GetRid(rid);
        ixih.InsertEntry(data + pkOffset[0], rid);
    }

    if(flag == false){
        ixm->DestroyIndex(relName, tableInfo.indexNum);
        if(rc == OK_RC){
            rc = SM_DUPLICATE_ENTRY;
            return rc;
        }
    }

    strcpy(tableInfo.index[tableInfo.indexNum].name, "Primary");
    tableInfo.index[tableInfo.indexNum].columnID = tableInfo.primaryKey.columnID[0];
    tableInfo.indexNum++;
    WriteData(relName, &tableInfo);
    return OK_RC;
}

RC SM_Manager::GetTableInfo(const char *relName, TableInfo &tableInfo){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    ReadData(relName, &tableInfo);
    return OK_RC;
}

int SM_Manager::CntAttrOffset(TableInfo *tableInfo, const int &id){
    int offset = 0;
    for(int i = 0; i < id; ++i){
        offset += tableInfo->columnAttr[i].attrLength;
    }
    return offset;
}

std::string SM_Manager::RMName(const char *relName){
    std::string s(relName, relName + strlen(relName));
    s += ".rec";
    return s;
}

/*RC SM_Manager::CheckColumn(cnost char *relName, const char *relName_t, const char *attrName_t){

    return OK_RC;
}*/

