#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <string>
#include <vector>
#include <map>

// 语种编码映射（Tesseract标准编码）
const std::map<std::string, std::string> LANG_CODE_MAP = {
    {"英语", "eng"},
    {"法语", "fra"},
    {"意大利语", "ita"},
    {"土耳其语", "tur"},
    {"西班牙语", "spa"},
    {"葡萄牙语", "por"},
    {"泰语", "tha"},
    {"阿拉伯语", "ara"},
    {"俄语", "rus"},
    {"德语", "deu"}
};

// CSV解析后的元数据结构体
struct CsvMeta {
    int line_num;               // CSV行号
    std::string seq_id;         // 序号（如24438）
    std::string module;         // 模块（如MultiLanguageTable（Operation））
    std::string desc;           // 描述（如车机已启动）
    std::string lang_text;      // 目标文言（精准提取的内容）
    std::string string_id;      // 核心标识（如MM_00_06_04）
    std::string screen_id;      // 屏幕ID（如MM_00_06_04）
    std::string part_id;        // 部件ID（如1_1_1_A_1）
    std::string lang;           // 语种（自动识别，如英语）
};

// 单条OCR识别结果
struct OcrResult {
    // 基础字段
    std::string lang;          // 语种（中文名称，如：英语）
    std::string lang_code;     // 语种编码（如：eng）
    std::string img_id;        // 图片ID（完整文件名，如MM_02_01_01_04）
    std::string text;          // 识别文本
    bool is_ok;                // 识别是否成功
    int count;                 // 该文本出现次数
    std::string annotated_img; // 标注后图片路径
    std::vector<int> box;      // 识别点位框 [x, y, w, h]
    
    // CSV元数据字段
    std::string seq_id;         // CSV序号（24438）
    std::string string_id;      // String ID（MM_00_06_04）
    std::string screen_id;      // ScreenID
    std::string part_id;        // PartID
    std::string doc_position;   // 文档位置（CSV行号+模块）
};

// 多语种任务结构体（线程池用）- 完全移除mutex
struct LangTask {
    std::string lang;                      // 语种（中文名称）
    std::string lang_code;                 // 语种编码（Tesseract用）
    std::vector<std::pair<std::string, CsvMeta>> img_meta_list; // 图片路径+CSV元数据
    double confidence_threshold;           // 识别置信度阈值
    std::string output_dir;                // 本地输出目录（替代U盘）

    // 显式声明默认构造/拷贝/移动（确保编译器生成）
    LangTask() = default;
    LangTask(const LangTask&) = default;
    LangTask(LangTask&&) = default;
    LangTask& operator=(const LangTask&) = default;
    LangTask& operator=(LangTask&&) = default;
    ~LangTask() = default;
};

// 全局统计：文本出现次数
extern std::map<std::string, int> g_text_count_map;

#endif // DATA_STRUCT_H