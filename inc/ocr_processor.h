#ifndef OCR_PROCESSOR_H
#define OCR_PROCESSOR_H

#include <vector>
#include <string>
#include "data_struct.h"

/**
 * 指定语种进行图片OCR识别
 * @param img_path 图片路径
 * @param lang 语种（如"eng"/"ita"/"chi_sim"）
 * @param results 输出：OCR识别结果列表
 * @return 0:成功, -1:失败
 */
int ocr_image_by_lang(const std::string& img_path, const std::string& lang, std::vector<OcrResult>& results);

/**
 * 计算两个字符串的相似度（Levenshtein编辑距离）
 * @param s1 字符串1
 * @param s2 字符串2
 * @return 相似度（0~1，1为完全匹配）
 */
double calculate_similarity(const std::string& s1, const std::string& s2);

/**
 * 在图片中绘制标注框并保存
 * @param input_img 输入图片路径
 * @param output_img 输出标注图片路径
 * @param stats 该图片的文言统计信息
 * @return 0:成功, -1:失败
 */
int annotate_image(const std::string& input_img, const std::string& output_img, const std::vector<TextStats>& stats);

#endif // OCR_PROCESSOR_H