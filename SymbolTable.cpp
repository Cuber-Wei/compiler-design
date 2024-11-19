//
// Created by l0v3ch4n on 24-11-18.
//

#include "SymbolTable.h"
#include "constants.h"
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "utils.h"
#include "variables.h"

size_t SymTable::sp = 0;
std::vector<SymTableItem> SymTable::table; // 一个程序唯一的符号表
std::vector<size_t> SymTable::display; // 过程的嵌套层次表

std::string cat_map[6] = {
    "null",
    "array",
    "var",
    "procedure",
    "const",
    "formal var"
};
// std::string 转 int
int w_str2int(std::string num_str) {
    if (num_str.empty()) {
        std::cout << "Cannot transfer empty string!" << std::endl;
        return 0;
    }
    int num = 0;
    // 先遍历一遍字符串，判断合法性
    const size_t size = num_str.size();
    for (const char ch: num_str) {
        if (!(ch <= '9' && ch >= '0')) {
            std::cout << "Illegal string to transfer!" << std::endl;
            return 0;
        }
    }
    for (size_t i = 0; i < size; i++) {
        num = (num << 3) + (num << 1); // num*10
        num += (num_str[i] - '0');
    }
    return num;
}

Information::Information() {
    this->offset = 0;
    this->cat = NIL;
    this->level = 0;
}

void Information::show() {
    std::cout << std::setw(10) << "cat: " << std::setw(13) << cat_map[cat]
            << std::setw(10) << "offset: " << std::setw(5) << offset
            << std::setw(10) << "level: " << std::setw(5) << level;
}

VarInfo::VarInfo() {
    this->value = 0;
    this->type = INTEGER;
}

void VarInfo::setValue(const std::string &val_str) { this->value = w_str2int(val_str); }

int VarInfo::getValue() { return this->value; }

void VarInfo::show() {
    std::cout << std::setw(10) << "cat:" << std::setw(15) << cat_map[cat]
            << std::setw(10) << "offset:" << std::setw(5) << offset
            << std::setw(10) << "level:" << std::setw(5) << level
            << std::setw(10) << "value:" << std::setw(5) << value;
}

ProcInfo::ProcInfo() {
    this->entry = 0;
    this->isDefined = false;
}

void ProcInfo::setEntry(const size_t entry) { this->entry = entry; }

size_t ProcInfo::getEntry() { return this->entry; }

void ProcInfo::show() {
    std::cout << std::setw(10) << "cat:" << std::setw(15) << cat_map[cat]
            << std::setw(10) << "size:" << std::setw(5) << offset
            << std::setw(10) << "level:" << std::setw(5) << level
            << std::setw(10) << "entry:" << std::setw(5) << entry
            << std::setw(17) << "form var list:";
    if (form_var_list.empty())
        std::cout << std::setw(5) << "null";
    for (const size_t mem: form_var_list) {
        std::cout << std::setw(5) << SymTable::table[mem].name;
    }
}

void SymTableItem::show() const {
    std::cout << std::setw(10) << name << std::setw(10) << pre_item;
    info->show();
    std::cout << std::endl;
}

void SymTable::mkTable() {
    sp = table.size();
}

int SymTable::enter(const std::string &name, const size_t offset, const int cat) {
    // 如果查找到重复符号，且必须在同一层级，不为形参、过程名，则说明出现变量名重定义
    if (const int pos = lookUpVar(name); pos != -1 && table[pos].info->level == level) {
        Utils::log(ERROR, 0, "Redeclared " + name);
        return -1;
    }
    // 记录当前即将登入的符号表项的地址
    const size_t cur_addr = table.size();
    SymTableItem item;
    // 当前符号表项的前一项是display[level]
    item.pre_item = display[level];
    item.name = name;
    // 更新display[level]为当前符号表项的地址
    display[level] = cur_addr;
    auto *varInfo = new VarInfo;
    varInfo->offset = offset;
    varInfo->cat = cat;
    varInfo->level = level;
    varInfo->value = 0;
    item.info = varInfo;
    table.push_back(item);
    // std::cout << std::setw(5) << table[cur_addr].name << std::setw(5) << table[cur_addr].pre_item << std::endl;
    // 返回当前符号表项的地址
    return cur_addr;
}

int SymTable::enterProc(const std::string &name) {
    // 若查找到重复符号，且为同一层级的过程名，则出现过程重定义
    if (const int pos = lookUpProc(name); pos != -1 && table[pos].info->level == level + 1) {
        Utils::log(ERROR, 0, "Redeclared " + name);
        return -1;
    }
    const size_t cur_addr = table.size();
    SymTableItem item;
    // 当前符号表项的前一项是display[level]
    item.pre_item = display[level];
    item.name = name;
    // 更新display[level]为当前符号表项的地址
    display[level] = cur_addr;
    auto *procInfo = new ProcInfo;
    procInfo->offset = 0;
    procInfo->cat = PROCEDURE;
    procInfo->level = level + 1;
    procInfo->entry = 0;
    item.info = procInfo;
    table.push_back(item);
    // std::cout << std::setw(5) << table[cur_addr].name << std::setw(5) << table[cur_addr].pre_item << std::endl;
    // 返回当前符号表项的地址
    return cur_addr;
}

void SymTable::enterProgram(const std::string &name) {
    SymTableItem item;
    item.pre_item = 0;
    item.name = name;
    auto *procInfo = new ProcInfo;
    procInfo->offset = 0;
    procInfo->cat = PROCEDURE;
    procInfo->level = 0;
    item.info = procInfo;
    table.push_back(item);
}

int SymTable::lookUpProc(const std::string &name) {
    unsigned int curAddr = 0;
    // i代表访问display的指针
    // 若查找主过程名，直接返回-1
    if (level == 0 && display[0] == 0)
        return -1;
    for (int i = level; i >= 0; i--) {
        curAddr = display[i];
        // 遍历当前display指针指向的过程下的所有过程符号，直到遇到最后一个符号(pre == 0)
        while (true) {
            if (table[curAddr].info->cat == PROCEDURE
                && table[curAddr].name == name) {
                return curAddr;
            }
            if (table[curAddr].pre_item == 0)
                break;
            curAddr = table[curAddr].pre_item;
        }
    }
    return -1;
}

int SymTable::lookUpVar(const std::string &name) {
    int curAddr = 0;
    // i代表访问display的指针
    // 若查找主过程名，直接返回-1
    if (level == 0 && display[0] == 0)
        return -1;
    for (int i = level; i >= 0; i--) {
        curAddr = display[i];
        if (curAddr < 0 || curAddr >= table.size())
            break;
        // 遍历当前display指针指向的过程下的所有变量符号，直到遇到最后一个符号(pre == 0)
        while (true) {
            if (table[curAddr].info->cat != PROCEDURE && table[curAddr].name == name) {
                return curAddr;
            }
            if (table[curAddr].pre_item == 0)
                break;
            curAddr = table[curAddr].pre_item;
        }
    }
    return -1;
}

void SymTable::addWidth(const size_t addr, const size_t width) {
    table[addr].info->offset = width;
    glo_offset = 0;
}

void SymTable::clear() {
    sp = 0;
    table.clear();
    display.clear();
    table.reserve(SYM_ITEMS_CNT);
    display.resize(1, 0);
}

void symTableTest() {
    std::cout <<
            "____________________________________________________SymTable_______________________________________________"
            << std::endl;
    for (const SymTableItem &mem: SymTable::table) {
        mem.show();
    }
    std::cout <<
            "___________________________________________________________________________________________________________"
            << std::endl;
}