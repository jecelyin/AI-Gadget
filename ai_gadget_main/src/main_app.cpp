#include "main_app.h"
// #include <Arduino.h>
#include "config.h"
// #include <Wire.h>
#include "uart_cmd.h"
#include "battery_task.h"
#include "light_task.h"
#include "rtc_task.h"
#include "sgp4x_task.h"
#include "ahtxx_task.h"
#include "weather_task.h"
#include "app_io.h"
#include "app_sdcard.h"
#include "prefs.h"
#include "player.h"
#include "vibration.h"
// #include <ESPmDNS.h>
#include "ai.h"
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "MainApp";
main_api_t* main_api = nullptr;


void main_app_setup(main_api_t& api)
{
  main_api = &api;
  // pinMode(VCC_EN_PIN, OUTPUT);
  // digitalWrite(VCC_EN_PIN, 1);
  // pinMode(LCD_EN_PIN, OUTPUT);
  // digitalWrite(LCD_EN_PIN, 1);
  // pinMode(MIC_EN_PIN, OUTPUT);
  // digitalWrite(MIC_EN_PIN, 0);
  // delay(1000);
  // 初始化传感器等硬件
  // Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  // scanI2CDevices();

  app_io_init();
  app_sdcard_init();
  pref_init();

  uart_cmd_setup();
  // delay(300);
  
  SGP4x_Task::setup();
  Ahtxx_Task::setup();
  vibration_setup();
  // WiFi_Task::setup();
  RTC_Task::setup();

  Weather_Task::setup();
  ESP_LOGI(TAG, "Free heap1: %ld", esp_get_free_heap_size());

  player_init();

  ai_init();
  ESP_LOGI(TAG, "Free heap2: %ld", esp_get_free_heap_size());


  ESP_LOGI(TAG, "[core:%d]main app setup completed!", xPortGetCoreID());
}
/*
WiFi连接管理:

持续运行，检测 WiFi 状态，重连网络。
RTC时钟同步管理:

周期性同步时钟，避免频繁触发，建议 1 小时或每天同步一次。
SGP41空气质量定时更新:

建议每隔 1~5 分钟更新一次。
温湿度定时更新:

建议每隔 1~5 分钟更新一次。
电量定时更新:

电量变化通常较慢，可以每隔 1~10 分钟更新一次。
光线定时判断:

光线变化通常较快，建议每隔 1 秒或 10 秒判断一次。
定时更新天气:

调用 API 更新天气数据，建议每隔 30 分钟或更长时间触发一次。
*/
void main_app_loop()
{
  // ESP_LOGI(TAG, "Free heap0: %ld", esp_get_free_heap_size());

  // Serial.printf("Free heap: %d bytes\n", esp_get_free_heap_size());
  vibration_loop();

  uart_cmd_loop();
  player_loop();
  ai_loop();

  // fix: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
  // delay(1); // 让出CPU控制权，喂狗，避免报错
}