#include "cmd_parser.h"
#include <getopt.h>
#include <iostream>
#include <cstring>

int parse_cmd_args(int argc, char* argv[], CmdArgs& args) {
    // 默认参数初始化
    args.csv_path = "";
    args.image_dir = "";
    std::string output_dir = "/home/he_yt/Multilingual_Recognition/code/language_result/pdf/";
    args.threshold = 0.7;

    // 短选项定义
    const char* optstring = "c:i:u:t:";
    int opt;
    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'c':
                args.csv_path = optarg;
                break;
            case 'i':
                args.image_dir = optarg;
                break;
            case 'u':
                output_dir = optarg;
                break;
            case 't':
                args.threshold = atof(optarg);
                if (args.threshold < 0.0 || args.threshold > 1.0) {
                    std::cerr << "阈值必须在0.0~1.0之间！" << std::endl;
                    return -1;
                }
                break;
            case '?':
                std::cerr << "未知选项：" << (char)optopt << std::endl;
                return -1;
            default:
                return -1;
        }
    }

    // 检查必填参数
    if (args.csv_path.empty() || args.image_dir.empty()) {
        std::cerr << "缺少必填参数！" << std::endl;
        std::cerr << "用法：" << argv[0] << " -c <CSV路径> -i <图片目录> [-u <本地输出目录>] [-t <匹配阈值>]" << std::endl;
        return -1;
    }

    return 0;
}