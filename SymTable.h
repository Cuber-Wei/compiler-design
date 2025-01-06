//
// Created by l0v3ch4n on 25-1-4.
//

#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <string>
#include <vector>


// 父类信息类型
class Information
{
public:
    virtual ~Information() = default;

    std::string cat; // 标识符种属
    size_t offset;
    size_t level;

    Information();

    virtual void setValue(const std::string& val_str)
    {
    }

    virtual int getValue() { return 0; }

    virtual void setEntry(size_t entry)
    {
    }

    virtual size_t getEntry() { return -1; }

    virtual void show();
};

// 变量信息，继承信息类型
class VarInfo final : public Information
{
public:
    std::string type; // 类型
    int value; // 数值

    VarInfo();

    void setValue(const std::string& val_str) override;

    int getValue() override;

    void show() override;
};

// 过程信息，继承信息类型
class ProcInfo final : public Information
{
public:
    bool isDefined; // 过程是否定义的标识
    size_t entry; // 过程的中间代码入口地址
    std::vector<size_t> form_var_list; // 过程的形参入口地址列表

    ProcInfo();

    void show() override;

    void setEntry(size_t entry) override;

    size_t getEntry() override;
};

// 符号表项
class SymTableItem
{
public:
    size_t pre_item;
    Information* info;
    std::string name; // 符号名
    void show() const;
};

// 符号表
class SymTable
{
public:
    static size_t sp; // 指向当前子过程符号表的首地址
    static std::vector<SymTableItem> table; // 一个程序唯一的符号表
    static std::vector<size_t> display; // 过程的嵌套层次表

    SymTable()
    {
        clear();
    }

    // 创建子符号表
    static void mkTable();

    // 将变量名登入符号表
    static size_t enter(const std::string& name, size_t offset, const std::string& cat);

    static void addWidth(size_t addr, size_t width);

    // 将过程名登入符号表
    static size_t enterProc(const std::string& name);

    static void enterProgram(const std::string& name);

    // 查找符号在符号表中位置
    static size_t lookUpVar(const std::string& name);

    static size_t lookUpProc(const std::string& name);

    // 清空符号表
    static void clear();
};

void showSymTable();

#endif //SYMTABLE_H
