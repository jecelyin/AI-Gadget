#include "display.h"
#include "config.h"
/*
 * SPDX-FileCopyrightText: 2022-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include <Arduino_GFX_Library.h>
#include "driver/i2c.h"
// #include <Adafruit_XCA9554.h>
#include <esp_io_expander_tca9554.h>
static const char *TAG = "Display";

// Adafruit_XCA9554 expander;
esp_io_expander_handle_t io_expander = NULL;

// 设置 PWM 参数
int pwmChannel = 0;       // PWM 通道
int pwmFrequency = 50000; // 设置 PWM 频率为 50kHz（大于 25kHz）
int pwmResolution = 8;    // 8 位分辨率（0 - 255）
int brightness = 5;       // 初始亮度（0 - 255）
int dutyCycle = 5;        // 占空比波 0 ~ 255
// LVGL相关
/*Set to your screen resolution and rotation*/
#define TFT_HOR_RES 480
#define TFT_VER_RES 480
#define TFT_ROTATION LV_DISPLAY_ROTATION_0

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;
lv_color_t *disp_draw_buf;
//////////////////////////////////////////////////////////////////////////////////
// 初始化SPI总线，用于与显示设备通信
Arduino_DataBus *bus = new Arduino_SWSPI(
    GFX_NOT_DEFINED /* DC */, TFT_SPI_CS, TFT_SPI_SCK, TFT_SPI_MOSI,
    GFX_NOT_DEFINED /* MISO */); // 初始化SPI总线，用于与显示设备通信
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    TFT_RGB_DE, TFT_RGB_VSYNC, TFT_RGB_HSYNC,
    TFT_RGB_PCLK, // 数据使能信号线，垂直同步信号线。水平同步信号线.时钟信号线
    TFT_RGB_B0, TFT_RGB_B1, TFT_RGB_B2, TFT_RGB_B3,
    TFT_RGB_B4, // - TFT_RGB_B0 到 TFT_RGB_B4: 蓝色像素数据线
    TFT_RGB_G0, TFT_RGB_G1, TFT_RGB_G2, TFT_RGB_G3, TFT_RGB_G4,
    TFT_RGB_G5, // - TFT_RGB_G0 到 TFT_RGB_G5: 绿色像素数据线
    TFT_RGB_R0, TFT_RGB_R1, TFT_RGB_R2, TFT_RGB_R3,
    TFT_RGB_R4, // - TFT_RGB_R0 到 TFT_RGB_R4: 红色像素数据线
    1 /* 水平同步信号极性，1 表示积极沿，0 表示消极沿 */,
    10 /* 水平同步前沿时间 */, 8 /* 水平同步脉冲宽度*/,
    20 /* 水平同步后沿时间*/,
    1 /* 垂直同步信号极性，1 表示积极沿，0 表示消极沿 */,
    10 /* 垂直同步前沿时间 */, 8 /* 垂直同步脉冲宽度 */,
    20 /* 垂直同步后沿时间*/);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    TFT_HOR_RES /* 宽度 */, TFT_VER_RES /* 高度 */, rgbpanel, 0 /* 旋转 */, true /* 自动刷新 */,
    bus, TFT_RST /* RST */, st7701_type9_init_operations,
    sizeof(st7701_type9_init_operations));

static uint32_t my_tick(void) { return millis(); }

// 将缓冲区内容刷新到显示屏
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{

    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);

    // 通知显示驱动刷新操作已完成
    lv_disp_flush_ready(disp);
}

lv_disp_t *display_create(void)
{
    if (!gfx->begin(12000000L))
    {
        ESP_LOGE(TAG, "gfx->begin() failed!");
        while (1)
            ;
    }
    gfx->fillScreen(BLACK);   // 填充屏幕为黑色
    gfx->setTextColor(WHITE); // 设置文本颜色为白色

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    lv_tick_set_cb(my_tick);
    lv_display_t *disp;
#if LV_USE_TFT_ESPI
    /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, TFT_ROTATION);

#else
    /*Else create a display yourself*/
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

    ESP_LOGI(TAG, "Display LVGL UI");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    // _lock_acquire(&lvgl_api_lock);
    // lvgl_demo_ui(display);
    // _lock_release(&lvgl_api_lock);
    return disp;
}
void i2c_init()
{
  i2c_config_t conf = {};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = (gpio_num_t)I2C_SDA;
  conf.scl_io_num = (gpio_num_t)I2C_SCL;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
  ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
  ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
}
void init_io()
{
    pinMode(BL_PWM, OUTPUT); // 设置背光 PWM 引脚为输出模式
    // 启动背光
    // digitalWrite(BL_PWM, HIGH); // 打开背光使能
    // esp_io_expander_handle_t io_expander = NULL;
    i2c_init();
    // Wire.begin(I2C_SDA, I2C_SCL);
    ESP_ERROR_CHECK(esp_io_expander_new_i2c_tca9554(I2C_NUM_0, ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, &io_expander));
    ESP_ERROR_CHECK(esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_1, IO_EXPANDER_OUTPUT));
    ESP_ERROR_CHECK(esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_1, 1));
    // if (!expander.begin(0x20))
    // { // Replace with actual I2C address if different
    //     Serial.println("Failed to find XCA9554 chip");
    //     while (1)
    //     ;
    // }
    // // 设置背光引脚
    // expander.pinMode(1, OUTPUT);
    // expander.digitalWrite(1, HIGH);
    // 设置 PWM 通道
    // bool ledcAttach(uint8_t pin, uint32_t freq, uint8_t resolution);
    ledcAttachChannel(BL_PWM, pwmFrequency, pwmResolution, pwmChannel);
    // ledcWrite(BL_PWM, dutyCycle);//指定通道输出一定占空比波形
    ledcWriteChannel(pwmChannel, dutyCycle);
}

lv_disp_t *init_display()
{
    init_io();
    return display_create();
}