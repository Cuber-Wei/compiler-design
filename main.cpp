#include <iostream>
#include <fstream>

#include "LexAnalyzer.h"
#include "SymbolTable.h"
#include "SyntaxAnalyzer.h"
#include "utils.h"
#include "variables.h"

int main()
{
    Utils::init();
    std::streambuf* coutbuf = nullptr;
    std::string fileName;
    std::cout << "输入待编译的文件路径：" << std::endl;
    std::cin >> fileName;
    // 读取文件内容
    programStr = Utils::readFile(fileName);
    // 选择信息输出方式
    // std::cout << "选择错误输出方式：\n1.文件\t2.控制台" << std::endl;
    // int choice = 2;
    // std::cin >> choice;
    // if (choice == 1)
    // {
    //     std::ofstream fout("./output/result.txt");
    //     coutbuf = std::cout.rdbuf(fout.rdbuf());
    // }
    // 词法分析
    const LexAnalyzer lexAnalyzer;
    // 语法分析
    SyntaxAnalyzer syntaxAnalyzer(lexAnalyzer);
    syntaxAnalyzer.analyze();
    // if (choice == 2)
    //     std::cout.rdbuf(coutbuf);
    if (errCnt == 0)
        symTableTest();
    return 0;
}
