#include "ui_lib.h"
#include "emoji_font.h"

lv_obj_t *toast = nullptr;

static void toast_deleted_cb(lv_anim_t *a)
{
    // 动画结束后删除 toast 对象
    if (toast) {
        lv_obj_del(toast);
        toast = nullptr; // 清空指针，防止悬空指针
    }
}

void show_toast(const char *text, uint32_t duration_ms)
{
    // 如果已有Toast，先删除它
    if (toast) {
        lv_obj_del(toast);
        toast = nullptr;
    }

    // 创建透明背景容器
    toast = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(toast); // 移除默认样式
    lv_obj_set_size(toast, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(toast);
    
    // 禁用点击事件
    lv_obj_add_flag(toast, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(toast, LV_OBJ_FLAG_CLICKABLE); // 确保不可点击
    lv_obj_add_flag(toast, LV_OBJ_FLAG_IGNORE_LAYOUT); // 忽略布局

    // 设置背景样式
    lv_obj_set_style_bg_color(toast, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(toast, LV_OPA_70, 0);
    lv_obj_set_style_radius(toast, 8, 0);
    lv_obj_set_style_pad_all(toast, 12, 0);

    // 添加文本
    lv_obj_t *label = lv_label_create(toast);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, imgfont, LV_PART_MAIN);
    lv_obj_center(label);

    // 设置动画 - 延迟 + 淡出
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, toast);
    lv_anim_set_delay(&a, duration_ms); // 延迟多少毫秒后开始
    lv_anim_set_time(&a, 500); // 淡出动画持续时间
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_set_values(&a, LV_OPA_70, 0);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_deleted_cb(&a, toast_deleted_cb); // 动画结束后删除对象
    lv_anim_start(&a);
}
