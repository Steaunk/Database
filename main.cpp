#include "ps/Myvistor.h"
#include <string>
#include "antlr4-runtime.h"

using namespace antlr4;

// 返回类型根据你的visitor决定
auto parse(std::String sSQL) {
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
 YourVisitor iVisitor{/*YourVisitor的构造函数*/};
 // visitor模式下执行SQL解析过程
 // --如果采用解释器方式可以在解析过程中完成执行过程（相对简单，但是很难进行进一步优化，功能上已经达到实验要求）
 // --如果采用编译器方式则需要生成自行设计的物理执行执行计划（相对复杂，易于进行进一步优化，希望有能力的同学自行调研尝试）
 auto iRes = iVisitor.visit(iTree);
 return iRes;
}
