// #include "udisk_handler.h"
// #include <iostream>
// #include <cstdio>
// #include <cstring>
// #include <sys/mount.h>
// #include <sys/stat.h>
// #include <unistd.h>
// #include <errno.h>

// int prepare_mount_point(const std::string& mount_point) {
//     struct stat st;
//     if (stat(mount_point.c_str(), &st) != 0) {
//         // 目录不存在，创建
//         if (mkdir(mount_point.c_str(), 0777) != 0) {
//             std::cerr << "创建挂载点失败：" << mount_point << "，错误：" << strerror(errno) << std::endl;
//             return -1;
//         }
//         std::cout << "创建挂载点：" << mount_point << std::endl;
//     } else if (!S_ISDIR(st.st_mode)) {
//         std::cerr << "挂载点不是目录：" << mount_point << std::endl;
//         return -1;
//     }
//     return 0;
// }

// int find_udisk_device(char* device, int len) {
//     // 执行命令查找U盘设备（/dev/sd*1）
//     FILE* fp = popen("ls /dev/sd*1 2>/dev/null | head -n 1", "r");
//     if (!fp) {
//         std::cerr << "执行命令失败：ls /dev/sd*1" << std::endl;
//         return -1;
//     }

//     if (fgets(device, len, fp) == NULL) {
//         pclose(fp);
//         std::cerr << "未找到U盘设备" << std::endl;
//         return -1;
//     }

//     // 去除换行符
//     device[strcspn(device, "\n")] = '\0';
//     pclose(fp);
//     return 0;
// }

// int mount_udisk(const char* device, const char* mount_point) {
//     // 检查并创建挂载点
//     if (prepare_mount_point(mount_point) != 0) {
//         return -1;
//     }

//     // 先尝试卸载（防止已挂载）
//     umount(mount_point);

//     // 挂载FAT32格式（优先）
//     int ret = mount(device, mount_point, "vfat", MS_NOATIME, "utf8=1");
//     if (ret != 0) {
//         // 尝试EXT4格式
//         ret = mount(device, mount_point, "ext4", MS_NOATIME, NULL);
//         if (ret != 0) {
//             std::cerr << "挂载失败：" << device << " -> " << mount_point 
//                       << "，错误：" << strerror(errno) << std::endl;
//             return -1;
//         }
//     }

//     std::cout << "U盘挂载成功：" << device << " -> " << mount_point << std::endl;
//     return 0;
// }

// int umount_udisk(const char* mount_point) {
//     if (umount(mount_point) != 0) {
//         std::cerr << "卸载U盘失败：" << mount_point << "，错误：" << strerror(errno) << std::endl;
//         return -1;
//     }
//     std::cout << "U盘卸载成功：" << mount_point << std::endl;
//     return 0;
// }