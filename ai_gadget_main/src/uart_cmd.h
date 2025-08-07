#pragma once

#include "data_struct.h"
#include <cmd.h>
#include <BinaryPacker.h>

#define SEND_COMMAND0(cmd) send_cmd(cmd, NULL, 0);
#define SEND_COMMAND(cmd, data) do { \
    const uint8_t *bytePtr = reinterpret_cast<const uint8_t *>(&data); \
    size_t dataSize = sizeof(data); \
    send_cmd(cmd, bytePtr, dataSize); \
} while (0);
#define SEND_BUFFER(s, cmd) sendBufferAsCmd(s, cmd);

void uart_cmd_setup();
void uart_cmd_loop();
void send_cmd(uint8_t cmd, const uint8_t* data, uint16_t dataLength);
void sendBufferAsCmd(BinaryPacker &s, uint8_t cmd);
void update_voice_status(VoiceStatus status);