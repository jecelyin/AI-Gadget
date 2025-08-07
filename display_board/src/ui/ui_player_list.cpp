#include "ui_player_list.h"
#include "FileMeta.h"
#include "ui_player_inc.h"
#include "ui_player_presenter.h"
#include "ui_presenter.h"
#include <data_struct.h>
#include <string>
#include <vector>

static lv_obj_t *list;
static lv_style_t style_scrollbar;
static lv_style_t style_btn;
static lv_style_t style_button_pr;
static lv_style_t style_button_chk;
static lv_style_t style_button_dis;
static lv_style_t style_title;
static lv_style_t style_time;

static bool update_scroll_running = false;
// 在全局或适当作用域定义loading状态变量
static bool is_loading = false;
static lv_obj_t *loading_spinner = NULL;

static lv_obj_t *add_audio_list_button(lv_obj_t *parent, const FileMeta &file);
static lv_obj_t *add_folder_list_button(lv_obj_t *parent,
                                        const FileMeta &folder);

void player_notify_data_clear() {
  lv_obj_t *child = lv_obj_get_child(list, 0);
  while (child != NULL) {
    lv_obj_t *next_child = lv_obj_get_child(list, 0);
    lv_obj_del(child);
    child = next_child;
  }
}
void player_notify_data_remove(int index) {
  lv_obj_t *child = lv_obj_get_child(list, index);
  lv_obj_del(child);
}
void player_notify_data_add_front(FileMeta &data) {
  lv_obj_t *new_item = data.isFile ? add_audio_list_button(list, data)
                                   : add_folder_list_button(list, data);
  lv_obj_move_to_index(new_item, 0);
}
void player_notify_data_add_back(FileMeta &data) {
  data.isFile ? add_audio_list_button(list, data)
              : add_folder_list_button(list, data);
}

// 设置列表loading状态的函数
static void set_list_loading(lv_obj_t *list, bool loading) {
  is_loading = loading;

  if (loading) {
    // 显示loading spinner
    lv_obj_clear_flag(loading_spinner, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(loading_spinner);

    // 禁用滚动和点击
    lv_obj_add_state(list, LV_STATE_DISABLED);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);
  } else {
    if(loading_spinner == NULL) {
      LV_LOG_ERROR("loading_spinner is NULL!");
      return;
    }
    // 隐藏loading spinner
    // lv_obj_add_flag(loading_spinner, LV_OBJ_FLAG_HIDDEN);

    // 启用滚动和点击
    lv_obj_clear_state(list, LV_STATE_DISABLED);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
  }
}

void ui_player_update_list() {
#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
  lv_obj_set_scroll_snap_y(list, LV_SCROLL_SNAP_CENTER);
#endif
  // track_load(track_id);
  // _player_list_button_check(0, true);
  lv_obj_update_layout(list);
  set_list_loading(list, false);
}

static void load_data(uint16_t page) {
  if (page > player_data_total_page()) {
    LV_LOG_TRACE("loaded all page: %d", player_data_total_page());
    return;
  }
  if (page < 1) {
    LV_LOG_TRACE("refresh first page!");
    return;
  }
  set_list_loading(list, true);
  PlayerPageDataStruct data;
  data.page = page;
  SEND_COMMAND1(CMD_PLAYER_FILES_REQ, data);
}

static void update_scroll(lv_obj_t *obj) {
  /* do not re-enter this function when `lv_obj_scroll_by`
   * triggers this callback again.
   */
  if (update_scroll_running)
    return;
  update_scroll_running = true;
  uint16_t curr_page = player_get_curr_page();
  // LV_LOG_USER("update_scroll");
  /* load items we're getting close to */
  if (lv_obj_get_scroll_bottom(obj) < 200) {
    if (curr_page >= player_get_total_page()) {
      LV_LOG_TRACE("current is last page!");
      return;
    }
    LV_LOG_TRACE("loaded bottom page: %d", curr_page + 1);
    load_data(curr_page + 1);
    // lv_obj_update_layout(obj);
  } else if (lv_obj_get_scroll_top(obj) < 200) {
    LV_LOG_TRACE("loaded top page: %d", curr_page - 1);
    // int32_t bottom_before = lv_obj_get_scroll_bottom(obj);
    load_data(curr_page - 1);
    // lv_obj_t * new_item = load_item(obj, top_num);
    // lv_obj_move_to_index(new_item, 0);
    // lv_obj_update_layout(obj);
    // int32_t bottom_after = lv_obj_get_scroll_bottom(obj);
    // lv_obj_scroll_by(obj, 0, bottom_before - bottom_after, LV_ANIM_OFF);
  }

  /* delete far-away items */
  // while(lv_obj_get_scroll_bottom(obj) > 600) {
  //     bottom_num += 1;
  //     lv_obj_t * child = lv_obj_get_child(obj, -1);
  //     lv_obj_delete(child);
  //     lv_obj_update_layout(obj);
  //     LV_LOG_USER("deleted bottom num: %"PRId32, bottom_num);
  // }
  // while(lv_obj_get_scroll_top(obj) > 600) {
  //     top_num -= 1;
  //     int32_t bottom_before = lv_obj_get_scroll_bottom(obj);
  //     lv_obj_t * child = lv_obj_get_child(obj, 0);
  //     lv_obj_delete(child);
  //     lv_obj_update_layout(obj);
  //     int32_t bottom_after = lv_obj_get_scroll_bottom(obj);
  //     lv_obj_scroll_by(obj, 0, bottom_before - bottom_after, LV_ANIM_OFF);
  //     LV_LOG_USER("deleted top num: %"PRId32, top_num);
  // }

  update_scroll_running = false;
}

static void scroll_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target_obj(e);
  update_scroll(obj);
}

static void folder_btn_click_event_cb(lv_event_t *e) {
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);

  uint32_t idx = lv_obj_get_index(btn);
  LV_LOG_TRACE("click folder item 1");
  FileMeta *data = player_get_data(idx);
  SEND_COMMAND(CMD_PLAYER_OPEN_DIR,
               reinterpret_cast<const uint8_t *>(data->fileName),
               strlen(data->fileName) + 1);
}

static lv_obj_t *add_folder_list_button(lv_obj_t *parent,
                                        const FileMeta &folder) {
  const char *title = folder.fileName;

  lv_obj_t *btn = lv_obj_create(parent);
  lv_obj_remove_style_all(btn);
#if LV_DEMO_MUSIC_LARGE
  lv_obj_set_size(btn, lv_pct(100), 110);
#else
  lv_obj_set_size(btn, lv_pct(100), 60);
#endif

  lv_obj_add_style(btn, &style_btn, 0);
  lv_obj_add_style(btn, &style_button_pr, LV_STATE_PRESSED);
  lv_obj_add_style(btn, &style_button_chk, LV_STATE_CHECKED);
  lv_obj_add_style(btn, &style_button_dis, LV_STATE_DISABLED);
  lv_obj_add_event_cb(btn, folder_btn_click_event_cb, LV_EVENT_CLICKED, NULL);

  // lv_obj_add_state(btn, LV_STATE_DISABLED);

  lv_obj_t *icon = lv_image_create(btn);
  lv_image_set_src(icon, &img_player_list_folder);
  lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0,
                       2);

  lv_obj_t *title_label = lv_label_create(btn);
  lv_label_set_text(title_label, title);
  lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 2);
  lv_obj_add_style(title_label, &style_title, 0);

  LV_IMAGE_DECLARE(img_player_list_border);
  lv_obj_t *border = lv_image_create(btn);
  lv_image_set_src(border, &img_player_list_border);
  lv_obj_set_width(border, lv_pct(120));
  lv_obj_align(border, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_flag(border, LV_OBJ_FLAG_IGNORE_LAYOUT);

  return btn;
}

static void audio_btn_click_event_cb(lv_event_t *e) {
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);

  uint32_t idx = lv_obj_get_index(btn);
  LV_LOG_TRACE("click audio item 1");
  player_play(idx);
  LV_LOG_TRACE("click audio item 2");
  FileMeta *data = player_get_data(idx);
  LV_LOG_TRACE("click and play: %s", data->fileName);
  SEND_COMMAND(CMD_PLAYER_LOAD_AND_PLAY,
               reinterpret_cast<const uint8_t *>(data->fileName),
               strlen(data->fileName) + 1);
}

static lv_obj_t *add_audio_list_button(lv_obj_t *parent, const FileMeta &file) {
  uint32_t t = file.duration;
  char time[32];
  lv_snprintf(time, sizeof(time), "%" LV_PRIu32 ":%02" LV_PRIu32, t / 60,
              t % 60);
  const char *title = file.fileName;

  lv_obj_t *btn = lv_obj_create(parent);
  lv_obj_remove_style_all(btn);
#if LV_DEMO_MUSIC_LARGE
  lv_obj_set_size(btn, lv_pct(100), 110);
#else
  lv_obj_set_size(btn, lv_pct(100), 60);
#endif

  lv_obj_add_style(btn, &style_btn, 0);
  lv_obj_add_style(btn, &style_button_pr, LV_STATE_PRESSED);
  lv_obj_add_style(btn, &style_button_chk, LV_STATE_CHECKED);
  lv_obj_add_style(btn, &style_button_dis, LV_STATE_DISABLED);
  lv_obj_add_event_cb(btn, audio_btn_click_event_cb, LV_EVENT_CLICKED, NULL);

  // lv_obj_add_state(btn, LV_STATE_DISABLED);

  lv_obj_t *icon = lv_image_create(btn);
  lv_image_set_src(icon, &img_player_list_play);
  lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0,
                       2);

  lv_obj_t *title_label = lv_label_create(btn);
  lv_label_set_text(title_label, title);
  lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 2);
  lv_obj_add_style(title_label, &style_title, 0);
  // lv_obj_set_style_bg_opa(title_label, LV_OPA_TRANSP, 0); //
  // 背景透明度设置为透明

  lv_obj_t *time_label = lv_label_create(btn);
  lv_label_set_text(time_label, time);
  lv_obj_add_style(time_label, &style_time, 0);
  lv_obj_set_grid_cell(time_label, LV_GRID_ALIGN_END, 2, 1,
                       LV_GRID_ALIGN_CENTER, 0, 2);

  LV_IMAGE_DECLARE(img_player_list_border);
  lv_obj_t *border = lv_image_create(btn);
  lv_image_set_src(border, &img_player_list_border);
  lv_obj_set_width(border, lv_pct(120));
  lv_obj_align(border, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_flag(border, LV_OBJ_FLAG_IGNORE_LAYOUT);

  if (player_is_current_playing(file.fileName)) {
    lv_obj_add_state(btn, LV_STATE_CHECKED);
  }

  return btn;
}

lv_obj_t *player_list_create(lv_obj_t *parent) {
  LV_LOG_TRACE("player_list_create");
  lv_style_init(&style_scrollbar);
  lv_style_set_width(&style_scrollbar, 4);
  lv_style_set_bg_opa(&style_scrollbar, LV_OPA_COVER);
  lv_style_set_bg_color(&style_scrollbar, lv_color_hex3(0xeee));
  lv_style_set_radius(&style_scrollbar, LV_RADIUS_CIRCLE);
  lv_style_set_pad_right(&style_scrollbar, 4);

  static const int32_t grid_cols[] = {LV_GRID_CONTENT, LV_GRID_FR(1),
                                      LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#if LV_DEMO_MUSIC_LARGE
  static const int32_t grid_rows[] = {35, 30, LV_GRID_TEMPLATE_LAST};
#else
  static const int32_t grid_rows[] = {22, 17, LV_GRID_TEMPLATE_LAST};
#endif
  lv_style_init(&style_btn);
  lv_style_set_bg_opa(&style_btn, LV_OPA_TRANSP);
  lv_style_set_grid_column_dsc_array(&style_btn, grid_cols);
  lv_style_set_grid_row_dsc_array(&style_btn, grid_rows);
  lv_style_set_grid_row_align(&style_btn, LV_GRID_ALIGN_CENTER);
  lv_style_set_layout(&style_btn, LV_LAYOUT_GRID);
#if LV_DEMO_MUSIC_LARGE
  lv_style_set_pad_right(&style_btn, 30);
#else
  lv_style_set_pad_right(&style_btn, 20);
#endif
  lv_style_init(&style_button_pr);
  lv_style_set_bg_opa(&style_button_pr, LV_OPA_COVER);
  lv_style_set_bg_color(&style_button_pr, lv_color_hex(0x4c4965));

  lv_style_init(&style_button_chk);
  lv_style_set_bg_opa(&style_button_chk, LV_OPA_COVER);
  lv_style_set_bg_color(&style_button_chk, lv_color_hex(0x4c4965));

  lv_style_init(&style_button_dis);
  lv_style_set_text_opa(&style_button_dis, LV_OPA_40);
  lv_style_set_image_opa(&style_button_dis, LV_OPA_40);

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, LV_FONT_DEFAULT);
  lv_style_set_text_color(&style_title, lv_color_hex(0xffffff));
  lv_style_set_bg_opa(&style_title, LV_OPA_TRANSP);
  // lv_style_set_bg_color(&style_title, lv_color_hex(0xffffff));

  lv_style_init(&style_time);
  lv_style_set_text_font(&style_time, LV_FONT_DEFAULT);
  lv_style_set_text_color(&style_time, lv_color_hex(0xffffff));

  /*Create an empty transparent container*/
  list = lv_obj_create(parent);
  lv_obj_remove_style_all(list);
  // lv_obj_set_size(list, LV_HOR_RES, LV_VER_RES -
  // LV_DEMO_MUSIC_HANDLE_SIZE); lv_obj_set_y(list,
  // LV_DEMO_MUSIC_HANDLE_SIZE);
  lv_obj_set_size(list, LV_PCT(90), 0);
  lv_obj_set_flex_grow(list, 1); // 使列表撑满剩余空间
  lv_obj_add_style(list, &style_scrollbar, LV_PART_SCROLLBAR);
  lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_event_cb(list, scroll_cb, LV_EVENT_SCROLL, NULL);
  lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);

  // 创建loading spinner（初始时不可见）
  loading_spinner = lv_spinner_create(list);
  lv_obj_set_size(loading_spinner, 40, 40);
  lv_obj_center(loading_spinner);
  // lv_obj_add_flag(loading_spinner, LV_OBJ_FLAG_HIDDEN); // 初始隐藏
  return list;
}

void player_list_button_check(uint32_t track_id, bool state) {
  lv_obj_t *btn = lv_obj_get_child(list, track_id);
  lv_obj_t *icon = lv_obj_get_child(btn, 0);
  if (state) {
    lv_obj_add_state(btn, LV_STATE_CHECKED);
    lv_obj_scroll_to_view(btn, LV_ANIM_ON);
    lv_image_set_src(icon, &img_player_list_pause);
  } else {
    lv_obj_remove_state(btn, LV_STATE_CHECKED);
    lv_image_set_src(icon, &img_player_list_play);
  }
}