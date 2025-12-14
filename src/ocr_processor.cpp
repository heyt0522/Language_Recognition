#include "csv_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <regex>
#include <algorithm>
#include <cstdio>
#include "data_struct.h"

// 解析带嵌套换行的CSV字段（处理双引号包裹的多行内容）
std::vector<std::string> parse_csv_line(const std::string& line) {
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
    fields.push_back(current_field); // 最后一个字段
    return fields;
}

// 精准提取「确认文言表示,」之后、「Y,Y,Y」之前的文言内容
std::string extract_target_text(const std::string& line) {
    // 第一步：找到"确认文言表示,"的位置
    std::string start_marker = "确认文言表示,";
    size_t start_pos = line.find(start_marker);
    if (start_pos == std::string::npos) {
        return "";
    }
    start_pos += start_marker.length();

    // 第二步：找到"Y,Y,Y"的位置（结束标记）
    std::string end_marker = "Y,Y,Y";
    size_t end_pos = line.find(end_marker, start_pos);
    if (end_pos == std::string::npos) {
        return "";
    }

    // 提取中间内容并去除首尾空格/换行
    std::string target_text = line.substr(start_pos, end_pos - start_pos);
    // 去除首尾空白字符
    target_text.erase(0, target_text.find_first_not_of(" \t\n\r"));
    target_text.erase(target_text.find_last_not_of(" \t\n\r") + 1);

    return target_text;
}

// 从CSV行提取元信息（核心：精准文言提取）
CsvMeta extract_csv_meta(const std::vector<std::string>& fields, const std::string& original_line, int line_num) {
    CsvMeta meta;
    meta.line_num = line_num;
    meta.seq_id = fields[0];          // 第1列：序号（24438）
    
    if (fields.size() >= 3) meta.module = fields[2];          // 第3列：模块
    if (fields.size() >= 4) meta.desc = fields[3];            // 第4列：描述
    
    // 核心：提取目标文言内容
    meta.lang_text = extract_target_text(original_line);
    if (meta.lang_text.empty()) {
        std::cerr << "CSV第" << line_num << "行：未提取到目标文言内容，跳过！" << std::endl;
        return meta;
    }

    // 解析第5列的元信息（String ID/ScreenID/PartID）
    if (fields.size() >= 5 && !fields[4].empty()) {
        std::string meta_str = fields[4];
        // 提取ScreenID
        std::regex screen_regex(R"(ScreenID：(.+)\n)");
        std::smatch screen_match;
        if (std::regex_search(meta_str, screen_match, screen_regex)) {
            meta.screen_id = screen_match.str(1);
        }

        // 提取PartID
        std::regex part_regex(R"(PartID:(.+)\n)");
        std::smatch part_match;
        if (std::regex_search(meta_str, part_match, part_regex)) {
            meta.part_id = part_match.str(1);
        }

        // 提取String ID
        std::regex string_regex(R"(String ID:(.+))");
        std::smatch string_match;
        if (std::regex_search(meta_str, string_match, string_regex)) {
            meta.string_id = string_match.str(1);
        }
    }

    // 自动识别语种（基于提取的文言内容）
    std::string lower_text = meta.lang_text;
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    
    if (lower_text.find("english") != std::string::npos || lower_text.find("uk") != std::string::npos || lower_text.find("englist") != std::string::npos) {
        meta.lang = "英语";
    } else if (lower_text.find("français") != std::string::npos || lower_text.find("french") != std::string::npos) {
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
        meta.lang = "英语"; // 默认英语
    }

    return meta;
}

// 解析CSV文件（核心：适配非标格式+精准文言提取）
std::map<std::string, std::vector<CsvMeta>> parse_csv(const std::string& csv_path) {
    std::map<std::string, std::vector<CsvMeta>> csv_data; // 语种->元数据列表

    // 检查文件存在性
    if (access(csv_path.c_str(), F_OK) != 0) {
        throw std::runtime_error("CSV文件不存在：" + csv_path);
    }

    // 以UTF-8编码打开CSV
    std::ifstream csv_file(csv_path, std::ios::in | std::ios::binary);
    if (!csv_file) {
        throw std::runtime_error("无法打开CSV文件：" + csv_path);
    }

    std::string line;
    int line_num = 0;
    while (std::getline(csv_file, line)) {
        line_num++;
        if (line.empty()) continue;

        // 保存原始行（用于精准提取文言）
        std::string original_line = line;

        // 解析CSV行（处理嵌套换行）
        std::vector<std::string> fields = parse_csv_line(line);
        if (fields.size() < 8) { // 至少8列有效数据
            std::cerr << "CSV第" << line_num << "行格式错误（字段数不足），跳过！" << std::endl;
            continue;
        }

        // 提取元数据（传入原始行用于精准文言提取）
        CsvMeta meta = extract_csv_meta(fields, original_line, line_num);
        if (meta.lang.empty() || meta.string_id.empty() || meta.lang_text.empty()) {
            continue;
        }

        // 按语种分组
        csv_data[meta.lang].push_back(meta);
    }

    csv_file.close();
    return csv_data;
}

// 匹配图片（按String ID，支持扩展后缀）
std::string match_image_by_string_id(const std::string& img_dir, const std::string& string_id) {
    // 遍历图片目录，匹配以String ID开头的png文件
    std::string cmd = "find " + img_dir + " -name '" + string_id + "*' -name '*.png' | head -1";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    char buffer[256];
    std::string img_path;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        img_path = buffer;
        // 去除换行符
        img_path.erase(std::remove(img_path.begin(), img_path.end(), '\n'), img_path.end());
    }
    pclose(pipe);
    return img_path;
}

// 按语种拆分任务（关联图片+CSV元数据）
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
        // 映射语种编码
        auto lang_it = LANG_CODE_MAP.find(lang);
        task.lang_code = (lang_it != LANG_CODE_MAP.end()) ? lang_it->second : "eng";
        task.confidence_threshold = confidence;
        task.output_dir = output_dir + "/" + lang; // 按语种分目录

        // 匹配图片并关联元数据
        for (const auto& meta : meta_list) {
            if (meta.string_id.empty()) continue;
            // 按String ID匹配图片
            std::string img_path = match_image_by_string_id(img_dir, meta.string_id);
            if (img_path.empty()) {
                std::cerr << "未找到String ID[" << meta.string_id << "]对应的图片，跳过！" << std::endl;
                continue;
            }
            // 保存图片路径+元数据
            task.img_meta_list.emplace_back(img_path, meta);
        }

        if (!task.img_meta_list.empty()) {
            tasks.push_back(task);
        }
    }

    return tasks;
}