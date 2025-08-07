
#pragma once

#include "driver/gpio.h"

#define I2C_SDA GPIO_NUM_15
#define I2C_SCL GPIO_NUM_7
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

#define BL_PWM    6
// 定义屏幕  0为2.1寸华显  1为2.8寸华显 2为博虎2.1寸
#define TFT 1
/******************引脚定义*****************************/
#define TFT_RST 4
// TFT SPI 接口定义
#define TFT_SPI_CS 42   // SPI 从设备选择信号
#define TFT_SPI_SCK 2  // SPI 时钟信号
#define TFT_SPI_MOSI 1 // SPI 数据输出信号

// TFT RGB 接口定义
#define TFT_RGB_DE 40    // 数据使能信号
#define TFT_RGB_VSYNC 39 // 垂直同步信号
#define TFT_RGB_HSYNC 38 // 水平同步信号
#define TFT_RGB_PCLK 41  // 显示时钟信号

// TFT RGB565 颜色通道引脚定义
#define TFT_RGB_B0 5  // 蓝色通道0
#define TFT_RGB_B1 45  // 蓝色通道1
#define TFT_RGB_B2 48  // 蓝色通道2
#define TFT_RGB_B3 47  // 蓝色通道3
#define TFT_RGB_B4 21 // 蓝色通道4

#define TFT_RGB_G0 14  // 绿色通道0
#define TFT_RGB_G1 13 // 绿色通道1
#define TFT_RGB_G2 12  // 绿色通道2
#define TFT_RGB_G3 11 // 绿色通道3
#define TFT_RGB_G4 10  // 绿色通道4
#define TFT_RGB_G5 9 // 绿色通道5

#define TFT_RGB_R0 46 // 红色通道0
#define TFT_RGB_R1 3 // 红色通道1
#define TFT_RGB_R2 8 // 红色通道2
#define TFT_RGB_R3 18 // 红色通道3
#define TFT_RGB_R4 17 // 红色通道4

#define HOST_NAME        "lcd.gadget.ai"
