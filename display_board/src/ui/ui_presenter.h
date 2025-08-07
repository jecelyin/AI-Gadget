#pragma once

/**
 * @brief 
 * SEND_COMMAND(cmd, data, length)
 */
#if LV_USE_SDL
#define SEND_COMMAND0(cmd)
#define SEND_COMMAND1(cmd, data)
#define SEND_COMMAND(cmd, data, length)
#else
#include "uart_cmd.h"
#define SEND_COMMAND0(cmd) \
  do { \
    send_command(cmd, NULL, 0); \
  } while (0)
#define SEND_COMMAND1(cmd, data) do { \
    const uint8_t *bytePtr = reinterpret_cast<const uint8_t *>(&data); \
    size_t dataSize = sizeof(data); \
    send_command(cmd, bytePtr, dataSize); \
} while (0);
#define SEND_COMMAND(cmd, data, length) \
  do { \
    send_command(cmd, data, length); \
  } while (0)

#endif

