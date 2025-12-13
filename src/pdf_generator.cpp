#include "pdf_generator.h"
#include <hpdf.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

// PDF错误处理回调函数
void pdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    std::cerr << "PDF生成错误：" << error_no << "，详情：" << detail_no << std::endl;
}

int generate_pdf(const std::string& pdf_path, const ResultSet& result_set) {
    // 创建PDF文档
    HPDF_Doc pdf = HPDF_New(pdf_error_handler, NULL);
    if (!pdf) {
        std::cerr << "创建PDF文档失败" << std::endl;
        return -1;
    }

    // 设置字体（支持UTF-8）
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    if (!font) {
        std::cerr << "加载字体失败" << std::endl;
        HPDF_Free(pdf);
        return -1;
    }

    // 遍历每个图片的结果，生成PDF页面
    for (const auto& [img_id, stats_list] : result_set.all_results) {
        // 添加新页面（A4横向）
        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_LANDSCAPE);
        HPDF_Page_SetFontAndSize(page, font, 8);

        // --------------------- 绘制图片（如果存在） ---------------------
        std::string img_path = "./annotated_" + img_id + ".png"; // 标注后的图片路径
        if (std::filesystem::exists(img_path)) {
            HPDF_Image image = HPDF_LoadPngImageFromFile(pdf, img_path.c_str());
            if (image) {
                // 计算图片缩放比例（适配页面宽度）
                float page_width = HPDF_Page_GetWidth(page) - 40; // 左右边距20
                float page_height = HPDF_Page_GetHeight(page) / 2 - 20; // 上半部分显示图片
                float img_width = HPDF_Image_GetWidth(image);
                float img_height = HPDF_Image_GetHeight(image);
                float scale = std::min(page_width / img_width, page_height / img_height);

                float draw_x = 20;
                float draw_y = HPDF_Page_GetHeight(page) - img_height * scale - 20;
                HPDF_Page_DrawImage(page, image, draw_x, draw_y, img_width * scale, img_height * scale);
            }
        }

        // --------------------- 绘制统计表格 ---------------------
        // 表格列宽（六列：语种、图片ID、文言内容、文档位置、识别结果、出现次数）
        float col_widths[] = {60, 80, 200, 80, 60, 60};
        int col_count = 6;
        int row_count = stats_list.size() + 1; // +1表头
        float table_x = 20;
        float table_y = 20; // 页面底部开始
        float row_height = 20;

        // 绘制表头
        std::string headers[] = {"语种", "图片ID", "文言内容", "文档位置", "识别结果", "出现次数"};
        float current_y = table_y + (row_count - 1) * row_height;
        for (int i = 0; i < col_count; i++) {
            // 绘制单元格边框
            HPDF_Page_Rectangle(page, table_x, current_y, col_widths[i], row_height);
            HPDF_Page_Stroke(page);
            // 绘制表头文字
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + 2, current_y + row_height/2 - 4, headers[i].c_str());
            HPDF_Page_EndText(page);
            table_x += col_widths[i];
        }

        // 绘制表格内容
        table_x = 20;
        current_y -= row_height;
        for (const auto& stats : stats_list) {
            // 1. 语种
            HPDF_Page_Rectangle(page, table_x, current_y, col_widths[0], row_height);
            HPDF_Page_Stroke(page);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + 2, current_y + row_height/2 - 4, stats.lang.c_str());
            HPDF_Page_EndText(page);
            table_x += col_widths[0];

            // 2. 图片ID
            HPDF_Page_Rectangle(page, table_x, current_y, col_widths[1], row_height);
            HPDF_Page_Stroke(page);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + 2, current_y + row_height/2 - 4, stats.image_id.c_str());
            HPDF_Page_EndText(page);
            table_x += col_widths[1];

            // 3. 文言内容（截断过长文本）
            std::string content = stats.content;
            if (content.size() > 30) {
                content = content.substr(0, 27) + "...";
            }
            HPDF_Page_Rectangle(page, table_x, current_y, col_widths[2], row_height);
            HPDF_Page_Stroke(page);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + 2, current_y + row_height/2 - 4, content.c_str());
            HPDF_Page_EndText(page);
            table_x += col_widths[2];

            // 4. 文档位置（行号拼接）
            std::string doc_lines;
            for (size_t i = 0; i < stats.doc_lines.size(); i++) {
                if (i > 0) doc_lines += ",";
                doc_lines += std::to_string(stats.doc_lines[i]);
            }
            if (doc_lines.size() > 15) {
                doc_lines = doc_lines.substr(0, 12) + "...";
            }
            HPDF_Page_Rectangle(page, table_x, current_y, col_widths[3], row_height);
            HPDF_Page_Stroke(page);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + 2, current_y + row_height/2 - 4, doc_lines.c_str());
            HPDF_Page_EndText(page);
            table_x += col_widths[3];

            // 5. 识别结果
            std::string result = (stats.match_count > 0) ? "OK" : "NG";
            HPDF_Page_Rectangle(page, table_x, current_y, col_widths[4], row_height);
            HPDF_Page_Stroke(page);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + 2, current_y + row_height/2 - 4, result.c_str());
            HPDF_Page_EndText(page);
            table_x += col_widths[4];

            // 6. 出现次数
            std::string count = std::to_string(stats.total_occurrences);
            HPDF_Page_Rectangle(page, table_x, current_y, col_widths[5], row_height);
            HPDF_Page_Stroke(page);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + 2, current_y + row_height/2 - 4, count.c_str());
            HPDF_Page_EndText(page);
            table_x += col_widths[5];

            // 重置X坐标，移动到下一行
            table_x = 20;
            current_y -= row_height;
        }
    }

    // 保存PDF文件
    if (HPDF_SaveToFile(pdf, pdf_path.c_str()) != HPDF_OK) {
        std::cerr << "保存PDF失败：" << pdf_path << std::endl;
        HPDF_Free(pdf);
        return -1;
    }

    // 释放资源
    HPDF_Free(pdf);
    std::cout << "PDF报告生成成功：" << pdf_path << std::endl;
    return 0;
}