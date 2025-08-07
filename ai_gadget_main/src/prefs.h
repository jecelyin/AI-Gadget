#pragma once
#include <Preferences.h>
#include <WString.h>
#include <vector>

// 结构体：绑定键名和类型
struct PreferenceKey {
    const char* key;        // 键名
    PreferenceType type;    // 类型
};

void pref_init();
String pref_get_player_last_path();
size_t pref_set_player_last_path(const char* path);

String pref_get_player_last_audio();
size_t pref_set_player_last_audio(const char* name);

uint16_t pref_get_player_last_position();
size_t pref_set_player_last_position(uint16_t pos);

uint16_t pref_get_player_last_duration();
size_t pref_set_player_last_duration(uint16_t duration);

String pref_get_ai_api_url();
size_t pref_set_ai_api_url(const char* url);

void pref_set_wifi(const char *ssid, const char *pass);
bool pref_get_all_wifi(std::vector<std::pair<String, String>> &credentials);
bool pref_del_wifi(const char *ssid);
void pref_clear_all_wifi();

String pref_get_json_array(const PreferenceKey keys[], size_t keysSize);
bool pref_save_key(const char* key, const String& value, PreferenceType type);