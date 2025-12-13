cmake_minimum_required(VERSION 3.10)
project(text_matcher)

# ===================== 基础配置 =====================
# 设置C++11标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译模式（Debug/Release）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)  # 生产环境用Release，调试用Debug
endif()

# 编译选项（优化+警告）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")  # Debug模式保留调试信息
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")  # Release模式优化

# ===================== 依赖库查找 =====================
# 1. 查找Leptonica（Tesseract依赖）
find_package(PkgConfig REQUIRED)
pkg_check_modules(LEPTONICA REQUIRED lept)

# 2. 查找Tesseract OCR
pkg_check_modules(TESSERACT REQUIRED tesseract)

# 3. 查找libharu（PDF生成）
pkg_check_modules(LIBHARU REQUIRED libharu)  # Ubuntu20.04+可能是libharu，需替换为libharu

# 4. 查找pthread（多线程）
find_package(Threads REQUIRED)

# 5. 查找X11（CImg图像处理）
find_package(X11 REQUIRED)

# ===================== 头文件/库路径 =====================
# 包含头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${LEPTONICA_INCLUDE_DIRS}
    ${TESSERACT_INCLUDE_DIRS}
    ${LIBHARU_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
)

# 链接库目录
link_directories(
    ${LEPTONICA_LIBRARY_DIRS}
    ${TESSERACT_LIBRARY_DIRS}
    ${LIBHARU_LIBRARY_DIRS}
)

# ===================== 源文件 =====================
# 收集所有源码文件
file(GLOB SRC_FILES 
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 排除Ubuntu端不需要的U盘处理（可选）
# list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/src/udisk_handler.cpp)

# ===================== 生成可执行文件 =====================
add_executable(${PROJECT_NAME} ${SRC_FILES})

# ===================== 链接依赖库 =====================
target_link_libraries(${PROJECT_NAME}
    ${LEPTONICA_LIBRARIES}
    ${TESSERACT_LIBRARIES}
    ${LIBHARU_LIBRARIES}
    ${X11_LIBRARIES}
    Threads::Threads
    m  # 数学库
)

# ===================== 安装配置（可选） =====================
# 安装可执行文件到/usr/local/bin
install(TARGETS ${PROJECT_NAME} 
        RUNTIME DESTINATION /usr/local/bin)

# 安装Tesseract语言包到系统目录
install(DIRECTORY ${PROJECT_SOURCE_DIR}/config/tessdata/
        DESTINATION /usr/share/tessdata
        FILES_MATCHING PATTERN "*.traineddata")