//
// Created by l0v3ch4n on 24-11-18.
//

#ifndef LEXANALYZER_H
#define LEXANALYZER_H
#include <string>
#include <unordered_map>

#include "constants.h"

class LexAnalyzer {
public:
    //保留字表
    std::string rsvTable[RSV_WORD_MAX] = {
        "odd", "begin", "end", "if", "then", "while", "do", "call",
        "const", "var", "procedure", "write", "read", "program", "else"
    };
    // 运算符号表
    char oprTable[OPR_MAX] = {
        '+', '-', '*', '/', '=', '<',
        '>', '(', ')', ',', ';'
    };
    char ch = 0; //最近一次读出的字符
    unsigned long sym = NUL; // 最近一次识别出来的 token 的类型
    std::string strToken; //当前拼成的token
    size_t currNum = 0; //当前字符编号
    size_t tokenNum = 0; //当前token编号
    size_t line = 1; //当前行数
    size_t col = 1; //当前列数
    size_t pre_word_col = 0; // 上一个非空白合法词尾列指针
    size_t pre_word_row = 0; // 上一个非空白合法词行指针

    std::string programStrToHandle; //程序字符串

    LexAnalyzer();

    static bool isLetter(char ch); //判断是否是字母
    static bool isDigit(char ch); //判断是否是数字
    static bool isTerminate(char ch); //判断是否终止

    [[nodiscard]] int getOperator(char ch) const; //获取操作符编号

    //跳过空白字符
    void skipBlank();

    void getCh(); //获取下一个字符

    void retract(); //回调一个字符，同时将ch置0

    void contract(); //将当前字符追加到token中

    [[nodiscard]] int reserve(const std::string &str) const; //获取保留字编号

    void getToken(); //分析词汇，判断是否合法
};

#endif //LEXANALYZER_H
