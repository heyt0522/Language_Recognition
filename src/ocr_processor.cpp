#include "ocr_processor.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <CImg.h>
#include <iostream>
#include <algorithm>

using namespace cimg_library;

int ocr_image_by_lang(const std::string& img_path, const std::string& lang, std::vector<OcrResult>& results) {
    // 初始化Tesseract OCR
    tesseract::TessBaseAPI api;
    // 设置Tesseract数据路径（开发板端需确保tessdata存在）
    if (api.Init("./config/tessdata", lang.c_str())) {
        std::cerr << "Tesseract初始化失败（语种：" << lang << "）" << std::endl;
        return -1;
    }

    // 加载图片
    Pix* pix = pixRead(img_path.c_str());
    if (!pix) {
        std::cerr << "无法加载图片：" << img_path << std::endl;
        api.End();
        return -1;
    }

    // 设置识别图片
    api.SetImage(pix);
    // 获取识别结果（按单词级别）
    tesseract::ResultIterator* ri = api.GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

    if (ri) {
        do {
            const char* text = ri->GetUTF8Text(level);
            if (text && *text != '\0') {
                int left, top, right, bottom;
                ri->BoundingBox(level, &left, &top, &right, &bottom);
                
                OcrResult res;
                res.text = text;
                res.left = left;
                res.top = top;
                res.width = right - left;
                res.height = bottom - top;
                results.push_back(res);
            }
            delete[] text;
        } while (ri->Next(level));
        delete ri;
    }

    // 释放资源
    pixDestroy(&pix);
    api.End();
    return 0;
}

double calculate_similarity(const std::string& s1, const std::string& s2) {
    // 统一转为小写
    std::string a = s1, b = s2;
    std::transform(a.begin(), a.end(), a.begin(), ::tolower);
    std::transform(b.begin(), b.end(), b.begin(), ::tolower);

    // 计算Levenshtein编辑距离
    int m = a.size(), n = b.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            int cost = (a[i-1] == b[j-1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i-1][j] + 1, dp[i][j-1] + 1, dp[i-1][j-1] + cost});
        }
    }

    // 计算相似度（1 - 编辑距离/最长字符串长度）
    int max_len = std::max(m, n);
    if (max_len == 0) return 1.0;
    return 1.0 - (double)dp[m][n] / max_len;
}

int annotate_image(const std::string& input_img, const std::string& output_img, const std::vector<TextStats>& stats) {
    try {
        CImg<unsigned char> img(input_img.c_str());
        // 定义颜色：绿色（匹配成功）、红色（匹配失败）
        const unsigned char green[] = {0, 255, 0};
        const unsigned char red[] = {255, 0, 0};
        const int line_width = 2;

        // 遍历所有文言统计信息，绘制标注框
        for (const auto& stat : stats) {
            for (const auto& anno : stat.annotations) {
                if (anno.left < 0 || anno.top < 0) continue;
                // 绘制标注框
                img.draw_rectangle(anno.left, anno.top, 
                                  anno.left + anno.width, anno.top + anno.height,
                                  anno.is_ok ? green : red, line_width);
                
                // 绘制勾选/叉号
                int center_x = anno.left + anno.width / 2;
                int center_y = anno.top + anno.height / 2;
                int mark_size = std::min(anno.width, anno.height) / 4;

                if (anno.is_ok) {
                    // 勾选符号
                    img.draw_line(center_x - mark_size, center_y, 
                                 center_x, center_y + mark_size, green, line_width + 1);
                    img.draw_line(center_x, center_y + mark_size, 
                                 center_x + mark_size, center_y - mark_size, green, line_width + 1);
                } else {
                    // 叉号
                    img.draw_line(center_x - mark_size, center_y - mark_size, 
                                 center_x + mark_size, center_y + mark_size, red, line_width + 1);
                    img.draw_line(center_x + mark_size, center_y - mark_size, 
                                 center_x - mark_size, center_y + mark_size, red, line_width + 1);
                }
            }
        }

        // 保存标注后的图片
        img.save(output_img.c_str());
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "图片标注失败：" << e.what() << std::endl;
        return -1;
    }
}