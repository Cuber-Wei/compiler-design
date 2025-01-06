//
// Created by l0v3ch4n on 25-1-4.
//
#include <fstream>
#include <iostream>
#include "Utils.h"

#include "PCode.h"
#include "SymTable.h"
#include "variables.h"

void Utils::init()
{
    code_str.clear(); //程序字符串
    lex_err_cnt = 0; //错误数量
    lex_err_cnt = 0; //错误数量
    syntax_err_cnt = 0; //错误数量
    sym_err_cnt = 0; //错误数量
    level = 0; // 层差
    glo_offset = 0; // 全局offset
    success("初始化完成！");
}

void Utils::readFile(const std::string& path)
{
    const std::string file_name = path.substr(path.find_last_of('/') + 1,
                                              path.find_last_of('.') - path.find_last_of('/') - 1);
    output_path = "/home/l0v3ch4n/Desktop/program-repo/compiler/output/" + file_name + ".pcode";
    info("文件名：" + file_name);
    info("输出路径：" + output_path);
    //读取文件
    std::ifstream code(path);
    if (!code.is_open())
    {
        error("文件打开失败!");
        exit(0);
    }
    std::string content((std::istreambuf_iterator<char>(code)), std::istreambuf_iterator<char>());
    if (content.empty())
    {
        error("文件为空文件！");
        exit(0);
    }
    code.close();
    int line_num = 0; // 行数
    std::string line;
    for (int i = 0; i < content.length(); i++)
    {
        //以'\n'分行
        if (content[i] == '\n')
        {
            code_line[++line_num] = line;
            line = "";
            continue;
        }
        line += content[i];
    }
    line_num++;
    success("源文件读取成功！");
    info("代码行数：" + std::to_string(line_num));
    code_str = content;
}

void Utils::printResult()
{
    if (lex_err_cnt == 0 && syntax_err_cnt == 0 && sym_err_cnt == 0)
    {
        success("编译成功！");
        // 展示符号表
        showSymTable();
        PCode::printCode(to_file);
    }
    else
    {
        error("编译失败！");
        info("总错误数量：" + std::to_string(lex_err_cnt + syntax_err_cnt + sym_err_cnt));
        info("词法分析错误数量：" + std::to_string(lex_err_cnt));
        info("语法分析错误数量：" + std::to_string(syntax_err_cnt));
        info("语义分析错误数量：" + std::to_string(sym_err_cnt));
    }
}

void Utils::info(const std::string& message)
{
    std::cout << "\033[1;34m[INFO]\t\t" << message << "\033[0m" << std::endl;
}

void Utils::info_with_no_endl(const std::string& message)
{
    std::cout << "\033[1;34m[INFO]\t\t\033[0m" << message;
}

void Utils::info(const std::string& message, const unsigned int line_num, const unsigned int col_num)
{
    std::cout << "\033[1;34m[INFO]\t\t\033[0m" << message << " at line " << line_num << ", col " << col_num <<
        std::endl;
}

void Utils::error(const std::string& message)
{
    std::cout << "\033[1;31m[ERROR]\t\t" << message << "\033[0m" << std::endl;
}

void Utils::error(const std::string& message, const unsigned int line_num, const unsigned int col_num)
{
    std::cout << "\033[1;31m[ERROR]\t\t" << "At line " << line_num << ", col " << col_num << "\033[0m" <<
        std::endl;
    std::cout << "\033[1;31m[ERROR]\t\t\033[0m" << "\033[1;34m" << code_line[line_num] << "\033[0m" << std::endl;
    std::cout << "\033[1;31m[ERROR]\t\t";
    for (int i = 1; i < col_num - 2; i++)
        std::cout << " ";
    std::cout << "^\033[0m" << std::endl;
    std::cout << "\033[1;31m[ERROR]\t\t" << message << "\033[0m" << std::endl;
}

void Utils::success(const std::string& message)
{
    std::cout << "\033[1;32m[SUCCESS]\t" << message << "\033[0m" << std::endl;
}

void Utils::success(const std::string& message, const unsigned int line_num, const unsigned int col_num)
{
    std::cout << "\033[1;32m[SUCCESS]\t\033[0m" << message << " at line " << line_num << ", col " << col_num <<
        std::endl;
}

void Utils::warning(const std::string& message)
{
    std::cout << "\033[1;33m[WARNING]\t" << message << "\033[0m" << std::endl;
}

void Utils::warning(const std::string& message, const unsigned int line_num, const unsigned int col_num)
{
    std::cout << "\033[1;33m[SUCCESS]\t\033[0m" << message << " at line " << line_num << ", col " << col_num <<
        std::endl;
}

void Utils::output(const int number)
{
    std::cout << "\033[1;35m[OUTPUT]\t" << std::to_string(number) << "\033[0m";
}

void Utils::input_prompt()
{
    std::cout << "\033[1;35m[INPUT]\t>>>\t\033[0m";
}
