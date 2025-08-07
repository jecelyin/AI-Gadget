#pragma once

typedef enum {
    TIME_UI_PAGE,
    AI_UI_PAGE,
    PLAYER_UI_PAGE,
} UI_PAGE;

void main_ui();
UI_PAGE current_ui_page();