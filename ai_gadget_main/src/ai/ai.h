#pragma once

#include <stdint.h>
void ai_init();
void handle_voice_start(uint8_t *src, uint16_t length);
void handle_voice_end(uint8_t *src, uint16_t length);
void handle_voice_cancel(uint8_t *src, uint16_t length);
void ai_loop();