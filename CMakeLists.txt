cmake_minimum_required(VERSION 3.10)
project(text_matcher)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖库
find_package(PkgConfig REQUIRED)
pkg_check_modules(TESSERACT REQUIRED tesseract)
pkg_check_modules(LEPTONICA REQUIRED lept)
pkg_check_modules(HPDF REQUIRED libharu)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${TESSERACT_INCLUDE_DIRS}
    ${LEPTONICA_INCLUDE_DIRS}
    ${HPDF_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

# 源文件列表（确保包含所有cpp）
set(SOURCES
    src/Language_main.cpp
    src/csv_parser.cpp
    src/ocr_processor.cpp
    src/thread_pool.cpp
    src/pdf_generator.cpp
    src/cmd_parser.cpp
    # 如果有Language_main.cpp，替换main.cpp
    # src/Language_main.cpp
)

# 生成可执行文件
add_executable(text_matcher ${SOURCES})

# 在target_link_libraries前添加：
set(LIBHARU_LIB "/usr/lib/x86_64-linux-gnu/libhpdf.so")  # 替换为你的实际路径
# 链接库
target_link_libraries(text_matcher
    ${TESSERACT_LIBRARIES}
    ${LEPTONICA_LIBRARIES}
    ${LIBHARU_LIB}
    ${OpenCV_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    pthread
    m
)