//
// Created by l0v3ch4n on 24-11-18.
//

#ifndef UTILS_H
#define UTILS_H
#include <string>

#include "constants.h"

namespace Utils {
    std::string getToken(); //获取下一个词
    std::string readFile(const std::string &path); //读取文件

    void log(logLevel level, size_t lineNo = 0, const std::string& message = "");

    void init(); //初始化过程

    void complete(int flag = 0); //打印结束
}


#endif //UTILS_H
