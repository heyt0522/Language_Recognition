#include <iostream>
#include "cmd_parser.h"
#include "csv_parser.h"
#include "ocr_processor.h"  // 引入OCR函数声明
#include "pdf_generator.h"
#include "thread_pool.h"
#include "data_struct.h"

std::map<std::string, int> g_text_count_map;

int main(int argc, char** argv) {
    // 1. 解析命令行参数
    CmdParams params = parse_cmd_args(argc, argv);
    if (!params.is_valid) {
        print_usage();
        return -1;
    }

    // 2. 初始化OCR引擎
    if (!init_ocr_engine("/home/he_yt/Multilingual_Recognition/code/Language_Recognition/config/tessdata")) {
        std::cerr << "OCR引擎初始化失败！" << std::endl;
        return -1;
    }

    // 3. 解析CSV文件
    auto csv_data = parse_csv(params.csv_path);
    if (csv_data.empty()) {
        std::cerr << "CSV解析失败或无有效多语种数据！" << std::endl;
        return -1;
    }

    // 4. 按语种拆分任务
    auto tasks = split_tasks_by_lang(
        csv_data,
        params.img_dir,
        params.confidence,
        params.pdf_output.substr(0, params.pdf_output.find_last_of("/"))
    );

    // 5. 初始化线程池并提交任务
    if (!init_thread_pool(8)) {
        std::cerr << "线程池初始化失败！" << std::endl;
        return -1;
    }
    submit_tasks(tasks);

    // 6. 获取识别结果并生成PDF
    auto all_results = get_all_results();
    if (!generate_pdf(params.pdf_output, all_results)) {
        std::cerr << "PDF生成失败！" << std::endl;
        return -1;
    }

    // 7. 资源释放
    release_ocr_engine();
    destroy_thread_pool();

    std::cout << "多语种识别任务完成！" << std::endl;
    std::cout << "PDF路径：" << params.pdf_output << std::endl;
    return 0;
}