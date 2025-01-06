#include <iostream>
#include <fstream>

#include "Interpreter.h"
#include "LexAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "Utils.h"
#include "variables.h"

int main()
{
    Utils::init();
    char choice1, choice2;
    Utils::warning("选择功能以使用：1.编译\t2.解释运行\t3.编译并解释运行\t4.退出");
    Utils::input_prompt();
    std::cin >> choice1;
    if (choice1 == '1' || choice1 == '3')
    {
        std::string code_file;
        Utils::warning("请输入要编译的代码文件路径：");
        Utils::input_prompt();
        std::cin >> code_file;
        Utils::warning("是否打印代码到控制台？[y/n]");
        Utils::input_prompt();
        std::cin >> choice2;
        if (choice2 == 'Y' || choice2 == 'y')
            to_file = false;
        else
            to_file = true;
        Utils::readFile(code_file);
        // 建立词法分析器
        const LexAnalyzer lexer(code_str);
        // 建立语法分析器
        SyntaxAnalyzer syntaxer(lexer);
        syntaxer.analyze();
        // 展示总结
        Utils::printResult();
        if (choice1 == '3')
        {
            // 解释运行
            const Interpreter inter(output_path);
            inter.runCode();
        }
    }
    else if (choice1 == '2')
    {
        Utils::warning("请输入要解释运行的P代码文件路径：");
        Utils::input_prompt();
        std::cin >> output_path;
        // 解释运行
        const Interpreter inter(output_path);
        inter.runCode();
    }
    else
        exit(0);

    return 0;
}
