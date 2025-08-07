#include <stdio.h>
#include <string.h>
#include <lvgl.h>
#include "touch.h"
#include "driver/i2c.h"
#include "config.h"

// CST836U 触摸屏 I2C 地址
#define TOUCH_I2C_ADDR 0x15

// 触摸芯片寄存器地址
#define REG_TOUCH_NUM 0x02
#define REG_TOUCH1_XH 0x03
#define REG_TOUCH1_XL 0x04
#define REG_TOUCH1_YH 0x05
#define REG_TOUCH1_YL 0x06
#define REG_TOUCH1_PRESSURE 0x07
#define REG_TOUCH1_AREA 0x08
#define REG_TOUCH2_XH 0x09
#define REG_TOUCH2_XL 0x10
#define REG_TOUCH2_YH 0x11
#define REG_TOUCH2_YL 0x12
#define REG_TOUCH2_PRESSURE 0x13
#define REG_TOUCH2_AREA 0x14
#define REG_SLEEP_MODE 0xA5
#define REG_FW_VERSION_LOW 0xA6
#define REG_FW_VERSION_HIGH 0xA7
#define REG_MODULE_ID 0xA8
#define REG_CHIP_TYPE_LOW 0xAA
#define REG_CHIP_TYPE_HIGH 0xAB
#define REG_PROX_MODE 0xB0
#define REG_GESTURE_MODE 0xD0
#define REG_GESTURE_ID 0xD3

#define MAX_TOUCH_POINTS 2



void touch_init() {

}

uint8_t i2c_read_byte(uint8_t reg_addr) {
    uint8_t data = 0;
    i2c_master_write_read_device(I2C_MASTER_NUM, TOUCH_I2C_ADDR, &reg_addr, 1, &data, 1, pdMS_TO_TICKS(100));
    return data;
}

void i2c_write_byte(uint8_t reg_addr, uint8_t value) {
    uint8_t buf[2] = {reg_addr, value};
    i2c_master_write_to_device(I2C_MASTER_NUM, TOUCH_I2C_ADDR, buf, 2, pdMS_TO_TICKS(100));
}

void i2c_read_bytes(uint8_t reg_addr, uint8_t *buffer, int length) {
    i2c_master_write_read_device(I2C_MASTER_NUM, TOUCH_I2C_ADDR, &reg_addr, 1, buffer, length, pdMS_TO_TICKS(100));
}

void touch_read(lv_indev_t * indev, lv_indev_data_t * data) {
  /*For example  ("my_..." functions needs to be implemented by you)
    int32_t x, y;
    bool touched = my_get_touch( &x, &y );

    if(!touched) {
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        data->state = LV_INDEV_STATE_PRESSED;

        data->point.x = x;
        data->point.y = y;
    }
  */
  uint8_t touch_num = i2c_read_byte(REG_TOUCH_NUM);

  if (touch_num == 0) {
    // 没有触摸点
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    // 存在触摸点，读取每个触摸点的 X 和 Y 坐标
    for (uint8_t i = 0; i < touch_num && i < MAX_TOUCH_POINTS; i++) {
      uint8_t base_addr = REG_TOUCH1_XH + i * 6;
      uint8_t buffer[4];
      i2c_read_bytes(base_addr, buffer, 4);

      uint8_t xh = buffer[0];
      uint8_t xl = buffer[1];
      uint8_t yh = buffer[2];
      uint8_t yl = buffer[3];

      // 计算 X 和 Y 坐标
      uint16_t x = ((xh & 0x0F) << 8) | xl;
      uint16_t y = ((yh & 0x0F) << 8) | yl;

      // 读取压力和触摸区域
      uint8_t pressure = i2c_read_byte(base_addr + 4);
      uint8_t area = i2c_read_byte(base_addr + 5);

      // 将坐标传递给 LVGL
      if (i == 0) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
      }

      // 输出调试信息
      // Serial.print("Touch ");
      // Serial.print(i + 1);
      // Serial.print(" detected at X: ");
      // Serial.print(x);
      // Serial.print(", Y: ");
      // Serial.print(y);
      // Serial.print(", Pressure: ");
      // Serial.print(pressure);
      // Serial.print(", Area: ");
      // Serial.println(area);
    }
  }  
}

void set_sleep_mode(bool enable) {
    i2c_write_byte(REG_SLEEP_MODE, enable ? 0x03 : 0x00);
}

uint16_t get_fw_version() {
    uint8_t buffer[2];
    i2c_read_bytes(REG_FW_VERSION_LOW, buffer, 2);
    return (buffer[1] << 8) | buffer[0];
}

uint16_t get_chip_type() {
    uint8_t buffer[2];
    i2c_read_bytes(REG_CHIP_TYPE_LOW, buffer, 2);
    return (buffer[1] << 8) | buffer[0];
}

void set_proximity_mode(bool enable) {
    i2c_write_byte(REG_PROX_MODE, enable ? 0x01 : 0x00);
}

void set_gesture_mode(bool enable) {
    i2c_write_byte(REG_GESTURE_MODE, enable ? 0x01 : 0x00);
}

uint8_t get_gesture_id() {
    return i2c_read_byte(REG_GESTURE_ID);
}

