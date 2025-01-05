//
// Created by l0v3ch4n on 25-1-4.
//

#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H
#include "LexAnalyzer.h"
#include "PCode.h"
#include "SymTable.h"


class SyntaxAnalyzer
{
public:
    LexAnalyzer lexer;
    SymTable sym_table;
    PCode pcoder;
    bool is_valid_token = false; // 默认token不合法
    int exit_status = SYN_SUCCESS; // 默认退出状态为0(成功)

    explicit SyntaxAnalyzer(LexAnalyzer lexAnalyzer, SymTable sym_table, PCode pcoder) : lexer(std::move(lexAnalyzer)),
        sym_table(std::move(sym_table)), pcoder(std::move(pcoder))
    {
    }

    void analyze();

    void recover(unsigned long s1, unsigned long s2, const std::string& errorMessage);

    void prog();

    void exp();

    void factor();

    void term();

    void lexp();

    void statement();

    void block();

    void constDef();

    void condecl();

    void vardecl();

    void proc();

    void body();
};


#endif //SYNTAXANALYZER_H
