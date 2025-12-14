#include "thread_pool.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <thread>
#include "ocr_processor.h"
#include "data_struct.h"

// 线程池全局变量
static pthread_t* g_threads = nullptr;
static int g_thread_num = 0;
static std::vector<LangTask> g_tasks;
static std::vector<OcrResult> g_all_results;
static std::mutex g_task_mutex;
static std::mutex g_result_mutex;
static bool g_stop = false;

// 线程工作函数
void* worker_thread(void* arg) {
    int thread_id = *(int*)arg;
    delete (int*)arg;

    while (!g_stop) {
        LangTask task;
        bool has_task = false;

        {
            std::lock_guard<std::mutex> lock(g_task_mutex);
            if (!g_tasks.empty()) {
                task = g_tasks.back();
                g_tasks.pop_back();
                has_task = true;
            }
        }

        if (!has_task) {
            usleep(10000);
            continue;
        }

        std::cout << "线程" << thread_id << "开始处理语种：" << task.lang << "（编码：" << task.lang_code << "）" << std::endl;
        for (const auto& [img_path, csv_meta] : task.img_meta_list) {
            OcrResult res = process_image(
                img_path, 
                csv_meta, 
                task.confidence_threshold
            );
            
            {
                std::lock_guard<std::mutex> lock(g_result_mutex);
                g_all_results.push_back(res);
            }
        }
        std::cout << "线程" << thread_id << "完成语种：" << task.lang << "的处理！共处理" << task.img_meta_list.size() << "张图片" << std::endl;
    }

    pthread_exit(nullptr);
}

// 初始化线程池
bool init_thread_pool(int thread_num) {
    if (thread_num <= 0) thread_num = std::min(10, (int)std::thread::hardware_concurrency());
    g_thread_num = thread_num;
    g_threads = new pthread_t[thread_num];

    for (int i = 0; i < thread_num; i++) {
        int* id = new int(i);
        if (pthread_create(&g_threads[i], nullptr, worker_thread, id) != 0) {
            std::cerr << "线程" << i << "创建失败！" << std::endl;
            return false;
        }
    }

    g_stop = false;
    return true;
}

// 提交任务（恢复const引用）
void submit_tasks(const std::vector<LangTask>& tasks) {
    std::lock_guard<std::mutex> lock(g_task_mutex);
    g_tasks = tasks;
}

// 获取所有识别结果
std::vector<OcrResult> get_all_results() {
    while (true) {
        std::lock_guard<std::mutex> lock(g_task_mutex);
        if (g_tasks.empty()) break;
        usleep(100000);
    }

    g_stop = true;
    for (int i = 0; i < g_thread_num; i++) {
        pthread_join(g_threads[i], nullptr);
    }

    return g_all_results;
}

// 销毁线程池
void destroy_thread_pool() {
    if (g_threads) {
        delete[] g_threads;
        g_threads = nullptr;
    }
    g_thread_num = 0;
    g_all_results.clear();
}