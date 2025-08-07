
#include "face_ui.h"
#include "resources.h"
#include "ui_presenter.h"
#include <stdio.h>
#include "ui_lib.h"


lv_obj_t *qrcode_desc = nullptr;
lv_obj_t *qrcode_image = nullptr;
lv_obj_t *container = nullptr;
lv_obj_t *setting_icon = nullptr;
lv_obj_t *setting_win = nullptr;
lv_obj_t *status_label = nullptr;

static void setting_btn_event_cb(lv_event_t *e);
void show_setting_win(void);

void face_ui_page(lv_obj_t *parent) {
  container = lv_obj_create(parent);
  lv_obj_set_size(container, LV_PCT(100), LV_PCT(100)); // Full-screen container
  // lv_obj_set_layout(container, LV_LAYOUT_FLEX);
  // lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
  lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

  status_label = lv_label_create(container);
  lv_label_set_text(status_label, "");
  lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);

  setting_icon = lv_btn_create(container);
  lv_obj_set_size(setting_icon, ic_setting_active.header.w,
                  ic_setting_active.header.h);
  lv_obj_center(setting_icon);
  lv_obj_set_y(setting_icon, 200);
  lv_obj_set_style_bg_opa(setting_icon, LV_OPA_TRANSP, 0);
  // lv_image_set_src(setting_icon, &ic_setting_active);
  lv_obj_add_event_cb(setting_icon, setting_btn_event_cb, LV_EVENT_CLICKED,
                      NULL);
  // lv_obj_add_flag(setting_icon, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_set_style_bg_img_src(setting_icon, &ic_setting_inactive,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_img_src(setting_icon, &ic_setting_active,
                              LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_bg_img_src(setting_icon, &ic_setting_active,
                              LV_PART_MAIN | LV_STATE_CHECKED);
  // 为按钮启用 Toggle 功能（使其支持 CHECKED 状态）
  lv_obj_add_flag(setting_icon, LV_OBJ_FLAG_CHECKABLE);

  // 设置按钮为选中状态 (Checked)
  // lv_obj_add_state(setting_icon, LV_STATE_CHECKED);

  // 取消按钮的选中状态 (Uncheck)
  // lv_obj_clear_state(setting_icon, LV_STATE_CHECKED);
  // face_ui_update_wifi(NULL);
}

void face_ui_show_wifi_ap_win(WiFiStruct& data) {
  if (!setting_win) {
    show_setting_win();
  }
  lv_label_set_text_fmt(qrcode_desc, "WiFi名称：%s\nWiFi密码：%s", data.ssid, data.pass);
	char wifi_info[65];
	snprintf(wifi_info, sizeof(wifi_info), "WIFI:S:%s;T:WPA;P:%s;H:false;;", data.ssid, data.pass);

  lv_qrcode_update(qrcode_image, wifi_info, strlen(wifi_info));
}

void face_ui_show_setting_url_win(const char* url) {
  if (!setting_win) {
    show_setting_win();
  }
  lv_label_set_text_fmt(qrcode_desc, "扫码或访问：%s", url);

  lv_qrcode_update(qrcode_image, url, strlen(url));
}

static void close_win() {
	if (setting_win){
		lv_obj_del(setting_win);
		setting_win = nullptr;
		SEND_COMMAND0(CMD_SETTING_EXIT);
	}
}

static void win_event_close_cb(lv_event_t *e) {
	close_win();
}

void show_setting_win(void) {
	close_win();
  setting_win = lv_win_create(lv_screen_active());
  // lv_obj_t *btn;
  lv_obj_t *header = lv_win_get_header(setting_win);
	lv_obj_set_flex_align(header, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_color(header, lv_color_hex(0x000000), LV_PART_MAIN);

	lv_obj_t* close_btn = lv_img_create(header);
	lv_img_set_src(close_btn, &ic_close);
	lv_obj_add_flag(close_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(close_btn, win_event_close_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *cont = lv_win_get_content(setting_win);
	lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  // lv_obj_t *qrcode_layout = lv_obj_create(cont);
  // lv_obj_set_size(qrcode_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  // lv_obj_set_layout(qrcode_layout, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  
	lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
  lv_color_t fg_color = lv_palette_darken(LV_PALETTE_BLUE, 4);
  qrcode_image = lv_qrcode_create(cont);
  lv_qrcode_set_size(qrcode_image, 150);

  lv_obj_center(qrcode_image);

  /*Add a border with bg_color*/
  // lv_obj_set_style_border_color(qrcode_image, bg_color, 0);
  lv_obj_set_style_border_width(qrcode_image, 5, 0);

	// lv_qrcode_update(qrcode_image, "test", 4);

  qrcode_desc = lv_label_create(cont);
  lv_label_set_text(qrcode_desc, "设置");
  lv_label_set_long_mode(qrcode_desc, LV_LABEL_LONG_WRAP);
}


static void setting_btn_event_cb(lv_event_t *e) {
#if LV_USE_SDL
	show_setting_win();
#else
	SEND_COMMAND0(CMD_SETTING_ENTER);
#endif
}

void face_ui_set_status(const char* text) {
    lv_label_set_text(status_label, text);
}

void face_ui_show_notification(const char* msg) {
  show_toast(msg);
}

void face_ui_show_emotion(const char* emotion) {
  show_toast(emotion);
}

