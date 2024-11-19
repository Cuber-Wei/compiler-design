//
// Created by l0v3ch4n on 24-11-18.
//

#include "SyntaxAnalyzer.h"

#include "LexAnalyzer.h"
#include "PartialCode.h"
#include "SymbolTable.h"
#include "utils.h"
#include "variables.h"

// 用于错误恢复的函数，若当前符号在s1中，则读取下一符号；若当前符号不在s1中，则报错，接着循环查找下一个在中s1 ∪ s2的符号
void SyntaxAnalyzer::recover(const unsigned long s1, const unsigned long s2,
                             const std::string &errorMessage) {
    if (!(lexAnalyzer.sym & s1)) // 当前符号不在s1中
    {
        Utils::log(ERROR, lexAnalyzer.line, errorMessage);
        const unsigned long s3 = s1 | s2; // 把s2补充进s1

        while (!(lexAnalyzer.sym & s3)) // 循环找到下一个合法的符号
        {
            if (lexAnalyzer.ch == '\0')
                Utils::complete(0);
            lexAnalyzer.getToken(); // 继续词法分析
        }
        if (lexAnalyzer.sym & s1)
            lexAnalyzer.getToken();
    } else
        lexAnalyzer.getToken();
}

// 开始语法分析
// <prog> -> program id; <block>
void SyntaxAnalyzer::prog() {
    // 找到第一次出现的program
    recover(PROGM_SYM, 0, "Missing token 'program'.");
    // <prog> -> program id
    if (lexAnalyzer.sym == IDENT) {
        // 将过程名登入符号表
        // SymTable::mkTable();
        // SymTable::enterProgram(lexAnalyzer.strToken);
        lexAnalyzer.getToken();
    }
    // 缺失 <prog> -> program ;
    else if (lexAnalyzer.sym == SEMICOLON) {
        Utils::log(ERROR, lexAnalyzer.line, "Missing program identifier.");
    }
    //  <prog> -> program {~id} ;
    else {
        Utils::log(ERROR, lexAnalyzer.line, "Find '" + lexAnalyzer.strToken + "' in identifier.");
        lexAnalyzer.getToken();
    }
    // <prog> -> program id;
    if (lexAnalyzer.sym == SEMICOLON) {
        lexAnalyzer.getToken();
    }
    // <prog> -> program id {~';'} <block>
    else {
        // 判断是否仅仅是缺失分号
        recover(SEMICOLON, first_block, "Missing token ';'.");
    }
    // 主过程的入口地址登入符号表，待回填
    // const size_t entry = PartialCodeList::emit(jmp, 0, 0);
    // SymTable::table[0].info->setEntry(entry);
    //<prog> -> program id; <block>
    block();
    //<prog> -> program id; <block>#
    // 执行返回，并弹栈
    // PartialCodeList::emit(opr, 0, 0);
    if (lexAnalyzer.sym == NUL) {
    }
}

// <block> -> [<condecl>][<vardecl>][<proc>]<body>
void SyntaxAnalyzer::block() {
    // <block> -> [<condecl>]
    if (lexAnalyzer.sym == CONST_SYM) {
        condecl();
    }
    // <block> -> [<condecl>][<vardecl>]
    if (lexAnalyzer.sym == VAR_SYM) {
        vardecl();
    }
    // 将过程所需的内存大小写入符号表
    // const size_t cur_proc = SymTable::sp;
    // auto *curInfo = dynamic_cast<ProcInfo *>(SymTable::table[cur_proc].info);
    // SymTable::addWidth(
    //     cur_proc,
    //     glo_offset);
    // <block> -> [<condecl>][<vardecl>][<proc>]]
    if (lexAnalyzer.sym == PROC_SYM) {
        proc();
    }
    // 为子过程开辟活动记录空间，其中为display开辟level + 1个单元
    // const size_t entry = PartialCodeList::emit(alloc, 0, curInfo->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + level + 1);
    // const size_t target = curInfo->entry;
    // 将过程入口地址回填至过程的跳转语句
    // PartialCodeList::backpatch(target, entry);
    // 过程体开始，过程已定义
    // if (cur_proc)
    //     curInfo->isDefined = true;
    // <block> -> [<condecl>][<vardecl>][<proc>]<body>
    body();
}

// const -> id:=number
void SyntaxAnalyzer::constDef() {
    if (lexAnalyzer.sym == IDENT) {
        // 将常量登入符号表，常量属于右值，不需要记录offset
        // const std::string name = lexAnalyzer.strToken;
        // SymTable::enter(name, 0, CST);
        lexAnalyzer.getToken();
        // const -> id:=
        if (lexAnalyzer.sym & (ASSIGN | EQL)) {
            if (lexAnalyzer.sym == EQL) {
                Utils::log(ERROR, lexAnalyzer.line, "Expect ':=' rather than '='.");
            }
            lexAnalyzer.getToken();
            // const -> id:=number
            if (lexAnalyzer.sym == NUMBER) {
                // SymTable::table[SymTable::table.size() - 1].info->setValue(lexAnalyzer.strToken);
                lexAnalyzer.getToken();
            } else {
                Utils::log(ERROR, lexAnalyzer.line, "Number expected.");
            }
        }
    } else {
        // 没有找到id则跳转到第一个constDef的follow集中的符号
        recover(0, follow_constdef, "Illegal defined ConstDef.");
    }
}

// <condecl> -> const <constDef>{,<constDef>};
void SyntaxAnalyzer::condecl() {
    // const
    if (lexAnalyzer.sym == CONST_SYM) {
        // const <constDef>
        lexAnalyzer.getToken();
        constDef();
        // const <constDef> {,<constDef>}
        while (lexAnalyzer.sym & (COMMA | IDENT)) {
            recover(COMMA, IDENT, "Missing ','.");
            if (lexAnalyzer.sym == IDENT) // FIRST(<constDef>)
            {
                constDef();
            } else {
                recover(0, IDENT | SEMICOLON, "ConstDef expected."); // todo expect常量定义式
            }
        }
        // const <constDef> {,<constDef>};
        if (lexAnalyzer.sym == SEMICOLON) {
            lexAnalyzer.getToken();
            return;
        } else {
            recover(0, follow_condecl, "Missing ';'."); // todo expect ;
        }
    } else {
        recover(0, follow_condecl, "Illegal defined condecl.");
    }
}

// <vardecl> -> var <id>{,<id>};
void SyntaxAnalyzer::vardecl() {
    // var
    if (lexAnalyzer.sym == VAR_SYM) {
        lexAnalyzer.getToken();
        // var <id>
        if (lexAnalyzer.sym == IDENT) {
            // 将标识符登入到符号表
            // SymTable::enter(lexAnalyzer.strToken, glo_offset, VAR);
            // glo_offset += VAR_WIDTH;
            lexAnalyzer.getToken();
        } else {
            recover(0, COMMA, "Missing identifier");
        }
        // var <id>{,<id>}
        while (lexAnalyzer.sym == COMMA) {
            lexAnalyzer.getToken();
            if (lexAnalyzer.sym == IDENT) {
                // 将标识符登入到符号表
                // SymTable::enter(lexAnalyzer.strToken, glo_offset, VAR);
                // glo_offset += VAR_WIDTH;
                lexAnalyzer.getToken();
            } else {
                Utils::log(ERROR, lexAnalyzer.line, "Redundant ','."); // todo expect 标识符
            }
        }
        // var <id>{,<id>};
        if (lexAnalyzer.sym == SEMICOLON) {
            lexAnalyzer.getToken();
        } else {
            recover(0, follow_vardecl, "Missing ';'.");
        }
    } else {
        recover(0, follow_vardecl, "Illegal defined vardecl.");
    }
}

// <proc> -> procedure id ([id {,id}]);<block> {;<proc>}
void SyntaxAnalyzer::proc() {
    if (lexAnalyzer.sym == PROC_SYM) {
        lexAnalyzer.getToken();
        // <proc> -> procedure id
        // ProcInfo *curInfo = nullptr; // 临时变量，记录当前过程符号表项的信息
        if (lexAnalyzer.sym == IDENT) {
            // 将过程名登入符号表
            // SymTable::mkTable();
            // if (const int cur_proc = SymTable::enterProc(lexAnalyzer.strToken); cur_proc != -1) {
            //     curInfo = dynamic_cast<ProcInfo *>(SymTable::table[cur_proc].info);
            //     // 子过程的入口地址登入符号表，待回填
            //     const size_t entry = PartialCodeList::emit(jmp, 0, 0);
            //     SymTable::table[SymTable::table.size() - 1].info->setEntry(entry);
            // }
            lexAnalyzer.getToken();
        } else {
            recover(0, LPAREN, "Find '" + lexAnalyzer.strToken + "' in identifier");
        }
        // <proc> -> procedure id (
        if (lexAnalyzer.sym == LPAREN) {
            // 层级增加，display表扩张
            // SymTable::display.push_back(0);
            // level++;
            lexAnalyzer.getToken();
        } else {
            recover(0, IDENT | RPAREN, "Missing '('.");
        }
        // <proc> -> procedure id ([id {,id}]
        // 分析至形参列表
        if (lexAnalyzer.sym == IDENT) {
            // 将过程的形参登入符号表，并与相应的过程绑定
            // const int form_var = SymTable::enter(lexAnalyzer.strToken, glo_offset, FORM);
            // glo_offset += VAR_WIDTH;
            // if (curInfo)
            //     curInfo->form_var_list.push_back(form_var);
            lexAnalyzer.getToken();
            while (lexAnalyzer.sym == COMMA) {
                lexAnalyzer.getToken();
                if (lexAnalyzer.sym == IDENT) {
                    // 将过程的形参登入符号表，并与相应的过程绑定
                    // int form_var = SymTable::enter(lexAnalyzer.strToken, glo_offset, FORM);
                    // glo_offset += VAR_WIDTH;
                    // if (curInfo)
                    //     curInfo->form_var_list.push_back(form_var);
                    lexAnalyzer.getToken();
                } else {
                    Utils::log(ERROR, lexAnalyzer.line, "Redundant ','");
                }
            }
        }
        // <proc> -> procedure id ([id {,id}])
        if (lexAnalyzer.sym == RPAREN) {
            lexAnalyzer.getToken();
        } else {
            recover(0, SEMICOLON, "Missing ')'.");
        }
        // <proc> -> procedure id ([id {,id}]);
        if (lexAnalyzer.sym == SEMICOLON) {
            lexAnalyzer.getToken();
        } else {
            recover(0, first_block, "Missing ';'.");
        }
        // <proc> -> procedure id ([id {,id}]);<block> {;<proc>}
        if (lexAnalyzer.sym & first_block) {
            block();
            // 执行返回，并弹栈
            // PartialCodeList::emit(opr, 0, OPR_RETURN);
            // // 层级减少，display表弹出
            // SymTable::display.pop_back();
            // level--;
            // 当前过程结束，开始分析下面的过程
            while (lexAnalyzer.sym == SEMICOLON) {
                lexAnalyzer.getToken();
                // FIRST(proc)
                if (lexAnalyzer.sym == PROC_SYM) {
                    proc();
                } else {
                    Utils::log(ERROR, lexAnalyzer.line, "Redundant ';'");
                }
            }
        } else {
            recover(0, follow_block, "Illegal defined block");
        }
    } else {
        recover(0, follow_proc, "Illegal defined procedure");
    }
}

// <exp> -> [+|-] <term>{[+|-] <term>}
void SyntaxAnalyzer::exp() {
    // <exp> -> [+|-]
    unsigned long aop = NUL;
    if (lexAnalyzer.sym == PLUS || lexAnalyzer.sym == MINUS) {
        aop = lexAnalyzer.sym;
        lexAnalyzer.getToken();
    }
    // <exp> -> <term>{[+|-] <term>}
    if (lexAnalyzer.sym & first_term) // FIRST(term) 、 FIRST(factor)
    {
        term();
        // 若有负号，栈顶取反
        // if (aop == MINUS)
        //     PartialCodeList::emit(opr, 0, OPR_NEGTIVE);
        while (lexAnalyzer.sym == PLUS || lexAnalyzer.sym == MINUS) {
            aop = lexAnalyzer.sym;
            lexAnalyzer.getToken();
            // FIRST(term)
            if (lexAnalyzer.sym & first_term) {
                term();
                // 减
                // if (aop == MINUS)
                //     PartialCodeList::emit(opr, 0, OPR_SUB);
                //     // 加
                // else
                //     PartialCodeList::emit(opr, 0, OPR_ADD);
            } else {
                Utils::log(ERROR, lexAnalyzer.line, "Redundant " + lexAnalyzer.strToken);
            }
        }
    } else {
        recover(0, follow_exp, "Illegal defined expression.");
    }
}

// <factor> -> id|number|(<exp>)
void SyntaxAnalyzer::factor() {
    if (lexAnalyzer.sym == IDENT) {
        // 查找变量符号
        // const int pos = SymTable::lookUpVar(lexAnalyzer.strToken);
        // VarInfo *curInfo = nullptr;
        // if (pos == -1)
        //     Utils::log(ERROR, lexAnalyzer.line, "Undeclared " + lexAnalyzer.strToken);
        //     // 若为常量，直接获取其符号表中的右值
        //     // 用临时变量记录当前查到的信息
        // else
        //     curInfo = dynamic_cast<VarInfo *>(SymTable::table[pos].info);
        // if (curInfo) {
        //     if (curInfo->cat == CST) {
        //         const int val = curInfo->getValue();
        //         PartialCodeList::emit(lit, curInfo->level, val);
        //     }
        //     // 若为变量，取左值
        //     else {
        //         PartialCodeList::emit(load, curInfo->level,
        //                               curInfo->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + curInfo->level + 1);
        //     }
        // }
        lexAnalyzer.getToken();
    } else if (lexAnalyzer.sym == NUMBER) {
        // 数值，直接入栈
        // PartialCodeList::emit(lit, 0, w_str2int(lexAnalyzer.strToken));
        lexAnalyzer.getToken();
    } else if (lexAnalyzer.sym == LPAREN) {
        lexAnalyzer.getToken();
        exp();
        if (lexAnalyzer.sym == RPAREN) {
            lexAnalyzer.getToken();
        } else {
            recover(0, follow_factor, "Missing ')'.");
        }
    } else {
        recover(0, follow_factor, "Illegal defined factor.");
    }
}

// <term> -> <factor>{[*|/] <factor>}
void SyntaxAnalyzer::term() {
    if (lexAnalyzer.sym & first_term) {
        factor();
        // factor()执行完毕后，当前栈顶即为factor的值
        while (lexAnalyzer.sym == MULTI || lexAnalyzer.sym == DIVIS) {
            const unsigned long nop = lexAnalyzer.sym;
            lexAnalyzer.getToken();
            if (lexAnalyzer.sym & first_term) {
                factor();
                // 乘
                // if (nop == MULTI)
                //     PartialCodeList::emit(opr, 0, OPR_MULTI);
                //     // 除
                // else
                //     PartialCodeList::emit(opr, 0, OPR_DIVIS);
            } else {
                Utils::log(ERROR, lexAnalyzer.line, "Redundant " + lexAnalyzer.strToken);
            }
        }
    } else {
        recover(0, follow_term, "Illegal defined term.");
    }
}

// <lexp> -> <exp> <lop> <exp> | odd <exp>
void SyntaxAnalyzer::lexp() {
    // <lexp> -> odd <exp>
    if (lexAnalyzer.sym == ODD_SYM) {
        lexAnalyzer.getToken();
        if (lexAnalyzer.sym & first_exp) {
            exp();
            // odd
            // PartialCodeList::emit(opr, 0, OPR_ODD);
        } else {
            Utils::log(ERROR, lexAnalyzer.line, "Expect expression");
        }
    }
    // <lexp> -> <exp> <lop> <exp>
    else if (lexAnalyzer.sym & first_exp) {
        exp();
        if (lexAnalyzer.sym & first_lop) {
            const unsigned int lop = lexAnalyzer.sym;
            lexAnalyzer.getToken();
            exp();
            // switch (lop) {
            //     // <
            //     case LSS:
            //         PartialCodeList::emit(opr, 0, OPR_LSS);
            //         break;
            //     // <=
            //     case LEQ:
            //         PartialCodeList::emit(opr, 0, OPR_LEQ);
            //         break;
            //     // >
            //     case GRT:
            //         PartialCodeList::emit(opr, 0, OPR_GRT);
            //         break;
            //     // >=
            //     case GEQ:
            //         PartialCodeList::emit(opr, 0, OPR_GEQ);
            //         break;
            //     // <>
            //     case NEQ:
            //         PartialCodeList::emit(opr, 0, OPR_NEQ);
            //         break;
            //     // =
            //     case EQL:
            //         PartialCodeList::emit(opr, 0, OPR_EQL);
            //         break;
            //     default:
            //         break;
            // }
        } else {
            recover(0, first_exp, "Missing '<' or '<=' or '>' or '>=' or '<>' or '='.");
            exp();
        }
    } else {
        recover(0, follow_lexp, "Illegal defined lexp.");
    }
}

void SyntaxAnalyzer::statement() {
    // <statement> -> id := <exp>
    if (lexAnalyzer.sym == IDENT) {
        // 查找当前变量是否在符号表中
        // const int pos = SymTable::lookUpVar(lexAnalyzer.strToken);
        // const VarInfo *curInfo = nullptr;
        // // 未查找到符号
        // if (pos == -1)
        //     Utils::log(ERROR, lexAnalyzer.line, "Undeclared " + lexAnalyzer.strToken);
        // else
        //     curInfo = static_cast<VarInfo *>(SymTable::table[pos].info);
        lexAnalyzer.getToken();
        if (lexAnalyzer.sym == ASSIGN) {
            // 查找到右值，右值不可被赋值
            // if (curInfo && curInfo->cat == CST)
            //     Utils::log(ERROR, lexAnalyzer.line, "ILLEGAL_RVALUE_ASSIGN");
            lexAnalyzer.getToken();
        } // 不是赋值号：=而是等于号=
        else if (lexAnalyzer.sym == EQL) {
            Utils::log(ERROR, lexAnalyzer.line, "Find '=' rather than ':='");
            lexAnalyzer.getToken();
        } else {
            // 跳过非法符号，直到遇到exp的follow集
            recover(0, follow_exp, "Missing ':='.");
        }
        exp();
        // if (curInfo)
        //     // 赋值的P代码，当前栈顶为计算出的表达式
        //     PartialCodeList::emit(store, curInfo->level,
        //                           curInfo->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + curInfo->level + 1);
    }
    // <statement> -> if <lexp> then <statement> [else <statement>]
    else if (lexAnalyzer.sym == IF_SYM) {
        lexAnalyzer.getToken();
        lexp();
        int entry_jpc = -1, entry_jmp = -1;

        // 当前栈顶为条件表达式的布尔值
        if (lexAnalyzer.sym == THEN_SYM) {
            // 条件为假跳转，待回填else入口地址或if外地址
            // entry_jpc = PartialCodeList::emit(jpc, 0, 0);
            lexAnalyzer.getToken();
        } else {
            recover(0, first_stmt, "Missing then.");
        }
        // <statement> -> if <lexp> then <statement>
        statement();

        if (lexAnalyzer.sym == ELSE_SYM) {
            // 待回填if外的入口地址
            // entry_jmp = PartialCodeList::emit(jmp, 0, 0);
            lexAnalyzer.getToken();
            // 将else入口地址回填至jpc
            // PartialCodeList::backpatch(entry_jpc, PartialCodeList::codeList.size());
            statement();
            //有else，则将if外入口地址回填至jmp
            // PartialCodeList::backpatch(entry_jmp, PartialCodeList::codeList.size());
        } else {
            // 没有else，则将if外入口地址回填至jpc
            // PartialCodeList::backpatch(entry_jpc, PartialCodeList::codeList.size());
        }
    }
    // <statement> -> while <lexp> do <statement>
    else if (lexAnalyzer.sym == WHILE_SYM) {
        lexAnalyzer.getToken();
        //FIRST(lexp);
        // const size_t condition = PartialCodeList::codeList.size();
        lexp();
        // 当前栈顶为条件表达式的布尔值
        // 条件为假跳转，待回填循环出口地址
        // const size_t loop = PartialCodeList::emit(jpc, 0, 0);
        if (lexAnalyzer.sym == DO_SYM) {
            lexAnalyzer.getToken();
            statement();
            // 无条件跳转至循环条件判断前
            // PartialCodeList::emit(jmp, 0, condition);
        } else {
            recover(0, first_stmt, "Missing do.");
        }
        // 将下一条语句回填至jpc
        // PartialCodeList::backpatch(loop, PartialCodeList::codeList.size());
    }
    // <statement> -> call id ([{<exp>{,<exp>}])
    else if (lexAnalyzer.sym == CALL_SYM) {
        lexAnalyzer.getToken();
        // const ProcInfo *curInfo = nullptr;
        // <statement> -> call id
        if (lexAnalyzer.sym == IDENT) {
            // 查找过程的符号名
            // const int pos = SymTable::lookUpProc(lexAnalyzer.strToken);
            // 未查找到过程名
            // if (pos == -1)
            //     Utils::log(ERROR, lexAnalyzer.line, "Undeclared " + lexAnalyzer.strToken);
            // else
            //     curInfo = dynamic_cast<ProcInfo *>(SymTable::table[pos].info);
            // // 若调用未定义的过程
            // if (curInfo && !curInfo->isDefined)
            //     Utils::log(ERROR, lexAnalyzer.line, "Undeclared " + lexAnalyzer.strToken);
            lexAnalyzer.getToken();
        } else {
            recover(0, LPAREN, "Find '" + lexAnalyzer.strToken + "' in identifier");
        }
        // <statement> -> call id (
        if (lexAnalyzer.sym == LPAREN) {
            lexAnalyzer.getToken();
        } else {
            recover(0, first_exp | RPAREN, "Missing '('.");
        }
        // <statement> -> call id ([{<exp>
        if (lexAnalyzer.sym & first_exp) {
            exp();
            // 将实参传入即将调用的子过程
            // if (curInfo)
            //     PartialCodeList::emit(store, -1, ACT_PRE_REC_SIZE + curInfo->level + 1);
            size_t i = 1;
            // <statement> -> call id ([{<exp>{,<exp>}]
            while (lexAnalyzer.sym == COMMA) {
                lexAnalyzer.getToken();
                if (lexAnalyzer.sym & first_exp) {
                    exp();
                    // 将实参传入即将调用的子过程
                    // if (curInfo)
                    //     PartialCodeList::emit(store, -1, ACT_PRE_REC_SIZE + curInfo->level + 1 + i++);
                } else {
                    recover(0, first_exp, "Redundant ','.");
                    exp();
                }
            }
            // 若实参列表与形参列表变量数不兼容，报错
            // if (curInfo && i != curInfo->form_var_list.size()) {
            //     Utils::log(ERROR, lexAnalyzer.line, "INCOMPATIBLE_VAR_LIST");
            // }
        }
        // <statement> -> call id ([{<exp>{,<exp>}])
        if (lexAnalyzer.sym == RPAREN) {
            lexAnalyzer.getToken();
            // // 调用子过程
            // if (curInfo)
            //     PartialCodeList::emit(call, curInfo->level, curInfo->entry);
        }
    }
    // <statement> -> <body>
    else if (lexAnalyzer.sym == BEGIN_SYM) {
        body();
    }
    // <statement> -> read (id{,id})
    else if (lexAnalyzer.sym == READ_SYM) {
        lexAnalyzer.getToken();
        if (lexAnalyzer.sym == LPAREN) {
            lexAnalyzer.getToken();
        } else {
            recover(0, IDENT, "Missing '('.");
        }
        // <statement> -> read (id
        if (lexAnalyzer.sym == IDENT) {
            // 查找变量符号
            // const int pos = SymTable::lookUpVar(lexAnalyzer.strToken);
            // 未查找到符号
            // const VarInfo *curInfo = nullptr;
            // if (pos == -1)
            //     Utils::log(ERROR, lexAnalyzer.line, "Undeclared " + lexAnalyzer.strToken);
            //     // 用临时变量记录当前查到的信息
            // else
            //     curInfo = dynamic_cast<VarInfo *>(SymTable::table[pos].info);
            // 右值不可被赋值
            // if (curInfo) {
            //     if (curInfo->cat == CST)
            //         Utils::log(ERROR, lexAnalyzer.line, "ILLEGAL_RVALUE_ASSIGN");
            //     // 从命令行读一个数据到栈顶
            //     PartialCodeList::emit(red, 0, 0);
            //     // 将栈顶值送入变量所在地址
            //     PartialCodeList::emit(store, curInfo->level,
            //                           curInfo->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + curInfo->level + 1);
            // }
            lexAnalyzer.getToken();
        } else {
            recover(0, COMMA | RPAREN, "Find '" + lexAnalyzer.strToken + "' in identifier");
        }
        // <statement> -> read (id{,
        while (lexAnalyzer.sym == COMMA) {
            lexAnalyzer.getToken();
            if (lexAnalyzer.sym == IDENT) {
                // const int pos = SymTable::lookUpVar(lexAnalyzer.strToken);
                // // 未查找到符号
                // const VarInfo *curInfo = nullptr;
                // if (pos == -1)
                //     Utils::log(ERROR, lexAnalyzer.line, "Undeclared " + lexAnalyzer.strToken);
                //     // 用临时变量记录当前查到的信息
                // else
                //     curInfo = dynamic_cast<VarInfo *>(SymTable::table[pos].info);
                // // 右值不可被赋值
                // if (curInfo) {
                //     if (curInfo->cat == CST)
                //         Utils::log(ERROR, lexAnalyzer.line, "ILLEGAL_RVALUE_ASSIGN");
                //     // 从命令行读一个数据到栈顶
                //     PartialCodeList::emit(red, 0, 0);
                //     // 将栈顶值送入变量所在地址
                //     PartialCodeList::emit(store, curInfo->level,
                //                           curInfo->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + curInfo->level + 1);
                // }
                lexAnalyzer.getToken();
            } else {
                recover(0, IDENT, "Redundant " + lexAnalyzer.strToken);
            }
        }
        if (lexAnalyzer.sym == RPAREN) {
            lexAnalyzer.getToken();
        } else {
            recover(0, follow_stmt, "Missing ')'.");
        }
    }
    // <statement> -> write(<exp> {,<exp>})
    else if (lexAnalyzer.sym == WRITE_SYM) {
        lexAnalyzer.getToken();
        // <statement> -> write(
        if (lexAnalyzer.sym == LPAREN) {
            lexAnalyzer.getToken();
        } else {
            recover(0, first_exp, "Missing '('.");
        }
        // <statement> -> write(<exp>
        exp();
        // PartialCodeList::emit(wrt, 0, 0);
        // <statement> -> write(<exp> {,<exp>}
        while (lexAnalyzer.sym == COMMA) {
            lexAnalyzer.getToken();
            if (lexAnalyzer.sym == RPAREN)
                Utils::log(ERROR, lexAnalyzer.line, "Redundant ','.");
            else {
                exp();
                // PartialCodeList::emit(wrt, 0, 0);
            }
        }
        // <statement> -> write(<exp> {,<exp>})
        if (lexAnalyzer.sym == RPAREN) {
            lexAnalyzer.getToken();
        } else {
            recover(0, follow_stmt, "Missing ')'.");
        }
    } else
        recover(0, follow_stmt, "Illegal defined statement.");
}

// <body> -> begin <statement> {;<statement>} end
void SyntaxAnalyzer::body() {
    // 判断是否存在begin,是否仅缺少begin
    recover(BEGIN_SYM, first_stmt, "Missing 'begin'.");
    // begin之后为新的作用域
    level++;
    statement();
    // 这里如果end前多一个分号会多进行一次循环，并进入else分支
    while (lexAnalyzer.sym & (SEMICOLON | COMMA | first_stmt)) {
        // 判断是否存在分号，是否仅缺少分号,是否错写为逗号
        if (lexAnalyzer.sym == COMMA) {
            Utils::log(ERROR, lexAnalyzer.line, "Find ';' rather than ','.");
            lexAnalyzer.getToken();
        } else
            recover(SEMICOLON, first_stmt, "Missing ';'.");
        if (lexAnalyzer.sym & first_stmt) {
            statement();
        } else {
            Utils::log(ERROR, lexAnalyzer.line, "Redundant ';'.");
        }
    }
    // 判断是否缺少end
    recover(END_SYM, 0, "Missing end.");
    // end之后作用域结束
    level--;
}


void SyntaxAnalyzer::analyze() {
    lexAnalyzer.getToken();
    prog();
    Utils::complete(1);
}
