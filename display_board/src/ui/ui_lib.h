#pragma once

#include <lvgl.h>
#include <string>
#include <cstring>

void show_toast(const char *text, uint32_t duration_ms = 2000);

inline void safeCopyFileName(char *dst, size_t dstSize, const std::string &src) {
    size_t len = src.length();
    if (len >= dstSize) len = dstSize - 1;
    memcpy(dst, src.c_str(), len);
    dst[len] = '\0';
}