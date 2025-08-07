#pragma once

#include <stdint.h>
#include <lvgl.h>

void touch_init();
uint8_t i2c_read_byte(uint8_t reg_addr);
void i2c_write_byte(uint8_t reg_addr, uint8_t value);
void i2c_read_bytes(uint8_t reg_addr, uint8_t *buffer, int length);
void touch_read( lv_indev_t * indev, lv_indev_data_t * data );
void set_sleep_mode(bool enable);
uint16_t get_fw_version();
uint16_t get_chip_type();
void set_proximity_mode(bool enable);
void set_gesture_mode(bool enable);
uint8_t get_gesture_id();