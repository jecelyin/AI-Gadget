
#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "lcd_app.h"
// #include "driver/i2c.h"
#include "esp_event.h"
#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>

#define TAG "main"

extern "C" void lv_log_print(lv_log_level_t level, const char *buf)
{
  Serial.print("LVGL: ");
  Serial.println(buf);
  switch (level)
  {
  case LV_LOG_LEVEL_TRACE:
    ESP_LOGD(TAG, "%s", buf);
    break;
  case LV_LOG_LEVEL_INFO: // ARDUHAL_LOG_LEVEL_INFO
    ESP_LOGI(TAG, "%s", buf);
    break;
  case LV_LOG_LEVEL_WARN: // ARDUHAL_LOG_LEVEL_WARN
    ESP_LOGW(TAG, "%s", buf);
    break;
  case LV_LOG_LEVEL_ERROR:
    ESP_LOGE(TAG, "%s", buf);
    break;
  default:
    ESP_LOGI(TAG, "%s", buf);
    break;
  }
}

#if CONFIG_AUTOSTART_ARDUINO
void setup()
{
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  // esp_log_level_set("*", ESP_LOG_WARN); // 设置日志级别为DEBUG
  Serial.println("Starting AI Gadget...");
  // ESP_LOGI(TAG, "I2C initialized");
  // Wire.begin(I2C_SDA, I2C_SCL);
  vTaskDelay(1 / portTICK_PERIOD_MS);
  lcd_app_setup();
  lv_log_register_print_cb(lv_log_print);
}

void loop()
{
  lcd_app_loop();
  vTaskDelay(10 / portTICK_PERIOD_MS); // Prevent watchdog reset
}
#else
#error "CONFIG_AUTOSTART_ARDUINO is not enabled. Please enable it in the project configuration."
extern "C" void app_main(void)
{
  initArduino();
  Serial.begin(115200);
  // Initialize the default event loop
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // i2c_init();
  Wire.begin(I2C_SDA, I2C_SCL);
  ESP_LOGI(TAG, "I2C initialized");
  vTaskDelay(1 / portTICK_PERIOD_MS);
  lcd_app_setup();

  while (true)
  {
    lcd_app_loop();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Prevent watchdog reset
  }
}

#endif