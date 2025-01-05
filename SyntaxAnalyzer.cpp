//
// Created by l0v3ch4n on 25-1-4.
//

#include "SyntaxAnalyzer.h"

#include "Utils.h"
#include "variables.h"

/*
<prog> -> program <id>; <block>
<block> -> [<condecl>] [<vardecl>] [<proc>] <body>
<condecl> -> const <const> {, <const>};
<const> -> <id> := <integer>
<vardecl> -> var <id> {, <id>};
<proc> -> procedure <id> ([<id> {, <id>}]); <block> {; <proc>}
<body> -> begin <statement> {; <statement>} end
<statement> -> <id> := <exp>
            | if <lexp> then <statement> [else <statement>]
            | while <lexp> do <statement>
            | call <id>([<exp> {, <exp>}])
            | <body>
            | read (<id>{, <id>})
            | write (<exp>{, <exp>})
<lexp> -> <exp> <lop> <exp> | odd <exp>
<exp> -> [+ | -] <term> {<aop> <term>}
<term> -> <factor> {<mop> <factor>}
<factor>-><id> | <integer> | (<exp>)
<lop> -> = | <> | < | <= | > | >=
<aop> -> + | -
<mop> -> * | /
<id> -> l {l | d}  (注: l表示字母)
<integer> -> d {d}
注释:
<prog>: 程序; <block>: 块、程序体; <condecl>: 常量说明;
<const>: 常量; <vardecl>: 变量说明; <proc>: 分程序;
<body>: 复合语句; <statement>: 语句；<exp>: 表达式;
<lexp>: 条件; <term>：项; <factor>: 因子; <aop>: 加法运算符;
<mop>: 乘法运算符; <lop>: 关系运算符; odd: 判断表达式的奇偶性
 */

void SyntaxAnalyzer::analyze()
{
    prog();
    if (syntax_err_cnt == 0 && exit_status == SYN_SUCCESS)
    {
        Utils::success("语法分析成功！");
    }
    else
    {
        Utils::error("语法分析失败！");
    }
}

void SyntaxAnalyzer::prog()
{
    // <prog> -> program <id>; <block>
    is_valid_token = lexer.getToken();
    if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
        rsv_word_type_to_string(rsv_word_type::PROGRAM))
    {
        // 为程序开始, <prog>
        is_valid_token = lexer.getToken();
        if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
        {
            // 为标识符, <id>
            is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                opr_type_to_string(opr_type::SEMICOLON))
            {
                // 为分号, ;
                // 检查block定义
                block();
            }
            else
            {
                Utils::error("';' Expected, Found '" + lexer.strToken + "'.", lexer.curr_row, lexer.curr_col);
                syntax_err_cnt++;
                exit_status = SYN_MISS_SEMICOLON;
            }
        }
        else
        {
            Utils::error("Identifier Expected.", lexer.curr_row, lexer.curr_col);
            syntax_err_cnt++;
            exit_status = SYN_MISS_EXPR;
        }
    }
    else
    {
        // 找不到程序开始
        Utils::error("Can not find the start of program (Missing 'program')", lexer.curr_row, lexer.curr_col);
        syntax_err_cnt++;
        exit_status = SYN_MISS_PROG;
    }
}

void SyntaxAnalyzer::exp()
{
    // <exp> -> [+ | -] <term> {<aop> <term>}
    if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && (lexer.sym[1] == opr_type_to_string(opr_type::ADD)
        || lexer.sym[1] == opr_type_to_string(opr_type::SUB)))
    {
        is_valid_token = lexer.getToken();
    }
    term();
    // <aop> -> + | -
    if (lexer.sym[1] != opr_type_to_string(opr_type::ADD) && lexer.sym[1] != opr_type_to_string(opr_type::SUB))
        return;

    is_valid_token = lexer.getToken();
    term();
}

void SyntaxAnalyzer::factor()
{
    // <factor>-><id> | <integer> | (<exp>)
    if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
    {
    }
    else if (lexer.sym[0] == token_type_to_string(token_type::NUMBER))
    {
    }
    else if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
        opr_type::LEFT_BRACKET))
    {
        exp();
        is_valid_token = lexer.getToken();
        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
            opr_type::RIGHT_BRACKET))
        {
        }
        else
        {
            Utils::error("')' expected.", lexer.curr_row, lexer.curr_col);
            syntax_err_cnt++;
            exit_status = SYN_UNEXPECT_TOKEN;
        }
    }
    else
    {
        Utils::error("Identifier expected.", lexer.curr_row, lexer.curr_col);
        syntax_err_cnt++;
        exit_status = SYN_UNEXPECT_TOKEN;
    }
}

void SyntaxAnalyzer::term()
{
    // <term> -> <factor> {<mop> <factor>}
    factor();
    is_valid_token = lexer.getToken();
    if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && (lexer.sym[1] == opr_type_to_string(opr_type::MUL)
        || lexer.sym[1] == opr_type_to_string(opr_type::DIV)))
    {
        // <mop> -> * | /
        is_valid_token = lexer.getToken();
        factor();
        is_valid_token = lexer.getToken();
    }
}

void SyntaxAnalyzer::lexp()
{
    // <lexp> -> <exp> <lop> <exp> | odd <exp>
    is_valid_token = lexer.getToken();
    if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] == rsv_word_type_to_string(
        rsv_word_type::ODD))
    {
        is_valid_token = lexer.getToken();
        exp();
    }
    else
    {
        exp();

        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && (lexer.sym[1] ==
            opr_type_to_string(opr_type::EQ) || lexer.sym[1] == opr_type_to_string(opr_type::NEQ) || lexer.sym[1] ==
            opr_type_to_string(opr_type::LT) || lexer.sym[1] == opr_type_to_string(opr_type::LEQ) || lexer.sym[1] ==
            opr_type_to_string(opr_type::GT) || lexer.sym[1] == opr_type_to_string(opr_type::GEQ)))
        {
            // <lop> -> = | <> | < | <= | > | >=
            is_valid_token = lexer.getToken();
            exp();
            return;
        }
        is_valid_token = lexer.getToken();
    }
}

void SyntaxAnalyzer::statement()
{
    /*
    * <statement> -> <id> := <exp>
            | if <lexp> then <statement> [else <statement>]
            | while <lexp> do <statement>
            | call <id>([<exp> {, <exp>}])
            | <body>
            | read (<id>{, <id>})
            | write (<exp>{, <exp>})
     */
    is_valid_token = lexer.getToken();
    if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
    {
        // <id>
        // <id> :=
        is_valid_token = lexer.getToken();
        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
            opr_type::ASSIGN))
        {
            // <id> := <exp>
            is_valid_token = lexer.getToken();
            exp();
        }
        else
        {
            Utils::error("':=' Expected.", lexer.curr_row, lexer.curr_col);
            syntax_err_cnt++;
            exit_status = SYN_MISS_TOKEN;
        }
    }
    else
    {
        if (lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::IF))
        {
            lexp();
            if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] == rsv_word_type_to_string(
                rsv_word_type::THEN))
            {
                // if <lexp> then <statement>
                statement();
                // if <lexp> then <statement> [else <statement>]
                if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
                    rsv_word_type_to_string(rsv_word_type::ELSE))
                {
                    statement();
                }
            }
            else
            {
                Utils::error("'then' Expected.", lexer.curr_row, lexer.curr_col);
                syntax_err_cnt++;
                exit_status = SYN_MISS_TOKEN;
            }
        }
        else if (lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::WHILE))
        {
            // while <lexp> do <statement>
            lexp();
            // is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] == rsv_word_type_to_string(
                rsv_word_type::DO))
            {
                statement();
            }
            else
            {
                Utils::error("'do' Expected.", lexer.curr_row, lexer.curr_col);
                syntax_err_cnt++;
                exit_status = SYN_MISS_TOKEN;
            }
        }
        else if (lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::CALL))
        {
            // call <id>([<exp> {, <exp>}])
            is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
            {
                is_valid_token = lexer.getToken();
                if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
                    opr_type::LEFT_BRACKET))
                {
                    // (<exp> {, <exp>})
                    is_valid_token = lexer.getToken();
                    exp();
                    while (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                        opr_type_to_string(
                            opr_type::COMMA))
                    {
                        is_valid_token = lexer.getToken();
                        exp();
                        // is_valid_token = lexer.getToken();
                    }
                    if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
                        opr_type::RIGHT_BRACKET))
                    {
                        is_valid_token = lexer.getToken();
                    }
                    else
                    {
                        Utils::error("')' Expected.", lexer.curr_row, lexer.curr_col);
                        syntax_err_cnt++;
                        exit_status = SYN_MISS_TOKEN;
                    }
                }
                else
                {
                    Utils::error("'(' Expected.", lexer.curr_row, lexer.curr_col);
                    syntax_err_cnt++;
                    exit_status = SYN_MISS_TOKEN;
                }
            }
            else
            {
                Utils::error("Identifier Expected.", lexer.curr_row, lexer.curr_col);
                syntax_err_cnt++;
                exit_status = SYN_MISS_TOKEN;
            }
        }
        else if (lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::READ))
        {
            // read (<id>{, <id>})
            is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
                opr_type::LEFT_BRACKET))
            {
                is_valid_token = lexer.getToken();
                if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
                {
                }
                else
                {
                    Utils::error("Identifier expected.", lexer.curr_row, lexer.curr_col);
                    syntax_err_cnt++;
                    exit_status = SYN_UNEXPECT_TOKEN;
                }
                is_valid_token = lexer.getToken();
                while (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                    opr_type_to_string(
                        opr_type::COMMA))
                {
                    is_valid_token = lexer.getToken();
                    if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
                    {
                    }
                    else
                    {
                        Utils::error("Identifier expected.", lexer.curr_row, lexer.curr_col);
                        syntax_err_cnt++;
                        exit_status = SYN_UNEXPECT_TOKEN;
                    }
                    is_valid_token = lexer.getToken();
                }
                if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
                    opr_type::RIGHT_BRACKET))
                {
                    is_valid_token = lexer.getToken();
                }
                else
                {
                    Utils::error("')' Expected.", lexer.curr_row, lexer.curr_col);
                    syntax_err_cnt++;
                    exit_status = SYN_MISS_TOKEN;
                }
            }
            else
            {
                Utils::error("'(' Expected.", lexer.curr_row, lexer.curr_col);
                syntax_err_cnt++;
                exit_status = SYN_MISS_TOKEN;
            }
        }
        else if (lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::WRITE))
        {
            // write (<exp>{, <exp>})
            is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
                opr_type::LEFT_BRACKET))
            {
                is_valid_token = lexer.getToken();
                exp();
                while (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                    opr_type_to_string(
                        opr_type::COMMA))
                {
                    is_valid_token = lexer.getToken();
                    exp();
                    is_valid_token = lexer.getToken();
                }
                if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
                    opr_type::RIGHT_BRACKET))
                {
                    is_valid_token = lexer.getToken();
                }
                else
                {
                    Utils::error("')' Expected.", lexer.curr_row, lexer.curr_col);
                    syntax_err_cnt++;
                    exit_status = SYN_MISS_TOKEN;
                }
            }
            else
            {
                Utils::error("'(' Expected.", lexer.curr_row, lexer.curr_col);
                syntax_err_cnt++;
                exit_status = SYN_MISS_TOKEN;
            }
        }
        else
            body();
    }
}

void SyntaxAnalyzer::block()
{
    // <block> -> [<condecl>] [<vardecl>] [<proc>] <body>
    // [<condecl>]
    is_valid_token = lexer.getToken();
    if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
        rsv_word_type_to_string(rsv_word_type::CONST))
    {
        // 为常量定义, <condecl>
        condecl();
        is_valid_token = lexer.getToken();
    }
    // [<vardecl>]
    if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
        rsv_word_type_to_string(rsv_word_type::VAR))
    {
        // 为变量定义, <vardecl>
        vardecl();
        is_valid_token = lexer.getToken();
        // 检查是否为常量定义
        if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
            rsv_word_type_to_string(rsv_word_type::CONST))
        {
            Utils::error("Unexpected const definition.", lexer.curr_row, lexer.curr_col);
            syntax_err_cnt++;
            exit_status = SYN_UNEXPECT_EXPR;
        }
    }
    // [<proc>]
    if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
        rsv_word_type_to_string(rsv_word_type::PROCEDURE))
    {
        // 过程定义, <proc>
        proc();
        // 检查是否为常量定义或变量定义
        if ((lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && (
            lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::CONST) ||
            lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::VAR))))
        {
            if (lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::CONST))
                Utils::error("Unexpected const definition.", lexer.curr_row, lexer.curr_col);
            else
                Utils::error("Unexpected var definition.", lexer.curr_row, lexer.curr_col);
            syntax_err_cnt++;
            exit_status = SYN_UNEXPECT_EXPR;
        }
    }
    // 必须得有 <body>
    // <body>
    body();
}

void SyntaxAnalyzer::constDef()
{
    // <const> -> <id> := <integer>
    // <id>
    if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
    {
        // 标识符定位：常量名
        lexer.sym[1] = identifier_type_to_string(identifier_type::CONSTANT);
        is_valid_token = lexer.getToken();
        // :=
        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
            opr_type_to_string(opr_type::ASSIGN))
        {
            is_valid_token = lexer.getToken();
            // <integer>
            if (lexer.sym[0] == token_type_to_string(token_type::NUMBER))
            {
                // 将数字加入到符号表
            }
            else
            {
                Utils::error("Integer expected.", lexer.curr_row, lexer.curr_col);
                syntax_err_cnt++;
                exit_status = SYN_UNEXPECT_TOKEN;
            }
        }
        else
        {
            Utils::error("':=' Expected.", lexer.curr_row, lexer.curr_col);
        }
    }
    else
    {
        Utils::error("Identifier expected.", lexer.curr_row, lexer.curr_col);
        syntax_err_cnt++;
        exit_status = SYN_UNEXPECT_TOKEN;
    }
}

void SyntaxAnalyzer::condecl()
{
    // <condecl> -> const <const> {, <const>};
    is_valid_token = lexer.getToken();
    constDef();
    is_valid_token = lexer.getToken();
    while (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] !=
        opr_type_to_string(opr_type::SEMICOLON)) // 到分号截止
    {
        // ,
        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
            opr_type_to_string(opr_type::COMMA))
        {
            is_valid_token = lexer.getToken();
            constDef();
            is_valid_token = lexer.getToken(); // 获取下一个Token
        }
        else
        {
            Utils::error("',' expected.", lexer.pre_row, lexer.pre_col);
            syntax_err_cnt++;
            exit_status = SYN_MISS_SEMICOLON;
        }
    }
}

void SyntaxAnalyzer::vardecl()
{
    // <vardecl> -> var <id> {, <id>};
    is_valid_token = lexer.getToken();
    if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
    {
        // <id>
        // 标识符定位：变量名
        lexer.sym[1] = identifier_type_to_string(identifier_type::VARIABLE);
        is_valid_token = lexer.getToken();
        while (lexer.sym[1] !=
            opr_type_to_string(opr_type::SEMICOLON)) // 到分号截止
        {
            // ,
            if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                opr_type_to_string(opr_type::COMMA))
            {
                is_valid_token = lexer.getToken();
                if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
                {
                    // 标识符定位：变量名
                    lexer.sym[1] = identifier_type_to_string(identifier_type::VARIABLE);
                    is_valid_token = lexer.getToken(); // 获取下一个Token
                }
                else if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD))
                {
                    // 错误命名
                    Utils::error("Invalid identifier name.", lexer.curr_row, lexer.curr_col);
                    syntax_err_cnt++;
                    exit_status = SYN_INV_DED;
                }
                else
                {
                    Utils::error("Identifier expected.", lexer.curr_row, lexer.curr_col);
                    syntax_err_cnt++;
                    exit_status = SYN_UNEXPECT_TOKEN;
                }
            }
            else
            {
                Utils::error("',' expected.", lexer.pre_row, lexer.pre_col);
                syntax_err_cnt++;
                exit_status = SYN_MISS_SEMICOLON;
                return;
            }
        }
    }
    else if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD))
    {
        // 错误命名
        Utils::error("Invalid identifier name (Conflict with reserve word '" + lexer.sym[2] + "'.",
                     lexer.curr_row,
                     lexer.curr_col);
        syntax_err_cnt++;
        exit_status = SYN_INV_DED;
    }
    else
    {
        Utils::error("Identifier expected.", lexer.curr_row, lexer.curr_col);
        syntax_err_cnt++;
        exit_status = SYN_UNEXPECT_TOKEN;
    }
}

void SyntaxAnalyzer::proc()
{
    // <proc> -> procedure <id> ([<id> {, <id>}]); <block> {; <proc>}
    is_valid_token = lexer.getToken();
    if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
    {
        // <proc> -> procedure <id>
        // 标识符定位：过程名
        lexer.sym[1] = identifier_type_to_string(identifier_type::PROCEDURE);
        is_valid_token = lexer.getToken();
        // <proc> -> procedure <id> ([<id> {, <id>}])
        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
            opr_type_to_string(opr_type::LEFT_BRACKET))
        {
            is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
            {
                // <id>
                // 标识符定位：参数名
                lexer.sym[1] = identifier_type_to_string(identifier_type::PARAMETER);
                is_valid_token = lexer.getToken();
                while (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] !=
                    opr_type_to_string(opr_type::RIGHT_BRACKET))
                {
                    // ,
                    if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                        opr_type_to_string(opr_type::COMMA))
                    {
                        is_valid_token = lexer.getToken();
                        if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
                        {
                            // 标识符定位：变量名
                            lexer.sym[1] = identifier_type_to_string(identifier_type::VARIABLE);
                            is_valid_token = lexer.getToken(); // 获取下一个Token
                        }
                        else if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD))
                        {
                            // 错误命名
                            Utils::error(
                                "Invalid identifier name (Conflict with reserve word '" + lexer.sym[2] + "'.",
                                lexer.curr_row, lexer.curr_col);
                            syntax_err_cnt++;
                            exit_status = SYN_INV_DED;
                        }
                        else
                        {
                            Utils::error("Identifier expected.", lexer.curr_row, lexer.curr_col);
                            syntax_err_cnt++;
                            exit_status = SYN_UNEXPECT_TOKEN;
                        }
                    }
                    else
                    {
                        Utils::error("',' expected.", lexer.pre_row, lexer.pre_col);
                        syntax_err_cnt++;
                        exit_status = SYN_MISS_SEMICOLON;
                    }
                }
            }
        }
        else
        {
            Utils::error("'(' expected.", lexer.pre_row, lexer.pre_col);
        }
        // <proc> -> procedure <id> ([<id> {, <id>}]);
        is_valid_token = lexer.getToken();
        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
            opr_type_to_string(opr_type::SEMICOLON))
        {
            // <proc> -> procedure <id> ([<id> {, <id>}]); <block>
            block();
            // <proc> -> procedure <id> ([<id> {, <id>}]); <block> {; <proc>}
            while (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                opr_type_to_string(opr_type::SEMICOLON))
            {
                is_valid_token = lexer.getToken();
                if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
                    rsv_word_type_to_string(rsv_word_type::PROCEDURE))
                {
                    proc();
                    if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
                        rsv_word_type_to_string(rsv_word_type::BEGIN))
                        return;
                    is_valid_token = lexer.getToken();
                }
            }
        }
        else
        {
            Utils::error("';' expected.", lexer.pre_row, lexer.pre_col);
            syntax_err_cnt++;
            exit_status = SYN_MISS_SEMICOLON;
        }
    }
    else if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD))
    {
        // 错误命名
        Utils::error("Invalid identifier name.", lexer.curr_row, lexer.curr_col);
        syntax_err_cnt++;
        exit_status = SYN_INV_DED;
    }
    else
    {
        Utils::error("Identifier expected.", lexer.curr_row, lexer.curr_col);
        syntax_err_cnt++;
        exit_status = SYN_UNEXPECT_TOKEN;
    }
}

void SyntaxAnalyzer::body()
{
    Utils::warning("body start");
    // <body> -> begin <statement> {; <statement>} end
    // <body> -> begin
    if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] == rsv_word_type_to_string(
        rsv_word_type::BEGIN))
    {
        // <body> -> begin <statement>
        statement();
        // <body> -> begin <statement> {; <statement>}
        while (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
            opr_type_to_string(opr_type::SEMICOLON))
        {
            statement();
        }
        // <body> -> begin <statement> {; <statement>} end
        if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
            rsv_word_type_to_string(
                rsv_word_type::END))
        {
            is_valid_token = lexer.getToken();
        }
        else
        {
            Utils::error("'end' expected.", lexer.pre_row, lexer.pre_col);
            syntax_err_cnt++;
            exit_status = SYN_MISS_TOKEN;
        }
    }
    else
    {
        Utils::error("'begin' expected.", lexer.pre_row, lexer.pre_col);
        syntax_err_cnt++;
        exit_status = SYN_MISS_TOKEN;
    }
    Utils::warning("body end");
}
