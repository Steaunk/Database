#include "ps/MyVisitor.h"
#include "ps/SQLLexer.h"
#include <string>
#include "antlr4-runtime.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>

namespace fs = std::filesystem;

using namespace antlr4;

// 返回类型根据你的visitor决定

PF_Manager pfm;
RM_Manager rmm(pfm);
IX_Manager ixm(pfm);
SM_Manager smm(ixm, rmm);
QL_Manager qlm(smm, ixm, rmm);

auto parse(std::string sSQL) {
 // 解析SQL语句sSQL的过程
 // 转化为输入流
 ANTLRInputStream sInputStream(sSQL);
 // 设置Lexer
 SQLLexer iLexer(&sInputStream);
 CommonTokenStream sTokenStream(&iLexer);
 // 设置Parser
 SQLParser iParser(&sTokenStream);
 auto iTree = iParser.program();
 // 构造你的visitor
 MyVisitor iVisitor(smm, qlm);
 // visitor模式下执行SQL解析过程
 // --如果采用解释器方式可以在解析过程中完成执行过程（相对简单，但是很难进行进一步优化，功能上已经达到实验要求）
 // --如果采用编译器方式则需要生成自行设计的物理执行执行计划（相对复杂，易于进行进一步优化，希望有能力的同学自行调研尝试）
 auto iRes = iVisitor.visit(iTree);
 return iRes;
}
int main(){
	//std::fstream fin("test.in");
	std::string s;
	if(!fs::exists("dbs")) fs::create_directories("dbs");
	chdir("dbs");
	while(true){
		std::cout << ">>> ";
		getline(std::cin,s);
		parse(s);
	}
}