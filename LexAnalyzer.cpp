//
// Created by l0v3ch4n on 24-12-12.
//

#include "LexAnalyzer.h"

#include <iostream>
#include <variant>
#include <oneapi/tbb/info.h>

#include "Utils.h"
#include "variables.h"

bool LexAnalyzer::isLetter(const char ch)
{
    // 判断是否是字母
    return (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z');
}

bool LexAnalyzer::isDigit(const char ch)
{
    // 判断是否是数字
    return (ch >= '0' && ch <= '9');
}

bool LexAnalyzer::isTerminate(const char ch)
{
    // 判断是否终止
    return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '#' || ch == '\0');
}

int LexAnalyzer::getOperator(const std::string& opCode) const
{
    // 获取操作符编号
    for (int i = 0; i < OPR_MAX; ++i)
    {
        if (opr_token[i] == opCode)
        {
            return i;
        }
    }
    return -1;
}

int LexAnalyzer::getReserve(const std::string& str) const
{
    // 获取保留字编号
    for (int i = 0; i < RSV_WORD_MAX; ++i)
    {
        if (rsv_word[i] == str)
        {
            return i;
        }
    }
    return -1;
}

void LexAnalyzer::skipBlank()
{
    // 连续跳过空白字符
    char tmp = programStrToHandle[curr_no];
    while (tmp && tmp == ' ' || tmp == '\t' || tmp == '\n')
    {
        switch (tmp)
        {
        case '\t':
            curr_col += TAB_WIDTH;
            break;
        case '\n':
            pre_col = curr_col;
            pre_row = curr_row;
            curr_col = 0;
            curr_row++;
            break;
        case ' ':
        default:
            curr_col++;
            break;
        }
        tmp = programStrToHandle[++curr_no];
    }
}

void LexAnalyzer::getCh()
{
    // 获取下一个字符
    // Utils::info("getCh: " + std::to_string(curr_no) + " " + std::to_string(curr_col) + " " + std::to_string(curr_row));
    ch = programStrToHandle[curr_no];
    curr_no++;
    curr_col++;
}

void LexAnalyzer::retract()
{
    // 回调一个字符，同时将ch置0
    ch = 0;
    curr_no--;
}

void LexAnalyzer::contract()
{
    // 将当前字符追加到token中
    strToken += ch;
}

bool LexAnalyzer::getToken()
{
    bool is_error = false;
    // 分析词汇，判断是否合法
    // 每次只读取并分析一个Token
    skipBlank();
    // 初始化字符串
    strToken.clear();
    sym[0].clear();
    sym[1].clear();
    sym[2].clear();
    getCh();
    while (!isTerminate(ch)) // 直到终止
    {
        // 纯字母，一直读取
        if (isLetter(ch))
        {
            while (isLetter(ch) || isDigit(ch))
            {
                contract();
                getCh();
            }
            retract(); // 回退一个字符
            // 查表，判断类型
            const int res = getReserve(strToken);
            if (res == -1)
            {
                sym[0] = token_type_to_string(token_type::IDENTIFY);
                sym[2] = strToken;
            }
            else
            {
                sym[0] = token_type_to_string(token_type::RSV_WORD);
                sym[2] = strToken;
                switch (res)
                {
                case 0:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::ODD);
                    break;
                case 1:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::BEGIN);
                    break;
                case 2:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::END);
                    break;
                case 3:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::IF);
                    break;
                case 4:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::THEN);
                    break;
                case 5:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::ELSE);
                    break;
                case 6:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::WHILE);
                    break;
                case 7:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::DO);
                    break;
                case 8:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::CALL);
                    break;
                case 9:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::CONST);
                    break;
                case 10:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::VAR);
                    break;
                case 11:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::PROCEDURE);
                    break;
                case 12:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::WRITE);
                    break;
                case 13:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::READ);
                    break;
                case 14:
                    sym[1] = rsv_word_type_to_string(rsv_word_type::PROGRAM);
                    break;
                default:
                    sym[1] = token_type_to_string(token_type::ERROR);
                    Utils::error("Invalid Token '" + strToken + "'", curr_row, curr_col);
                    is_error = true;
                }
            }
        }
        else if (isDigit(ch))
        {
            while (isDigit(ch))
            {
                contract();
                getCh();
            }
            if (isLetter(ch)) // 数字后跟字母，非法
            {
                while (isLetter(ch) || isDigit(ch))
                {
                    contract();
                    getCh();
                }
                sym[0] = token_type_to_string(token_type::ERROR);
                Utils::error("Invalid Token '" + strToken + "'", curr_row, curr_col);
                is_error = true;
            }
            retract();
            sym[0] = token_type_to_string(token_type::NUMBER);
            sym[1] = rsv_word_type_to_string(rsv_word_type::CONST);
            sym[2] = strToken;
        }
        else // 运算符和界符
        {
            contract();
            if (ch == ':' || ch == '>')
            {
                getCh();
                if (ch == '=')
                {
                    contract();
                    sym[0] = token_type_to_string(token_type::OPCODE);
                    sym[2] = strToken;
                    if (strToken == ":=")
                        sym[1] = opr_type_to_string(opr_type::ASSIGN);
                    else
                        sym[1] = opr_type_to_string(opr_type::ASSIGN);
                }
                else
                {
                    retract(); // 回退
                    if (strToken == ">")
                    {
                        sym[0] = token_type_to_string(token_type::OPCODE);
                        sym[1] = opr_type_to_string(opr_type::GT);
                        sym[2] = strToken;
                    }
                    else
                    {
                        sym[0] = token_type_to_string(token_type::ERROR);
                        Utils::error("Invalid Token '" + strToken + "'", curr_row, curr_col);
                        is_error = true;
                    }
                }
                break;
            }
            if (ch == '<')
            {
                getCh();
                if (ch == '=' || ch == '>')
                {
                    contract();
                    sym[0] = token_type_to_string(token_type::OPCODE);
                    sym[2] = strToken;
                    if (strToken == "<=")
                        sym[1] = opr_type_to_string(opr_type::LEQ);
                    else
                        sym[1] = opr_type_to_string(opr_type::NEQ);
                }
                else
                {
                    retract(); // 回退
                    sym[0] = token_type_to_string(token_type::OPCODE);
                    sym[1] = opr_type_to_string(opr_type::LT);
                    sym[2] = strToken;
                }
                break;
            }
            // 常规单字符opr
            const int res = getOperator(strToken);
            // Utils::info(strToken + std::to_string(res));
            if (res == -1)
                sym[0] = token_type_to_string(token_type::ERROR);
            else
            {
                sym[0] = token_type_to_string(token_type::OPCODE);
                sym[2] = strToken;
                switch (res)
                {
                case 0:
                    sym[1] = opr_type_to_string(opr_type::ADD);
                    break;
                case 1:
                    sym[1] = opr_type_to_string(opr_type::SUB);
                    break;
                case 2:
                    sym[1] = opr_type_to_string(opr_type::MUL);
                    break;
                case 3:
                    sym[1] = opr_type_to_string(opr_type::DIV);
                    break;
                case 4:
                    sym[1] = opr_type_to_string(opr_type::EQ);
                    break;
                case 11:
                    sym[1] = opr_type_to_string(opr_type::LEFT_BRACKET);
                    break;
                case 12:
                    sym[1] = opr_type_to_string(opr_type::RIGHT_BRACKET);
                    break;
                case 13:
                    sym[1] = opr_type_to_string(opr_type::COMMA);
                    break;
                case 14:
                    sym[1] = opr_type_to_string(opr_type::SEMICOLON);
                    break;
                default:
                    sym[1] = token_type_to_string(token_type::ERROR);
                    Utils::error("Invalid Token '" + strToken + "'", curr_row, curr_col);
                    is_error = true;
                }
            }
            break;
        }
    }
    if (is_error)
    {
        lex_err_cnt++;
        sym[0] = token_type_to_string(token_type::IDENTIFY);
        sym[1] = token_type_to_string(token_type::ERROR);
        sym[2] = strToken;
    }
    Utils::info("'" + strToken + "'", curr_row, curr_col);
    Utils::info("\t大类: " + sym[0]);
    Utils::info("\t小类: " + sym[1]);
    Utils::info("\t属性值: " + sym[2]);
    return !is_error;
}

bool LexAnalyzer::isEOF()
{
    return curr_no == programStrToHandle.length();
}
