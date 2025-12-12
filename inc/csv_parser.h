#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <map>
#include <string>
#include "data_struct.h"

/**
 * 解析CSV文件，提取文言信息并按内容去重统计
 * @param csv_path CSV文件路径
 * @param text_db 输出：文言库（内容->统计信息）
 * @return 0:成功, -1:失败
 */
int parse_csv(const std::string& csv_path, std::map<std::string, TextStats>& text_db);

/**
 * 从图片路径提取图片ID（适配自定义命名规则）
 * @param img_path 图片路径
 * @return 图片ID字符串
 */
std::string extract_image_id_from_path(const std::string& img_path);

#endif // CSV_PARSER_H