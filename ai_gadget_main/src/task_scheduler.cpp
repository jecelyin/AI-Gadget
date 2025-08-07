#include "task_scheduler.h"
#include "sgp4x_task.h"
#include "ahtxx_task.h"
#include "light_task.h"
#include "battery_task.h"
#include "rtc_task.h"
#include "weather_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "audio_player.h"
#include "player.h"

static const char *TAG = "TaskScheduler";
// ============ 配置参数 ============
#define WIFI_CHECK_INTERVAL 10000        // WiFi 状态检查间隔（毫秒）
#define RTC_SYNC_INTERVAL 10000          // RTC 同步间隔（1 小时）
#define SGP41_UPDATE_INTERVAL 10000      // SGP41 更新间隔（1 分钟）
#define TEMP_HUMID_UPDATE_INTERVAL 10000 // 温湿度更新间隔（1 分钟）
#define BATTERY_UPDATE_INTERVAL 300000   // 电量更新间隔（5 分钟）
#define LIGHT_CHECK_INTERVAL 1000        // 光线判断间隔（1 秒）
#define WEATHER_UPDATE_INTERVAL 1800000  // 天气更新间隔（30 分钟）
#define DELAY_TS 5000

// ============ 全局变量 ============
unsigned int lastWiFiCheck = 0;
unsigned int lastRTCSync = 0;
unsigned int lastSGP41Update = 0;
unsigned int lastTempHumidUpdate = 0;
unsigned int lastBatteryUpdate = 0;
unsigned int lastLightCheck = 0;
unsigned int lastWeatherUpdate = 0;
// --------------------
// extern AudioPlayer player;
// 调度任务
void schedulerTask(void *parameter) {
    ESP_LOGI(TAG, "Task Scheduler started");
    float temperature = 25;
    float humidity = 50;
    while (true) {
        unsigned int now = esp_timer_get_time() / 1000U;

        // RTC 时钟同步
        if (!lastRTCSync || now - lastRTCSync >= RTC_SYNC_INTERVAL) {
            lastRTCSync = now;
            RTC_Task::rtc_sync();
        }

        // 温湿度更新
        if (now - lastTempHumidUpdate >= TEMP_HUMID_UPDATE_INTERVAL) {
            lastTempHumidUpdate = now;
            Ahtxx_Task::updateTemperatureAndHumidity(&temperature, &humidity);
        }

        // SGP41 空气质量更新
        if (now - lastSGP41Update >= SGP41_UPDATE_INTERVAL) {
            lastSGP41Update = now;
            SGP4x_Task::updateVOCandNOX(temperature, humidity);
        }

// #if USE_BATTERY
//         // 电量更新
//         if (now - lastBatteryUpdate >= BATTERY_UPDATE_INTERVAL) {
//             lastBatteryUpdate = now;
//             Battery_Task::updateBatteryLevel();
//         }
// #endif
        // 光线判断
        if (now - lastLightCheck >= LIGHT_CHECK_INTERVAL) {
            lastLightCheck = now;
            Light_Task::updateLightSensorValue();
        }

        // 天气更新
        if (!lastWeatherUpdate || now - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
            if (Weather_Task::updateWeather()) {
                lastWeatherUpdate = now;
            } else {
                lastWeatherUpdate = now - WEATHER_UPDATE_INTERVAL + DELAY_TS;
            }
        }

        // player.getAudio()->performAudioTask();
        // player_loop();

        // 延迟 10ms，降低 CPU 占用
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// 启动任务调度器
void startTaskScheduler() {
    ESP_LOGI(TAG, "Starting Task Scheduler");
    // 创建调度任务
    xTaskCreatePinnedToCore(
        schedulerTask,  // 调度任务函数
        "Scheduler",    // 调度任务名称
        4096,           // 栈大小
        NULL,           // 参数
        1,              // 优先级
        NULL,           // 
        1               // 绑定到核心 1
    );
}
