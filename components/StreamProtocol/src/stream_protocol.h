#pragma once

#include <string.h>
#include "driver/uart.h"

#define SP_OK 0

void stream_send_cmd(uart_port_t uart_num, uint8_t cmd, const uint8_t *data, uint16_t length);
bool stream_read_cmd(uart_port_t uart_num, uint8_t *cmd, uint8_t **data, uint16_t *length, char *error, size_t error_size);