//
// Created by l0v3ch4n on 24-11-18.
//

#include "LexAnalyzer.h"

#include <cstring>

#include "utils.h"
#include "variables.h"

LexAnalyzer::LexAnalyzer() {
    //符号名表初始化
    symMap[NUL] = "NU";
    symMap[IDENT] = "IDENT";
    symMap[NUMBER] = "NUMBER";
    symMap[PLUS] = "PLUS";
    symMap[MINUS] = "MINUS";
    symMap[MULTI] = "MULTI";
    symMap[DIVIS] = "DIVS";
    symMap[ODD_SYM] = "ODD_SYM";
    symMap[EQL] = "EQ";
    symMap[NEQ] = "NEQ";
    symMap[LSS] = "LSS";
    symMap[LEQ] = "LEQ";
    symMap[GRT] = "GRT";
    symMap[GEQ] = "GEQ";
    symMap[LPAREN] = "LPAREN";
    symMap[RPAREN] = "RPAREN";
    symMap[COMMA] = "COMMA";
    symMap[SEMICOLON] = "SEMICOLON";
    symMap[ASSIGN] = "BECOMES";
    symMap[BEGIN_SYM] = "BEGIN_SYM";
    symMap[END_SYM] = "END_SYM";
    symMap[IF_SYM] = "IF_SYM";
    symMap[THEN_SYM] = "THEN_SYM";
    symMap[WHILE_SYM] = "WHILE_SYM";
    symMap[DO_SYM] = "DO_SYM";
    symMap[CALL_SYM] = "CALL_SYM";
    symMap[CONST_SYM] = "CONST_SYM";
    symMap[VAR_SYM] = "VAR_SYM";
    symMap[PROC_SYM] = "PROC_SYM";
    symMap[WRITE_SYM] = "WRITE_SYM";
    symMap[READ_SYM] = "READ_SYM";
    symMap[PROGM_SYM] = "PROGRAM_SYM";
    symMap[ELSE_SYM] = "ELSE_SYM";

    programStrToHandle = programStr;
}

void LexAnalyzer::getCh() {
    //获取下一个字符
    ch = programStrToHandle[currNum];
    currNum++;
}

void LexAnalyzer::retract() {
    //回退一个字符
    ch = 0;
    currNum--;
}

bool LexAnalyzer::isLetter(const char ch) {
    //判断是否是字母
    return (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z');
}

bool LexAnalyzer::isDigit(const char ch) {
    //判断是否是数字
    return (ch >= '0' && ch <= '9');
}

bool LexAnalyzer::isTerminate(const char ch) {
    //是否终止
    return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '#' || ch == '\0' || ch == ';' || ch == ',');
}

int LexAnalyzer::getOperator(const char ch) const {
    for (int i = 0; i < OPR_MAX; ++i) {
        if (oprTable[i] == ch) {
            return i;
        }
    }
    return -1;
}

//跳过空白字符
void LexAnalyzer::skipBlank() {
    while (programStrToHandle[currNum] && (programStrToHandle[currNum] == ' ' || programStrToHandle[currNum] == '\t')) {
        col++;
        currNum++;
    }
}

void LexAnalyzer::contract() {
    strToken += ch;
}

//获取保留字编号
int LexAnalyzer::reserve(const std::string &str) const {
    for (int i = 0; i < RSV_WORD_MAX; ++i) {
        if (rsvTable[i] == str) {
            return i;
        }
    }
    return -1;
};

void LexAnalyzer::getToken() {
    if (ch != '\n') {
        //换行，本行读完
        pre_word_col = col;
        pre_word_row = line;
    }
    strToken.clear(); //清空字符串
    skipBlank();
    getCh();
    if (ch == '\0')
        return;
    // 跳过连续的回车符
    if (ch == '\n') {
        col = 0;
        line++;
        getToken();
        return;
    }
    if (ch == '#') {
        contract();
        sym = NUL;
    }
    // 纯字母
    else if (isLetter(ch)) {
        while (isLetter(ch) || isDigit(ch)) {
            contract();
            getCh();
        }
        // 查表，判断是否为保留字
        switch (reserve(strToken)) {
            case -1:
                sym = IDENT;
                break;
            case 0:
                sym = ODD_SYM;
                break;
            case 1:
                sym = BEGIN_SYM;
                break;
            case 2:
                sym = END_SYM;
                break;
            case 3:
                sym = IF_SYM;
                break;
            case 4:
                sym = THEN_SYM;
                break;
            case 5:
                sym = WHILE_SYM;
                break;
            case 6:
                sym = DO_SYM;
                break;
            case 7:
                sym = CALL_SYM;
                break;
            case 8:
                sym = CONST_SYM;
                break;
            case 9:
                sym = VAR_SYM;
                break;
            case 10:
                sym = PROC_SYM;
                break;
            case 11:
                sym = WRITE_SYM;
                break;
            case 12:
                sym = READ_SYM;
                break;
            case 13:
                sym = PROGM_SYM;
                break;
            case 14:
                sym = ELSE_SYM;
                break;
            default:
                sym = NUL;
                break;
        }
        retract();
    }
    // 开头为数字，判断是否为数值类型
    else if (isDigit(ch)) {
        while (isDigit(ch)) {
            contract();
            getCh();
        }
        // 遇到字母
        if (isLetter(ch)) {
            Utils::log(ERROR, line, "不合法的名称：'" + strToken + "'");
            // 跳过错误至下一个终止符
            while (!isTerminate(ch))
                getCh();
            retract();
            strToken.clear();
            sym = NUL;
        }
        // 遇到其他字符
        else {
            sym = NUMBER;
            retract();
        }
    } // 遇到:判断是否为赋值符号
    else if (ch == ':') {
        contract();
        getCh();
        if (ch == '=') {
            contract();
            pre_word_col++;
            sym = ASSIGN;
        } else {
            Utils::log(ERROR, line, "缺少：'='");
            strToken.clear();
            sym = NUL;
        }
    } else if (ch == '<') {
        contract();
        getCh();
        if (ch == '=') {
            contract();
            pre_word_col++;
            sym = LEQ;
        } else if (ch == '>') {
            contract();
            pre_word_col++;
            sym = NEQ;
        } else {
            sym = LSS;
            retract();
        }
    } else if (ch == '>') {
        contract();
        getCh();
        if (ch == '=') {
            contract();
            pre_word_col++;
            sym = GEQ;
        } else {
            sym = GRT;
            retract();
        }
    } else {
        if (const int code = getOperator(ch); code != -1) {
            contract();
            switch (code) {
                case 0:
                    sym = PLUS;
                    break;
                case 1:
                    sym = MINUS;
                    break;
                case 2:
                    sym = MULTI;
                    break;
                case 3:
                    sym = DIVIS;
                    break;
                case 4:
                    sym = EQL;
                    break;
                // 这里注释掉是因为前面判断是不是复合符号的时候已经判断过了，
                // 能进入到这段逻辑肯定不是'<'和'>'了
                // case 5:
                //     sym = LSS;
                //     break;
                // case 6:
                //     sym = GRT;
                //     break;
                case 7:
                    sym = LPAREN;
                    break;
                case 8:
                    sym = RPAREN;
                    break;
                case 9:
                    sym = COMMA;
                    break;
                case 10:
                    sym = SEMICOLON;
                    break;
                default:
                    break;
            }
        } else {
            contract();
            Utils::log(ERROR, line, "不合法的名称：'" + strToken + "'");
            sym = NUL;
        }
    }
    // Utils::log(INFO, 0, "当前TOKEN：'" + strToken + "'");
    tokens[++tokenNum] = strToken;
}
