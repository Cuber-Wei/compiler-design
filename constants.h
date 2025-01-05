//
// Created by l0v3ch4n on 24-12-12.
//

#ifndef CONSTANTS_H
#define CONSTANTS_H
// 源文件配置相关常量
#define TAB_WIDTH 4 // 制表符宽度
#define MAX_LINE_LENGTH 100 // 最大行长度
#define MAX_FILE_LINE 100 // 最大文件行数

// 词法分析器相关常量
#define RSV_WORD_MAX 15/* 保留字的数量 */
#define OPR_MAX 15

// 语法分析器相关常量
// 语法分析器报错退出错误种类
#define SYN_SUCCESS 0
#define SYN_MISS_PROG 1 // 找不到程序起始位置。
#define SYN_INV_DED 2 // 区块被错误地定义。
#define SYN_UNEXPECT_TOKEN 3 // 预期之外的Token。
#define SYN_UNEXPECT_EXPR 4 // 预期之外的表达式。
#define SYN_MISS_TOKEN 5 // 缺少必要的Token。
#define SYN_REDUNDANT_TOKEN 6 // 冗余的Token。
#define SYN_MISS_EXPR 7 // 缺少必要的表达式。
#define SYN_MISS_SEMICOLON 8 // 缺少分号。


// 符号表相关常量
#define VAR_WIDTH 4 /*变量大小*/
#define CST_WIDTH 4 /*常量大小*/
#define SYM_ITEMS_CNT 100 // 符号表最大项数

// 中间代码相关常量
#define P_CODE_CNT 10 // P-Code的种类数
#define UNIT_SIZE 4 // 一个内存单元的字节大小
#define ACT_PRE_REC_SIZE 3 // 活动记录的预先大小（RA、DL、全局Display）

#define OPR_RETURN 0
#define OPR_NEGTIVE 1
#define OPR_ADD 2
#define OPR_SUB 3
#define OPR_MUL 4
#define OPR_DIV 5
#define OPR_ODD 6
#define OPR_EQL 7
#define OPR_NEQ 8
#define OPR_LSS 9
#define OPR_GEQ 10
#define OPR_GRT 11
#define OPR_LEQ 12
#define OPR_PRINT 13
#define OPR_PRINTLN 14

// 解释器相关常量
#define RA 0
#define DL 1
#define GLO_DIS 2
#define DISPLAY 3
#define MAX_STACK_SIZE 200 // 栈的最大大小

enum class token_type
{
    IDENTIFY = 1, // 标识符
    RSV_WORD = 2, // 保留字
    NUMBER = 3, // 数字
    OPCODE = 4, // 操作符
    END = 5, // 结束符
    ERROR = -1, // 错误
    NUL = 0 // 空
};

enum class identifier_type
{
    PROCEDURE = 1, // 过程
    CONSTANT = 2, // 常量
    VARIABLE = 3, // 变量
    PARAMETER = 4, // 参数
    NUMBER = 5, // 数字
    ARRAY = 6, // 数组
    NUL = 0, // 初始化
};

enum class opr_type
{
    ADD = 1, // +
    SUB = 2, // -
    MUL = 3, // *
    DIV = 4, // /
    EQ = 5, // =
    NEQ = 6, // <>
    LT = 7, // <
    GT = 8, // >
    LEQ = 9, // <=
    GEQ = 10, // >=
    ASSIGN = 11, // :=
    LEFT_BRACKET = 12, // (
    RIGHT_BRACKET = 13, // )
    COMMA = 14, // ,
    SEMICOLON = 15 // ;
};

enum class rsv_word_type
{
    // 保留字
    ODD = 1, // odd
    BEGIN = 2, // begin
    END = 3, // end
    IF = 4, // if
    THEN = 5, // then
    ELSE = 6, // else
    WHILE = 7, // while
    DO = 8, // do
    CALL = 9, // call
    CONST = 10, // const
    VAR = 11, // var
    PROCEDURE = 12, // procedure
    WRITE = 13, // write
    READ = 14, // read
    PROGRAM = 15, // program
};

// 中间代码指令集
enum Operation
{
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
    nil // 无操作
};

inline std::string token_type_to_string(const token_type type)
{
    switch (type)
    {
    case token_type::IDENTIFY:
        return "Identifier";
    case token_type::RSV_WORD:
        return "ReserveWord";
    case token_type::NUMBER:
        return "Number";
    case token_type::OPCODE:
        return "Operator";
    case token_type::END:
        return "EOF";
    case token_type::ERROR:
        return "InvalidToken";
    case token_type::NUL:
        return "NULL";
    default:
        return "";
    }
}

inline std::string identifier_type_to_string(const identifier_type type)
{
    switch (type)
    {
    case identifier_type::PROCEDURE:
        return "Procedure";
    case identifier_type::VARIABLE:
        return "Variable";
    case identifier_type::CONSTANT:
        return "Constant";
    case identifier_type::PARAMETER:
        return "Parameter";
    case identifier_type::NUMBER:
        return "Number";
    case identifier_type::ARRAY:
        return "Array";
    case identifier_type::NUL:
        return "Null";
    default:
        return "";
    }
}

inline std::string opr_type_to_string(const opr_type type)
{
    switch (type)
    {
    case opr_type::ADD:
        return "Add";
    case opr_type::SUB:
        return "Sub";
    case opr_type::MUL:
        return "Multiply";
    case opr_type::DIV:
        return "Divide";
    case opr_type::EQ:
        return "Equal";
    case opr_type::NEQ:
        return "NotEqual";
    case opr_type::LT:
        return "LowerThan";
    case opr_type::GT:
        return "GreaterThan";
    case opr_type::LEQ:
        return "LowerEqual";
    case opr_type::GEQ:
        return "GreaterEqual";
    case opr_type::ASSIGN:
        return "Assign";
    case opr_type::LEFT_BRACKET:
        return "LeftBracket";
    case opr_type::RIGHT_BRACKET:
        return "RightBracket";
    case opr_type::COMMA:
        return "Comma";
    case opr_type::SEMICOLON:
        return "Semicolon";
    default: return "";
    }
}

inline std::string rsv_word_type_to_string(const rsv_word_type type)
{
    switch (type)
    {
    case rsv_word_type::ODD:
        return "Odd";
    case rsv_word_type::BEGIN:
        return "Begin";
    case rsv_word_type::END:
        return "End";
    case rsv_word_type::IF:
        return "If";
    case rsv_word_type::THEN:
        return "Then";
    case rsv_word_type::ELSE:
        return "Else";
    case rsv_word_type::WHILE:
        return "While";
    case rsv_word_type::DO:
        return "Do";
    case rsv_word_type::CALL:
        return "Call";
    case rsv_word_type::CONST:
        return "Const";
    case rsv_word_type::VAR:
        return "Var";
    case rsv_word_type::PROCEDURE:
        return "Procedure";
    case rsv_word_type::WRITE:
        return "Write";
    case rsv_word_type::READ:
        return "Read";
    case rsv_word_type::PROGRAM:
        return "Program";
    default:
        return "";
    }
}

#endif //CONSTANTS_H
