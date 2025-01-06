//
// Created by l0v3ch4n on 25-1-4.
//

#include "Interpreter.h"

#include <fstream>
#include <iostream>

#include "Utils.h"

size_t Interpreter::pc = 0; // 指令寄存器
size_t Interpreter::top = 0; // 活动记录栈顶，并非实际开辟的空间栈顶
size_t Interpreter::sp = 0; // 当前活动记录基地址
std::vector<int> Interpreter::running_stack; // 数据运行栈
std::string Interpreter::code_stack[MAX_STACK_SIZE]; // 代码存放栈

void Interpreter::runCode() const
{
    std::ifstream code_file(code_path);
    std::string line;
    int line_num = 0;
    if (!code_file.is_open())
    {
        Utils::error("代码文件打开失败！");
        return;
    }
    while (std::getline(code_file, line))
    {
        code_stack[line_num++] = line;
    }
    code_file.close();
    for (size_t linno = 0; linno < line_num - 1; linno = pc)
    {
        std::string each_line = code_stack[linno];
        if (each_line.empty())
            break;
        // 分割指令
        std::string op = each_line.substr(each_line.find_first_of(' ') + 1, 3);
        int l = std::stoi(each_line.substr(each_line.find_first_of(' ') + 5,
                                           each_line.find_last_of(' ') - each_line.find_first_of(' ') - 5));
        int a = std::stoi(each_line.substr(each_line.find_last_of(' ') + 1));
        // Utils::info("当前代码行数 " + std::to_string(linno) + ": " + each_line);
        // Utils::warning("参数 l = " + std::to_string(l) + ", a = " + std::to_string(a));
        // 匹配指令模式
        if (op == "LIT")
        {
            // LIT   0 a	将常数值取到栈顶，a为常数值
            // 内存栈顶等于数据栈顶，需要额外开辟空间
            if (top == running_stack.size())
                running_stack.push_back(a);
                // 内存栈顶大于数据栈顶，直接赋值
            else
                running_stack[top] = a;
            top++;
            pc++;
        }
        else if (op == "LOD")
        {
            // LOD  l a	将变量值取到栈顶，a为偏移量，l为层差
            // 根据层级和偏移量，查找display表
            // running_stack[sp + DISPLAY + L]即指定层级L的活动记录基地址
            // 内存栈顶等于数据栈顶，需要额外开辟空间
            if (top == running_stack.size())
                running_stack.push_back(running_stack[running_stack[sp + DISPLAY + l] + a]);
                // 内存栈顶大于数据栈顶，直接赋值
            else
                running_stack[top] = running_stack[running_stack[sp + DISPLAY + l] + a];
            top++;
            pc++;
        }
        else if (op == "STO")
        {
            //STO  l a	将栈顶内容送入某变量单元中，a为偏移量，l为层差
            if (l >= 0)
            {
                // 根据层级和偏移量，查找display表
                // running_stack[sp + DISPLAY + L]即指定层级L的活动记录基地址
                running_stack[running_stack[sp + DISPLAY + l] + a] = running_stack[top - 1];
                top--;
            }
            // L为-1，说明这是形参传递的代码，需要预先开辟空间
            else
            {
                size_t cur_size = running_stack.size();
                // 取出栈顶值(top-1处)
                int val = running_stack[top - 1];
                top--;
                // 预先开辟空间，个数为a+1-（cur_size-top）（当前已额外开辟的空间）
                for (int ii = static_cast<int>(cur_size - top); ii <= a; ii++)
                    running_stack.push_back(0);
                // 将形参传递至指定位置
                running_stack[top + a] = val;
            }
            pc++;
        }
        else if (op == "CAL")
        {
            // CAL  l a	调用过程，a为过程地址，l为层差
            // 保存断点
            running_stack[top + RA] = static_cast<int>(pc + 1);
            // 复制全局display的L+1个单元到即将开辟的活动记录
            // running_stack[sp + GLO_DIS]表示当前全局display表的基地址
            // top + DISPLAY表示即将开辟的活动记录的display表基地址
            for (int i = 0; i <= l; i++)
                running_stack[top + DISPLAY + i] = running_stack[running_stack[sp + GLO_DIS] + i];
            // 第L+1个单元是即将开辟的活动记录的基地址
            running_stack[top + DISPLAY + l] = static_cast<int>(top);
            // 记录老sp，并调整sp到即将开辟的活动记录
            running_stack[top + DL] = static_cast<int>(sp);
            sp = top;
            // 跳转
            pc = a;
        }
        else if (op == "INT")
        {
            // INT   0 a	在运行栈中为被调用的过程开辟a个单元的数据区
            size_t cur_size = running_stack.size();
            // 若当前额外空间满足要求，直接移动数据栈顶指针
            if (a <= cur_size - top)
                top += a;
            else
            {
                // 开辟空间时减去已经额外开辟的空间
                for (int i = 0; i < a - (cur_size - top); i++)
                    running_stack.push_back(0);
                // 内存栈顶与数据栈顶对齐
                top = running_stack.size();
            }
            // 将新的display地址送到新的活动记录中的全局display处
            running_stack[sp + GLO_DIS] = static_cast<int>(sp) + DISPLAY;
            pc++;
        }
        else if (op == "JMP")
        {
            // JMP  0 a	无条件跳转至a地址
            pc = a;
        }
        else if (op == "JPC")
        {
            // JPC  0 a	条件跳转，当栈顶布尔值非真则跳转至a地址，否则顺序执行
            // 栈顶条件为假
            if (!running_stack[top - 1])
                pc = a;
                // 栈顶条件为真
            else
                pc++;
            top--;
        }
        else if (op == "WRT")
        {
            // WRT 0 0	输出栈顶
            Utils::output(running_stack[top - 1]);
            top--;
            pc++;
        }
        else if (op == "RED")
        {
            // RED 0  0	从命令行读入一个输入置于栈顶
            int tmp_value;
            Utils::input_prompt();
            std::cin >> tmp_value;
            // 数据入栈
            if (top == running_stack.size())
                running_stack.push_back(tmp_value);
            else
                running_stack[top] = tmp_value;
            top++;
            pc++;
        }
        else if (op == "OPR")
        {
            int res, old_sp;
            switch (a)
            {
            case 0:
                // OPR 0 0	过程调用结束后,返回调用点并退栈
                // 恢复断点，此处-1是因为这个函数末尾有个+1，debug了半天才发现
                pc = running_stack[sp + RA] - 1;
                old_sp = running_stack[sp + DL];
            // top指针还原到上一个活动记录位置
                top -= top - sp;
            // 恢复老sp
                sp = old_sp;
                break;
            case 1:
                // OPR 0 1	栈顶元素取反(反码 + 1)
                running_stack[top - 1] = ~running_stack[top - 1] + 1;
                break;
            case 2:
                // OPR 0 2	次栈顶与栈顶相加，退两个栈元素，结果值进栈
                res = running_stack[top - 2] + running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 3:
                // OPR 0 3	次栈顶减去栈顶，退两个栈元素，结果值进栈
                res = running_stack[top - 2] - running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 4:
                // OPR 0 4	次栈顶乘以栈顶，退两个栈元素，结果值进栈
                res = running_stack[top - 2] * running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 5:
                // OPR 0 5	次栈顶除以栈顶，退两个栈元素，结果值进栈
                res = running_stack[top - 2] / running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 6:
                // OPR 0 6	栈顶元素的奇偶判断，结果值在栈顶
                running_stack[top - 1] = (running_stack[top - 1] & 0b1) == 1;
                break;
            case 7:
                // OPR 0 7	次栈顶与栈顶是否相等，退两个栈元素，结果值进栈
                res = running_stack[top - 2] == running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 8:
                // OPR 0 8	次栈顶与栈顶是否不等，退两个栈元素，结果值进栈
                res = running_stack[top - 2] != running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 9:
                // OPR 0 9	次栈顶是否小于栈顶，退两个栈元素，结果值进栈
                res = running_stack[top - 2] < running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 10:
                // OPR 0 10	次栈顶是否小于等于栈顶，退两个栈元素，结果值进栈
                res = running_stack[top - 2] <= running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 11:
                // OPR 0 11	次栈顶是否大于栈顶，退两个栈元素，结果值进栈
                res = running_stack[top - 2] > running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 12:
                // OPR 0 12	次栈顶是否大于等于栈顶，退两个栈元素，结果值进栈
                res = running_stack[top - 2] >= running_stack[top - 1];
            // 直接在次栈顶赋值
                running_stack[top - 2] = res;
                top--;
                break;
            case 13:
                // OPR 0 13	屏幕输出换行
                std::cout << std::endl;
                break;
            default: break;
            }
            pc++;
        }
    }
}

void Interpreter::clear()
{
    running_stack.clear();
    sp = 0;
    top = 0;
    pc = 0;
}
