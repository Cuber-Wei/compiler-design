//
// Created by l0v3ch4n on 25-1-4.
//

#include "PCode.h"

#include <fstream>
#include <iostream>

#include "variables.h"

std::string PCode::output_path = ::output_path;
std::vector<Instruction> PCode::codeList;
std::string op_map[P_CODE_CNT] = {
    "LIT",
    "OPR",
    "LOD",
    "STO",
    "CA",
    "INT",
    "JMP",
    "JPC",
    "RED",
    "WRT"
};

unsigned long PCode::emit(Operation op, int L, int a)
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
    if (is_to_file)
    {
        std::ofstream out_file(output_path);
        for (size_t i = 0; i < codeList.size(); i++)
        {
            out_file << i << "  " << op_map[codeList[i].op] << ", " << codeList[i].L << ", " << codeList[i].a
                << std::endl;
        }
        out_file.close();
    }
    else
    {
        for (size_t i = 0; i < codeList.size(); i++)
        {
            std::cout << i << "  " << op_map[codeList[i].op] << ", " << codeList[i].L << ", " << codeList[i].a
                << std::endl;
        }
    }
}

void PCode::clear()
{
    codeList.clear();
}
