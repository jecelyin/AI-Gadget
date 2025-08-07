#include "ai_ui.h"
#include "resources.h"
#include <stdio.h>
#include "ui_presenter.h"
#include "ui_lib.h"
#include "emoji_font.h"


// lv_obj_t *msgList = nullptr;
lv_obj_t *voice_icon = nullptr;
lv_obj_t *buttom_layout = nullptr;
lv_obj_t *chat_container = nullptr;
lv_obj_t *arc = nullptr;
lv_obj_t* content_ = nullptr;
lv_obj_t* chat_message_label_ = nullptr;
lv_obj_t *emotion_btn = nullptr;
lv_obj_t *emotion_icon = nullptr;
bool is_cancel = false;

#if !defined(lv_area_is_point_on)
#define lv_area_is_point_on _lv_area_is_point_on
#endif

#if CONFIG_IDF_TARGET_ESP32P4
#define  MAX_MESSAGES 40
#else
#define  MAX_MESSAGES 20
#endif
void ai_ui_set_chat_message(const char* role, const char* content) {
    //避免出现空的消息框
    if(strlen(content) == 0) return;
    
    // 检查消息数量是否超过限制
    uint32_t child_count = lv_obj_get_child_cnt(content_);
    if (child_count >= MAX_MESSAGES) {
        // 删除最早的消息（第一个子对象）
        lv_obj_t* first_child = lv_obj_get_child(content_, 0);
        lv_obj_t* last_child = lv_obj_get_child(content_, child_count - 1);
        if (first_child != nullptr) {
            lv_obj_del(first_child);
        }
        // Scroll to the last message immediately
        if (last_child != nullptr) {
            lv_obj_scroll_to_view_recursive(last_child, LV_ANIM_OFF);
        }
    }
    
    // 折叠系统消息（如果是系统消息，检查最后一个消息是否也是系统消息）
    if (strcmp(role, "system") == 0 && child_count > 0) {
        // 获取最后一个消息容器
        lv_obj_t* last_container = lv_obj_get_child(content_, child_count - 1);
        if (last_container != nullptr && lv_obj_get_child_cnt(last_container) > 0) {
            // 获取容器内的气泡
            lv_obj_t* last_bubble = lv_obj_get_child(last_container, 0);
            if (last_bubble != nullptr) {
                // 检查气泡类型是否为系统消息
                void* bubble_type_ptr = lv_obj_get_user_data(last_bubble);
                if (bubble_type_ptr != nullptr && strcmp((const char*)bubble_type_ptr, "system") == 0) {
                    // 如果最后一个消息也是系统消息，则删除它
                    lv_obj_del(last_container);
                }
            }
        }
    }
    
    // Create a message bubble
    lv_obj_t* msg_bubble = lv_obj_create(content_);
    lv_obj_set_width(msg_bubble, LV_HOR_RES - 80);
    lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
    lv_obj_set_style_margin_bottom(msg_bubble, 10, 0);
    // lv_obj_set_style_pad_top(msg_bubble, 10, 0);
    

    // lv_obj_set_scrollbar_mode(msg_bubble, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_set_style_border_width(msg_bubble, 1, 0);
    // lv_obj_set_style_border_color(msg_bubble, current_theme.border, 0);

    // Create the message text
    lv_obj_t* msg_text = lv_label_create(msg_bubble);
    lv_obj_set_width(msg_text, LV_SIZE_CONTENT);
    lv_obj_set_height(msg_text, LV_SIZE_CONTENT);
    lv_obj_set_style_max_width(msg_text, LV_HOR_RES - 100, 0);
    lv_label_set_text(msg_text, content);
    lv_obj_set_style_radius(msg_text, 8, 0);
    lv_obj_set_style_bg_opa(msg_text, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(msg_text, 8, 0);
    
    lv_label_set_long_mode(msg_text, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(msg_text, lv_color_black(), 0);
    // Set alignment and style based on message role
    if (strcmp(role, "user") == 0) {
        // User messages are right-aligned with green background
        lv_obj_set_style_bg_color(msg_text, lv_color_hex(0x3eb574), 0);
        
        // Right align the bubble in the container
        lv_obj_align(msg_text, LV_ALIGN_RIGHT_MID, 0, 0);
        // 设置自定义属性标记气泡类型
        lv_obj_set_user_data(msg_bubble, (void*)"user");
    } else if (strcmp(role, "assistant") == 0) {
        // Assistant messages are left-aligned with white background
        lv_obj_set_style_bg_color(msg_text, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_text_color(msg_text, lv_color_white(), 0);
        lv_obj_align(msg_text, LV_ALIGN_LEFT_MID, 0, 0);
        // 设置自定义属性标记气泡类型
        lv_obj_set_user_data(msg_bubble, (void*)"assistant");
    } else if (strcmp(role, "system") == 0) {
        // System messages are center-aligned with light gray background
        lv_obj_set_style_bg_color(msg_text, lv_color_hex(0xffffff), 0);

        // 将气泡居中对齐在容器中
        lv_obj_align(msg_text, LV_ALIGN_CENTER, 0, 0);
        // 设置自定义属性标记气泡类型
        lv_obj_set_user_data(msg_bubble, (void*)"system");
    }

    // 自动滚动底部
    lv_obj_scroll_to_view_recursive(msg_bubble, LV_ANIM_ON);
    // Store reference to the latest message label
    chat_message_label_ = msg_text;
}

void clean_event_handler(lv_event_t *e)
{
  // 获取事件触发的对象
  // lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  show_toast("已经清除！");
  lv_obj_clean(content_);
}

static lv_timer_t *voice_timer = NULL;
static void voice_timer_cb(lv_timer_t *timer)
{
  lv_indev_t *indev = (lv_indev_t *)(timer->user_data);
  lv_point_t point;
  lv_indev_get_point(indev, &point); // 获取当前触摸点坐标

  // 获取按钮的区域
  lv_area_t button_area;
  lv_obj_get_coords(buttom_layout, &button_area);

  // 判断触摸点是否在按钮外部
  if (!lv_area_is_point_on(&button_area, &point, 0))
  {
    lv_img_set_src(voice_icon, &ic_voice_cancel);
    is_cancel = true;
  }
  else
  {
    lv_img_set_src(voice_icon, &ic_voice_press);
    is_cancel = false;
  }
}

void voice_event_handler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  // lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);

  if (code == LV_EVENT_PRESSED)
  {
    // 更换为按下去的图片
    lv_img_set_src(voice_icon, &ic_voice_press);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_HIDDEN);
    lv_indev_t *indev = lv_event_get_indev(e);
    voice_timer = lv_timer_create(voice_timer_cb, 300, indev);
    SEND_COMMAND0(CMD_VOICE_START);
    // ai_ui_set_chat_message("user", "近20家A股上市公司本周披露并购重组最新公告 中国船舶吸收合并中国重工事项获得证监会同意注册批复");
    // ai_ui_set_chat_message("assistant", "阅读提示：阿廖沙的母亲改嫁了，面对这一消息阿廖沙感觉周遭的一切都开始变得模糊，于是他闭上眼睛，假装自己晕过去了。为什么母亲改嫁会给阿廖沙带来如此大的冲击呢？母亲又为什么会失声痛哭呢？📕打开书本，开始今日阅读，感受阿廖沙与母亲之间逐渐疏远的情感，理解成年人在贫困压力下的艰难抉择。.");
    // ai_ui_set_chat_message("system", "正在录音...");
    // ai_ui_set_chat_message("", "亲爱的同学们，今天是《童年》第8次阅读\n【今日任务】今日事今日毕\n\n阅读内容：《拾壹》（P175 - P198）");
  }
  else if (code == LV_EVENT_RELEASED)
  {
    lv_img_set_src(voice_icon, &ic_voice_normal);
    lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);
    if (voice_timer != NULL)
    {
      lv_timer_del(voice_timer);
      voice_timer = NULL;
    }
    SEND_COMMAND0(is_cancel ? CMD_VOICE_CANCEL : CMD_VOICE_END);
  }
}

void ai_ui_page(lv_obj_t *parent)
{
  chat_container = lv_obj_create(parent);
  // lv_obj_set_y(chat_container, 4);

  lv_obj_set_size(chat_container, LV_HOR_RES, LV_VER_RES); // 自动大小
  lv_obj_set_flex_flow(chat_container, LV_FLEX_FLOW_COLUMN); // 垂直排列
  // 水平居中对齐
  lv_obj_set_flex_align(chat_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *emotion_btn = lv_btn_create(chat_container);
  // lv_obj_set_size(emotion_btn, 60, 60);
  // lv_obj_set_style_pad_all(emotion_btn, 10, 0);
  lv_obj_set_style_bg_opa(emotion_btn, LV_OPA_TRANSP, 0);
  emotion_icon = lv_label_create(emotion_btn);
  // lv_obj_set_size(emotion_icon, 60, 60);
  
  lv_obj_set_style_text_font(emotion_icon, imgfont, LV_PART_MAIN);
  lv_label_set_text(emotion_icon, "\U0001F620");

  // 为emotion图标添加点击事件处理器
  lv_obj_add_event_cb(emotion_btn, clean_event_handler, LV_EVENT_CLICKED, NULL);

  // 创建列表区域
  // msgList = lv_list_create(chat_container);
  // lv_obj_set_size(msgList, LV_PCT(90), 0); // 高度自适应
  // lv_obj_set_flex_grow(msgList, 1);        // 使列表撑满剩余空间
  // lv_obj_set_style_border_opa(msgList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_border_width(msgList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_color(msgList, lv_color_hex(0x222222), 0);
  // lv_obj_set_style_bg_opa(msgList, LV_OPA_COVER, 0);

  /* Content - Chat area */
  content_ = lv_obj_create(chat_container);
  lv_obj_set_style_radius(content_, 0, 0);
  lv_obj_set_width(content_, LV_PCT(80));
  lv_obj_set_flex_grow(content_, 1);
  // lv_obj_set_style_pad_all(content_, 10, 0);
  lv_obj_set_style_bg_color(content_, lv_color_hex(0x222222), 0); // Background for chat area
  lv_obj_set_style_bg_opa(content_, LV_OPA_COVER, 0);
  // lv_obj_set_style_border_color(content_, current_theme.border, 0); // Border color for chat area

  // Enable scrolling for chat content
  lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_dir(content_, LV_DIR_VER);
  
  // Create a flex container for chat messages
  lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  // lv_obj_set_style_pad_row(content_, 10, 0); // Space between messages
  // We'll create chat messages dynamically in SetChatMessage
  chat_message_label_ = nullptr;

  // 创建底部按钮区域
  buttom_layout = lv_obj_create(chat_container);
  lv_obj_set_size(buttom_layout, LV_PCT(100), LV_SIZE_CONTENT);                                           // 设置高度
  lv_obj_set_flex_align(buttom_layout, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // 居中对齐
 
  // lv_obj_set_style_pad_all(buttom_layout, 0, 0);

  lv_obj_t *bottom_button = lv_btn_create(buttom_layout);
  lv_obj_set_size(bottom_button, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // 设置高度
  lv_obj_set_style_pad_all(bottom_button, 0, 0);
  lv_obj_set_style_bg_opa(bottom_button, LV_OPA_TRANSP, 0);

  lv_obj_t *voice_layout = lv_obj_create(bottom_button);
  lv_obj_set_size(voice_layout, 80, 80);
  lv_obj_remove_flag(voice_layout, LV_OBJ_FLAG_CLICKABLE);
  // lv_obj_remove_style_all(voice_layout);
  lv_obj_set_style_bg_opa(voice_layout, LV_OPA_TRANSP, 0);
  // lv_obj_set_style_pad_all(voice_layout, 0, 0);
//    lv_obj_set_style_bg_color(voice_layout, lv_color_hex(0x222222), LV_PART_MAIN);
//   lv_obj_set_style_bg_opa(voice_layout, LV_OPA_COVER, 0);
// if(1){return;}
  int32_t arc_width = 8;
  arc = lv_arc_create(voice_layout);
  lv_obj_set_size(arc, LV_PCT(100), LV_PCT(100));
  lv_arc_set_rotation(arc, 270);
  lv_arc_set_bg_angles(arc, 0, 360);
  lv_obj_set_style_arc_width(arc, arc_width, LV_PART_MAIN);      // Changes background arc width
  lv_obj_set_style_arc_width(arc, arc_width, LV_PART_INDICATOR); // Changes set part width
  lv_obj_remove_style(arc, NULL, LV_PART_KNOB);          /*Be sure the knob is not displayed*/
  lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);        /*To not allow adjusting by click*/
  lv_obj_set_style_pad_all(arc, 0, 0);
  lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
  lv_arc_set_range(arc, 0, 100);
  lv_arc_set_value(arc, 0);
  lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);

  voice_icon = lv_img_create(voice_layout);
  lv_img_set_src(voice_icon, &ic_voice_normal); // 设置默认图标
  lv_obj_align(voice_icon, LV_ALIGN_CENTER, 0, 0);
  lv_obj_remove_flag(voice_icon, LV_OBJ_FLAG_CLICKABLE);
  // lv_obj_set_size(arc, ic_voice_normal.header.w + arc_width + 10, ic_voice_normal.header.h + arc_width + 10);
  // lv_obj_set_height(voice_layout, ic_voice_normal.header.h + arc_width + 30);

  // 移除背景
  // lv_obj_set_style_bg_color(voice_icon, lv_color_hex(0xffffff), 0); // 背景透明
  lv_obj_set_style_bg_opa(voice_icon, LV_OPA_TRANSP, 0); // 背景透明度设置为透明

  // 为voice图标添加事件处理器
  lv_obj_add_event_cb(bottom_button, voice_event_handler, LV_EVENT_RELEASED, NULL);
  lv_obj_add_event_cb(bottom_button, voice_event_handler, LV_EVENT_PRESSED, NULL);

  // 设置布局为水平排列
  lv_obj_set_flex_flow(bottom_button, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(bottom_button, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  
}

/*
  VOICE_STATUS_IDLE,
  VOICE_STATUS_PROGRESS,
  VOICE_STATUS_ERROR,
  VOICE_STATUS_END
  */
void update_voice_status(VoiceStatusStruct &data)
{
  if (data.status == VOICE_STATUS_PROCESSING)
  {
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_HIDDEN);
    uint8_t progress = 0;
    UNPACK_UINT8(progress, data.data);
    lv_arc_set_value(arc, progress);
    return;
  }
  lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);
  if (data.status == VOICE_STATUS_ERROR)
  {
    char *msg = (char *)data.data;
    show_toast(msg);
  }
  lv_img_set_src(voice_icon, &ic_voice_normal);
}

void ai_ui_set_emotion(const char *emotion) {
    lv_label_set_text(emotion_icon, emotion);
}