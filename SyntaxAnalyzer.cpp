//
// Created by l0v3ch4n on 24-11-18.
//

#include "SyntaxAnalyzer.h"

#include "LexAnalyzer.h"
#include "PartialCode.h"
#include "SymbolTable.h"
#include "utils.h"
#include "variables.h"

// 用于错误恢复的函数，若当前符号在s1中，则读取下一符号；若当前符号不在s1中，则报错，接着循环查找下一个在中s1 U s2的符号
void SyntaxAnalyzer::recover(const unsigned long s1, const unsigned long s2,
                             const std::string& errorMessage)
{
    if (!(lexAnalyzer.sym & s1)) // 当前符号不在s1中
    {
        Utils::log(ERROR, lexAnalyzer.pre_word_row, errorMessage);
        if (errorMessage == "Missing token 'program'.")
        {
            return;
        }
        const unsigned long s3 = s1 | s2; // 把s2补充进s1

        while (!(lexAnalyzer.sym & s3)) // 循环找到下一个合法的符号
        {
            if (lexAnalyzer.ch == '\0')
                Utils::complete(0);
            lexAnalyzer.getToken(); // 继续词法分析
        }
        if (lexAnalyzer.sym & s1)
            lexAnalyzer.getToken();
    }
    else
        lexAnalyzer.getToken();
}

// 开始语法分析
// <prog> -> program id; <block>
void SyntaxAnalyzer::prog()
{
    // 找到第一次出现的program
    recover(PROGM_SYM, 0, "Missing token 'program'.");
    // <prog> -> program id
    if (lexAnalyzer.sym == IDENT)
    {
        lexAnalyzer.getToken();
    }
    // 缺失 <prog> -> program ;
    else if (lexAnalyzer.sym == SEMICOLON)
    {
        Utils::log(ERROR, lexAnalyzer.line, "Missing program identifier.");
    }
    //  <prog> -> program {~id} ;
    else
    {
        Utils::log(ERROR, lexAnalyzer.line, "Find '" + lexAnalyzer.strToken + "' in identifier.");
        lexAnalyzer.getToken();
    }
    // <prog> -> program id;
    if (lexAnalyzer.sym == SEMICOLON)
    {
        lexAnalyzer.getToken();
    }
    // <prog> -> program id {~';'} <block>
    else
    {
        // 判断是否仅仅是缺失分号
        recover(SEMICOLON, first_block, "Missing token ';'.");
    }
    //<prog> -> program id; <block>
    block();
    //<prog> -> program id; <block>#
    if (lexAnalyzer.sym == NUL)
    {
    }
}

// <block> -> [<condecl>][<vardecl>][<proc>]<body>
void SyntaxAnalyzer::block()
{
    // <block> -> [<condecl>]
    if (lexAnalyzer.sym == CONST_SYM)
    {
        condecl();
    }
    // <block> -> [<condecl>][<vardecl>]
    if (lexAnalyzer.sym == VAR_SYM)
    {
        vardecl();
    }
    // <block> -> [<condecl>][<vardecl>][<proc>]]
    if (lexAnalyzer.sym == PROC_SYM)
    {
        proc();
    }
    // <block> -> [<condecl>][<vardecl>][<proc>]<body>
    body();
}

// const -> id:=number
void SyntaxAnalyzer::constDef()
{
    if (lexAnalyzer.sym == IDENT)
    {
        lexAnalyzer.getToken();
        // const -> id:=
        if (lexAnalyzer.sym & (ASSIGN | EQL))
        {
            if (lexAnalyzer.sym == EQL)
            {
                Utils::log(ERROR, lexAnalyzer.line, "Expect ':=' rather than '='.");
            }
            lexAnalyzer.getToken();
            // const -> id:=number
            if (lexAnalyzer.sym == NUMBER)
            {
                lexAnalyzer.getToken();
            }
            else
            {
                Utils::log(ERROR, lexAnalyzer.line, "Number expected.");
            }
        }
    }
    else
    {
        // 没有找到id则跳转到第一个constDef的follow集中的符号
        recover(0, follow_constdef, "Illegal defined ConstDef.");
    }
}

// <condecl> -> const <constDef>{,<constDef>};
void SyntaxAnalyzer::condecl()
{
    // const
    if (lexAnalyzer.sym == CONST_SYM)
    {
        // const <constDef>
        lexAnalyzer.getToken();
        constDef();
        // const <constDef> {,<constDef>}
        while (lexAnalyzer.sym & (COMMA | IDENT))
        {
            recover(COMMA, IDENT, "Missing ','.");
            if (lexAnalyzer.sym == IDENT) // FIRST(<constDef>)
            {
                constDef();
            }
            else
            {
                recover(0, IDENT | SEMICOLON, "ConstDef expected."); // todo expect常量定义式
            }
        }
        // const <constDef> {,<constDef>};
        if (lexAnalyzer.sym == SEMICOLON)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, follow_condecl, "Missing ';'."); // todo expect ;
        }
    }
    else
    {
        recover(0, follow_condecl, "Illegal defined condecl.");
    }
}

// <vardecl> -> var <id>{,<id>};
void SyntaxAnalyzer::vardecl()
{
    // var
    if (lexAnalyzer.sym == VAR_SYM)
    {
        lexAnalyzer.getToken();
        // var <id>
        if (lexAnalyzer.sym == IDENT)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, COMMA, "Missing identifier");
        }
        // var <id>{,<id>}
        while (lexAnalyzer.sym == COMMA)
        {
            lexAnalyzer.getToken();
            if (lexAnalyzer.sym == IDENT)
            {
                lexAnalyzer.getToken();
            }
            else
            {
                Utils::log(ERROR, lexAnalyzer.line - 1, "Redundant ','."); // todo expect 标识符
            }
        }
        // var <id>{,<id>};
        if (lexAnalyzer.sym == SEMICOLON)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, follow_vardecl, "Missing ';'.");
        }
    }
    else
    {
        recover(0, follow_vardecl, "Illegal defined vardecl.");
    }
}

// <proc> -> procedure id ([id {,id}]);<block> {;<proc>}
void SyntaxAnalyzer::proc()
{
    if (lexAnalyzer.sym == PROC_SYM)
    {
        lexAnalyzer.getToken();
        // <proc> -> procedure id
        if (lexAnalyzer.sym == IDENT)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, LPAREN, "Find '" + lexAnalyzer.strToken + "' in identifier");
        }
        // <proc> -> procedure id (
        if (lexAnalyzer.sym == LPAREN)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, IDENT | RPAREN, "Missing '('.");
        }
        // <proc> -> procedure id ([id {,id}]
        // 分析至形参列表
        if (lexAnalyzer.sym == IDENT)
        {
            lexAnalyzer.getToken();
            while (lexAnalyzer.sym == COMMA)
            {
                lexAnalyzer.getToken();
                if (lexAnalyzer.sym == IDENT)
                {
                    lexAnalyzer.getToken();
                }
                else
                {
                    Utils::log(ERROR, lexAnalyzer.line, "Redundant ','");
                }
            }
        }
        // <proc> -> procedure id ([id {,id}])
        if (lexAnalyzer.sym == RPAREN)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, SEMICOLON, "Missing ')'.");
        }
        // <proc> -> procedure id ([id {,id}]);
        if (lexAnalyzer.sym == SEMICOLON)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, first_block, "Missing ';'.");
        }
        // <proc> -> procedure id ([id {,id}]);<block> {;<proc>}
        if (lexAnalyzer.sym & first_block)
        {
            block();
            // 当前过程结束，开始分析下面的过程
            while (lexAnalyzer.sym == SEMICOLON)
            {
                lexAnalyzer.getToken();
                // FIRST(proc)
                if (lexAnalyzer.sym == PROC_SYM)
                {
                    proc();
                }
                else
                {
                    Utils::log(ERROR, lexAnalyzer.line, "Redundant ';'");
                }
            }
        }
        else
        {
            recover(0, follow_block, "Illegal defined block");
        }
    }
    else
    {
        recover(0, follow_proc, "Illegal defined procedure");
    }
}

// <exp> -> [+|-] <term>{[+|-] <term>}
void SyntaxAnalyzer::exp()
{
    // <exp> -> [+|-]
    unsigned long aop = NUL;
    if (lexAnalyzer.sym == PLUS || lexAnalyzer.sym == MINUS)
    {
        aop = lexAnalyzer.sym;
        lexAnalyzer.getToken();
    }
    // <exp> -> <term>{[+|-] <term>}
    if (lexAnalyzer.sym & first_term) // FIRST(term) 、 FIRST(factor)
    {
        term();
        while (lexAnalyzer.sym == PLUS || lexAnalyzer.sym == MINUS)
        {
            aop = lexAnalyzer.sym;
            lexAnalyzer.getToken();
            // FIRST(term)
            if (lexAnalyzer.sym & first_term)
            {
                term();
            }
            else
            {
                Utils::log(ERROR, lexAnalyzer.line, "Redundant " + lexAnalyzer.strToken);
            }
        }
    }
    else
    {
        recover(0, follow_exp, "Illegal defined expression.");
    }
}

// <factor> -> id|number|(<exp>)
void SyntaxAnalyzer::factor()
{
    if (lexAnalyzer.sym == IDENT)
    {
        lexAnalyzer.getToken();
    }
    else if (lexAnalyzer.sym == NUMBER)
    {
        // 数值，直接入栈
        lexAnalyzer.getToken();
    }
    else if (lexAnalyzer.sym == LPAREN)
    {
        lexAnalyzer.getToken();
        exp();
        if (lexAnalyzer.sym == RPAREN)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, follow_factor, "Missing ')'.");
        }
    }
    else
    {
        recover(0, follow_factor, "Illegal defined factor.");
    }
}

// <term> -> <factor>{[*|/] <factor>}
void SyntaxAnalyzer::term()
{
    if (lexAnalyzer.sym & first_term)
    {
        factor();
        // factor()执行完毕后，当前栈顶即为factor的值
        while (lexAnalyzer.sym == MULTI || lexAnalyzer.sym == DIVIS)
        {
            const unsigned long nop = lexAnalyzer.sym;
            lexAnalyzer.getToken();
            if (lexAnalyzer.sym & first_term)
            {
                factor();
            }
            else
            {
                Utils::log(ERROR, lexAnalyzer.line, "Redundant " + lexAnalyzer.strToken);
            }
        }
    }
    else
    {
        recover(0, follow_term, "Illegal defined term.");
    }
}

// <lexp> -> <exp> <lop> <exp> | odd <exp>
void SyntaxAnalyzer::lexp()
{
    // <lexp> -> odd <exp>
    if (lexAnalyzer.sym == ODD_SYM)
    {
        lexAnalyzer.getToken();
        if (lexAnalyzer.sym & first_exp)
        {
            exp();
            // odd
        }
        else
        {
            Utils::log(ERROR, lexAnalyzer.line, "Expect expression");
        }
    }
    // <lexp> -> <exp> <lop> <exp>
    else if (lexAnalyzer.sym & first_exp)
    {
        exp();
        if (lexAnalyzer.sym & first_lop)
        {
            const unsigned int lop = lexAnalyzer.sym;
            lexAnalyzer.getToken();
            exp();
        }
        else
        {
            recover(0, first_exp, "Missing '<' or '<=' or '>' or '>=' or '<>' or '='.");
            exp();
        }
    }
    else
    {
        recover(0, follow_lexp, "Illegal defined lexp.");
    }
}

void SyntaxAnalyzer::statement()
{
    // <statement> -> id := <exp>
    if (lexAnalyzer.sym == IDENT)
    {
        lexAnalyzer.getToken();
        if (lexAnalyzer.sym == ASSIGN)
        {
            lexAnalyzer.getToken();
        } // 不是赋值号：=而是等于号=
        else if (lexAnalyzer.sym == EQL)
        {
            Utils::log(ERROR, lexAnalyzer.line, "Find '=' rather than ':='");
            lexAnalyzer.getToken();
        }
        else
        {
            // 跳过非法符号，直到遇到exp的follow集
            recover(0, follow_exp, "Missing ':='.");
        }
        exp();
    }
    // <statement> -> if <lexp> then <statement> [else <statement>]
    else if (lexAnalyzer.sym == IF_SYM)
    {
        lexAnalyzer.getToken();
        lexp();

        // 当前栈顶为条件表达式的布尔值
        if (lexAnalyzer.sym == THEN_SYM)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, first_stmt, "Missing 'then'.");
        }
        // <statement> -> if <lexp> then <statement>
        statement();

        if (lexAnalyzer.sym == ELSE_SYM)
        {
            lexAnalyzer.getToken();
            statement();
        }
        else
        {
            // 没有else
        }
    }
    // <statement> -> while <lexp> do <statement>
    else if (lexAnalyzer.sym == WHILE_SYM)
    {
        lexAnalyzer.getToken();
        //FIRST(lexp);
        lexp();
        if (lexAnalyzer.sym == DO_SYM)
        {
            lexAnalyzer.getToken();
            statement();
        }
        else
        {
            recover(0, first_stmt, "Missing 'do'.");
        }
    }
    // <statement> -> call id ([{<exp>{,<exp>}])
    else if (lexAnalyzer.sym == CALL_SYM)
    {
        lexAnalyzer.getToken();
        // <statement> -> call id
        if (lexAnalyzer.sym == IDENT)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, LPAREN, "Find '" + lexAnalyzer.strToken + "' in identifier");
        }
        // <statement> -> call id (
        if (lexAnalyzer.sym == LPAREN)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, first_exp | RPAREN, "Missing '('.");
        }
        // <statement> -> call id ([{<exp>
        if (lexAnalyzer.sym & first_exp)
        {
            exp();
            size_t i = 1;
            // <statement> -> call id ([{<exp>{,<exp>}]
            while (lexAnalyzer.sym == COMMA)
            {
                lexAnalyzer.getToken();
                if (lexAnalyzer.sym & first_exp)
                {
                    exp();
                }
                else
                {
                    recover(0, first_exp, "Redundant ','.");
                    exp();
                }
            }
        }
        // <statement> -> call id ([{<exp>{,<exp>}])
        if (lexAnalyzer.sym == RPAREN)
        {
            lexAnalyzer.getToken();
        }
    }
    // <statement> -> <body>
    else if (lexAnalyzer.sym == BEGIN_SYM)
    {
        body();
    }
    // <statement> -> read (id{,id})
    else if (lexAnalyzer.sym == READ_SYM)
    {
        lexAnalyzer.getToken();
        if (lexAnalyzer.sym == LPAREN)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, IDENT, "Missing '('.");
        }
        // <statement> -> read (id
        if (lexAnalyzer.sym == IDENT)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, COMMA | RPAREN, "Find '" + lexAnalyzer.strToken + "' in identifier");
        }
        // <statement> -> read (id{,
        while (lexAnalyzer.sym == COMMA)
        {
            lexAnalyzer.getToken();
            if (lexAnalyzer.sym == IDENT)
            {
                lexAnalyzer.getToken();
            }
            else
            {
                recover(0, IDENT, "Redundant " + lexAnalyzer.strToken);
            }
        }
        if (lexAnalyzer.sym == RPAREN)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, follow_stmt, "Missing ')'.");
        }
    }
    // <statement> -> write(<exp> {,<exp>})
    else if (lexAnalyzer.sym == WRITE_SYM)
    {
        lexAnalyzer.getToken();
        // <statement> -> write(
        if (lexAnalyzer.sym == LPAREN)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, first_exp, "Missing '('.");
        }
        // <statement> -> write(<exp>
        exp();
        // <statement> -> write(<exp> {,<exp>}
        while (lexAnalyzer.sym == COMMA)
        {
            lexAnalyzer.getToken();
            if (lexAnalyzer.sym == RPAREN)
                Utils::log(ERROR, lexAnalyzer.line, "Redundant ','.");
            else
            {
                exp();
            }
        }
        // <statement> -> write(<exp> {,<exp>})
        if (lexAnalyzer.sym == RPAREN)
        {
            lexAnalyzer.getToken();
        }
        else
        {
            recover(0, follow_stmt, "Missing ')'.");
        }
    }
    else
        recover(0, follow_stmt, "Illegal defined statement.");
}

// <body> -> begin <statement> {;<statement>} end
void SyntaxAnalyzer::body()
{
    // 判断是否存在begin,是否仅缺少begin
    recover(BEGIN_SYM, first_stmt, "Missing 'begin'.");
    // begin之后为新的作用域
    statement();
    // 这里如果end前多一个分号会多进行一次循环，并进入else分支
    while (lexAnalyzer.sym & (SEMICOLON | COMMA | first_stmt))
    {
        // 判断是否存在分号，是否仅缺少分号,是否错写为逗号
        if (lexAnalyzer.sym == COMMA)
        {
            Utils::log(ERROR, lexAnalyzer.line, "Find ',' rather than ';'.");
            lexAnalyzer.getToken();
        }
        else
            recover(SEMICOLON, first_stmt, "Missing ';'.");
        if (lexAnalyzer.sym & first_stmt)
        {
            statement();
        }
        else
        {
            Utils::log(ERROR, lexAnalyzer.line, "Redundant ';'.");
        }
    }
    // 判断是否缺少end
    recover(END_SYM, 0, "Missing 'end'.");
    // end之后作用域结束
}


void SyntaxAnalyzer::analyze()
{
    lexAnalyzer.getToken();
    prog();
    Utils::complete(1);
}
