#pragma once

#include <lvgl.h>

lv_obj_t *create_ctrl_box(lv_obj_t *parent);
void ui_player_update_last_info(const char* title, uint16_t duration, uint16_t position);