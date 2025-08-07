#pragma once

#include <lvgl.h>
#include <data_struct.h>

void face_ui_page(lv_obj_t *parent);
void face_ui_show_wifi_ap_win(WiFiStruct& data);
void face_ui_show_setting_url_win(const char* url);
void face_ui_set_status(const char* text);
void face_ui_show_notification(const char* msg);
void face_ui_show_emotion(const char* emotion);
