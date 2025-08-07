#include "prefs.h"
#include <cJSON.h>
#include <Preferences.h>
#include "esp_log.h"
#include <stdio.h>

Preferences prefs;
static const char *TAG = "Prefs";
// 保存所有SSID和密码的键名
const char* WIFI_CREDENTIALS_KEY = "wifi_list";

void pref_init() { prefs.begin("mainapp"); }

// 保存一个SSID和密码
void pref_set_wifi(const char *ssid, const char *pass) {
    // 读取现有的Wi-Fi凭证
    String jsonStr = prefs.getString(WIFI_CREDENTIALS_KEY, "{}");

    // 解析JSON
    cJSON *root = cJSON_Parse(jsonStr.c_str());
    if (!root) {
        root = cJSON_CreateObject();
    }

    // 添加或更新SSID和密码
    cJSON_AddStringToObject(root, ssid, pass);

    // 将JSON转换回字符串并保存
    char *newJsonStr = cJSON_PrintUnformatted(root);
    prefs.putString(WIFI_CREDENTIALS_KEY, newJsonStr);
    
    // 释放内存
    free(newJsonStr);
    cJSON_Delete(root);
}

// 获取所有保存的SSID和密码
bool pref_get_all_wifi(std::vector<std::pair<String, String>> &credentials) {
    // 读取JSON字符串
    String jsonStr = prefs.getString(WIFI_CREDENTIALS_KEY, "{}");
    
    // 解析JSON
    cJSON *root = cJSON_Parse(jsonStr.c_str());
    if (!root) {
        return false;
    }

    // 遍历JSON对象，提取SSID和密码
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, root) {
        if (cJSON_IsString(item)) {
            credentials.push_back(std::make_pair(item->string, item->valuestring));
        }
    }
    
    cJSON_Delete(root);
    return true;
}

// 删除指定的SSID和密码
bool pref_del_wifi(const char *ssid) {
    // 读取现有的Wi-Fi凭证
    String jsonStr = prefs.getString(WIFI_CREDENTIALS_KEY, "{}");

    // 解析JSON
    cJSON *root = cJSON_Parse(jsonStr.c_str());
    if (!root) {
        return false;
    }

    // 检查SSID是否存在并删除
    cJSON *item = cJSON_GetObjectItem(root, ssid);
    if (item) {
        cJSON_DeleteItemFromObject(root, ssid);
        
        // 将JSON转换回字符串并保存
        char *newJsonStr = cJSON_PrintUnformatted(root);
        prefs.putString(WIFI_CREDENTIALS_KEY, newJsonStr);
        free(newJsonStr);
        
        cJSON_Delete(root);
        return true;
    }
    
    cJSON_Delete(root);
    return false;
}

// 清除所有保存的SSID和密码
void pref_clear_all_wifi() {
    prefs.remove(WIFI_CREDENTIALS_KEY);
}

String pref_get_player_last_audio() {
    return prefs.getString("p_last_audio");
}

size_t pref_set_player_last_audio(const char *name) {
    return prefs.putString("p_last_audio", name);
}

String pref_get_player_last_path() {
    return prefs.getString("p_last_path", "/");
}

size_t pref_set_player_last_path(const char *path) {
    return prefs.putString("p_last_path", path);
}

uint16_t pref_get_player_last_position() {
    return prefs.getUShort("p_last_pos");
}

size_t pref_set_player_last_position(uint16_t pos) {
    return prefs.putUShort("p_last_pos", pos);
}

uint16_t pref_get_player_last_duration() {
    return prefs.getUShort("p_last_dur");
}

size_t pref_set_player_last_duration(uint16_t duration) {
    return prefs.putUShort("p_last_dur", duration);
}

String pref_get_ai_api_url() { return prefs.getString("ai_api_url"); }

size_t pref_set_ai_api_url(const char *url) {
    return prefs.putString("ai_api_url", url);
}

// 生成 JSON 数组字符串的函数
String pref_get_json_array(const PreferenceKey keys[], size_t keysSize) {
    // 创建 JSON 数组
    cJSON *jsonArray = cJSON_CreateArray();

    // 遍历 PreferenceKey 数组，动态读取值并构造 JSON 数组
    for (size_t i = 0; i < keysSize; i++) {
        const char* key = keys[i].key;
        PreferenceType type = keys[i].type;

        // 创建一个新的 JSON 对象
        cJSON *jsonObj = cJSON_CreateObject();
        cJSON_AddStringToObject(jsonObj, "name", key); // 添加 key 字段

        switch (type) {
        case PT_I8: {
            int8_t value = prefs.getChar(key, 0); // getChar 读取有符号 8 位整数
            cJSON_AddNumberToObject(jsonObj, "value", value); // 添加 value 字段
            break;
        }
        case PT_U8: {
            uint8_t value = prefs.getUChar(key, 0); // getUChar 读取无符号 8 位整数
            cJSON_AddNumberToObject(jsonObj, "value", value); // 添加 value 字段
            break;
        }
        case PT_I16: {
            int16_t value = prefs.getShort(key, 0); // getShort 读取有符号 16 位整数
            cJSON_AddNumberToObject(jsonObj, "value", value); // 添加 value 字段
            break;
        }
        case PT_U16: {
            uint16_t value = prefs.getUShort(key, 0); // getUShort 读取无符号 16 位整数
            cJSON_AddNumberToObject(jsonObj, "value", value); // 添加 value 字段
            break;
        }
        case PT_I32: {
            int32_t value = prefs.getInt(key, 0); // getInt 读取有符号 32 位整数
            cJSON_AddNumberToObject(jsonObj, "value", value); // 添加 value 字段
            break;
        }
        case PT_U32: {
            uint32_t value = prefs.getUInt(key, 0); // getUInt 读取无符号 32 位整数
            cJSON_AddNumberToObject(jsonObj, "value", value); // 添加 value 字段
            break;
        }
        case PT_I64: {
            int64_t value = prefs.getLong64(key, 0); // getLong64 读取有符号 64 位整数
            cJSON_AddNumberToObject(jsonObj, "value", (double)value); // cJSON 使用 double 存储所有数字
            break;
        }
        case PT_U64: {
            uint64_t value = prefs.getULong64(key, 0); // getULong64 读取无符号 64 位整数
            cJSON_AddNumberToObject(jsonObj, "value", (double)value); // cJSON 使用 double 存储所有数字
            break;
        }
        case PT_STR: {
            String value = prefs.getString(key, ""); // getString 读取字符串
            cJSON_AddStringToObject(jsonObj, "value", value.c_str()); // 添加 value 字段
            break;
        }
        case PT_BLOB: {
            // 对于 BLOB 类型，可能需要读取到缓冲区
            size_t len = prefs.getBytesLength(key); // 获取 BLOB 数据的长度
            if (len > 0) {
                uint8_t* buffer = new uint8_t[len]; // 动态分配缓冲区
                prefs.getBytes(key, buffer, len);   // 读取 BLOB 数据
                String blobStr = ""; // 将 BLOB 转为十六进制字符串
                for (size_t i = 0; i < len; i++) {
                    char hex[3];
                    sprintf(hex, "%02X", buffer[i]);
                    blobStr += hex;
                }
                cJSON_AddStringToObject(jsonObj, "value", blobStr.c_str()); // 添加 BLOB 数据到 JSON 对象
                delete[] buffer;            // 释放缓冲区
            }
            break;
        }
        case PT_INVALID:
        default:
            ESP_LOGE(TAG, "Key: %s is invalid or not found", key);
            break;
        }
        
        // 将对象添加到数组
        cJSON_AddItemToArray(jsonArray, jsonObj);
    }

    // 序列化 JSON 数组为字符串
    char *jsonString = cJSON_PrintUnformatted(jsonArray);
    String result(jsonString);
    
    // 释放内存
    free(jsonString);
    cJSON_Delete(jsonArray);
    
    return result;
}

// 定义保存函数
bool pref_save_key(const char* key, const String& value, PreferenceType type) {
    switch (type) {
        case PT_I8: {
            int8_t intValue = value.toInt(); // 将字符串转为整数
            prefs.putChar(key, intValue);   // 保存为有符号 8 位整数
            return true;
        }
        case PT_U8: {
            uint8_t uintValue = value.toInt(); // 转为无符号整数
            prefs.putUChar(key, uintValue);    // 保存为无符号 8 位整数
            return true;
        }
        case PT_I16: {
            int16_t intValue = value.toInt();
            prefs.putShort(key, intValue); // 保存为有符号 16 位整数
            return true;
        }
        case PT_U16: {
            uint16_t uintValue = value.toInt();
            prefs.putUShort(key, uintValue); // 保存为无符号 16 位整数
            return true;
        }
        case PT_I32: {
            int32_t intValue = value.toInt();
            prefs.putInt(key, intValue); // 保存为有符号 32 位整数
            return true;
        }
        case PT_U32: {
            uint32_t uintValue = value.toInt();
            prefs.putUInt(key, uintValue); // 保存为无符号 32 位整数
            return true;
        }
        case PT_I64: {
            int64_t intValue = value.toInt();
            prefs.putLong64(key, intValue); // 保存为有符号 64 位整数
            return true;
        }
        case PT_U64: {
            uint64_t uintValue = value.toInt();
            prefs.putULong64(key, uintValue); // 保存为无符号 64 位整数
            return true;
        }
        case PT_STR: {
            prefs.putString(key, value); // 保存为字符串
            return true;
        }
        case PT_BLOB: {
            // 假设 BLOB 数据以十六进制字符串提交
            size_t len = value.length() / 2; // 每 2 个字符表示 1 个字节
            uint8_t* buffer = new uint8_t[len];
            for (size_t i = 0; i < len; i++) {
                String byteStr = value.substring(i * 2, i * 2 + 2);
                buffer[i] = strtol(byteStr.c_str(), nullptr, 16);
            }
            prefs.putBytes(key, buffer, len); // 保存为 BLOB
            delete[] buffer;
            return true;
        }
        default:
            ESP_LOGE(TAG, "Unsupported type for key: %s", key);
            return false;
    }
}