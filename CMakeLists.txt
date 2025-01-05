cmake_minimum_required(VERSION 3.30)
project(compiler)

set(CMAKE_CXX_STANDARD 20)

add_executable(compiler main.cpp
        LexAnalyzer.cpp
        LexAnalyzer.h
        constants.h
        Utils.cpp
        Utils.h
        variables.h
        SyntaxAnalyzer.cpp
        SyntaxAnalyzer.h
        SymTable.cpp
        SymTable.h
        PCode.cpp
        PCode.h
        Interpreter.cpp
        Interpreter.h)
