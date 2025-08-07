#pragma once
#include <lvgl.h>
#include "data_struct.h"

void ai_ui_page(lv_obj_t *parent);
void update_voice_status(VoiceStatusStruct &data);
void ai_ui_set_chat_message(const char* role, const char* content);
void ai_ui_set_emotion(const char *emotion);