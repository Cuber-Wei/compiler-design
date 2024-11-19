//
// Created by l0v3ch4n on 24-11-18.
//

#ifndef PARTIALCODE_H
#define PARTIALCODE_H
#include <vector>
#include <cstdlib>

#include "constants.h"


class PartialCode {
public:
    Operation op; // 伪操作码
    int L; // 层级
    int a; // 相对地址
    PartialCode(const Operation op, const int L, const int a) {
        this->op = op;
        this->L = L;
        this->a = a;
    };
};

class PartialCodeList {
public:
    static std::vector<PartialCode> codeList;

    static int emit(Operation op, int L, int a);

    static void backpatch(size_t target, size_t addr);

    static void printCode();

    static void clear();
};

#endif //PARTIALCODE_H
