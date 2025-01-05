//
// Created by l0v3ch4n on 25-1-4.
//

#ifndef UTILS_H
#define UTILS_H
#include <string>


class Utils
{
public:
    static void init(); //初始化过程
    static void readFile(const std::string& path); //读取文件
    static void printResult();

    static void info(const std::string& message); // 打印提示信息
    static void info(const std::string&, unsigned int line_num, unsigned int col_num); // 打印提示信息

    static void error(const std::string& message); // 打印错误信息
    static void error(const std::string& message, unsigned int line_num, unsigned int col_num); // 打印错误信息

    static void success(const std::string& message); // 打印成功信息
    static void success(const std::string& message, unsigned int line_num, unsigned int col_num); // 打印成功信息
    static void warning(const std::string& message);
    static void warning(const std::string& message, unsigned int line_num, unsigned int col_num);
    static void output(int number);
    static void input_prompt();
};


#endif //UTILS_H
