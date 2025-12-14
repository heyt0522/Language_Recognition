cmake_minimum_required(VERSION 3.10)
project(text_matcher)

# C++17 标准（支持std::filesystem）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -g")

# 查找系统依赖
find_package(PkgConfig REQUIRED)
pkg_check_modules(LEPTONICA REQUIRED lept)
pkg_check_modules(TESSERACT REQUIRED tesseract)

# libharu 配置（避免-lharu找不到，直接指定路径）
set(LIBHARU_INCLUDE_DIR /usr/include)
set(LIBHARU_LIBRARY /usr/lib/x86_64-linux-gnu/libhpdf.so)

# 头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${LEPTONICA_INCLUDE_DIRS}
    ${TESSERACT_INCLUDE_DIRS}
    ${LIBHARU_INCLUDE_DIR}
)

# 源文件列表
set(SOURCE_FILES
    src/Language_main.cpp
    src/cmd_parser.cpp
    src/csv_parser.cpp
    src/ocr_processor.cpp
    src/udisk_handler.cpp  # 空文件，仅兼容结构
    src/pdf_generator.cpp
    src/thread_pool.cpp
)

# 生成可执行文件
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# 链接依赖库
target_link_libraries(${PROJECT_NAME}
    ${LEPTONICA_LIBRARIES}
    ${TESSERACT_LIBRARIES}
    ${LIBHARU_LIBRARY}
    pthread  # 多线程依赖
    stdc++fs # 文件系统库
    m        # 数学库
)

# 安装配置（可选）
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION /usr/local/bin
)

# 复制tessdata（可选，开发板端使用）
if(EXISTS ${PROJECT_SOURCE_DIR}/config/tessdata)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/config/tessdata
        DESTINATION /usr/local/share/text_matcher
    )
endif()