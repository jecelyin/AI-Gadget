#include "lcd_app.h"
#include "app_theme.h"
#include "config.h"
#include "main_ui.h"
#include "touch.h"
#include "uart_cmd.h"
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <lvgl.h>
#include "display.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
// #include <Arduino_GFX_Library.h>
// #include <Adafruit_XCA9554.h>
#include "app_theme.h"

static const char *TAG = "lcd_app";

void init_LVGL()
{
  // 初始化 I2C 触摸
  touch_init();

  lv_disp_t* disp = init_display();

  lv_theme_t *th = app_default_theme_init(disp);
  lv_display_set_theme(disp, th);
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touch_read);

  uint16_t fw_version = get_fw_version();
  ESP_LOGI(TAG, "Firmware Version: 0x%04X", fw_version);
  uint16_t chip_type = get_chip_type();
  ESP_LOGI(TAG, "Chip Type: 0x%04X", chip_type);
}

void uart_task(void *parameter)
{
  while (1)
  {
    uart_task_loop();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void lcd_app_setup()
{
  ESP_LOGI(TAG, "Total heap: %d", heap_caps_get_total_size(MALLOC_CAP_DEFAULT));
  ESP_LOGI(TAG, "Free heap: %d", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  ESP_LOGI(TAG, "Total PSRAM: %d", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
  ESP_LOGI(TAG, "Free PSRAM: %d", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

  // 设置背光引脚为输出
  gpio_set_direction((gpio_num_t)BL_PWM, GPIO_MODE_OUTPUT);

  // 配置 PWM
  // ledc_timer_config_t ledc_timer = {
  //     .speed_mode = LEDC_SPEED_MODE_MAX,
  //     .timer_num = LEDC_TIMER_0,
  //     .duty_resolution = LEDC_TIMER_8_BIT,
  //     .freq_hz = pwmFrequency,
  //     .clk_cfg = LEDC_AUTO_CLK};
  // ledc_timer_config(&ledc_timer);

  // ledc_channel_config_t ledc_channel = {
  //     .gpio_num = BL_PWM,
  //     .speed_mode = LEDC_SPEED_MODE_MAX,
  //     .channel = (ledc_channel_t)pwmChannel,
  //     .intr_type = LEDC_INTR_DISABLE,
  //     .timer_sel = LEDC_TIMER_0,
  //     .duty = dutyCycle,
  //     .hpoint = 0};
  // ledc_channel_config(&ledc_channel);
  gpio_set_level((gpio_num_t)BL_PWM, 1);

  ESP_LOGI(TAG, "init LCD...");
  ESP_LOGI(TAG, "init LVGL...");
  init_LVGL();
  ESP_LOGI(TAG, "start main UI..");
  main_ui();
  ESP_LOGI(TAG, "end main UI.");
  uart_init();
  xTaskCreatePinnedToCore(
      uart_task,
      "UART",
      10000,
      NULL,
      1,
      NULL,
      0);
  ESP_LOGI(TAG, "setup() is running on core: %d", xPortGetCoreID());
  ESP_LOGI(TAG, "end setup.");
}

void lcd_app_loop()
{
  lv_timer_handler();
  uart_lvgl_loop();

  vTaskDelay(pdMS_TO_TICKS(5));
}


