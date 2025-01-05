//
// Created by l0v3ch4n on 25-1-4.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <string>

#include "PCode.h"

/*
两个存储器：存储器CODE，用来存放P的代码
            数据存储器STACK（栈）用来动态分配数据空间
四个寄存器：
一个指令寄存器I:存放当前要执行的代码
一个栈顶指示器寄存器T：指向数据栈STACK的栈顶
一个基地址寄存器B：存放当前运行过程的数据区在STACK中的起始地址
一个程序地址寄存器P：存放下一条要执行的指令地址
该假想机没有供运算用的寄存器。所有运算都要在数据栈STACK的栈顶两个单元之间进行，并用运算结果取代原来的两个运算对象而保留在栈顶。
*/

class Interpreter
{
public:
    static size_t pc; // 指令寄存器
    static size_t top; // 活动记录栈顶，并非实际开辟的空间栈顶
    static size_t sp; // 当前活动记录基地址
    static std::vector<int> running_stack; // 数据运行栈
    static std::string code_stack[MAX_STACK_SIZE]; // 代码存放栈

    explicit Interpreter(const std::string& pcode_path)
    {
        code_path = pcode_path;
    }

    static void clear();
    void runCode() const;

private:
    std::string code_path;
};


#endif //INTERPRETER_H
