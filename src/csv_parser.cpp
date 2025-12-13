#include "csv_parser.h"
#include <fstream>
#include <regex>
#include <iostream>
#include <filesystem>

int parse_csv(const std::string& csv_path, std::map<std::string, TextStats>& text_db) {
    std::ifstream csv_file(csv_path);
    if (!csv_file.is_open()) {
        std::cerr << "无法打开CSV文件：" << csv_path << std::endl;
        return -1;
    }

    std::string line;
    int line_num = 0;
    // 正则表达式：提取"确认文言表示,"和",Y,Y,Y"之间的内容
    std::regex text_regex(R"(确认文言表示,(.*?),Y,Y,Y)");
    // 正则表达式：提取语种（文言设置为:Italian）
    std::regex lang_regex(R"(文言设置为:(\w+))");
    // 正则表达式：提取图片ID（ScreenID：MM_00_11_07）
    std::regex img_id_regex(R"(ScreenID：(MM_\d+_\d+_\d+))");

    while (std::getline(csv_file, line)) {
        line_num++;
        std::smatch match;

        // 提取目标文言内容
        if (!std::regex_search(line, match, text_regex) || match.size() < 2) {
            continue;
        }
        std::string content = match[1].str();
        if (content.empty()) {
            continue;
        }

        // 提取语种
        std::string lang = "Unknown";
        if (std::regex_search(line, match, lang_regex) && match.size() >= 2) {
            lang = match[1].str();
        }

        // 提取图片ID
        std::string img_id = "Unknown";
        if (std::regex_search(line, match, img_id_regex) && match.size() >= 2) {
            img_id = match[1].str();
        }

        // 更新文言统计信息
        if (text_db.find(content) == text_db.end()) {
            TextStats stats;
            stats.lang = lang;
            stats.image_id = img_id;
            stats.content = content;
            stats.total_occurrences = 0;
            stats.match_count = 0;
            text_db[content] = stats;
        }

        text_db[content].doc_lines.push_back(line_num);
        text_db[content].total_occurrences++;
    }

    csv_file.close();
    return 0;
}

std::string extract_image_id_from_path(const std::string& img_path) {
    // 从路径中提取文件名（不含扩展名）作为图片ID
    std::filesystem::path path(img_path);
    std::string filename = path.stem().string();
    // 如果文件名包含MM_前缀，直接返回，否则返回完整文件名
    if (filename.find("MM_") != std::string::npos) {
        return filename;
    }
    return filename;
}