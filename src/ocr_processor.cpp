#include "ocr_processor.h"
#include "csv_utils.h"  // 引入工具函数，避免重复定义
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <mutex>
#include <cstdio>
#include <unistd.h>

// 全局OCR引擎
static tesseract::TessBaseAPI* g_tess_api = nullptr;
static std::mutex g_ocr_mutex;

// 实现OCR引擎初始化函数
bool init_ocr_engine(const std::string& tessdata_dir) {
    g_tess_api = new tesseract::TessBaseAPI();
    // 初始化通用引擎（后续按语种切换）
    if (g_tess_api->Init(tessdata_dir.c_str(), "eng") != 0) {
        std::cerr << "Tesseract初始化失败！" << std::endl;
        delete g_tess_api;
        g_tess_api = nullptr;
        return false;
    }
    // 设置OCR模式（仅识别文本）
    g_tess_api->SetPageSegMode(tesseract::PSM_AUTO);
    return true;
}

// 实现OCR引擎释放函数
void release_ocr_engine() {
    std::lock_guard<std::mutex> lock(g_ocr_mutex);
    if (g_tess_api) {
        g_tess_api->End();
        delete g_tess_api;
        g_tess_api = nullptr;
    }
}

// 实现图片处理函数
OcrResult process_image(const std::string& img_path, const CsvMeta& csv_meta, double confidence_threshold) {
    OcrResult res;
    // 初始化结果元数据
    res.seq_id = csv_meta.seq_id;
    res.string_id = csv_meta.string_id;
    res.screen_id = csv_meta.screen_id;
    res.part_id = csv_meta.part_id;
    res.lang = csv_meta.lang;
    res.lang_code = LANG_CODE_MAP.at(csv_meta.lang);
    res.img_id = img_path.substr(img_path.find_last_of("/") + 1);
    res.is_ok = false;

    // 检查图片文件
    if (access(img_path.c_str(), F_OK) != 0) {
        std::cerr << "图片不存在：" << img_path << std::endl;
        return res;
    }

    // 读取图片（Leptonica）
    PIX* pix = pixRead(img_path.c_str());
    if (!pix) {
        std::cerr << "读取图片失败：" << img_path << std::endl;
        return res;
    }

    std::lock_guard<std::mutex> lock(g_ocr_mutex);
    if (!g_tess_api) {
        std::cerr << "OCR引擎未初始化！" << std::endl;
        pixDestroy(&pix);
        return res;
    }

    try {
        // 切换语种
        if (g_tess_api->Init(nullptr, res.lang_code.c_str()) != 0) {
            std::cerr << "切换语种失败：" << res.lang_code << std::endl;
            pixDestroy(&pix);
            return res;
        }

        // 设置图片并识别
        g_tess_api->SetImage(pix);
        char* out_text = g_tess_api->GetUTF8Text();
        if (!out_text) {
            std::cerr << "识别文本为空：" << img_path << std::endl;
            pixDestroy(&pix);
            return res;
        }

        // 获取置信度
        double avg_conf = g_tess_api->MeanTextConf() / 100.0;
        res.text = out_text;
        res.is_ok = (avg_conf >= confidence_threshold);
        res.count = ++g_text_count_map[res.text];

        // 生成标注图片（简化版：保存原图片路径）
        res.annotated_img = img_path;
        res.box = {0, 0, pixGetWidth(pix), pixGetHeight(pix)};

        // 释放资源
        delete[] out_text;
    } catch (const std::exception& e) {
        std::cerr << "处理图片异常：" << img_path << " - " << e.what() << std::endl;
    }

    pixDestroy(&pix);
    return res;
}