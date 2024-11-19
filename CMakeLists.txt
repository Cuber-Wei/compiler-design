cmake_minimum_required(VERSION 3.29)
project(compiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(compiler main.cpp
        utils.h
        LexAnalyzer.cpp
        LexAnalyzer.h
        SyntaxAnalyzer.cpp
        SyntaxAnalyzer.h
        Interpreter.cpp
        Interpreter.h
        utils.cpp
        constants.h
        variables.h
        SymbolTable.cpp
        SymbolTable.h
        PartialCode.cpp
        PartialCode.h
)
