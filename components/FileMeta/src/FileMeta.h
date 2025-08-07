#ifndef FILE_META_H
#define FILE_META_H

#include <stdint.h>

struct FileMeta {
    bool isFile;        // 标识是否为文件
    char fileName[80];  // 文件名
    uint32_t size;      // 文件大小
    uint32_t duration;  // 文件时长（如音乐文件时长）
};

bool compareFileMeta(const FileMeta& a, const FileMeta& b);

#endif // FILE_META_H
