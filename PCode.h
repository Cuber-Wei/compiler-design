//
// Created by l0v3ch4n on 25-1-4.
//

#ifndef PCODE_H
#define PCODE_H
#include <vector>

#include "constants.h"

/*
LIT 0 ，a 取常量a放入数据栈栈顶
OPR 0 ，a 执行运算，a表示执行某种运算
LOD L ，a 取变量（相对地址为a，层差为L）放到数据栈的栈顶
STO L ，a 将数据栈栈顶的内容存入变量（相对地址为a，层次差为L）
CAL L ，a 调用过程（转子指令）（入口地址为a，层次差为L）
INT 0 ，a 数据栈栈顶指针增加a
JMP 0 ，a无条件转移到地址为a的指令
JPC 0 ，a 条件转移指令，转移到地址为a的指令
RED L ，a 读数据并存入变量（相对地址为a，层次差为L）
WRT 0 ，0 将栈顶内容输出
代码的具体形式：

F   L   A

其中：F段代表伪操作码
	  L段代表调用层与说明层的层差值
      A段代表位移量（相对地址）
进一步说明：
INT：为被调用的过程（包括主过程）在运行栈S中开辟数据区，这时A段为所需数据单元个数（包括三个连接数据）；L段恒为0。
CAL：调用过程，这时A段为被调用过程的过程体（过程体之前一条指令）在目标程序区的入口地址。
LIT：将常量送到运行栈S的栈顶，这时A段为常量值。
LOD：将变量送到运行栈S的栈顶，这时A段为变量所在说明层中的相对位置。
STO：将运行栈S的栈顶内容送入某个变量单元中，A段为变量所在说明层中的相对位置。
JMP：无条件转移，这时A段为转向地址（目标程序）。
JPC：条件转移，当运行栈S的栈顶的布尔值为假（0）时，则转向A段所指目标程序地址；否则顺序执行。
OPR：关系或算术运算，A段指明具体运算，例如A=2代表算术运算“＋”；A＝12代表关系运算“>”；A＝16代表“读入”操作等等。运算对象取自运行栈S的栈顶及次栈顶。
 */
class Instruction
{
public:
    Operation op; // 伪操作码
    int L; // 层级
    int a; // 相对地址
    Instruction(const Operation op, const int L, const int a)
    {
        this->op = op;
        this->L = L;
        this->a = a;
    };
};

class PCode
{
public:
    static std::vector<Instruction> codeList;

    static size_t emit(Operation op, int L, int a);
    static void backpatch(size_t target, size_t addr);
    static void printCode(bool is_to_file);
    static void clear();
};


#endif //PCODE_H
