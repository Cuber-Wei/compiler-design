//
// Created by l0v3ch4n on 25-1-4.
//

#include "PCode.h"

#include <fstream>
#include <iostream>
#include <oneapi/tbb/info.h>

#include "Utils.h"
#include "variables.h"

std::vector<Instruction> PCode::codeList;
std::string op_map[P_CODE_CNT] = {
    "LIT",
    "OPR",
    "LOD",
    "STO",
    "CAL",
    "INT",
    "JMP",
    "JPC",
    "RED",
    "WRT"
};

size_t PCode::emit(Operation op, int L, int a)
{
    codeList.emplace_back(op, L, a);
    return codeList.size() - 1;
}

void PCode::backpatch(const size_t target, const size_t addr)
{
    if (addr == -1)
        return;
    codeList[target].a = static_cast<int>(addr);
}

void PCode::printCode(const bool is_to_file)
{
    if (!is_to_file)
    {
        for (size_t i = 0; i < codeList.size(); i++)
            std::cout << i << "  " << op_map[codeList[i].op] << ", " << codeList[i].L << ", " << codeList[i].a
                << std::endl;
    }
    std::ofstream out_file(output_path);
    if (!out_file.is_open())
    {
        Utils::error("打开文件失败！");
        return;
    }
    for (size_t i = 0; i < codeList.size(); i++)
        out_file << i << " " << op_map[codeList[i].op] << " " << codeList[i].L << " " << codeList[i].a
            << std::endl;
    out_file.close();
}

void PCode::clear()
{
    codeList.clear();
}
