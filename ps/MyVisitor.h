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
    int cnt2 = 0;
    int cnt3 = 0;
    int cnt4 = 0;
    AttrInfo *attrInfo;
    Condition *conditions;
    AttrInfo *attrInfo2;
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
        attrInfo2 = new AttrInfo[attrCount];
        //ctx->field_list()->
        cnt = 0;
        cnt2 = 0;
        cnt3 = 0;
        auto retval = visitChildren(ctx);
        if(cnt2 > 1){
            std::cout << "Multiple primary key defined\n";
        }
        else {
            RC rc = sm->CreateTable(s.c_str(), cnt, attrInfo);
            if(rc != OK_RC) SM_PrintError(rc, s.c_str());
            else if(cnt2 == 1){
                rc = sm->AddPrimaryKey(s.c_str(), cnt3, attrInfo2);
                if(rc != OK_RC){
                    sm->DropTable(s.c_str());
                    SM_PrintError(rc, s.c_str());
                }
            }
       }
        delete[] attrInfo;
        delete[] attrInfo2;
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
        for(auto i : ctx->identifiers()->Identifier()){
            attrInfo2[cnt3].attrName = (char *)malloc(i->getText().length());
            strcpy(attrInfo2[cnt3].attrName, i->getText().c_str());
            cnt3++;
        }
        cnt2++;
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

    bool AnalyseValue(Value &value, SQLParser::ValueContext *v){
        if(v->Integer() != NULL){
            value.type = INT;
            value.data = (void *)(new int(stoi(v->Integer()->getText())));
        }
        else if(v->Float() != NULL){
            value.type = FLOAT;
            value.data = (void *)(new float(stof(v->Float()->getText())));
        }
        else if(v->String() != NULL){
            value.type = STRING;
            std::string str = v->String()->getText();
            char *tmp = (char *) malloc(str.length());
            strcpy(tmp, str.c_str());
            value.data = (void *)tmp;
        }
        else return false;
        return true;
    }

    virtual antlrcpp::Any visitInsert_into_table(SQLParser::Insert_into_tableContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        int cnt_s = 0;
        int cnt_f = 0;
        for(auto vl : ctx->value_lists()->value_list()){
            Value value[vl->value().size()];
            int i = 0;
            for(auto v : vl->value()){
                if(!AnalyseValue(value[i], v)) {
                    std::cout << "error : parser error\n";
                    return visitChildren(ctx);
                }
                ++i;
            }
            RC rc = qlm->Insert(s.c_str(), i, value);
            if(rc == OK_RC);
            else if(rc == QL_DATA_NOT_MATCH || rc == QL_DUPLICATE_ENTRY){
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

    virtual antlrcpp::Any visitDelete_from_table(SQLParser::Delete_from_tableContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();
        int size = ctx->where_and_clause()->where_clause().size();
        conditions = new Condition[size];
        cnt = 0;
        auto retval = visitChildren(ctx);
        SM_PRINT(qlm->Delete(s.c_str(), cnt, conditions), s.c_str()); 
        return retval;
    }
    virtual antlrcpp::Any visitUpdate_table(SQLParser::Update_tableContext *ctx) override {
        std::string s = ctx->Identifier()->getSymbol()->getText();

        int size = ctx->where_and_clause()->where_clause().size();
        conditions = new Condition[size];
        cnt = 0;
        auto retval = visitChildren(ctx);

        size = ctx->set_clause()->value().size();
        Value *values = new Value[size];
        RelAttr *relAttr = new RelAttr[size];
        auto sc = ctx->set_clause();
        for(int i = 0; i < size; ++i){
            std::string s = sc->Identifier(i)->getSymbol()->getText().c_str();
            relAttr[i].attrName = (char *) malloc(s.length());
            strcpy(relAttr[i].attrName, s.c_str());
            AnalyseValue(values[i], sc->value(i));
        }

        SM_PRINT(qlm->Update(s.c_str(), size, relAttr, values, cnt, conditions), s.c_str()); 
        return retval;
    }

    virtual antlrcpp::Any visitAlter_table_add_pk(SQLParser::Alter_table_add_pkContext *ctx) override {
        std::string s = ctx->Identifier(0)->getSymbol()->getText();
        std::string c = ctx->Identifier(1)->getSymbol()->getText();
        int size = ctx->identifiers()->Identifier().size();
        attrInfo = new AttrInfo[size];
        int u = 0;
        for(auto i : ctx->identifiers()->Identifier()){
            attrInfo[u].attrName = (char *)malloc(i->getText().length());
            cout << i->getText() << std::endl;
            strcpy(attrInfo[u].attrName, i->getText().c_str());
            u++;
        }
        RC rc = sm->AddPrimaryKey(s.c_str(), size, attrInfo);
        if(rc != OK_RC){ SM_PrintError(rc, s); }
        else std::cout << "Primary key added\n";
        delete[] attrInfo;
        return visitChildren(ctx);
    }
    virtual antlrcpp::Any visitAlter_table_drop_pk(SQLParser::Alter_table_drop_pkContext *ctx) override {
        std::string s = ctx->Identifier(0)->getSymbol()->getText();

        RC rc = sm->DropPrimaryKey(s.c_str());
        if(rc == OK_RC) std::cout << "Primary key dropped\n";
        else SM_PrintError(rc, s);

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitSelect_table(SQLParser::Select_tableContext *ctx) override {
        int nSelAttrs = ctx->selectors()->selector().size();
        int cntC = 0, cntA = 0;
        for(auto sel : ctx->selectors()->selector()){
            if(sel->column() != 0) cntC++;
            else cntA++; 
        }
        int nRelations = ctx->identifiers()->Identifier().size();
        char *relations[nRelations];
        string relt[nRelations];
        int i = 0;
        for(auto rel : ctx->identifiers()->Identifier()){
            relt[i] = rel->getText();
            relations[i] = (char *)malloc(relt[i].length());
            strcpy(relations[i], relt[i].c_str());
            i++;
        }
        int size = ctx->where_and_clause()->where_clause().size();
        conditions = new Condition[size];
        RelAttr relAttr[cntC];
        string strA[cntC];
        string strB[cntC];
                
        if(cntC && cntA){
            std::cout << "Error" << endl;
        }
        else{
            if(cntC){
                int i = 0;
                for(auto sel : ctx->selectors()->selector()){
                    strA[i] = sel->column()->Identifier(0)->getText();
                    strB[i] = sel->column()->Identifier(1)->getText();
                    relAttr[i].relName = (char *)malloc(strA[i].length());
                    relAttr[i].attrName = (char *)malloc(strB[i].length());
                    strcpy(relAttr[i].relName, strA[i].c_str());
                    strcpy(relAttr[i].attrName, strB[i].c_str());
                    i++;
                }
           }
        }
        auto retval = visitChildren(ctx);
        SM_PRINT(qlm->Select(cntC, relAttr, nRelations, relations, size, conditions), "");
        return retval;
    }

    virtual antlrcpp::Any visitWhere_operator_expression(SQLParser::Where_operator_expressionContext *ctx) override {
        //cout << &(ctx->column()) << " ???? " << end;;
        conditions[cnt].lhsAttr.relName = (char *) malloc(ctx->column()->Identifier(0)->getText().length());
        strcpy(conditions[cnt].lhsAttr.relName, ctx->column()->Identifier(0)->getText().c_str()); //<< endl;
        conditions[cnt].lhsAttr.attrName = (char *) malloc(ctx->column()->Identifier(1)->getText().length());
        strcpy(conditions[cnt].lhsAttr.attrName, ctx->column()->Identifier(1)->getText().c_str()); //<< endl;
        
        if(ctx->operate()->EqualOrAssign() != 0x0) conditions[cnt].op = EQ_OP;
        if(ctx->operate()->Less() != 0x0) conditions[cnt].op = LT_OP;
        if(ctx->operate()->LessEqual() != 0x0) conditions[cnt].op = LE_OP;
        if(ctx->operate()->Greater() != 0x0) conditions[cnt].op = GT_OP;
        if(ctx->operate()->GreaterEqual() != 0x0) conditions[cnt].op = GE_OP;
        if(ctx->operate()->NotEqual() != 0x0) conditions[cnt].op = NE_OP;

        if(ctx->expression()->column() != 0x0){
            conditions[cnt].rhsAttr.relName = (char *) malloc(ctx->expression()->column()->Identifier(0)->getText().length());
            strcpy(conditions[cnt].rhsAttr.relName, ctx->expression()->column()->Identifier(0)->getText().c_str()); //<< endl;
            conditions[cnt].rhsAttr.attrName = (char *) malloc(ctx->expression()->column()->Identifier(1)->getText().length());
            strcpy(conditions[cnt].rhsAttr.attrName, ctx->expression()->column()->Identifier(1)->getText().c_str()); //<< endl;
            conditions[cnt].bRhsIsAttr = true;
        }
        else if(ctx->expression()->value() != 0x0){
            AnalyseValue(conditions[cnt].rhsValue, ctx->expression()->value());
            conditions[cnt].bRhsIsAttr = false;

        }

        cnt++;
        return visitChildren(ctx);
    }
    
     
    virtual antlrcpp::Any visitAlter_add_index(SQLParser::Alter_add_indexContext *ctx) override {
        auto attrs = (ctx->identifiers()->Identifier());
        std::string rel = ctx->Identifier()->getSymbol()->getText();
        RC rc = sm->CreateIndex(rel.c_str(), (attrs[0]->getText()).c_str());
        if(rc != OK_RC)SM_PrintError(rc, rel);
        else {
            cout << "Index added\n";
        }
        return visitChildren(ctx);
    }
    virtual antlrcpp::Any visitAlter_drop_index(SQLParser::Alter_drop_indexContext *ctx) override {
        auto attrs = (ctx->identifiers()->Identifier());
        std::string rel = ctx->Identifier()->getSymbol()->getText();
        RC rc = sm->DropIndex(rel.c_str(), (attrs[0]->getText()).c_str());
        if(rc != OK_RC)SM_PrintError(rc, rel);
        else cout << "Index dropped\n";
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitShow_indexes(SQLParser::Show_indexesContext *ctx) override {
        RC rc = sm->ShowIndexes();
        if(rc != OK_RC)SM_PrintError(rc, NULL);
        return visitChildren(ctx);
    }

};