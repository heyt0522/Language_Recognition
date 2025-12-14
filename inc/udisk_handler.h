// #ifndef UDISK_HANDLER_H
// #define UDISK_HANDLER_H

// #include <string>

// /**
//  * 查找U盘设备节点（匹配/dev/sd*1）
//  * @param device 输出：设备节点路径（如/dev/sdb1）
//  * @param len 设备节点缓冲区长度
//  * @return 0:找到, -1:未找到/失败
//  */
// int find_udisk_device(char* device, int len);

// /**
//  * 挂载U盘
//  * @param device 设备节点路径
//  * @param mount_point 挂载点路径
//  * @return 0:成功, -1:失败
//  */
// int mount_udisk(const char* device, const char* mount_point);

// /**
//  * 卸载U盘
//  * @param mount_point 挂载点路径
//  * @return 0:成功, -1:失败
//  */
// int umount_udisk(const char* mount_point);

// /**
//  * 检查挂载点是否存在，不存在则创建
//  * @param mount_point 挂载点路径
//  * @return 0:成功, -1:失败
//  */
// int prepare_mount_point(const std::string& mount_point);

// #endif // UDISK_HANDLER_H