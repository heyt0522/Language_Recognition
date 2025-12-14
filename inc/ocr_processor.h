#ifndef OCR_PROCESSOR_H
#define OCR_PROCESSOR_H

#include <string>
#include "data_struct.h"

// 初始化OCR引擎（Tesseract）
bool init_ocr_engine(const std::string& tessdata_path = "");

// 识别单张图片，返回标注后的图片路径+识别结果
OcrResult process_image(
    const std::string& img_path,
    const CsvMeta& csv_meta,
    double confidence
);

// 释放OCR引擎资源
void release_ocr_engine();

// 递归创建目录
bool create_dir(const std::string& dir_path);

#endif // OCR_PROCESSOR_H