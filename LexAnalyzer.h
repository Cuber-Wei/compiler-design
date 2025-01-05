//
// Created by l0v3ch4n on 24-12-12.
//

#ifndef LEXANALYZER_H
#define LEXANALYZER_H
#include <string>
#include <utility>

#include "constants.h"


class LexAnalyzer
{
public:
    // 保留字表
    std::string rsv_word[RSV_WORD_MAX] = {
        "odd", "begin", "end", "if", "then", "else", "while", "do",
        "call", "const", "var", "procedure", "write", "read", "program"
    };
    // 运算符号表
    std::string opr_token[OPR_MAX] = {
        "+", "-", "*", "/", "=", "<>", "<", ">",
        "<=", ">=", ":=", "(", ")", ",", ";"
    };
    char ch; // 字符变量，存放最新读进的源程序字符
    std::string sym[3]; // 最近一次识别出来的 token 的类型, [大类, 小类, 属性值]
    std::string programStrToHandle; //程序字符串
    std::string strToken; // 字符串，存放构成单词符号的字符串
    unsigned int curr_no, token_no; //当前字符编号，当前token编号
    unsigned int curr_col, curr_row, pre_col, pre_row; // 当前行列位置和前一个非空白合法Token的行列位置

    explicit LexAnalyzer(std::string programStr)
    {
        // 初始化
        curr_no = 0;
        token_no = 0;
        curr_col = 0;
        curr_row = 1;
        pre_col = 0;
        pre_row = 1;
        ch = ' ';
        sym[0] = token_type_to_string(token_type::NUL); // 大类, token_type
        sym[1] = token_type_to_string(token_type::NUL); // 小类, identifier_type / opr_type / rsv_word_type
        sym[2] = token_type_to_string(token_type::NUL); // 属性值, name / number
        programStrToHandle = std::move(programStr);
    }

    static bool isLetter(char ch); // 判断是否是字母
    static bool isDigit(char ch); // 判断是否是数字
    static bool isTerminate(char ch); // 判断是否终止

    [[nodiscard]] int getOperator(const std::string& opCode) const; // 获取操作符编号
    [[nodiscard]] int getReserve(const std::string& str) const; // 获取保留字编号

    void skipBlank(); // 连续跳过空白字符
    void getCh(); // 获取下一个字符
    void retract(); // 回调一个字符，同时将ch置0
    void contract(); // 将当前字符追加到token中
    bool getToken(); // 分析词汇，判断是否合法

    bool isEOF(); // 判断是否到达文件末尾
};


#endif //LEXANALYZER_H
