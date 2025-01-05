//
// Created by l0v3ch4n on 25-1-4.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <string>


class Interpreter {
public:
    std::string code_path;
    Interpreter(const std::string& pcode_path)
    {
        code_path = pcode_path;
    }
    void runCode();
};



#endif //INTERPRETER_H
