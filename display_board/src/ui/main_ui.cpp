
#include "main_ui.h"
#include "ai_ui.h"
#include "face_ui.h"
#include "time_ui.h"
#include "ui_player.h"
#include "ui_presenter.h"
#include <lvgl.h>
#include "emoji_font.h"

lv_obj_t *scr1;

static lv_style_t base_style;
static lv_style_t flex_layout_style;
lv_obj_t *face_ui;
lv_obj_t *time_ui;
lv_obj_t *ai_ui;
lv_obj_t *player_ui;
UI_PAGE current_page = TIME_UI_PAGE;

static void main_ui_tab_changed(lv_event_t *e);


void main_ui() {
  LV_LOG_INFO("main_ui() called");
  scr1 = lv_scr_act();

  emoji_font_init();

  lv_obj_t *tileview = lv_tileview_create(scr1);
  lv_obj_set_size(tileview, LV_HOR_RES, LV_VER_RES); // Full-screen container
  lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(scr1, LV_HOR_RES, LV_VER_RES);
  // lv_obj_t *tabview = lv_tabview_create(lv_screen_active());
  // lv_obj_set_size(tabview, LV_HOR_RES, LV_VER_RES);
  // /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
  // time_ui = lv_tabview_add_tab(tabview, "Tab 1");
  // ai_ui = lv_tabview_add_tab(tabview, "Tab 2");
  // player_ui = lv_tabview_add_tab(tabview, "Tab 3");

  // face_ui = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_RIGHT);
  time_ui = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
  ai_ui = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);
  player_ui = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_LEFT);
  lv_obj_set_style_bg_opa(tileview, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(tileview, lv_color_hex(0x000000), 0);

  // lv_obj_set_size(time_ui, LV_HOR_RES, LV_VER_RES);
  // lv_obj_set_size(ai_ui, LV_HOR_RES, LV_VER_RES);
  // lv_obj_set_size(player_ui, LV_HOR_RES, LV_VER_RES);
  // face_ui_page(face_ui);
  time_ui_page(time_ui);
  ai_ui_page(ai_ui);
  ui_player_page(player_ui);

  // lv_tileview_set_tile_by_index(tileview, 2, 0, LV_ANIM_OFF);
  lv_obj_add_event_cb(tileview, main_ui_tab_changed, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_update_layout(scr1);

  lv_scr_load(scr1);
}

UI_PAGE current_ui_page()
{
    return current_page;
}

static void main_ui_tab_changed(lv_event_t *e) {
  lv_obj_t *tileview = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *tab = lv_tileview_get_tile_act(tileview);
  if (tab == player_ui) {
    current_page = PLAYER_UI_PAGE;
    LV_LOG_INFO("Switched to Player UI Page");
    SEND_COMMAND0(CMD_PLAYER_LAST_REQ);
  } else if (tab == ai_ui) {
    current_page = AI_UI_PAGE;
    LV_LOG_INFO("Switched to AI UI Page");
  } else if (tab == time_ui) {
    current_page = TIME_UI_PAGE;
    LV_LOG_INFO("Switched to Time UI Page");
  }
}