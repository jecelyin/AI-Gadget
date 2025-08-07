#pragma once
#include "FileMeta.h"

void player_clean_data(uint16_t page);
void player_add_data(uint16_t page, FileMeta& data);
void player_flush_page(uint16_t page, uint16_t total_page);
uint16_t player_data_total_page();
FileMeta* player_get_data(int index);
FileMeta* player_current_data();
int player_data_size();

bool player_is_current_playing(const char *title);
int find_next_audio(bool forward);
void player_track_load(uint32_t id);

void player_notify_data_clear();
void player_notify_data_remove(int index);
void player_notify_data_add_front(FileMeta &data);
void player_notify_data_add_back(FileMeta &data);

uint16_t player_get_curr_page();
uint16_t player_get_total_page();