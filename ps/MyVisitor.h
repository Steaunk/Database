#pragma once
#include "SQLBaseVisitor.h"
#include "../sm/sm.h"
#include "../ql/ql.h"
//#include "../"
#include <string>
#include <iostream>

#define SM_PRINT(func, msg) { RC rc = func; if(rc != OK_RC) SM_PrintError(rc, msg); }

class MyVisitor:public SQLBaseVisitor{
    SM_Manager *sm;
    QL_Manager *qlm;
    int cnt = 0;
    AttrInfo *attrInfo;
    public:
    MyVisitor():sm(nullptr){}
    MyVisitor(SM_Manager &smm, QL_Manager &qlm):sm(&smm), qlm(&qlm){}

    virtual antlrcpp::Any visitCreate_db(SQLParser::Create_dbContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        SM_PRINT(sm->CreateDb(s.c_str()), s);
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitDrop_db(SQLParser::Drop_dbContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        SM_PRINT(sm->DropDb(s.c_str()), s);
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitShow_dbs(SQLParser::Show_dbsContext *ctx) override {
        SM_PRINT(sm->ShowDbs(), "");
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitUse_db(SQLParser::Use_dbContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        SM_PRINT(sm->OpenDb(s.c_str()), s)
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitShow_tables(SQLParser::Show_tablesContext *ctx) override {
        SM_PRINT(sm->ShowTables(), "");
        return visitChildren(ctx);
    }
    virtual antlrcpp::Any visitDescribe_table(SQLParser::Describe_tableContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        SM_PRINT(sm->DescTable(s.c_str()), s);
        return visitChildren(ctx);
    }
    virtual antlrcpp::Any visitCreate_table(SQLParser::Create_tableContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        int attrCount = ctx->field_list()->field().size();
        /*for(auto f : ctx->field_list()->field()){
            cout << f->getRuleIndex() << "?";
        }*/
        attrInfo = new AttrInfo[attrCount];
        //ctx->field_list()->
        cnt = 0;
        auto retval = visitChildren(ctx);
        SM_PRINT(sm->CreateTable(s.c_str(), attrCount, attrInfo), s.c_str());
        delete attrInfo;
        return retval;
    }

    virtual antlrcpp::Any visitNormal_field(SQLParser::Normal_fieldContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        attrInfo[cnt].attrName = (char *) malloc(s.length());
        strcpy(attrInfo[cnt].attrName, s.c_str());
        if(ctx->type_()->getText() == string("INT")){
            attrInfo[cnt].attrType = INT;
            attrInfo[cnt].attrLength = 4;
        }
        else if(ctx->type_()->getText() == string("FLOAT")){
            attrInfo[cnt].attrType = FLOAT;
            attrInfo[cnt].attrLength = 4;
        }
        else {
            attrInfo[cnt].attrType = STRING;
            attrInfo[cnt].attrLength = stoi(ctx->type_()->Integer()->getText());
        }
        cnt++;
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitPrimary_key_field(SQLParser::Primary_key_fieldContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitForeign_key_field(SQLParser::Foreign_key_fieldContext *ctx) override {
        return visitChildren(ctx);
    }


    virtual antlrcpp::Any visitDrop_table(SQLParser::Drop_tableContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        SM_PRINT(sm->DropTable(s.c_str()), s.c_str());
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitInsert_into_table(SQLParser::Insert_into_tableContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        int cnt_s = 0;
        int cnt_f = 0;
        for(auto vl : ctx->value_lists()->value_list()){
            Value value[vl->value().size()];
            int i = 0;
            for(auto v : vl->value()){
                if(v->Integer() != NULL){
                    value[i].type = INT;
                    value[i].data = (void *)(new int(stoi(v->Integer()->getText())));
                }
                else if(v->Float() != NULL){
                    value[i].type = FLOAT;
                    value[i].data = (void *)(new float(stof(v->Float()->getText())));
                }
                else if(v->String() != NULL){
                    value[i].type = STRING;
                    std::string str = v->String()->getText();
                    //value[i].data = 
                    char *tmp = (char *) malloc(str.length());
                    strcmp(tmp, str.c_str());
                    value[i].data = (char *)tmp;
                }
                else {
                    std::cout << "error : parser error\n";
                    return visitChildren(ctx);
                }
                ++i;
            }
            RC rc = qlm->Insert(s.c_str(), i, value);
            if(rc == OK_RC) ;
            else if(rc == QL_DATA_NOT_MATCH){
                //std::cout << vl->getText() << " : type error\n";
                cnt_f++;
            }
            else {
                SM_PrintError(rc, s);
                return visitChildren(ctx);
            }
            cnt_s++;
        }
        std::cout << "Query OK, " << cnt_s << " rows affected, " << cnt_f << " rows failed " << std::endl;

        return visitChildren(ctx);
    }


};