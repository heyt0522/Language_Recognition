#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <string>
#include <vector>
#include <map>
#include <mutex>

// OCR识别结果
struct OcrResult {
    std::string text;   // 识别文字
    int left;           // 左上角X
    int top;            // 左上角Y
    int width;          // 宽度
    int height;         // 高度
};

// 标注信息
struct Annotation {
    int left;          // X坐标
    int top;           // Y坐标
    int width;         // 宽度
    int height;        // 高度
    bool is_ok;        // 匹配是否成功
};

// 单条文言统计
struct TextStats {
    std::string lang;          // 语种（如Italiano/English）
    std::string image_id;      // 图片ID
    std::string content;       // 文言内容
    std::vector<int> doc_lines;// 文档中出现行号
    int total_occurrences;     // 总出现次数
    int match_count;           // 匹配成功次数
    std::vector<Annotation> annotations; // 标注列表
};

// 按语种分组的任务
struct LangTask {
    std::string lang;                          // 语种
    std::map<std::string, TextStats> text_db;  // 该语种的文言库
    std::vector<std::string> image_paths;      // 待处理图片路径
    double threshold;                          // 匹配阈值
    std::map<std::string, std::vector<TextStats>> result; // 该语种处理结果
    // std::mutex mutex;                          // 线程安全锁
    std::string udisk_mount;                   // U盘挂载点
    std::string output_dir = "/home/he_yt/Multilingual_Recognition/code/language_result";    //新增本地存储点
};

// 命令行参数
struct CmdArgs {
    std::string csv_path;       // CSV文件路径
    std::string image_dir;      // 图片目录
    double threshold;           // 匹配阈值（默认0.7）
    std::string udisk_mount;    // U盘挂载点
};

// 全局结果集
struct ResultSet {
    std::map<std::string, std::vector<TextStats>> all_results; // 按图片ID分组
    std::mutex mutex;                                          // 全局结果锁
};

#endif // DATA_STRUCT_H