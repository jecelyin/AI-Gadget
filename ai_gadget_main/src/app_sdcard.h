#pragma once


#include <BinaryPacker.h>

#define MOUNT_POINT "/sdcard"

bool is_sdcard_mounted();
void app_sdcard_init();
void listFiles(const char* dirPath, BinaryPacker& b, bool hasParent, uint16_t page);
bool file_exists(const char* file);
String getParentPath(String path);
void writeFile(const char *file, const uint8_t *data, size_t size, const char *mode = "wb");