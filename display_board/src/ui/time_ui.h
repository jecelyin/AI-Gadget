
#pragma once

#include <lvgl.h>
#include "data_struct.h"

void time_ui_page(lv_obj_t *parent);
void time_ui_update_datetime(DateTimeStruct data);
void time_ui_update_air(SGP41Struct data);
void time_ui_update_temp_humidity(Sht4xStruct data);
void time_ui_update_battery(float voltage);
void time_ui_update_footer(const char* text);