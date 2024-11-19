//
// Created by l0v3ch4n on 24-11-18.
//
#include "utils.h"
#include "constants.h"
#include <fstream>
#include <iostream>

#include "PartialCode.h"
#include "SymbolTable.h"
#include "variables.h"

namespace Utils {
    void log(const logLevel level, size_t lineNo, const std::string &message) {
        switch (level) {
            case INFO:
                std::cout << "\033[1;34m[INFO]\t\t\033[0m";
                break;
            case WARNING:
                std::cout << "\033[1;33m[WARNING]\t\033[0m";
                break;
            case ERROR:
                std::cout << "\033[1;31m[ERROR]\t\t\033[0m";
                if (lineNo != 0) {
                    errCnt++;
                    std::cout << "第" + std::to_string(lineNo) + "行：" << programLines[lineNo] << std::endl
                            << "\033[1;31m[ERROR " + std::to_string(errCnt) + "]\t\033[0m";
                }
                break;
            case SUCCESS:
                std::cout << "\033[1;32m[SUCCESS]\t\033[0m";
                break;
        }
        std::cout << message << std::endl;
    }

    std::string readFile(const std::string &path) {
        log(INFO, 0, "Reading in file: " + path);
        //返回文件内容为一个字符串，换行使用`\n`
        std::ifstream file(path);
        if (!file.is_open()) {
            log(ERROR, 0, "打开文件失败！");
            exit(OPEN_FILE_ERROR);
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        // log(INFO, "文件内容:\n" + content);
        if (content.empty()) {
            log(ERROR, 0, "文件为空文件！");
            exit(EMPTY_FILE_ERROR);
        }

        size_t lineNo = 0;
        std::string line;
        for (int i = 0; i < content.length(); i++) {
            //以'\n'分行
            if (content[i] == '\n') {
                programLines[++lineNo] = line;
                line = "";
                continue;
            }
            line += content[i];
        }
        log(SUCCESS, 0, "文件读取成功！");
        log(INFO, 0, "程序总行数：" + std::to_string(lineNo + 1));
        return content;
    }

    void init() {
        programStr.clear(); //程序字符串
        errCnt = 0; //错误数量
        level = 0; // 层差
        glo_offset = 0; // 全局offset
        SymTable::clear();
        PartialCodeList::clear();
        log(INFO, 0, "初始化完成！");
    }


    void complete(const int flag) {
        if (flag == 1 && errCnt != 0) {
            std::cout << std::endl;
            log(ERROR, 0, "代码编译失败！");
            log(INFO, 0, "错误数量：" + std::to_string(errCnt));
        } else if (errCnt == 0) {
            log(SUCCESS, 0, "代码编译成功！");
        }
    }
}

