//
// Created by l0v3ch4n on 24-11-18.
//

#ifndef CONSTANTS_H
#define CONSTANTS_H
/*
 * 定义程序会用到的各种常量
 */
#define NUL 0x0 /* 空 */
#define EQL 0x1 /* =  1*/
#define NEQ 0x2 /* <> 2*/
#define LSS 0x4 /* < 4*/
#define LEQ 0x8 /* <= 8*/
#define GRT 0x10 /* > 16*/
#define GEQ 0x20 /* >= 32*/
#define ODD_SYM 0x40 /* 奇数判断 64*/
#define IDENT 0x80 /* 标识符 */
#define NUMBER 0x100 /* 数值 */
#define PLUS 0x200 /* + */
#define MINUS 0x400 /* - */
#define MULTI 0x800 /* * */
#define DIVIS 0x1000 /* / */
#define LPAREN 0x2000 /* ( */
#define RPAREN 0x4000 /* ) */
#define COMMA 0x8000 /* , */
#define SEMICOLON 0x10000 /* ; */
#define ASSIGN 0x20000 /*:=*/

#define BEGIN_SYM 0x40000
#define END_SYM 0x80000
#define IF_SYM 0x100000
#define THEN_SYM 0x200000
#define WHILE_SYM 0x400000
#define DO_SYM 0x800000
#define CALL_SYM 0x1000000
#define CONST_SYM 0x2000000
#define VAR_SYM 0x4000000
#define PROC_SYM 0x8000000
#define WRITE_SYM 010000000
#define READ_SYM 0x20000000
#define PROGM_SYM 0x40000000
#define ELSE_SYM 0x80000000
//错误种类宏定义
#define EXPECT_STH_FIND_ANTH 0
#define EXPECT 1
#define EXPECT_NUMEBR_AFTER_BECOMES 2
#define ILLEGAL_DEFINE 3
#define ILLEGAL_WORD 4
#define ILLEGAL_RVALUE_ASSIGN 5
#define MISSING 6
#define REDUNDENT 7
#define UNDECLARED_IDENT 8
#define UNDECLARED_PROC 9
#define REDECLEARED_IDENT 10
#define REDECLEARED_PROC 11
#define INCOMPATIBLE_VAR_LIST 12
#define UNDEFINED_PROC 13
//解释器
#define RA 0
#define DL 1
#define GLO_DIS 2
#define DISPLAY 3
//中间代码
#define P_CODE_CNT 10 // P-Code的种类数
#define UNIT_SIZE 4 // 一个内存单元的字节大小
#define ACT_PRE_REC_SIZE 3 // 活动记录的预先大小（RA、DL、全局Display）

#define OPR_RETURN 0
#define OPR_NEGTIVE 1
#define OPR_ADD 2
#define OPR_SUB 3
#define OPR_MULTI 4
#define OPR_DIVIS 5
#define OPR_ODD 6
#define OPR_EQL 7
#define OPR_NEQ 8
#define OPR_LSS 9
#define OPR_GEQ 10
#define OPR_GRT 11
#define OPR_LEQ 12
#define OPR_PRINT 13
#define OPR_PRINTLN 14

#define RSV_WORD_MAX 15 /* 保留字的数量 */
#define N_MAX 14 /* 数字允许的最长位数 */
#define PROGRAM_CH_MAX 5000 /*源程序的最大字符数*/
#define PROGRAM_TK_MAX 1000 /*源程序的最大token数*/
#define PROGRAM_LINE_MAX 50 /*源程序的最大token数*/
#define ID_MAX 20 /* 标识符最大长度 */
#define OPR_MAX 11 /* 运算符号表最大长度*/
#define VAR_WIDTH 4 /*变量大小*/
#define CST_WIDTH 4 /*常量大小*/
#define SYM_ITEMS_CNT 100 // 符号表最大项数
#define PROC_CNT 40 // 过程嵌套数
#define ERR_CNT 70 // 报错种数
#include <map>

// first集
inline unsigned long first_block = CONST_SYM | VAR_SYM | PROC_SYM | BEGIN_SYM;
inline unsigned long first_stmt = IDENT | IF_SYM | WHILE_SYM | CALL_SYM | BEGIN_SYM | READ_SYM | WRITE_SYM;
inline unsigned long first_factor = IDENT | NUMBER | LPAREN;
inline unsigned long first_term = first_factor;
inline unsigned long first_exp = first_term | PLUS | MINUS;
inline unsigned long first_lexp = first_exp | ODD_SYM;
inline unsigned long first_lop = EQL | NEQ | LSS | LEQ | GRT | GEQ;

// follow集
inline unsigned long follow_condecl = VAR_SYM | PROC_SYM | BEGIN_SYM;
inline unsigned long follow_vardecl = PROC_SYM | BEGIN_SYM;
inline unsigned long follow_body = SEMICOLON | END_SYM | ELSE_SYM;
inline unsigned long follow_stmt = END_SYM | SEMICOLON | ELSE_SYM;
inline unsigned long follow_lexp = THEN_SYM | DO_SYM;
inline unsigned long follow_exp = follow_stmt | COMMA | RPAREN | first_lop | follow_lexp;
inline unsigned long follow_term = follow_exp | PLUS | MINUS;
inline unsigned long follow_factor = MULTI | DIVIS | follow_term;
inline unsigned long follow_ident = COMMA | SEMICOLON | LPAREN | RPAREN | follow_factor;
inline unsigned long follow_block = NUL | SEMICOLON;
inline unsigned long follow_constdef = COMMA | SEMICOLON;
inline unsigned long follow_proc = BEGIN_SYM;

enum Type { INTEGER };

enum Category {
    NIL, // 空
    ARR, // 数组
    VAR, // 变量
    PROCEDURE, // 过程
    CST, // 常量
    FORM, // 形参
};

// 中间代码指令集
enum Operation {
    lit, // 取常量a放入数据栈栈顶
    opr, // 执行运算，a表示执行某种运算
    load, // 取变量（相对地址为a，层差为L）放到数据栈的栈顶
    store, // 将数据栈栈顶的内容存入变量（相对地址为a，层次差为L）
    call, // 调用过程（转子指令）（入口地址为a，层次差为L）
    alloc, // 数据栈栈顶指针增加a
    jmp, // 条件转移到地址为a的指令
    jpc, // 条件转移指令，转移到地址为a的指令
    red, // 读数据并存入变量（相对地址为a，层次差为L）
    wrt, // 将栈顶内容输出
};

//文件出错
enum fileErrors {
    OPEN_FILE_ERROR = 1 << 0, //打开文件出错
    EMPTY_FILE_ERROR = 1 << 1, //阅读文件出错：空文件
};

//日志级别
enum logLevel {
    INFO = 0, //提示
    WARNING = 1, //警告
    ERROR = 2, //错误
    SUCCESS = 3 //成功
};

#endif //CONSTANTS_H
