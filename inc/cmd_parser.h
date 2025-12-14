#ifndef CMD_PARSER_H
#define CMD_PARSER_H

#include <string>

// 命令行参数结构体
struct CmdParams {
    std::string csv_path;        // CSV文件路径（必填）
    std::string img_dir;         // 图片目录路径（必填）
    std::string pdf_output;      // PDF输出路径（必填）
    double confidence = 0.8;     // 识别置信度阈值（默认0.8）
    bool is_valid = false;       // 参数是否有效
};

// 解析命令行参数
CmdParams parse_cmd_args(int argc, char** argv);

// 打印帮助信息
void print_usage();

#endif // CMD_PARSER_H