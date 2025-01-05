//
// Created by l0v3ch4n on 25-1-4.
//

#include "SyntaxAnalyzer.h"

#include <ranges>

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
            // 登入符号表
            SymTable::mkTable();
            SymTable::enterProgram(lexer.sym[2]);
            is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                opr_type_to_string(opr_type::SEMICOLON))
            {
                // 为分号, ;
                // 主程序入口地址登入符号表
                const size_t entry = PCode::emit(jmp, 0, 0);
                SymTable::table[0].info->setEntry(entry);
                // 检查block定义
                block();
                // 返回，弹栈
                PCode::emit(opr, 0, OPR_RETURN);
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
    std::string aop = lexer.sym[1];
    if (aop == opr_type_to_string(opr_type::SUB))
        PCode::emit(opr, 0, OPR_NEGTIVE);
    while (lexer.sym[1] == opr_type_to_string(opr_type::ADD) || lexer.sym[1] == opr_type_to_string(opr_type::SUB))
    {
        aop = lexer.sym[1];
        is_valid_token = lexer.getToken();
        term();
        if (aop == opr_type_to_string(opr_type::ADD))
            PCode::emit(opr, 0, OPR_ADD);
        else
            PCode::emit(opr, 0, OPR_SUB);
    }
}

void SyntaxAnalyzer::factor()
{
    // <factor>-><id> | <integer> | (<exp>)
    if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
    {
        // 查找变量
        const int pos = SymTable::lookUpVar(lexer.sym[2]);
        VarInfo* curr_info = nullptr;
        if (pos != -1)
        {
            Utils::error("Undefined variable '" + lexer.sym[2] + "'.", lexer.curr_row, lexer.curr_col);
            syntax_err_cnt++;
        }
        else
            curr_info = dynamic_cast<VarInfo*>(SymTable::table[pos].info);
        if (curr_info)
        {
            if (curr_info->cat == identifier_type_to_string(identifier_type::CONSTANT))
            {
                const int val = curr_info->getValue();
                PCode::emit(lit, static_cast<int>(curr_info->level), val);
            }
            else
            {
                // 为变量，取左值
                PCode::emit(load, static_cast<int>(curr_info->level),
                            static_cast<int>(curr_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + curr_info->level + 1));
            }
        }
    }
    else if (lexer.sym[0] == token_type_to_string(token_type::NUMBER))
    {
        // 数值，直接入栈
        PCode::emit(lit, 0, std::stoi(lexer.sym[2]));
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
        std::string mop = lexer.sym[1];
        // <mop> -> * | /
        is_valid_token = lexer.getToken();
        factor();
        if (mop == opr_type_to_string(opr_type::MUL))
            PCode::emit(opr, 0, OPR_MUL);
        else
            PCode::emit(opr, 0, OPR_DIV);
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
        PCode::emit(opr, 0, OPR_ODD);
    }
    else
    {
        exp();
        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && (lexer.sym[1] ==
            opr_type_to_string(opr_type::EQ) || lexer.sym[1] == opr_type_to_string(opr_type::NEQ) || lexer.sym[1] ==
            opr_type_to_string(opr_type::LT) || lexer.sym[1] == opr_type_to_string(opr_type::LEQ) || lexer.sym[1] ==
            opr_type_to_string(opr_type::GT) || lexer.sym[1] == opr_type_to_string(opr_type::GEQ)))
        {
            const std::string lop = lexer.sym[1];
            // <lop> -> = | <> | < | <= | > | >=
            is_valid_token = lexer.getToken();
            exp();
            if (lop == opr_type_to_string(opr_type::EQ))
                PCode::emit(opr, 0, OPR_EQL);
            else if (lop == opr_type_to_string(opr_type::NEQ))
                PCode::emit(opr, 0, OPR_NEQ);
            else if (lop == opr_type_to_string(opr_type::LT))
                PCode::emit(opr, 0, OPR_LSS);
            else if (lop == opr_type_to_string(opr_type::LEQ))
                PCode::emit(opr, 0, OPR_LEQ);
            else if (lop == opr_type_to_string(opr_type::GT))
                PCode::emit(opr, 0, OPR_GRT);
            else if (lop == opr_type_to_string(opr_type::GEQ))
                PCode::emit(opr, 0, OPR_GEQ);
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
        const int pos = SymTable::lookUpVar(lexer.sym[2]);
        VarInfo* curr_info = nullptr;
        if (pos != -1)
        {
            Utils::error("Undefined variable '" + lexer.sym[2] + "'.", lexer.curr_row, lexer.curr_col);
            sym_err_cnt++;
        }
        else
            curr_info = dynamic_cast<VarInfo*>(SymTable::table[pos].info);
        is_valid_token = lexer.getToken();
        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
            opr_type::ASSIGN))
        {
            // <id> := <exp>
            //查找到右值，右值不可被赋值
            if (curr_info && curr_info->cat == identifier_type_to_string(identifier_type::CONSTANT))
            {
                Utils::error("Constant can not be assigned.", lexer.curr_row, lexer.curr_col);
                sym_err_cnt++;
            }
            is_valid_token = lexer.getToken();
            exp();
        }
        else
        {
            Utils::error("':=' Expected.", lexer.curr_row, lexer.curr_col);
            syntax_err_cnt++;
            exit_status = SYN_MISS_TOKEN;
        }
        if (curr_info)
            PCode::emit(store, static_cast<int>(curr_info->level),
                        static_cast<int>(curr_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + curr_info->level + 1));
    }
    else
    {
        if (lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::IF))
        {
            lexp();
            size_t entry_jpc = -1, entry_jmp = -2;
            if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] == rsv_word_type_to_string(
                rsv_word_type::THEN))
            {
                entry_jpc = PCode::emit(jpc, 0, 0);
                // if <lexp> then <statement>
                statement();
                // if <lexp> then <statement> [else <statement>]
                if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] ==
                    rsv_word_type_to_string(rsv_word_type::ELSE))
                {
                    entry_jmp = PCode::emit(jmp, 0, 0);
                    // 将else入口地址回填至jpc
                    PCode::backpatch(entry_jpc, PCode::codeList.size());
                    statement();
                    // 有else，则将if外入口地址回填至jmp
                    PCode::backpatch(entry_jmp, PCode::codeList.size());
                }
                else
                // 没有else，则将if外入口地址回填至jpc
                    PCode::backpatch(entry_jpc, PCode::codeList.size());
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
            const size_t condition = PCode::codeList.size();
            lexp();
            // 当前栈顶为条件表达式的布尔值
            // 条件为假跳转，待回填循环出口地址
            const size_t loop = PCode::emit(jpc, 0, 0);
            // is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::RSV_WORD) && lexer.sym[1] == rsv_word_type_to_string(
                rsv_word_type::DO))
            {
                statement();
                // 无条件跳转至循环条件判断前
                PCode::emit(jmp, 0, static_cast<int>(condition));
            }
            else
            {
                Utils::error("'do' Expected.", lexer.curr_row, lexer.curr_col);
                syntax_err_cnt++;
                exit_status = SYN_MISS_TOKEN;
            }
            // 将下一条语句回填至jpc
            PCode::backpatch(loop, PCode::codeList.size());
        }
        else if (lexer.sym[1] == rsv_word_type_to_string(rsv_word_type::CALL))
        {
            // call <id>([<exp> {, <exp>}])
            is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
            {
                ProcInfo* curr_info = nullptr;
                // 查找过程的符号名
                int pos = SymTable::lookUpProc(lexer.sym[2]);
                // 未查找到过程名
                if (pos == -1)
                {
                    Utils::error("Undefined variable '" + lexer.sym[2] + "'.", lexer.curr_row, lexer.curr_col);
                    sym_err_cnt++;
                }
                else
                    curr_info = dynamic_cast<ProcInfo*>(SymTable::table[pos].info);
                // 若调用未定义的过程
                if (curr_info && !curr_info->isDefined)
                {
                    Utils::error("Undefined variable '" + lexer.sym[2] + "'.", lexer.curr_row, lexer.curr_col);
                    sym_err_cnt++;
                }
                is_valid_token = lexer.getToken();
                if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
                    opr_type::LEFT_BRACKET))
                {
                    // (<exp> {, <exp>})
                    is_valid_token = lexer.getToken();
                    exp();
                    // 将实参传入即将调用的子过程
                    if (curr_info)
                        PCode::emit(store, -1, ACT_PRE_REC_SIZE + curr_info->level + 1);
                    size_t i = 1;
                    while (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                        opr_type_to_string(
                            opr_type::COMMA))
                    {
                        is_valid_token = lexer.getToken();
                        exp();
                        if (curr_info)
                            PCode::emit(store, -1, ACT_PRE_REC_SIZE + curr_info->level + 1 + i++);
                        // is_valid_token = lexer.getToken();
                    }
                    // 实参与形参数量不匹配，报错
                    if (curr_info && i != curr_info->form_var_list.size())
                    {
                        Utils::error("Parameter number mismatch.", lexer.curr_row, lexer.curr_col);
                        sym_err_cnt++;
                    }

                    if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] == opr_type_to_string(
                        opr_type::RIGHT_BRACKET))
                    {
                        is_valid_token = lexer.getToken();
                        //调用子过程
                        if (curr_info)
                            PCode::emit(call, static_cast<int>(curr_info->level), static_cast<int>(curr_info->entry));
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
                    const int pos = SymTable::lookUpVar(lexer.sym[2]);
                    const VarInfo* curr_info = nullptr;
                    if (pos != -1)
                    {
                        Utils::error("Undefined variable '" + lexer.sym[2] + "'.", lexer.curr_row, lexer.curr_col);
                        sym_err_cnt++;
                    }
                    else
                        curr_info = dynamic_cast<VarInfo*>(SymTable::table[pos].info);
                    // 右值不可被赋值
                    if (curr_info)
                    {
                        if (curr_info->cat == identifier_type_to_string(identifier_type::CONSTANT))
                        {
                            Utils::error("Constant cannot be assigned.", lexer.curr_row, lexer.curr_col);
                            sym_err_cnt++;
                        }
                        // 读入一个数据到栈顶
                        PCode::emit(red, 0, 0);
                        // 将栈顶送入变量所在地址
                        PCode::emit(store, static_cast<int>(curr_info->level),
                                    static_cast<int>(curr_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + curr_info->level
                                        + 1));
                    }
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
                        const int pos = SymTable::lookUpVar(lexer.sym[2]);
                        const VarInfo* curr_info = nullptr;
                        if (pos != -1)
                        {
                            Utils::error("Undeclared variable '" + lexer.sym[2] + "'.", lexer.curr_row, lexer.curr_col);
                            sym_err_cnt++;
                        }
                        else
                            curr_info = dynamic_cast<VarInfo*>(SymTable::table[pos].info);
                        // 右值不可被赋值
                        if (curr_info)
                        {
                            if (curr_info->cat == identifier_type_to_string(identifier_type::CONSTANT))
                            {
                                Utils::error("Constant cannot be assigned.", lexer.curr_row, lexer.curr_col);
                                sym_err_cnt++;
                            }
                            PCode::emit(red, 0, 0);
                            PCode::emit(store, static_cast<int>(curr_info->level),
                                        static_cast<int>(curr_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + curr_info->
                                            level + 1));
                        }
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
                PCode::emit(wrt, 0, 0);
                while (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
                    opr_type_to_string(
                        opr_type::COMMA))
                {
                    is_valid_token = lexer.getToken();
                    exp();
                    PCode::emit(wrt, 0, 0);
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
    // 将所需内存写入符号表
    const size_t curr_proc = SymTable::sp;
    auto* curr_info = dynamic_cast<ProcInfo*>(SymTable::table[curr_proc].info);
    SymTable::addWidth(curr_proc, glo_offset);

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
    // 开辟子过程空间, 为display开辟level+1单元
    const size_t entry = PCode::emit(
        alloc, 0, static_cast<int>(curr_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + level + 1));
    const size_t target = curr_info->entry;
    // 过程入口语句地址回填到过程跳转语句
    PCode::backpatch(target, entry);
    if (curr_proc)
        curr_info->isDefined = true;
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
        SymTable::enter(lexer.sym[2], 0, identifier_type_to_string(identifier_type::CONSTANT));
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
                SymTable::table[SymTable::table.size() - 1].info->setValue(lexer.sym[2]);
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
        SymTable::enter(lexer.sym[2], glo_offset, identifier_type_to_string(identifier_type::VARIABLE));
        glo_offset += VAR_WIDTH;
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
                    SymTable::enter(lexer.sym[2], glo_offset, identifier_type_to_string(identifier_type::VARIABLE));
                    glo_offset += VAR_WIDTH;
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
        ProcInfo* curr_info = nullptr;
        // <proc> -> procedure <id>
        // 标识符定位：过程名
        lexer.sym[1] = identifier_type_to_string(identifier_type::PROCEDURE);
        SymTable::mkTable();
        int curr_proc = SymTable::enterProc(lexer.sym[2]);
        if (curr_proc != -1)
        {
            curr_info = dynamic_cast<ProcInfo*>(SymTable::table[curr_proc].info);
            // 子过程入口地址填入符号表，待回填
            const size_t entry = PCode::emit(jmp, 0, 0);
            SymTable::table[SymTable::table.size() - 1].info->setEntry(entry);
        }
        is_valid_token = lexer.getToken();
        // <proc> -> procedure <id> ([<id> {, <id>}])
        if (lexer.sym[0] == token_type_to_string(token_type::OPCODE) && lexer.sym[1] ==
            opr_type_to_string(opr_type::LEFT_BRACKET))
        {
            // 层级扩张
            SymTable::display.push_back(0);
            level++;
            is_valid_token = lexer.getToken();
            if (lexer.sym[0] == token_type_to_string(token_type::IDENTIFY))
            {
                // <id>
                // 标识符定位：参数名
                lexer.sym[1] = identifier_type_to_string(identifier_type::PARAMETER);
                // 将形参登入符号表，并与相应过程绑定
                int form_var = SymTable::enter(lexer.sym[2], glo_offset,
                                               identifier_type_to_string(identifier_type::PARAMETER));
                glo_offset += VAR_WIDTH;
                if (curr_info)
                    curr_info->form_var_list.push_back(form_var);
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
                            form_var = SymTable::enter(lexer.sym[2], glo_offset,
                                                       identifier_type_to_string(identifier_type::PARAMETER));
                            glo_offset += VAR_WIDTH;
                            if (curr_info)
                                curr_info->form_var_list.push_back(form_var);
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
            // 返回，弹栈
            PCode::emit(opr, 0, OPR_RETURN);
            // 层级减少，dislapy弹出
            level--;
            SymTable::display.pop_back();
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
