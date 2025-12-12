#ifndef PDF_GENERATOR_H
#define PDF_GENERATOR_H

#include "data_struct.h"
#include <string>

/**
 * 生成包含标注图片和统计表格的PDF报告
 * @param pdf_path PDF保存路径
 * @param result_set 全局结果集
 * @return 0:成功, -1:失败
 */
int generate_pdf(const std::string& pdf_path, const ResultSet& result_set);

#endif // PDF_GENERATOR_H