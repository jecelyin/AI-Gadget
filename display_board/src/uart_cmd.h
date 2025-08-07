
#pragma once

#include <stdint.h>
#include "cmd.h"

struct Command {
  uint8_t cmd;
  uint16_t length;
  uint8_t *data;
};

void uart_init();
void uart_task_loop();
void uart_lvgl_loop();
void send_command(uint8_t cmd, const uint8_t *data = nullptr, uint16_t length = 0);

