#pragma once
#include <stdint.h>
#include <string.h>

#define AUDIO_PAGE_SIZE 15
#define FILE_NAME_LEN 50
/**
    // 示例：打包和解包字符串与无符号整数
    VoiceStatusStruct voiceStatus;

    // 打包字符串
    const char* message = "Hello";
    PACK_STR(message, voiceStatus.data, sizeof(voiceStatus.data));

    // 打包无符号8位整数
    uint8_t number8 = 42;
    PACK_UINT8(number8, voiceStatus.data + strlen(message)); // 紧接着字符串后面

    // 打包无符号16位整数
    uint16_t number16 = 1234;
    PACK_UINT16(number16, voiceStatus.data + strlen(message) + sizeof(uint8_t)); // 紧接着无符号8位整数后面

    // 打包无符号32位整数
    uint32_t number32 = 56789;
    PACK_UINT32(number32, voiceStatus.data + strlen(message) + sizeof(uint8_t) + sizeof(uint16_t)); // 紧接着无符号16位整数后面


   // 输出打包后的数据
   Serial.print("Packed string: ");
   Serial.println((char*)voiceStatus.data);

   uint8_t unpackedNumber8;
   uint16_t unpackedNumber16;
   uint32_t unpackedNumber32;

   // 解包无符号8位整数
   UNPACK_UINT8(voiceStatus.data + strlen(message), unpackedNumber8);
   Serial.print("Unpacked uint8_t: ");
   Serial.println(unpackedNumber8);

   // 解包无符号16位整数
   UNPACK_UINT16(voiceStatus.data + strlen(message) + sizeof(uint8_t), unpackedNumber16);
   Serial.print("Unpacked uint16_t: ");
   Serial.println(unpackedNumber16);

   // 解包无符号32位整数
   UNPACK_UINT32(voiceStatus.data + strlen(message) + sizeof(uint8_t) + sizeof(uint16_t), unpackedNumber32);
   Serial.print("Unpacked uint32_t: ");
   Serial.println(unpackedNumber32);
 */
// 定义宏用于打包和解包
#define PACK_STR(dest, src, max_len) do { \
    strncpy((char*)dest, src, max_len); \
    dest[max_len - 1] = '\0'; /* 确保字符串以 null 结尾 */ \
} while(0);

#define UNPACK_STR(dest, src, max_len) do { \
    strncpy(dest, (char*)src, max_len); \
    dest[max_len - 1] = '\0'; /* 确保字符串以 null 结尾 */ \
} while(0);

#define PACK_UINT8(dest, src) do { \
    dest[0] = src; \
} while(0);

#define UNPACK_UINT8(dest, src) do { \
    dest = src[0]; \
} while(0);

#define PACK_UINT16(dest, src) do { \
    dest[0] = (src >> 8) & 0xFF; /* 高字节 */ \
    dest[1] = src & 0xFF;        /* 低字节 */ \
} while(0);

#define UNPACK_UINT16(dest, src) do { \
    dest = ((uint16_t)src[0] << 8) | (uint16_t)src[1]; \
} while(0);

#define PACK_UINT32(dest, src) do { \
    dest[0] = (src >> 24) & 0xFF; /* 字节3 */ \
    dest[1] = (src >> 16) & 0xFF; /* 字节2 */ \
    dest[2] = (src >> 8) & 0xFF;  /* 字节1 */ \
    dest[3] = src & 0xFF;         /* 字节0 */ \
} while(0);

#define UNPACK_UINT32(dest, src) do { \
    dest = ((uint32_t)src[0] << 24) | ((uint32_t)src[1] << 16) | ((uint32_t)src[2] << 8) | (uint32_t)src[3]; \
} while(0);

template <typename T>
void decode_data(T &target, uint8_t *src, uint16_t length) {
  memcpy(&target, src, length);
}

// 定义一个结构体来存储命令与对应的处理函数
struct CommandHandler
{
  uint8_t cmd;                          // 命令值
  void (*handler)(uint8_t *, uint16_t); // 指向处理函数的指针
};

struct DateTimeStruct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t week;
  uint8_t is_pm;
};

struct SGP41Struct
{
  uint16_t voc;
  uint16_t nox;
};

struct Sht4xStruct
{
  float temperature;
  float humidity;
};

struct WiFiStruct
{
  // WIFI:S:BSSID;T:WPA;P:密码;H:false;;
  char ssid[20];
  char pass[20];
};

struct WeatherStruct
{
  int16_t low_temp;
  int16_t high_temp;
  uint8_t day_code;
  char day_weather[15];
  uint8_t night_code;
  char night_weather[15];
};

struct WeathersStruct
{
  WeatherStruct today;
  WeatherStruct nextday1;
};

enum VoiceStatus {
  VOICE_STATUS_IDLE,
  VOICE_STATUS_LISTENING,
  VOICE_STATUS_PROCESSING,
  VOICE_STATUS_ERROR,
  VOICE_STATUS_COMPLETED
};

struct VoiceStatusStruct
{
  VoiceStatus status;
  uint8_t data[30];
};

struct PlayerPageDataStruct
{
  uint16_t page;
};

struct PlayerPlayStruct
{
  char name[80];
};