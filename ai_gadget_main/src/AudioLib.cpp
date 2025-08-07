#include "AudioLib.h"

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <cstdio>

// 比特率表 (单位: kbps)，按 Layer III 和 MPEG 1 标准
const int BITRATE_TABLE[16] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
// 采样率表 (单位: Hz)，按 MPEG 1 标准
const int SAMPLERATE_TABLE[4] = {44100, 48000, 32000, 0};

// 字符串比较，忽略大小写
int strcasecmp_ext(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        }
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}


void format_duration(uint32_t total_seconds, char *buffer) {
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;
    sprintf(buffer, "%02u:%02u", (unsigned int)minutes, (unsigned int)seconds);
}

// void setup() {
//     Serial.begin(9600);
//     if (!sd.begin(10)) { // 初始化 SD 卡，CS 引脚为 10
//         Serial.println("SD card initialization failed!");
//         return;
//     }

//     const char *file_path = "example.MP3"; // 替换为你的文件路径
//     char time_format[6];

//     uint32_t duration = get_audio_duration(file_path);
//     if (duration > 0) {
//         format_duration(duration, time_format);
//         Serial.print("Audio Duration: ");
//         Serial.println(time_format);
//     } else {
//         Serial.println("Could not determine audio duration.");
//     }
// }
