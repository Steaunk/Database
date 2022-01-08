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
    if(!fs::exists(dbName)){
        chdir(path.c_str());
        return SM_DB_NOT_EXISTS;
    }
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

RC SM_Manager::ShowIndexes(){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    std::string path = ".";
    for (const auto &entry : fs::directory_iterator(path)) if(!entry.path().has_extension()){
	    ShowIndexes(entry.path().filename().c_str());
        std::cout << std::endl;
	}
    return OK_RC;

}

RC SM_Manager::ShowIndexes(const char *relName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;

    TableInfo tableInfo;
    ReadData(relName, &tableInfo);

    std::cout << "### 数据表索引 " << relName << std::endl;

    for(int i = 0; i < MAX_COLUMN_NUM; ++i){
        int columnID = tableInfo.index[i].columnID;
        if(columnID == -1)continue;
        std::cout << tableInfo.columnAttr[columnID].name << " " << tableInfo.columnAttr[columnID].attrLength << " ";
        switch (tableInfo.columnAttr[columnID].attrType)
        {
        case INT:
            std::cout << "INT" << std::endl;
            break;
        case FLOAT:
            std::cout << "FLOAT" << std::endl;
            break;
        case STRING:
            std::cout << "CHAR(" << tableInfo.columnAttr[columnID].attrLength << ")" << std::endl;
            break;
        default:
            break;
        }
    }

    std::cout << "总计：" << tableInfo.indexNum << std::endl;
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

    TableInfo tableInfo;
    ReadData(relName, &tableInfo);
    for(int i = 0; i < tableInfo.indexNum; ++i)
        TRY(ixm->DestroyIndex(relName, i));

    if(tableInfo.primaryKey.keyNum > 0) TRY(ixm->DestroyIndex(relName, -1));

    fs::remove(relName);
    TRY(rmm->DestroyFile(RMName(relName).c_str()));

    std::cout << "Table '" << relName << "' dropped\n";
    return OK_RC;
}               // Destroy relation


RC SM_Manager::DescTable(const char *relName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;

    TableInfo tableInfo;
    ReadData(relName, &tableInfo);

    std::cout << "### 数据表 " << relName << "\n\n";

    for(int i = 0; i < tableInfo.columnNum; ++i){
        std::cout << tableInfo.columnAttr[i].name << " ";
        switch (tableInfo.columnAttr[i].attrType)
        {
        case INT:
            std::cout << "INT ";
            break;
        case FLOAT:
            std::cout << "FLOAT ";
            break;
        case STRING:
            std::cout << "CHAR(" << tableInfo.columnAttr[i].attrLength << ") ";
            break;
        default:
            break;
        }
        if(tableInfo.columnAttr[i].isPrimaryKey) std::cout << "PRI ";
        std::cout << std::endl;
    }

    std::cout << "\n#### 外键约束\n";

    //debug("tableInfo.foreignNum = %d\n", tableInfo.foreignNum);
    for(int i = 0; i < tableInfo.foreignNum; ++i){
        std::cout << tableInfo.foreignKey[i].name << " (";
        for(int j = 0; j < tableInfo.foreignKey[i].keyNum; ++j){
            std::cout << tableInfo.columnAttr[tableInfo.foreignKey[i].columnID[j]].name;
            if(j < tableInfo.foreignKey[i].keyNum - 1) std::cout << ", ";
        }
        std::cout << ") REFERENCES " << tableInfo.foreignKey[i].refRelName << " (";

        TableInfo refTableInfo;
        ReadData(tableInfo.foreignKey[i].refRelName, &refTableInfo);
        for(int j = 0; j < tableInfo.foreignKey[i].keyNum; ++j){
            std::cout << refTableInfo.columnAttr[tableInfo.foreignKey[i].refColumnID[j]].name;
            if(j < tableInfo.foreignKey[i].keyNum - 1) std::cout << ", ";
        }
        std::cout << ")\n";
    }

    std::cout << "\n总计：" << tableInfo.columnNum << std::endl;
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
            --table.indexNum;
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

RC SM_Manager::AddPrimaryKey(const char *relName, int keyNum, const AttrInfo *attributes){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
   
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
    ixm->CreateIndex(relName, -1, first_key.attrType, first_key.attrLength);
    IX_IndexHandle ixih;
    ixm->OpenIndex(relName, -1, ixih);
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
        //debug("AddPrimaryKey data = %d\n", *((int *)(data + 4)));
        ixis.OpenScan(ixih, EQ_OP, data + pkOffset[0]);  
        RM_Record trec;
        char *tdata;

        flag = true;
        while((rc = ixis.GetNextEntry(rid)) != IX_EOF){
            rc = rmfh.GetRec(rid, trec);
            if(rc != OK_RC) break;

            bool fflag = false;
            trec.GetData(tdata);
            for(int i = 1; i < keyNum; ++i){
                auto &t = tableInfo.columnAttr[tableInfo.primaryKey.columnID[i]];
                if(RM_FileHandle::Comp(t.attrType, t.attrLength, EQ_OP, data + pkOffset[i], tdata + pkOffset[i]) == false){
                    fflag = true;
                    break;
                }
            }
            if(fflag == false){
                flag = false;
                break;
            }
        }
        if(rc == IX_EOF) flag = true;
        if(flag == false) break;
        rec.GetRid(rid);
        ixih.InsertEntry(data + pkOffset[0], rid);
    }
    debug("AddPrimaryKey flag = %d\n", flag);
    if(flag == false){
        ixm->CloseIndex(ixih);
        ixm->DestroyIndex(relName, -1);
        debug("AddPrimaryKey FAIL !!!\n");
        if(rc == OK_RC){
            rc = SM_DUPLICATE_ENTRY;
            return rc;
        }
    }

    ixm->CloseIndex(ixih);
    WriteData(relName, &tableInfo);

    return OK_RC;
}

RC SM_Manager::AddForeignKey(const char *foreignName, const char *relName, const char *refRelName, int keyNum, const AttrInfo *foreignKey, const AttrInfo *referenceKey){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    if(!fs::exists(refRelName)) return SM_REF_TABLE_NOT_EXISTS;
    TableInfo tableInfo, refTableInfo;
    ReadData(relName, &tableInfo);
    ReadData(refRelName, &refTableInfo);
    if(refTableInfo.primaryKey.keyNum != keyNum) return SM_COLUMN_NOT_UNIQUE;
    //debug("Add ForeignKey unique\n");
    AttrType attrType[keyNum];
    int offsetL[keyNum], offsetR[keyNum], attrLength[keyNum];
    
    auto &info = tableInfo.foreignKey[tableInfo.foreignNum];
    strcpy(info.relName, relName);
    strcpy(info.refRelName, refRelName);

    for(int i = 0; i < keyNum; ++i){
        int lID, rID;
        TRY(GetColumnIDByName(foreignKey[i].attrName, &tableInfo, lID));
        TRY(GetColumnIDByName(referenceKey[i].attrName, &refTableInfo, rID));
        if(tableInfo.columnAttr[lID].attrType != refTableInfo.columnAttr[rID].attrType ||
            tableInfo.columnAttr[lID].attrLength != refTableInfo.columnAttr[rID].attrLength) return SM_COLUMN_TYPE_NOT_MATCH;
        if(refTableInfo.columnAttr[rID].isPrimaryKey == false) return SM_COLUMN_NOT_UNIQUE;
        offsetL[i] = CntAttrOffset(&tableInfo, lID);
        offsetR[i] = CntAttrOffset(&refTableInfo, rID);
        attrLength[i] = tableInfo.columnAttr[lID].attrLength;
        attrType[i] = tableInfo.columnAttr[lID].attrType;

        info.columnID[i] = lID;
        info.refColumnID[i] = rID;
        std::cout << rID;
    }
    for(int i = 0; i < tableInfo.foreignNum; ++i){
        if(strcmp(tableInfo.foreignKey[i].name, foreignName) == 0)
            return SM_DUPLICATE_NAME;
    }

    info.keyNum = keyNum;
    strcpy(info.name, foreignName);

    auto &refInfo = refTableInfo.primaryKey;
    memcpy(&refInfo.references[refInfo.referenceNum], &info, sizeof(ForeignKey));
    refInfo.referenceNum++;
    tableInfo.foreignNum++;

    RM_FileHandle rmfh, rrmfh;
    RM_FileScan rmfs;
    rmm->OpenFile(RMName(relName).c_str(), rmfh);
    rmfs.OpenScan(rmfh, AttrType(0), 0, 0, NO_OP, nullptr);
    rmm->OpenFile(RMName(refRelName).c_str(), rrmfh);

    debug("AddForeignKey Middle\n");
    RC rc;
    RM_Record rec, refrec;
    IX_IndexHandle ixih;
    ixm->OpenIndex(refRelName, -1, ixih);
    int offset = CntAttrOffset(&refTableInfo, refTableInfo.primaryKey.columnID[0]);
    while((rc = rmfs.GetNextRec(rec)) != RM_EOF){
        IX_IndexScan ixis;     
        char *data;
        rec.GetData(data);
        debug("AddForeignKey Loop begin %d %d %d\n", *((int *)data), *((int *)(data + 4)), *((int *)(data + 8)));
        ixis.OpenScan(ixih, EQ_OP, data + offset);
        RID rid;
        RC rct;
        char *refdata;
        bool flag = false;
        while((rct = ixis.GetNextEntry(rid)) != IX_EOF){
            rrmfh.GetRec(rid, refrec);
            refrec.GetData(refdata);
            bool notEqual = false;
            debug("first\n");
            for(int i = 0; i < keyNum; ++i){
                debug("offsetL [%d], offsetR [%d] <%d>\n", *(int*)(data+offsetL[i]), *(int*)(data+offsetR[i]), attrLength[i]);
                if(!RM_FileHandle::Comp(attrType[i], attrLength[i], EQ_OP, data + offsetL[i], refdata + offsetR[i])){
                    notEqual = true;
                    break;
                }
            }
            debug("Not Equal = %d\n", notEqual);
            if(notEqual == false){
                flag = true;
            }

        }

        if(flag == false){
            rc = SM_ENTRY_NOT_MATCH;
            break;
        }
    }
    if(rc == RM_EOF) rc = OK_RC; 
    rmm->CloseFile(rmfh);
    if(rc == OK_RC){
        WriteData(relName, &tableInfo);
        WriteData(refRelName, &refTableInfo);
    }
    return rc;
}

RC SM_Manager::DropForeignKey(const char *relName, const char *foreignName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;
    TableInfo tableInfo;
    ReadData(relName, &tableInfo);
    for(int i = 0; i < tableInfo.foreignNum; ++i){
        auto &info = tableInfo.foreignKey[i];
        if(strcmp(info.name, foreignName) == 0){
            TableInfo refTableInfo;
            ReadData(info.refRelName, &refTableInfo);
            for(int j = 0; j < refTableInfo.primaryKey.referenceNum; ++j){
                auto &refInfo = refTableInfo.primaryKey;
                if(strcmp(refInfo.references[j].name, info.name) == 0){
                    memcpy(&refInfo.references[j], &refInfo.references[refInfo.referenceNum - 1], sizeof(ForeignKey));
                    refInfo.referenceNum--;
                    break;
                }
            }
            WriteData(info.refRelName, &refTableInfo);
            memcpy(&info, &tableInfo.foreignKey[tableInfo.foreignNum - 1], sizeof(ForeignKey));
            tableInfo.foreignNum--;
            WriteData(relName, &tableInfo);
            return OK_RC;
        }
    }
    return SM_FOREIGNKEY_NOT_EXISTS;
}

RC SM_Manager::DropPrimaryKey(const char *relName){
    if(isOpenDb == false) return SM_DB_NOT_OPEN;
    if(!fs::exists(relName)) return SM_TABLE_NOT_EXISTS;

    TableInfo tableInfo;
    ReadData(relName, &tableInfo);
    if(tableInfo.primaryKey.keyNum == 0) return OK_RC;
    for(int i = 0; i < tableInfo.primaryKey.keyNum; ++i){
        tableInfo.columnAttr[tableInfo.primaryKey.columnID[i]].isPrimaryKey = false;        
    }
    tableInfo.primaryKey.keyNum = 0;
    debug("DropPrimaryKey %s\n", relName);
    ixm->DestroyIndex(relName, -1);
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

RC SM_Manager::TableExist(const char *relName){
    if(fs::exists(relName)) return OK_RC;
    return SM_TABLE_NOT_EXISTS;
}

/*RC SM_Manager::CheckColumn(cnost char *relName, const char *relName_t, const char *attrName_t){

    return OK_RC;
}*/

