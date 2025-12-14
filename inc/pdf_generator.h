#ifndef PDF_GENERATOR_H
#define PDF_GENERATOR_H

#include <vector>
#include "data_struct.h"

// 生成PDF报告（图片+结构化表格）
bool generate_pdf(
    const std::string& pdf_path,
    const std::vector<OcrResult>& all_results
);

// 递归创建目录（递归创建）
bool create_dir(const std::string& dir_path);

#endif // PDF_GENERATOR_H