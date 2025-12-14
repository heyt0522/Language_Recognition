#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>  // 新增：std::remove依赖
#include <cstdio>     // 新增：popen/pclose依赖
#include <unistd.h>   // 新增：access依赖
#include "data_struct.h"

// 声明为inline，避免重复定义
inline std::vector<std::string> parse_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string current_field;
    bool in_quote = false;

    for (char c : line) {
        if (c == '"' && !in_quote) {
            in_quote = true;
            continue;
        } else if (c == '"' && in_quote) {
            in_quote = false;
            continue;
        }

        if (c == ',' && !in_quote) {
            fields.push_back(current_field);
            current_field.clear();
        } else {
            current_field += c;
        }
    }
    fields.push_back(current_field);
    return fields;
}

inline std::string extract_target_text(const std::string& line) {
    std::string start_marker = "确认文言表示,";
    size_t start_pos = line.find(start_marker);
    if (start_pos == std::string::npos) return "";
    start_pos += start_marker.length();

    std::string end_marker = "Y,Y,Y";
    size_t end_pos = line.find(end_marker, start_pos);
    if (end_pos == std::string::npos) return "";

    std::string target_text = line.substr(start_pos, end_pos - start_pos);
    target_text.erase(0, target_text.find_first_not_of(" \t\n\r"));
    target_text.erase(target_text.find_last_not_of(" \t\n\r") + 1);

    return target_text;
}

inline std::string match_image_by_string_id(const std::string& img_dir, const std::string& string_id) {
    std::string cmd = "find " + img_dir + " -name '" + string_id + "*' -name '*.png' | head -1";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    char buffer[256] = {0};  // 初始化缓冲区，避免脏数据
    std::string img_path;
    // 修复：fgets返回空时直接返回，避免空字符串处理
    if (fgets(buffer, sizeof(buffer) - 1, pipe) != nullptr) {
        img_path = buffer;
        // 修复：正确移除换行符（迭代器类型匹配）
        img_path.erase(
            std::remove(img_path.begin(), img_path.end(), '\n'), 
            img_path.end()
        );
        // 额外：移除回车符（兼容Windows换行）
        img_path.erase(
            std::remove(img_path.begin(), img_path.end(), '\r'), 
            img_path.end()
        );
    }
    pclose(pipe);
    return img_path;
}

// 非inline函数声明（实现放在csv_parser.cpp）
CsvMeta extract_csv_meta(const std::vector<std::string>& fields, const std::string& original_line, int line_num);
std::map<std::string, std::vector<CsvMeta>> parse_csv(const std::string& csv_path);
std::vector<LangTask> split_tasks_by_lang(
    const std::map<std::string, std::vector<CsvMeta>>& csv_data,
    const std::string& img_dir,
    double confidence,
    const std::string& output_dir
);

#endif // CSV_UTILS_H