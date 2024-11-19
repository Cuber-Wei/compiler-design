//
// Created by l0v3ch4n on 24-11-18.
//

#include "PartialCode.h"
#include "constants.h"
#include <iomanip>
#include <iostream>
#include <vector>
using namespace std;

vector<PartialCode> PartialCodeList::codeList;

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

int PartialCodeList::emit(Operation op, int L, int a) {
    codeList.emplace_back(op, L, a);
    return codeList.size() - 1;
}

void PartialCodeList::backpatch(size_t target, size_t addr) {
    if (addr == -1)
        return;
    else
        codeList[target].a = addr;
}

void PartialCodeList::printCode() {
    for (size_t i = 0; i < codeList.size(); i++) {
        std::cout << setw(4) << i << "  " << op_map[codeList[i].op] << ", " << codeList[i].L << ", " << codeList[i].a
                << endl;
    }
}

void PartialCodeList::clear() {
    codeList.clear();
}
