#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>
#include <map>
#include "data_struct.h"

// 解析CSV文件，返回 语种->元数据列表 映射
std::map<std::string, std::vector<CsvMeta>> parse_csv(const std::string& csv_path);

// 按语种拆分任务
std::vector<LangTask> split_tasks_by_lang(
    const std::map<std::string, std::vector<CsvMeta>>& csv_data,
    const std::string& img_dir,
    double confidence,
    const std::string& output_dir
);

#endif // CSV_PARSER_H