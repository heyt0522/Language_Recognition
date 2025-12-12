#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <pthread.h>
#include "data_struct.h"

/**
 * 语种处理线程入口函数
 * @param arg 传入LangTask指针
 * @return 空指针
 */
void* lang_process_worker(void* arg);

/**
 * 按语种拆分CSV任务
 * @param text_db 全局文言库
 * @param image_paths 图片路径列表
 * @param threshold 匹配阈值
 * @return 按语种分组的任务列表
 */
std::vector<LangTask> split_tasks_by_lang(const std::map<std::string, TextStats>& text_db, 
                                         const std::vector<std::string>& image_paths,
                                         double threshold);

/**
 * 启动多线程处理语种任务
 * @param tasks 语种任务列表
 * @param global_result 全局结果集
 * @return 0:成功, -1:失败
 */
int run_multi_thread_tasks(std::vector<LangTask>& tasks, ResultSet& global_result);

#endif // THREAD_POOL_H