#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <unistd.h>
#include "cmd_parser.h"
#include "csv_parser.h"
#include "udisk_handler.h"
#include "thread_pool.h"
#include "pdf_generator.h"

/**
 * 获取指定目录下的所有图片路径
 * @param dir 目录路径
 * @return 图片路径列表
 */
std::vector<std::string> get_image_paths(const std::string& dir) {
    std::vector<std::string> paths;
    DIR* dp = opendir(dir.c_str());
    if (!dp) {
        std::cerr << "无法打开图片目录：" << dir << std::endl;
        return paths;
    }

    struct dirent* entry;
    while ((entry = readdir(dp)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }

        // 过滤图片格式（png/jpg/jpeg）
        std::string ext = name.substr(name.find_last_of('.') + 1);
        if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "PNG" || ext == "JPG" || ext == "JPEG") {
            paths.push_back(dir + "/" + name);
        }
    }

    closedir(dp);
    std::cout << "图片目录扫描完成，共发现 " << paths.size() << " 张图片" << std::endl;
    return paths;
}

int main(int argc, char* argv[]) {
    std::cout << "===== 正点原子I.MX6ULL多语言文字识别匹配系统 =====" << std::endl;

    // 1. 解析命令行参数
    CmdArgs args;
    if (parse_cmd_args(argc, argv, args) != 0) {
        return -1;
    }

    // 2. 检测并挂载U盘
    char udisk_dev[64];
    if (find_udisk_device(udisk_dev, sizeof(udisk_dev)) != 0) {
        return -1;
    }
    std::cout << "检测到U盘设备：" << udisk_dev << std::endl;

    if (mount_udisk(udisk_dev, args.udisk_mount.c_str()) != 0) {
        return -1;
    }

    // 3. 解析CSV文件
    std::map<std::string, TextStats> text_db;
    if (parse_csv(args.csv_path, text_db) != 0) {
        umount_udisk(args.udisk_mount.c_str());
        return -1;
    }
    std::cout << "CSV解析完成，共提取 " << text_db.size() << " 条独特文言" << std::endl;

    // 4. 获取图片列表
    std::vector<std::string> image_paths = get_image_paths(args.image_dir);
    if (image_paths.empty()) {
        std::cerr << "图片目录中无有效图片" << std::endl;
        umount_udisk(args.udisk_mount.c_str());
        return -1;
    }

    // 5. 按语种拆分任务
    std::vector<LangTask> lang_tasks = split_tasks_by_lang(text_db, image_paths, args.threshold);

    // 6. 多线程处理语种任务
    ResultSet global_result;
    if (run_multi_thread_tasks(lang_tasks, global_result) != 0) {
        std::cerr << "多线程处理失败" << std::endl;
        umount_udisk(args.udisk_mount.c_str());
        return -1;
    }

    // 7. 生成PDF报告（保存到U盘）
    std::string pdf_path = args.udisk_mount + "/recognition_result.pdf";
    if (generate_pdf(pdf_path, global_result) != 0) {
        std::cerr << "PDF报告生成失败" << std::endl;
    }

    // 8. 卸载U盘
    umount_udisk(args.udisk_mount.c_str());

    std::cout << "===== 处理完成！所有结果已保存至U盘 =====" << std::endl;
    return 0;
}