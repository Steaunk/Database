#include "sm.h"
#include "../base.h"
#include <unistd.h>
#include <filesystem>
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
    return OK_RC;
}

RC SM_Manager::DropTable   (const char *relName){
    return OK_RC;
}               // Destroy relation
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