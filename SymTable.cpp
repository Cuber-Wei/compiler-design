//
// Created by l0v3ch4n on 25-1-4.
//

#include "SymTable.h"

#include <iomanip>
#include <iostream>

#include "constants.h"
#include "Utils.h"
#include "variables.h"

size_t SymTable::sp = 0;
std::vector<SymTableItem> SymTable::table; // 一个程序唯一的符号表
std::vector<size_t> SymTable::display{0}; // 过程的嵌套层次表

Information::Information()
{
    this->offset = 0;
    this->cat = identifier_type_to_string(identifier_type::NUL);
    this->level = 0;
}

void Information::show()
{
    std::cout << std::setw(10) << "catalog: " << std::setw(13) << cat
        << std::setw(10) << "offset: " << std::setw(5) << offset
        << std::setw(10) << "level: " << std::setw(5) << level;
}

VarInfo::VarInfo()
{
    this->value = 0;
    this->type = identifier_type_to_string(identifier_type::NUMBER);
}

void VarInfo::setValue(const std::string& val_str) { this->value = std::stoi(val_str); }

int VarInfo::getValue() { return this->value; }

void VarInfo::show()
{
    std::cout << "\033[1;33m" << std::setw(15) << cat << "\033[0m"
        << "\033[1;34m" << std::setw(10) << "offset:" << std::setw(5) << offset << "\033[0m"
        << "\033[1;35m" << std::setw(10) << level << "\033[0m"
        << "\033[1;36m" << std::setw(10) << "value:" << std::setw(5);
    if (cat == identifier_type_to_string(identifier_type::CONSTANT))
        std::cout << value << "\033[0m";
    else
        std::cout << "null" << "\033[0m";
}

ProcInfo::ProcInfo()
{
    this->entry = 0;
    this->isDefined = false;
}

void ProcInfo::setEntry(const size_t entry) { this->entry = entry; }

size_t ProcInfo::getEntry() { return this->entry; }

void ProcInfo::show()
{
    std::cout << "\033[1;33m" << std::setw(15) << cat << "\033[0m"
        << "\033[1;34m" << std::setw(10) << "size:" << std::setw(5) << offset << "\033[0m"
        << "\033[1;35m" << std::setw(10) << level << "\033[0m"
        << "\033[1;36m" << std::setw(10) << "entry:" << std::setw(5) << entry << "\033[0m"
        << "\033[1;37m" << "\t\t";
    if (form_var_list.empty()) // 参数列表为空
        std::cout << " null";
    for (const size_t mem : form_var_list)
        std::cout << " " << SymTable::table[mem].name;
    std::cout << "\033[0m";
}

void SymTableItem::show() const
{
    Utils::info_with_no_endl("");
    std::cout << "\033[1;31m" << std::setw(10) << name << "\033[0m"
        << "\033[1;32m" << std::setw(10) << pre_item << "\033[0m";
    info->show();
    std::cout << std::endl;
}

void SymTable::mkTable()
{
    sp = table.size();
}

size_t SymTable::enter(const std::string& name, const size_t offset, const std::string& cat)
{
    // 如果查找到重复符号，且必须在同一层级，不为形参、过程名，则说明出现变量名重定义
    if (const size_t pos = lookUpVar(name); pos != -1 && table[pos].info->level == level)
        return -1;
    // 记录当前即将登入的符号表项的地址
    const size_t cur_addr = table.size();
    SymTableItem item;
    // 当前符号表项的前一项是display[level]
    item.pre_item = display[level];
    item.name = name;
    // 更新display[level]为当前符号表项的地址
    display[level] = cur_addr;
    auto* varInfo = new VarInfo;
    varInfo->offset = offset;
    varInfo->cat = cat;
    varInfo->level = level;
    varInfo->value = 0;
    item.info = varInfo;
    table.push_back(item);
    // 返回当前符号表项的地址
    return cur_addr;
}

size_t SymTable::enterProc(const std::string& name)
{
    // 若查找到重复符号，且为同一层级的过程名，则出现过程重定义
    if (const size_t pos = lookUpProc(name); pos != -1 && table[pos].info->level == level + 1)
        return -1;
    const size_t cur_addr = table.size();
    SymTableItem item;
    // 当前符号表项的前一项是display[level]
    item.pre_item = display[level];
    item.name = name;
    // 更新display[level]为当前符号表项的地址
    display[level] = cur_addr;
    auto* procInfo = new ProcInfo;
    procInfo->offset = 0;
    procInfo->cat = identifier_type_to_string(identifier_type::PROCEDURE);
    procInfo->level = level + 1;
    procInfo->entry = 0;
    item.info = procInfo;
    table.push_back(item);
    // 返回当前符号表项的地址
    return cur_addr;
}

void SymTable::enterProgram(const std::string& name)
{
    SymTableItem item;
    item.pre_item = 0;
    item.name = name;
    auto* procInfo = new ProcInfo;
    procInfo->offset = 0;
    procInfo->cat = identifier_type_to_string(identifier_type::PROCEDURE);
    procInfo->level = 0;
    item.info = procInfo;
    table.push_back(item);
}

size_t SymTable::lookUpProc(const std::string& name)
{
    size_t curAddr = 0;
    // i代表访问display的指针
    // 若查找主过程名，直接返回-1
    if (level == 0 && display[0] == 0)
        return -1;
    for (int i = level; i >= 0; i--)
    {
        curAddr = display[i];
        // 遍历当前display指针指向的过程下的所有过程符号，直到遇到最后一个符号(pre == 0)
        while (true)
        {
            if (table[curAddr].info->cat == identifier_type_to_string(identifier_type::PROCEDURE)
                && table[curAddr].name == name)
                return curAddr;
            if (table[curAddr].pre_item == 0)
                break;
            curAddr = table[curAddr].pre_item;
        }
    }
    return -1;
}

size_t SymTable::lookUpVar(const std::string& name)
{
    size_t curAddr = 0;
    // i代表访问display的指针
    // 若查找主过程名，直接返回-1
    if (display.empty())
        return -1;
    if (level == 0 && display[0] == 0)
        return -1;
    for (int i = level; i >= 0; i--)
    {
        // 由内向外查询名称
        curAddr = display[i];
        if (curAddr >= table.size()) // 防止越界
            break;
        // 遍历当前display指针指向的过程下的所有变量符号，直到遇到最后一个符号(pre == 0)
        while (true)
        {
            if (table[curAddr].info->cat != identifier_type_to_string(identifier_type::PROCEDURE) && table[curAddr].name
                == name)
                return curAddr;
            if (table[curAddr].pre_item == 0)
                break;
            curAddr = table[curAddr].pre_item;
        }
    }
    return -1;
}

void SymTable::addWidth(const size_t addr, const size_t width)
{
    table[addr].info->offset = width;
    glo_offset = 0;
}

void SymTable::clear()
{
    sp = 0;
    table.clear();
    display.clear();
    table.reserve(SYM_ITEMS_CNT);
    display.resize(1, 0);
}

void showSymTable()
{
    Utils::info("======================================  Symbol Table  ======================================");
    // 表头
    std::cout << "\033[1;34m[INFO]\t\t\033[0m"
        << "\033[1;31m" << std::setw(10) << "Name" << "\033[0m"
        << "\033[1;32m" << std::setw(10) << "PreItem" << "\033[0m"
        << "\033[1;33m" << std::setw(15) << "Type" << "\033[0m"
        << "\033[1;34m" << std::setw(15) << "SIze/Offset" << "\033[0m"
        << "\033[1;35m" << std::setw(10) << "Level" << "\033[0m"
        << "\033[1;36m" << std::setw(15) << "Entry/Value" << "\033[0m"
        << "\033[1;37m" << std::setw(15) << "Parameters" << "\033[0m"
        << std::endl;
    for (const SymTableItem& mem : SymTable::table)
        mem.show();
    Utils::info("============================================================================================");
}
