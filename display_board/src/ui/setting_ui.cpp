#include "setting_ui.h"
#include <config.h>
#include <Arduino.h>
// =========================================
// 事件回调函数区域
// =========================================

/**
 * 【亮度】滑块回调
 * 功能：更新 UI 数值显示 + 调节屏幕背光
 */
static void brightness_event_cb(lv_event_t *e) {
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *label =
      (lv_obj_t *)lv_event_get_user_data(e); // 获取传递进来的数值标签

  // 1. 获取滑块数值 (0 - 100)
  int32_t slider_val = lv_slider_get_value(slider);

  // 2. 更新 UI 显示
  if (label) {
    lv_label_set_text_fmt(label, "%d%%", (int)slider_val);
  }
  // 映射数值: Slider(0-100) -> PWM(0-255)
  // 注意：如果是 10位分辨率(BL_RES=10)，这里要映射到 0-1023
  uint16_t duty = map(slider_val, 0, 100, 0, 255);

  // 写 PWM 占空比 (新版 API 直接传 Pin 号)
  ledcWrite(BL_PWM, duty);
}

/**
 * 【音量】滑块回调
 * 功能：更新 UI 数值显示 + 调节系统音量
 */
static void volume_event_cb(lv_event_t *e) {
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *label =
      (lv_obj_t *)lv_event_get_user_data(e); // 获取传递进来的数值标签

  int32_t value = lv_slider_get_value(slider);

  // 1. 更新 UI 显示
  if (label) {
    lv_label_set_text_fmt(label, "%d%%", (int)value);
  }

  // 2. TODO: 在此处添加硬件控制代码
  // Set_Audio_Volume(value);
  // printf("Setting Volume to: %d\n", value);
}

/**
 * 【自动亮度】Checkbox 回调
 * 功能：开关自动调节逻辑，并禁用/启用手动滑块
 */
static void auto_check_event_cb(lv_event_t *e) {
  lv_obj_t *checkbox = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *slider =
      (lv_obj_t *)lv_event_get_user_data(e); // 获取传递进来的亮度滑块

  if (lv_obj_has_state(checkbox, LV_STATE_CHECKED)) {
    // 选中状态：启用自动调节，禁用手动滑块
    lv_obj_add_state(slider, LV_STATE_DISABLED);
    // TODO: 开启光线传感器检测任务
    // Start_Light_Sensor();
  } else {
    // 未选中状态：关闭自动调节，启用手动滑块
    lv_obj_remove_state(slider, LV_STATE_DISABLED);
    // TODO: 关闭光线传感器检测任务
    // Stop_Light_Sensor();
  }
}

// =========================================
// 界面构建辅助函数
// =========================================

// 辅助函数：创建 "标题 + 数值" 的行布局，返回数值 Label 对象
static lv_obj_t *create_slider_header(lv_obj_t *parent, const char *title_text,
                                      int32_t default_val) {
  lv_obj_t *cont = lv_obj_create(parent);
  lv_obj_remove_style_all(cont);
  lv_obj_set_width(cont, lv_pct(80));
  lv_obj_set_height(cont, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_margin_bottom(cont, 5, 0);

  lv_obj_t *label_title = lv_label_create(cont);
  lv_label_set_text(label_title, title_text);

  lv_obj_t *label_value = lv_label_create(cont);
  lv_label_set_text_fmt(label_value, "%d%%", (int)default_val);

  return label_value;
}

// =========================================
// 主页面构建函数
// =========================================

void setting_ui_page(lv_obj_t *parent) {
  // 1. 页面基础布局
  lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(parent, 20, 0);
  lv_obj_set_style_pad_row(parent, 20, 0);

  // -------------------------------------
  // 2. 自动亮度 Checkbox
  // -------------------------------------
  lv_obj_t *cb_auto = lv_checkbox_create(parent);
  lv_checkbox_set_text(cb_auto, "Auto Brightness");

  // -------------------------------------
  // 3. 屏幕亮度调节
  // -------------------------------------
  int32_t bright_val = 50;
  // 创建标题和数值显示 Label
  lv_obj_t *lbl_bright_val =
      create_slider_header(parent, "Screen Brightness", bright_val);

  // 创建滑块
  lv_obj_t *slider_bright = lv_slider_create(parent);
  lv_obj_set_width(slider_bright, lv_pct(80));
  lv_slider_set_range(slider_bright, 0, 100);
  lv_slider_set_value(slider_bright, bright_val, LV_ANIM_OFF);

  // 绑定【亮度专属】回调，并将 label 传给它以便更新显示的数值
  lv_obj_add_event_cb(slider_bright, brightness_event_cb,
                      LV_EVENT_VALUE_CHANGED, lbl_bright_val);

  // 关联 Checkbox 和 Slider 的禁用逻辑
  lv_obj_add_event_cb(cb_auto, auto_check_event_cb, LV_EVENT_VALUE_CHANGED,
                      slider_bright);

  // -------------------------------------
  // 4. 分割线
  // -------------------------------------
  lv_obj_t *line = lv_line_create(parent);
  static lv_point_precise_t line_points[] = {{0, 0}, {200, 0}};
  lv_line_set_points(line, line_points, 2);
  lv_obj_set_style_line_width(line, 1, 0);
  lv_obj_set_style_line_color(line, lv_palette_main(LV_PALETTE_GREY), 0);
  lv_obj_set_style_line_opa(line, LV_OPA_30, 0);

  // -------------------------------------
  // 5. 音量调节
  // -------------------------------------
  int32_t vol_val = 70;
  // 创建标题和数值显示 Label
  lv_obj_t *lbl_vol_val =
      create_slider_header(parent, "System Volume", vol_val);

  // 创建滑块
  lv_obj_t *slider_vol = lv_slider_create(parent);
  lv_obj_set_width(slider_vol, lv_pct(80));
  lv_slider_set_range(slider_vol, 0, 100);
  lv_slider_set_value(slider_vol, vol_val, LV_ANIM_OFF);

  // 绑定【音量专属】回调，并将 label 传给它以便更新显示的数值
  lv_obj_add_event_cb(slider_vol, volume_event_cb, LV_EVENT_VALUE_CHANGED,
                      lbl_vol_val);
}