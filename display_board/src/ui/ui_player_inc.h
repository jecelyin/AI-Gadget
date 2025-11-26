#pragma once

#include <lvgl.h>

LV_IMAGE_DECLARE(img_player_list_play);
LV_IMAGE_DECLARE(img_player_list_pause);
LV_IMAGE_DECLARE(img_player_list_folder);
LV_IMAGE_DECLARE(img_player_prev);
LV_IMAGE_DECLARE(img_player_next);
LV_IMAGE_DECLARE(img_player_list_border);
void player_on_track_load(uint32_t old_id, uint32_t new_id);
void player_play(uint32_t id);
void player_list_button_check(uint32_t track_id, bool state);