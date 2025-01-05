//
// Created by l0v3ch4n on 25-1-4.
//

#ifndef PCODE_H
#define PCODE_H
#include <string>


class PCode
{
public:
    std::string output_path;

    explicit PCode(const std::string& out_path)
    {
        output_path = out_path;
    }

    void genCode();
    void emit();
};


#endif //PCODE_H
