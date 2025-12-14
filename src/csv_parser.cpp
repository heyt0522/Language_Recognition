#include "csv_parser.h"
#include "csv_utils.h"  // 引入抽离的工具函数
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <regex>
#include <algorithm>
#include <cstdio>
#include <iostream>

// 仅保留非inline函数的实现
CsvMeta extract_csv_meta(const std::vector<std::string>& fields, const std::string& original_line, int line_num) {
    CsvMeta meta;
    meta.line_num = line_num;
    meta.seq_id = fields[0];
    
    if (fields.size() >= 3) meta.module = fields[2];
    if (fields.size() >= 4) meta.desc = fields[3];
    
    meta.lang_text = extract_target_text(original_line);
    if (meta.lang_text.empty()) {
        std::cerr << "CSV第" << line_num << "行：未提取到目标文言内容，跳过！" << std::endl;
        return meta;
    }

    if (fields.size() >= 5 && !fields[4].empty()) {
        std::string meta_str = fields[4];
        std::regex screen_regex(R"(ScreenID：(.+)\n)");
        std::smatch screen_match;
        if (std::regex_search(meta_str, screen_match, screen_regex)) {
            meta.screen_id = screen_match.str(1);
        }

        std::regex part_regex(R"(PartID:(.+)\n)");
        std::smatch part_match;
        if (std::regex_search(meta_str, part_match, part_regex)) {
            meta.part_id = part_match.str(1);
        }

        std::regex string_regex(R"(String ID:(.+))");
        std::smatch string_match;
        if (std::regex_search(meta_str, string_match, string_regex)) {
            meta.string_id = string_match.str(1);
        }
    }

    std::string lower_text = meta.lang_text;
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    
    if (lower_text.find("Englist") != std::string::npos || lower_text.find("uk") != std::string::npos || lower_text.find("englist") != std::string::npos) {
        meta.lang = "英语";
    } else if (lower_text.find("French") != std::string::npos || lower_text.find("french") != std::string::npos) {
        meta.lang = "法语";
    } else if (lower_text.find("deutsch") != std::string::npos || lower_text.find("german") != std::string::npos) {
        meta.lang = "德语";
    } else if (lower_text.find("русский") != std::string::npos || lower_text.find("russian") != std::string::npos) {
        meta.lang = "俄语";
    } else if (lower_text.find("español") != std::string::npos || lower_text.find("spanish") != std::string::npos) {
        meta.lang = "西班牙语";
    } else if (lower_text.find("português") != std::string::npos || lower_text.find("portuguese") != std::string::npos) {
        meta.lang = "葡萄牙语";
    } else if (lower_text.find("italiano") != std::string::npos || lower_text.find("italian") != std::string::npos) {
        meta.lang = "意大利语";
    } else if (lower_text.find("türkçe") != std::string::npos || lower_text.find("turkish") != std::string::npos) {
        meta.lang = "土耳其语";
    } else if (lower_text.find("ไทย") != std::string::npos || lower_text.find("thai") != std::string::npos) {
        meta.lang = "泰语";
    } else if (lower_text.find("العربية") != std::string::npos || lower_text.find("arabic") != std::string::npos) {
        meta.lang = "阿拉伯语";
    } else {
        meta.lang = "英语";
    }

    return meta;
}

std::map<std::string, std::vector<CsvMeta>> parse_csv(const std::string& csv_path) {
    std::map<std::string, std::vector<CsvMeta>> csv_data;

    if (access(csv_path.c_str(), F_OK) != 0) {
        throw std::runtime_error("CSV文件不存在：" + csv_path);
    }

    std::ifstream csv_file(csv_path, std::ios::in | std::ios::binary);
    if (!csv_file) {
        throw std::runtime_error("无法打开CSV文件：" + csv_path);
    }

    std::string line;
    int line_num = 0;
    while (std::getline(csv_file, line)) {
        line_num++;
        if (line.empty()) continue;

        std::string original_line = line;
        std::vector<std::string> fields = parse_csv_line(line);
        if (fields.size() < 8) {
            std::cerr << "CSV第" << line_num << "行格式错误（字段数不足），跳过！" << std::endl;
            continue;
        }

        CsvMeta meta = extract_csv_meta(fields, original_line, line_num);
        if (meta.lang.empty() || meta.string_id.empty() || meta.lang_text.empty()) {
            continue;
        }

        csv_data[meta.lang].push_back(meta);
    }

    csv_file.close();
    return csv_data;
}

std::vector<LangTask> split_tasks_by_lang(
    const std::map<std::string, std::vector<CsvMeta>>& csv_data,
    const std::string& img_dir,
    double confidence,
    const std::string& output_dir
) {
    std::vector<LangTask> tasks;

    for (const auto& [lang, meta_list] : csv_data) {
        LangTask task;
        task.lang = lang;
        auto lang_it = LANG_CODE_MAP.find(lang);
        task.lang_code = (lang_it != LANG_CODE_MAP.end()) ? lang_it->second : "eng";
        task.confidence_threshold = confidence;
        task.output_dir = output_dir + "/" + lang;

        for (const auto& meta : meta_list) {
            if (meta.string_id.empty()) continue;
            std::string img_path = match_image_by_string_id(img_dir, meta.string_id);
            if (img_path.empty()) {
                std::cerr << "未找到String ID[" << meta.string_id << "]对应的图片，跳过！" << std::endl;
                continue;
            }
            task.img_meta_list.emplace_back(img_path, meta);
        }

        if (!task.img_meta_list.empty()) {
            tasks.push_back(task);
        }
    }

    return tasks;
}