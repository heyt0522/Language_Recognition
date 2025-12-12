#ifndef CMD_PARSER_H
#define CMD_PARSER_H

#include "data_struct.h"
#include <string>

/**
 * 解析命令行参数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @param args 解析后的参数结构体
 * @return 0:成功, -1:失败
 */
int parse_cmd_args(int argc, char* argv[], CmdArgs& args);

#endif // CMD_PARSER_H