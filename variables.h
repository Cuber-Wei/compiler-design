//
// Created by l0v3ch4n on 24-11-18.
//

#ifndef VARIABLES_H
#define VARIABLES_H
#include <string>
#include <unordered_map>
//全局变量声明
inline std::string programStr; //程序字符串
inline std::string programLines[PROGRAM_LINE_MAX]; //程序行
inline std::string tokens[PROGRAM_TK_MAX]; //tokens
inline std::string symbolTable[SYM_ITEMS_CNT]; //符号表
inline std::unordered_map<unsigned long, std::string> symMap; // 保留字编号与字符串的映射
inline unsigned long errCnt = 0; //错误数量
inline size_t level; // 层差
inline size_t glo_offset; // 全局offset
#endif //VARIABLES_H
