#include <iostream>

#include "LexAnalyzer.h"
#include "SymbolTable.h"
#include "SyntaxAnalyzer.h"
#include "utils.h"
#include "variables.h"

int main() {
    Utils::init();
    std::string fileName;
    std::cout << "输入待编译的文件路径：" << std::endl;
    std::cin >> fileName;
    // 读取文件内容
    programStr = Utils::readFile(fileName);
    // 词法分析
    const LexAnalyzer lexAnalyzer;
    // 语法分析
    SyntaxAnalyzer syntaxAnalyzer(lexAnalyzer);
    syntaxAnalyzer.analyze();
    if (errCnt == 0)
        symTableTest();
    return 0;
}
