//
// Created by l0v3ch4n on 24-11-18.
//

#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H
#include <utility>

#include "LexAnalyzer.h"

class SyntaxAnalyzer {
public:
    LexAnalyzer lexAnalyzer;

    explicit SyntaxAnalyzer(LexAnalyzer lexAnalyzer) : lexAnalyzer(std::move(lexAnalyzer)) {
    }

    void analyze();

    void recover(unsigned long s1, unsigned long s2, const std::string &errorMessage);

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
