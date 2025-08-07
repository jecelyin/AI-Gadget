#include "ui_player_crtl.h"
#include "ui_presenter.h"
#include "ui_player_presenter.h"
#include "data_struct.h"
#include "ui_player_inc.h"

static lv_obj_t *play_obj;
static lv_obj_t *time_obj;
static lv_obj_t *slider_obj;
static lv_obj_t *title_label;
static lv_timer_t *sec_counter_timer;
static uint32_t time_act;
static bool playing;

void player_album_next(bool next);
void player_resume(void);
void player_pause(void);

void player_album_next(bool next) {
  uint32_t id = find_next_audio(next);
  if (id == -1) {
    player_pause();
    return;
  }

  if (playing) {
    player_play(id);
  } else {
    player_track_load(id);
  }
}

void player_resume(void) {
  if (player_data_size() == 0)return;
  playing = true;

  if (sec_counter_timer)
    lv_timer_resume(sec_counter_timer);
  
  lv_imagebutton_set_state(play_obj, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED);
  FileMeta* data = player_current_data();
  lv_slider_set_range(slider_obj, 0, data->duration);
}

void player_play(uint32_t id) {
  if (player_data_size() == 0)return;
  player_track_load(id);

  player_resume();
}



void player_pause(void) {
  if (player_data_size() == 0)return;
  playing = false;
  if (sec_counter_timer)
    lv_timer_pause(sec_counter_timer);

  lv_imagebutton_set_state(play_obj, LV_IMAGEBUTTON_STATE_RELEASED);
}


static void prev_click_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  player_album_next(false);
}

static void play_event_click_cb(lv_event_t *e) {
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  if (lv_obj_has_state(obj, LV_STATE_CHECKED)) {
    player_resume();
    SEND_COMMAND0(CMD_PLAYER_PLAY);
  } else {
    player_pause();
    SEND_COMMAND0(CMD_PLAYER_PAUSE);
  }
}

static void next_click_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    player_album_next(true);
  }
}

static void slider_event_cb(lv_event_t * e)
{
  lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
  uint32_t value = lv_slider_get_value(slider);
  uint8_t data[4] = {0};
  PACK_UINT32(data, value);
  player_pause();
  SEND_COMMAND(CMD_PLAYER_SEEK, data, sizeof(data));
  time_act = value;
  lv_label_set_text_fmt(time_obj, "%" LV_PRIu32 ":%02" LV_PRIu32, time_act / 60,
                        time_act % 60);
  lv_slider_set_value(slider_obj, time_act, LV_ANIM_ON);
}

static void del_counter_timer_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_DELETE && sec_counter_timer) {
    lv_timer_delete(sec_counter_timer);
    sec_counter_timer = NULL;
  }
}


void player_timer_cb(lv_timer_t *t) {
  LV_UNUSED(t);
  time_act++;
  lv_label_set_text_fmt(time_obj, "%" LV_PRIu32 ":%02" LV_PRIu32, time_act / 60,
                        time_act % 60);
  lv_slider_set_value(slider_obj, time_act, LV_ANIM_ON);
}

lv_obj_t *create_ctrl_box(lv_obj_t *parent) {

  /*Create the control box*/
  lv_obj_t *cont = lv_obj_create(parent);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_bottom(cont, 8, 0);
  static const int32_t grid_col[] = {
      LV_GRID_FR(2), LV_GRID_FR(3), LV_GRID_FR(5), LV_GRID_FR(5),
      LV_GRID_FR(5), LV_GRID_FR(3), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
  static const int32_t grid_row[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(cont, grid_col, grid_row);

  LV_IMAGE_DECLARE(ic_player_btn_loop);
  LV_IMAGE_DECLARE(ic_player_btn_rnd);
  LV_IMAGE_DECLARE(ic_player_btn_next);
  LV_IMAGE_DECLARE(ic_player_btn_prev);
  LV_IMAGE_DECLARE(ic_player_btn_play);
  LV_IMAGE_DECLARE(ic_player_btn_pause);

  lv_obj_t *icon;
  icon = lv_image_create(cont);
  lv_image_set_src(icon, &ic_player_btn_rnd);
  lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  icon = lv_image_create(cont);
  lv_image_set_src(icon, &ic_player_btn_loop);
  lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  icon = lv_image_create(cont);
  lv_image_set_src(icon, &ic_player_btn_prev);
  lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER,
                       1, 1);
  lv_obj_add_event_cb(icon, prev_click_event_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

  play_obj = lv_imagebutton_create(cont);
  lv_imagebutton_set_src(play_obj, LV_IMAGEBUTTON_STATE_RELEASED, NULL,
                         &ic_player_btn_play, NULL);
  lv_imagebutton_set_src(play_obj, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                         &ic_player_btn_pause, NULL);
  lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_grid_cell(play_obj, LV_GRID_ALIGN_CENTER, 3, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_add_event_cb(play_obj, play_event_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_width(play_obj, ic_player_btn_play.header.w);

  icon = lv_image_create(cont);
  lv_image_set_src(icon, &ic_player_btn_next);
  lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER,
                       1, 1);
  lv_obj_add_event_cb(icon, next_click_event_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

  LV_IMAGE_DECLARE(ic_player_slider_knob);
  slider_obj = lv_slider_create(cont);
  lv_obj_set_style_anim_duration(slider_obj, 100, 0);
  lv_obj_add_flag(slider_obj,
                  LV_OBJ_FLAG_CLICKABLE); /*No input from the slider*/
  lv_obj_set_width(slider_obj, LV_PCT(100));
  lv_obj_set_style_margin_left(slider_obj, 10, 0);

#if LV_DEMO_MUSIC_LARGE == 0
  lv_obj_set_height(slider_obj, 3);
#else
  lv_obj_set_height(slider_obj, 6);
#endif
  lv_obj_set_grid_cell(slider_obj, LV_GRID_ALIGN_STRETCH, 1, 4,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_set_style_bg_image_src(slider_obj, &ic_player_slider_knob,
                                LV_PART_KNOB);
  lv_obj_set_style_bg_opa(slider_obj, LV_OPA_TRANSP, LV_PART_KNOB);
  lv_obj_set_style_pad_all(slider_obj, 20, LV_PART_KNOB);
  lv_obj_set_style_bg_grad_dir(slider_obj, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider_obj, lv_color_hex(0x569af8),
                            LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_color(slider_obj, lv_color_hex(0xa666f1),
                                 LV_PART_INDICATOR);
  lv_obj_set_style_outline_width(slider_obj, 0, 0);
  lv_obj_add_event_cb(slider_obj, del_counter_timer_cb, LV_EVENT_DELETE, NULL);
  lv_obj_add_event_cb(slider_obj, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  time_obj = lv_label_create(cont);
  lv_obj_set_style_text_font(time_obj, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(time_obj, lv_color_white(), 0);
  lv_label_set_text(time_obj, "0:00");
  lv_obj_set_grid_cell(time_obj, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_CENTER,
                       2, 1);
  lv_obj_add_event_cb(time_obj, del_counter_timer_cb, LV_EVENT_DELETE, NULL);

  sec_counter_timer = lv_timer_create(player_timer_cb, 1000, NULL);
  lv_timer_pause(sec_counter_timer);

  title_label = lv_label_create(parent);
  lv_obj_set_style_text_font(title_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
  lv_label_set_text(title_label, "");
  lv_obj_set_width(title_label, LV_SIZE_CONTENT);
  lv_obj_set_height(title_label,
                    lv_font_get_line_height(LV_FONT_DEFAULT) * 3 / 2);
  lv_obj_set_style_flex_grow(title_label, 0, 0); // Prevent it from stretching

  return cont;
}

void player_on_track_load(uint32_t old_id, uint32_t new_id) {
  // if (id == track_id)
  //   return;

  time_act = 0;
  lv_slider_set_value(slider_obj, 0, LV_ANIM_OFF);
  lv_label_set_text(time_obj, "0:00");

  player_list_button_check(old_id, false);

  player_list_button_check(new_id, true);

  FileMeta* data = player_get_data(new_id);

  lv_label_set_text(title_label, data->fileName);
  
}

void ui_player_update_last_info(const char* title, uint16_t duration, uint16_t position) {
  lv_label_set_text(title_label, title);
  lv_slider_set_range(slider_obj, 0, duration);
  lv_slider_set_value(slider_obj, position, LV_ANIM_OFF);
}
