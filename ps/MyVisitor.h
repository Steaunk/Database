#pragma once
#include "SQLBaseVisitor.h"
#include "../sm/sm.h"
#include <string>
#include <iostream>

class MyVisitor:public SQLBaseVisitor{
    SM_Manager *psm;
    public:
    MyVisitor():psm(nullptr){}
    MyVisitor(SM_Manager *p):psm(p){}

    virtual antlrcpp::Any visitCreate_db(SQLParser::Create_dbContext *ctx) override {
        std::cout << "here" << std::endl;
        std::string s = ctx->Identifier()->getSymbol()->getText();
        std::cout << s << std::endl;
        psm->CreateDb(s.c_str());
        std::cout << s << std::endl;
        return visitChildren(ctx);
    }

};