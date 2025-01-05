#include <iostream>
#include <fstream>

#include "Interpreter.h"
#include "LexAnalyzer.h"
#include "PCode.h"
#include "SymTable.h"
#include "SyntaxAnalyzer.h"
#include "Utils.h"
#include "variables.h"

int main()
{
    Utils::init();
    std::string code_file;
    std::cout << "请输入要编译的代码文件路径：" << std::endl;
    std::cin >> code_file;
    Utils::readFile(code_file);
    // 建立词法分析器
    const LexAnalyzer lexer(code_str);
    // 建立符号表
    SymTable sym_table;
    // 建立中间代码成成器
    PCode pcoder(output_path);
    // 建立语法分析器
    SyntaxAnalyzer syntaxer(lexer, sym_table, pcoder);
    syntaxer.analyze();
    // 展示符号表
    sym_table.showSymTable();
    // 展示总结
    Utils::printResult();

    // 解释运行
    Interpreter inter(output_path);
    inter.runCode();

    return 0;
}
