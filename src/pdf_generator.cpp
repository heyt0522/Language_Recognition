#include "pdf_generator.h"
#include <hpdf.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include "data_struct.h"

// 递归创建目录
bool create_dir(const std::string& dir_path) {
    if (access(dir_path.c_str(), F_OK) == 0) return true;

    // 拆分父目录
    size_t pos = dir_path.find_last_of("/");
    if (pos != std::string::npos) {
        create_dir(dir_path.substr(0, pos));
    }

    // 创建当前目录
    if (mkdir(dir_path.c_str(), 0755) == -1) {
        std::cerr << "目录创建失败：" << dir_path << std::endl;
        return false;
    }
    return true;
}

// PDF错误回调
void pdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    std::cerr << "PDF错误：" << error_no << ", " << detail_no << std::endl;
}

// 加载多语种字体（适配特殊字符）
HPDF_Font load_multilang_font(HPDF_Doc pdf) {
    // 优先加载系统通用字体（支持多语种）
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", "UTF-8");
    if (!font) {
        font = HPDF_GetFont(pdf, "Courier", "UTF-8");
    }

    return font;
}

// 生成PDF报告（适配多语种）
bool generate_pdf(
    const std::string& pdf_path,
    const std::vector<OcrResult>& all_results
) {
    // 创建PDF输出目录
    size_t pos = pdf_path.find_last_of("/");
    if (pos != std::string::npos) {
        create_dir(pdf_path.substr(0, pos));
    }

    // 初始化PDF文档（UTF-8编码）
    HPDF_Doc pdf = HPDF_New(pdf_error_handler, nullptr);
    if (!pdf) {
        std::cerr << "PDF文档创建失败！" << std::endl;
        return false;
    }

    // 设置PDF全局编码为UTF-8
    HPDF_SetCurrentEncoder(pdf, "UTF-8");

    // 加载多语种字体
    HPDF_Font font = load_multilang_font(pdf);
    if (!font) {
        std::cerr << "加载多语种字体失败，使用默认字体！" << std::endl;
        font = HPDF_GetFont(pdf, "Helvetica", nullptr);
    }

    // 遍历结果生成PDF页面
    for (const auto& res : all_results) {
        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_LANDSCAPE); // 横向A4（适配多列）
        HPDF_Page_SetFontAndSize(page, font, 10); // 缩小字体适配多列

        // 1. 插入标注图片（适配png）
        if (access(res.annotated_img.c_str(), F_OK) == 0) {
            HPDF_Image img = HPDF_LoadImageFromFile(pdf, res.annotated_img.c_str());
            if (img) {
                float img_w = HPDF_Image_GetWidth(img);
                float img_h = HPDF_Image_GetHeight(img);
                float page_w = HPDF_Page_GetWidth(page) - 40;
                float scale = page_w / img_w;
                float draw_h = img_h * scale;
                float draw_y = HPDF_Page_GetHeight(page) - draw_h - 150;
                HPDF_Page_DrawImage(page, img, 20, draw_y, page_w, draw_h);
            }
        }

        // 2. 绘制扩展表格（9列：序号/StringID/ScreenID/PartID/语种/图片ID/文言/识别状态/出现次数）
        const float col_widths[9] = {50, 80, 80, 80, 50, 100, 120, 60, 50}; // 适配横向A4
        const float row_height = 25;
        float table_x = 20;
        float table_y = HPDF_Page_GetHeight(page) - 170;

        // 表格标题
        std::string headers[9] = {
            "序号", "String ID", "ScreenID", "PartID", 
            "语种", "图片ID", "文言内容", "识别状态", "出现次数"
        };
        HPDF_Page_SetRGBFill(page, 0, 0, 0);
        for (int i = 0; i < 9; i++) {
            HPDF_Page_Rectangle(page, table_x, table_y, col_widths[i], row_height);
            HPDF_Page_Stroke(page);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + 5, table_y + 8, headers[i].c_str());
            HPDF_Page_EndText(page);
            table_x += col_widths[i];
        }

        // 表格数据
        table_x = 20;
        table_y -= row_height;
        std::string status = res.is_ok ? "OK" : "FAIL";
        std::string data[9] = {
            res.seq_id, res.string_id, res.screen_id, res.part_id,
            res.lang, res.img_id, res.text, status, std::to_string(res.count)
        };

        // 适配右对齐语言（阿拉伯语）
        for (int i = 0; i < 9; i++) {
            HPDF_Page_Rectangle(page, table_x, table_y, col_widths[i], row_height);
            HPDF_Page_Stroke(page);
            HPDF_Page_BeginText(page);
            
            float text_x = table_x + 5;
            // 阿拉伯语文本右对齐（文言内容列）
            if (res.lang_code == "ara" && i == 6) {
                text_x = table_x + col_widths[i] - data[i].size()*8 - 5;
            }
            
            HPDF_Page_TextOut(page, text_x, table_y + 8, data[i].c_str());
            HPDF_Page_EndText(page);
            table_x += col_widths[i];
        }
    }

    // 保存PDF
    if (HPDF_SaveToFile(pdf, pdf_path.c_str()) != HPDF_OK) {
        std::cerr << "PDF保存失败！" << std::endl;
        HPDF_Free(pdf);
        return false;
    }

    HPDF_Free(pdf);
    return true;
}