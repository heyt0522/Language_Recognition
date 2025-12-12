// include/thread_pool.h
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <vector>
#include "data_struct.h"

// 线程处理函数（按语种处理）
void* lang_process_worker(void* arg);

// 拆分CSV数据为多语种任务
std::vector<LangTask> split_tasks_by_lang(const std::map<std::string, TextStats>& text_db, 
                                         const std::vector<std::string>& image_paths,
                                         double threshold);

// 启动多线程处理
int run_multi_thread_tasks(std::vector<LangTask>& tasks, ResultSet& global_result);

#endif // THREAD_POOL_H

// src/thread_pool.cpp
#include "thread_pool.h"
#include "ocr_processor.h"
#include <iostream>
#include <dirent.h>

// 单语种处理逻辑（线程入口）
void* lang_process_worker(void* arg) {
    LangTask* task = static_cast<LangTask*>(arg);
    std::cout << "线程启动：处理语种 [" << task->lang << "]，图片数：" << task->image_paths.size() << std::endl;

    for (const auto& img_path : task->image_paths) {
        // 1. OCR识别当前图片（指定语种）
        std::vector<OcrResult> ocr_results;
        if (ocr_image_by_lang(img_path, task->lang, ocr_results) != 0) {
            std::cerr << "OCR识别失败：" << img_path << std::endl;
            continue;
        }

        // 2. 文字匹配
        std::vector<TextStats> img_stats;
        for (auto& [content, stats] : task->text_db) {
            TextStats img_stat = stats;
            img_stat.annotations.clear();
            img_stat.match_count = 0;

            for (const auto& ocr : ocr_results) {
                double sim = calculate_similarity(content, ocr.text);
                if (sim >= task->threshold) {
                    Annotation anno{ocr.left, ocr.top, ocr.width, ocr.height, true};
                    img_stat.annotations.push_back(anno);
                    img_stat.match_count++;
                }
            }
            img_stats.push_back(img_stat);
        }

        // 3. 标注图片
        std::string img_name = img_path.substr(img_path.find_last_of('/') + 1);
        std::string annotated_img = task->udisk_mount + "/annotated_" + img_name;
        if (annotate_image(img_path, annotated_img, img_stats) != 0) {
            std::cerr << "图片标注失败：" << img_path << std::endl;
        }

        // 4. 保存结果到全局集
        std::lock_guard<std::mutex> lock(task->mutex);
        std::string img_id = extract_image_id_from_path(img_path);
        task->result[img_id] = img_stats;
    }

    std::cout << "线程完成：语种 [" << task->lang << "]" << std::endl;
    return nullptr;
}

// 按语种拆分任务
std::vector<LangTask> split_tasks_by_lang(const std::map<std::string, TextStats>& text_db,
                                         const std::vector<std::string>& image_paths,
                                         double threshold) {
    std::map<std::string, LangTask> lang_tasks_map;

    // 按语种分组文言
    for (const auto& [content, stats] : text_db) {
        std::string lang = stats.lang;
        if (lang_tasks_map.find(lang) == lang_tasks_map.end()) {
            lang_tasks_map[lang] = {
                lang,
                {},
                image_paths,
                threshold,
                {},
                std::mutex()
            };
        }
        lang_tasks_map[lang].text_db[content] = stats;
    }

    // 转换为vector
    std::vector<LangTask> tasks;
    for (auto& [lang, task] : lang_tasks_map) {
        tasks.push_back(std::move(task));
    }
    return tasks;
}

// 启动多线程
int run_multi_thread_tasks(std::vector<LangTask>& tasks, ResultSet& global_result) {
    std::vector<pthread_t> threads(tasks.size());

    // 创建线程
    for (size_t i = 0; i < tasks.size(); i++) {
        if (pthread_create(&threads[i], nullptr, lang_process_worker, &tasks[i]) != 0) {
            std::cerr << "创建线程失败：语种 " << tasks[i].lang << std::endl;
            return -1;
        }
    }

    // 等待所有线程完成
    for (size_t i = 0; i < threads.size(); i++) {
        pthread_join(threads[i], nullptr);

        // 合并线程结果到全局
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