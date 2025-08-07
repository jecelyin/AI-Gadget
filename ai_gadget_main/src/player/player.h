#pragma once
#include <stdint.h>

void player_init();
void player_loop();
void player_stop();
void handle_player_get_last(uint8_t *src, uint16_t length);
void handle_player_load_and_play(uint8_t *src, uint16_t length);
void handle_player_play(uint8_t *src, uint16_t length);
void handle_player_pause(uint8_t *src, uint16_t length);
void handle_player_seek(uint8_t *src, uint16_t length);
void handle_player_open_dir(uint8_t *src, uint16_t length);
void handle_player_file_list(uint8_t *src, uint16_t length);