#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <pthread.h>
#include "data_struct.h"

// 线程池初始化
bool init_thread_pool(int thread_num = 4);

// 恢复const引用参数
void submit_tasks(const std::vector<LangTask>& tasks);

// 获取所有识别结果
std::vector<OcrResult> get_all_results();

// 销毁线程池
void destroy_thread_pool();

#endif // THREAD_POOL_H