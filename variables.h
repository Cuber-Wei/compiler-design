//
// Created by l0v3ch4n on 25-1-4.
//

#ifndef VARIABLES_H
#define VARIABLES_H
#include <string>
#include <unordered_map>

#include "constants.h"
inline std::string output_path; // 中间代码输出路径
inline std::string code_str; // 程序字符串
inline std::string code_line[MAX_FILE_LINE]; // 程序行
inline std::string symbol_table[SYM_ITEMS_CNT]; // 符号表
inline unsigned long lex_err_cnt = 0; // 错误数量
inline unsigned long syntax_err_cnt = 0; // 错误数量
inline unsigned long sym_err_cnt = 0; // 错误数量
inline int level = 0; // 层差
inline int glo_offset = 0; // 全局offset
#endif //VARIABLES_H
