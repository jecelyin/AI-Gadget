#pragma once

#include <lvgl.h>
#include <data_struct.h>

void build_weather_layout(lv_obj_t *parent);
void update_weather(WeathersStruct &data);
