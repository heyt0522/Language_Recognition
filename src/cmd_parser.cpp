#include "cmd_parser.h"
#include <iostream>
#include <unistd.h>

CmdParams parse_cmd_args(int argc, char** argv) {
    CmdParams params;
    int opt;

    while ((opt = getopt(argc, argv, "c:i:o:t:")) != -1) {
        switch (opt) {
            case 'c':
                params.csv_path = optarg;
                break;
            case 'i':
                params.img_dir = optarg;
                break;
            case 'o':
                params.pdf_output = optarg;
                break;
            case 't':
                params.confidence = atof(optarg);
                break;
            default:
                params.is_valid = false;
                return params;
        }
    }

    // 校验必填参数
    params.is_valid = !(params.csv_path.empty() || params.img_dir.empty() || params.pdf_output.empty());
    return params;
}

void print_usage() {
    std::cout << "用法：./text_matcher -c <CSV路径> -i <图片目录> -o <PDF输出路径> [-t <置信度>]" << std::endl;
    std::cout << "  -c: 文言库CSV文件路径（必填，格式：序号,,模块,描述,元信息,确认文言表示,目标文言,Y,Y,Y）" << std::endl;
    std::cout << "  -i: 待识别图片目录（必填，图片命名：StringID+扩展.png）" << std::endl;
    std::cout << "  -o: PDF输出路径（必填，如：./output/result.pdf）" << std::endl;
    std::cout << "  -t: 识别置信度阈值（可选，默认0.8）" << std::endl;
}