#include <esp_log.h>
#include <esp_err.h>
#include <string>
#include <cstdlib>
#include <cstring>

#include "display.h"
#include "board.h"
#include "application.h"
#include "font_awesome_symbols.h"
#include "audio_codec.h"
#include "settings.h"
#include "assets/lang_config.h"
#include "uart_cmd.h"

#define TAG "Display"

Display::Display() {
    // Load theme from settings
    Settings settings("display", false);
    current_theme_name_ = settings.GetString("theme", "light");

    // Notification timer
    esp_timer_create_args_t notification_timer_args = {
        .callback = [](void *arg) {
            Display *display = static_cast<Display*>(arg);
            DisplayLockGuard lock(display);
            lv_obj_add_flag(display->notification_label_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(display->status_label_, LV_OBJ_FLAG_HIDDEN);
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "notification_timer",
        .skip_unhandled_events = false,
    };
    ESP_ERROR_CHECK(esp_timer_create(&notification_timer_args, &notification_timer_));

    // Update display timer
    esp_timer_create_args_t update_display_timer_args = {
        .callback = [](void *arg) {
            Display *display = static_cast<Display*>(arg);
            display->Update();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "display_update_timer",
        .skip_unhandled_events = true,
    };
    ESP_ERROR_CHECK(esp_timer_create(&update_display_timer_args, &update_timer_));
    ESP_ERROR_CHECK(esp_timer_start_periodic(update_timer_, 1000000));

    // Create a power management lock
    auto ret = esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, "display_update", &pm_lock_);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGI(TAG, "Power management not supported");
    } else {
        ESP_ERROR_CHECK(ret);
    }
}

Display::~Display() {
    if (notification_timer_ != nullptr) {
        esp_timer_stop(notification_timer_);
        esp_timer_delete(notification_timer_);
    }
    if (update_timer_ != nullptr) {
        esp_timer_stop(update_timer_);
        esp_timer_delete(update_timer_);
    }

    if (network_label_ != nullptr) {
        lv_obj_del(network_label_);
        lv_obj_del(notification_label_);
        lv_obj_del(status_label_);
        lv_obj_del(mute_label_);
        lv_obj_del(battery_label_);
        lv_obj_del(emotion_label_);
    }
    if( low_battery_popup_ != nullptr ) {
        lv_obj_del(low_battery_popup_);
    }
    if (pm_lock_ != nullptr) {
        esp_pm_lock_delete(pm_lock_);
    }
}

void Display::SetStatus(const char* status) {
    DisplayLockGuard lock(this);

    ESP_LOGI(TAG, "SetStatus: %s", status);
    binary_packer.writeString(status);
    SEND_BUFFER(binary_packer, CMD_SET_STATUS);
    binary_packer.clear();
    
    if (status_label_ == nullptr) {
        return;
    }
    lv_label_set_text(status_label_, status);
    lv_obj_clear_flag(status_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);
}

void Display::ShowNotification(const std::string &notification, int duration_ms) {
    ShowNotification(notification.c_str(), duration_ms);
}

void Display::ShowNotification(const char* notification, int duration_ms) {
    ESP_LOGI(TAG, "ShowNotification: %s", notification);
    DisplayLockGuard lock(this);

    binary_packer.writeString(notification);
    binary_packer.writeUint32(duration_ms);
    SEND_BUFFER(binary_packer, CMD_SET_NOTIFICATION);
    binary_packer.clear();

    if (notification_label_ == nullptr) {
        return;
    }
    lv_label_set_text(notification_label_, notification);
    lv_obj_clear_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(status_label_, LV_OBJ_FLAG_HIDDEN);

    esp_timer_stop(notification_timer_);
    ESP_ERROR_CHECK(esp_timer_start_once(notification_timer_, duration_ms * 1000));
}

void Display::Update() {
    auto& board = Board::GetInstance();
    auto codec = board.GetAudioCodec();

    {
        DisplayLockGuard lock(this);
        if (mute_label_ == nullptr) {
            return;
        }

        // 如果静音状态改变，则更新图标
        if (codec->output_volume() == 0 && !muted_) {
            muted_ = true;
            lv_label_set_text(mute_label_, FONT_AWESOME_VOLUME_MUTE);
        } else if (codec->output_volume() > 0 && muted_) {
            muted_ = false;
            lv_label_set_text(mute_label_, "");
        }
    }

    esp_pm_lock_acquire(pm_lock_);
    // 更新电池图标
    int battery_level;
    bool charging, discharging;
    const char* icon = nullptr;
    if (board.GetBatteryLevel(battery_level, charging, discharging)) {
        if (charging) {
            icon = FONT_AWESOME_BATTERY_CHARGING;
        } else {
            const char* levels[] = {
                FONT_AWESOME_BATTERY_EMPTY, // 0-19%
                FONT_AWESOME_BATTERY_1,    // 20-39%
                FONT_AWESOME_BATTERY_2,    // 40-59%
                FONT_AWESOME_BATTERY_3,    // 60-79%
                FONT_AWESOME_BATTERY_FULL, // 80-99%
                FONT_AWESOME_BATTERY_FULL, // 100%
            };
            icon = levels[battery_level / 20];
        }
        DisplayLockGuard lock(this);
        if (battery_label_ != nullptr && battery_icon_ != icon) {
            battery_icon_ = icon;
            lv_label_set_text(battery_label_, battery_icon_);
        }

        if (low_battery_popup_ != nullptr) {
            if (strcmp(icon, FONT_AWESOME_BATTERY_EMPTY) == 0 && discharging) {
                if (lv_obj_has_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN)) { // 如果低电量提示框隐藏，则显示
                    lv_obj_clear_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN);
                    auto& app = Application::GetInstance();
                    app.PlaySound(Lang::Sounds::P3_LOW_BATTERY);
                }
            } else {
                // Hide the low battery popup when the battery is not empty
                if (!lv_obj_has_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN)) { // 如果低电量提示框显示，则隐藏
                    lv_obj_add_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN);
                }
            }
        }
    }

    // 升级固件时，不读取 4G 网络状态，避免占用 UART 资源
    auto device_state = Application::GetInstance().GetDeviceState();
    static const std::vector<DeviceState> allowed_states = {
        kDeviceStateIdle,
        kDeviceStateStarting,
        kDeviceStateWifiConfiguring,
        kDeviceStateListening,
        kDeviceStateActivating,
    };
    if (std::find(allowed_states.begin(), allowed_states.end(), device_state) != allowed_states.end()) {
        icon = board.GetNetworkStateIcon();
        if (network_label_ != nullptr && icon != nullptr && network_icon_ != icon) {
            DisplayLockGuard lock(this);
            network_icon_ = icon;
            lv_label_set_text(network_label_, network_icon_);
        }
    }

    esp_pm_lock_release(pm_lock_);
}


void print_unicode_escape(const char *str) {
    while (*str) {
        // 处理多字节UTF-8字符
        if ((*str & 0x80) != 0) {  // 检测Unicode字符
            uint32_t code_point = 0;
            int bytes = 0;
            
            // 解析UTF-8编码
            if ((*str & 0xF0) == 0xF0) {  // 4字节
                code_point = (*str++ & 0x07) << 18;
                code_point |= (*str++ & 0x3F) << 12;
                code_point |= (*str++ & 0x3F) << 6;
                code_point |= (*str++ & 0x3F);
                bytes = 4;
            } 
            else if ((*str & 0xE0) == 0xE0) {  // 3字节
                code_point = (*str++ & 0x0F) << 12;
                code_point |= (*str++ & 0x3F) << 6;
                code_point |= (*str++ & 0x3F);
                bytes = 3;
            }
            else if ((*str & 0xC0) == 0xC0) {  // 2字节
                code_point = (*str++ & 0x1F) << 6;
                code_point |= (*str++ & 0x3F);
                bytes = 2;
            }
            
            printf("@@@@@@@ \\u%04x", code_point);  // 打印为\uxxxx格式
        } 
        else {  // ASCII字符直接输出
            putchar(*str++);
        }
    }
}
void Display::SetEmotion(const char* emotion) {
    ESP_LOGI(TAG, "SetEmotion: %s", emotion);
    struct Emotion {
        const char* icon;
        const char* text;
    };

    static const std::vector<Emotion> emotions = {
        {"😶", "neutral"},
        {"🙂", "happy"},
        {"😆", "laughing"},
        {"😂", "funny"},
        {"😔", "sad"},
        {"😠", "angry"},
        {"😭", "crying"},
        {"😍", "loving"},
        {"😳", "embarrassed"},
        {"😯", "surprised"},
        {"😱", "shocked"},
        {"🤔", "thinking"},
        {"😉", "winking"},
        {"😎", "cool"},
        {"😌", "relaxed"},
        {"🤤", "delicious"},
        {"😘", "kissy"},
        {"😏", "confident"},
        {"😴", "sleepy"},
        {"😜", "silly"},
        {"🙄", "confused"}
    };
    
    // 查找匹配的表情
    std::string_view emotion_view(emotion);
    auto it = std::find_if(emotions.begin(), emotions.end(),
        [&emotion_view](const Emotion& e) { return e.text == emotion_view; });
    
    if (it != emotions.end()) {
        // print_unicode_escape(it->icon);
        binary_packer.writeString(it->icon);
    } else {
        binary_packer.writeString("😶");
    }
    SEND_BUFFER(binary_packer, CMD_SET_EMOTION);
    binary_packer.clear();

    DisplayLockGuard lock(this);
    if (emotion_label_ == nullptr) {
        return;
    }

    // 如果找到匹配的表情就显示对应图标，否则显示默认的neutral表情
    if (it != emotions.end()) {
        lv_label_set_text(emotion_label_, it->icon);
    } else {
        lv_label_set_text(emotion_label_, "😶");
    }
}

void Display::SetIcon(const char* icon) {
    DisplayLockGuard lock(this);
    if (emotion_label_ == nullptr) {
        return;
    }
    lv_label_set_text(emotion_label_, icon);
}

void Display::SetChatMessage(const char* role, const char* content) {
    binary_packer.writeString(role);
    binary_packer.writeString(content);
    SEND_BUFFER(binary_packer, CMD_SET_CHAT_MESSAGE);
    binary_packer.clear();
    ESP_LOGI(TAG, "SetChatMessage: role=%s, content=%s", role, content);

    DisplayLockGuard lock(this);
    if (chat_message_label_ == nullptr) {
        return;
    }
    lv_label_set_text(chat_message_label_, content);
}

void Display::SetTheme(const std::string& theme_name) {
    current_theme_name_ = theme_name;
    Settings settings("display", true);
    settings.SetString("theme", theme_name);
}
