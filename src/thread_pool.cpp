#include "thread_pool.h"
#include "ocr_processor.h"
#include "csv_parser.h"
#include <iostream>
#include <cstdlib>

void* lang_process_worker(void* arg) {
    LangTask* task = static_cast<LangTask*>(arg);
    std::cout << "线程启动：处理语种 [" << task->lang << "]，待处理图片数：" << task->image_paths.size() << std::endl;

    // 遍历该语种的所有待处理图片
    for (const auto& img_path : task->image_paths) {
        // 1. 执行OCR识别（指定当前语种）
        std::vector<OcrResult> ocr_results;
        if (ocr_image_by_lang(img_path, task->lang, ocr_results) != 0) {
            std::cerr << "OCR识别失败：" << img_path << "（语种：" << task->lang << "）" << std::endl;
            continue;
        }

        // 2. 文字匹配并生成统计信息
        std::vector<TextStats> img_stats;
        std::string img_id = extract_image_id_from_path(img_path);

        for (auto& [content, stats] : task->text_db) {
            TextStats img_stat = stats;
            img_stat.annotations.clear();
            img_stat.match_count = 0;

            // 遍历OCR结果，匹配当前文言
            for (const auto& ocr : ocr_results) {
                double sim = calculate_similarity(content, ocr.text);
                if (sim >= task->threshold) {
                    Annotation anno{ocr.left, ocr.top, ocr.width, ocr.height, true};
                    img_stat.annotations.push_back(anno);
                    img_stat.match_count++;
                }
            }

            // 如果无匹配，添加空标注（标记为失败）
            if (img_stat.annotations.empty()) {
                Annotation anno{-1, -1, 0, 0, false};
                img_stat.annotations.push_back(anno);
            }

            img_stats.push_back(img_stat);
        }

        // 3. 生成标注图片（保存到U盘）
        std::string img_name = img_path.substr(img_path.find_last_of('/') + 1);
        std::string annotated_img = task->udisk_mount + "/annotated_" + img_name;
        if (annotate_image(img_path, annotated_img, img_stats) != 0) {
            std::cerr << "图片标注失败：" << img_path << std::endl;
        }

        // 4. 保存结果到当前语种任务的结果集
        // std::lock_guard<std::mutex> lock(task->mutex);
        task->result[img_id] = img_stats;
    }

    std::cout << "线程完成：语种 [" << task->lang << "]" << std::endl;
    return nullptr;
}

std::vector<LangTask> split_tasks_by_lang(const std::map<std::string, TextStats>& text_db,
                                         const std::vector<std::string>& image_paths,
                                         double threshold) {
    std::map<std::string, LangTask> lang_task_map;

    // 按语种分组文言库
    for (const auto& [content, stats] : text_db) {
        std::string lang = stats.lang;
        if (lang_task_map.find(lang) == lang_task_map.end()) {
            LangTask task;
            task.lang = lang;
            task.threshold = threshold;
            task.image_paths = image_paths;
            lang_task_map[lang] = task;
        }
        lang_task_map[lang].text_db[content] = stats;
    }

    // 转换为vector返回
    std::vector<LangTask> tasks;
    for (auto& [lang, task] : lang_task_map) {
        tasks.push_back(std::move(task));
    }

    std::cout << "按语种拆分任务完成，共生成 " << tasks.size() << " 个语种任务" << std::endl;
    return tasks;
}

int run_multi_thread_tasks(std::vector<LangTask>& tasks, ResultSet& global_result) {
    if (tasks.empty()) {
        std::cerr << "无语种任务可处理" << std::endl;
        return -1;
    }

    // 创建线程数组
    std::vector<pthread_t> threads(tasks.size());

    // 启动每个语种的处理线程
    for (size_t i = 0; i < tasks.size(); i++) {
        if (pthread_create(&threads[i], nullptr, lang_process_worker, &tasks[i]) != 0) {
            std::cerr << "创建线程失败：语种 [" << tasks[i].lang << "]" << std::endl;
            // 销毁已创建的线程
            for (size_t j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
                pthread_join(threads[j], nullptr);
            }
            return -1;
        }
        std::cout << "创建线程：语种 [" << tasks[i].lang << "]，线程ID：" << threads[i] << std::endl;
    }

    // 等待所有线程完成，并合并结果到全局结果集
    for (size_t i = 0; i < tasks.size(); i++) {
        pthread_join(threads[i], nullptr);

        // 合并结果
        std::lock_guard<std::mutex> lock(global_result.mutex);
        for (auto& [img_id, stats] : tasks[i].result) {
            global_result.all_results[img_id].insert(
                global_result.all_results[img_id].end(),
                stats.begin(), stats.end()
            );
        }
    }

    return 0;
}