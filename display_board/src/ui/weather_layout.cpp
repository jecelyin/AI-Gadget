#include "weather_layout.h"
#include "resources.h"
#include <stdio.h> // 包含 snprintf 的头文件

#define WEATHER_DEMO  1
lv_obj_t *weather_grid = nullptr; // 全局变量，用于存储网格对象

typedef struct {
  int code;
  const lv_img_dsc_t *img;
} CodeImageMap;

// 创建一个映射表，将特定的code值映射到图像
CodeImageMap code_image_map[] = {
    {0, &weather0},   {1, &weather1},   {2, &weather2},   {3, &weather3},
    {4, &weather4},   {5, &weather5},   {6, &weather6},   {7, &weather7},
    {8, &weather8},   {9, &weather9},   {10, &weather10}, {11, &weather11},
    {12, &weather12}, {13, &weather13}, {14, &weather14}, {15, &weather15},
    {16, &weather16}, {17, &weather17}, {18, &weather18}, {19, &weather19},
    {20, &weather20}, {21, &weather21}, {22, &weather22}, {23, &weather23},
    {24, &weather24}, {25, &weather25}, {26, &weather26}, {27, &weather27},
    {28, &weather28}, {29, &weather29}, {30, &weather30}, {31, &weather31},
    {32, &weather32}, {33, &weather33}, {34, &weather34}, {35, &weather35},
    {36, &weather36}, {37, &weather37}, {38, &weather38}, {99, &weather99},
};

#define CODE_IMAGE_MAP_SIZE (sizeof(code_image_map) / sizeof(CodeImageMap))

const lv_img_dsc_t *get_weather_icon(int code) {
  for (int i = 0; i < CODE_IMAGE_MAP_SIZE; i++) {
    if (code_image_map[i].code == code) {
      return code_image_map[i].img;
    }
  }
  return NULL; // 找不到匹配项时返回NULL
}

void update_weather2(const WeatherStruct &today, const WeatherStruct &nextday1);

#if WEATHER_DEMO
// 定时器回调函数
static void timer_callback(lv_timer_t *timer) {
  // 构造天气数据
  WeatherStruct today = {0, 0, 99, "未知", 99, "未知"};
  WeatherStruct nextday1 = {0, 0, 99, "未知", 99, "未知"};

  // 调用更新天气函数
  update_weather2(today, nextday1);

  // 删除定时器（如果需要只执行一次）
  lv_timer_del(timer);
}
#endif

// 缓存天气布局中的对象
typedef struct {
  lv_obj_t *today_temp_label;
  lv_obj_t *nextday1_temp_label;

  // 白天和夜晚的 flex container
  lv_obj_t *today_day_cont;
  lv_obj_t *nextday1_day_cont;
  lv_obj_t *today_night_cont;
  lv_obj_t *nextday1_night_cont;

  // 白天和夜晚的 icon 和 label
  lv_obj_t *today_day_icon;
  lv_obj_t *today_day_label;
  lv_obj_t *nextday1_day_icon;
  lv_obj_t *nextday1_day_label;
  lv_obj_t *today_night_icon;
  lv_obj_t *today_night_label;
  lv_obj_t *nextday1_night_icon;
  lv_obj_t *nextday1_night_label;
} WeatherLayoutCache;

WeatherLayoutCache weather_cache;

// 创建天气布局
void build_weather_layout(lv_obj_t *parent) {
  // 创建网格布局
  weather_grid = lv_obj_create(parent);
  lv_obj_set_size(weather_grid, 360, LV_SIZE_CONTENT); // 设置网格大小
  lv_obj_align(weather_grid, LV_ALIGN_CENTER, 0, 0);   // 居中显示

  // 设置网格布局
  lv_obj_set_layout(weather_grid, LV_LAYOUT_GRID); // 关键：设置布局为网格布局
  // 禁用滚动条
  lv_obj_set_scrollbar_mode(weather_grid, LV_SCROLLBAR_MODE_OFF);

  // 设置网格列数和行数
  static lv_coord_t col_dsc[] = {
      50, 150, 150, LV_GRID_TEMPLATE_LAST}; // 第1列20%，第2、3列各40%
  static lv_coord_t row_dsc[] = {30, 30, 30,
                                 LV_GRID_TEMPLATE_LAST}; // 3行高度，每行40px
  lv_obj_set_style_grid_column_dsc_array(weather_grid, col_dsc, 0);
  lv_obj_set_style_grid_row_dsc_array(weather_grid, row_dsc, 0);

  // 设置表头
  lv_obj_t *label;

  label = lv_label_create(weather_grid);
  lv_label_set_text(label, " ");
  lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER,
                       0, 1); // 左上角留空

  weather_cache.today_temp_label = lv_label_create(weather_grid);
  lv_label_set_text(weather_cache.today_temp_label, "今天");
  lv_obj_set_grid_cell(weather_cache.today_temp_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER,
                       0, 1); // 今天

  weather_cache.nextday1_temp_label = lv_label_create(weather_grid);
  lv_label_set_text(weather_cache.nextday1_temp_label, "明天");
  lv_obj_set_grid_cell(weather_cache.nextday1_temp_label, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER,
                       0, 1); // 明天

  // 设置行标题
  label = lv_label_create(weather_grid);
  lv_label_set_text(label, "白天");
  lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER,
                       1, 1); // 白天

  label = lv_label_create(weather_grid);
  lv_label_set_text(label, "夜晚");
  lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER,
                       2, 1); // 夜晚

  // 创建并缓存白天和夜晚的 flex container
  weather_cache.today_day_cont = lv_obj_create(weather_grid);
  lv_obj_set_size(weather_cache.today_day_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(weather_cache.today_day_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(weather_cache.today_day_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(weather_cache.today_day_cont, 5, 0);
  lv_obj_set_grid_cell(weather_cache.today_day_cont, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
  lv_obj_set_scrollbar_mode(weather_cache.today_day_cont, LV_SCROLLBAR_MODE_OFF);

  weather_cache.nextday1_day_cont = lv_obj_create(weather_grid);
  lv_obj_set_size(weather_cache.nextday1_day_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(weather_cache.nextday1_day_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(weather_cache.nextday1_day_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(weather_cache.nextday1_day_cont, 5, 0);
  lv_obj_set_grid_cell(weather_cache.nextday1_day_cont, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
  lv_obj_set_scrollbar_mode(weather_cache.nextday1_day_cont, LV_SCROLLBAR_MODE_OFF);

  weather_cache.today_night_cont = lv_obj_create(weather_grid);
  lv_obj_set_size(weather_cache.today_night_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(weather_cache.today_night_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(weather_cache.today_night_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(weather_cache.today_night_cont, 5, 0);
  lv_obj_set_grid_cell(weather_cache.today_night_cont, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
  lv_obj_set_scrollbar_mode(weather_cache.today_night_cont, LV_SCROLLBAR_MODE_OFF);

  weather_cache.nextday1_night_cont = lv_obj_create(weather_grid);
  lv_obj_set_size(weather_cache.nextday1_night_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(weather_cache.nextday1_night_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(weather_cache.nextday1_night_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(weather_cache.nextday1_night_cont, 5, 0);
  lv_obj_set_grid_cell(weather_cache.nextday1_night_cont, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
  lv_obj_set_scrollbar_mode(weather_cache.nextday1_night_cont, LV_SCROLLBAR_MODE_OFF);

  // 创建并缓存 icon 和 label
  weather_cache.today_day_icon = lv_img_create(weather_cache.today_day_cont);
  weather_cache.today_day_label = lv_label_create(weather_cache.today_day_cont);

  weather_cache.nextday1_day_icon = lv_img_create(weather_cache.nextday1_day_cont);
  weather_cache.nextday1_day_label = lv_label_create(weather_cache.nextday1_day_cont);

  weather_cache.today_night_icon = lv_img_create(weather_cache.today_night_cont);
  weather_cache.today_night_label = lv_label_create(weather_cache.today_night_cont);

  weather_cache.nextday1_night_icon = lv_img_create(weather_cache.nextday1_night_cont);
  weather_cache.nextday1_night_label = lv_label_create(weather_cache.nextday1_night_cont);

#if WEATHER_DEMO
  // 创建定时器，延迟 3 秒后更新天气
  lv_timer_create(timer_callback, 100, nullptr); // 3000ms = 3秒
#endif
}

// 更新天气信息
void update_weather2(const WeatherStruct &today,
                     const WeatherStruct &nextday1) {
  if (!weather_grid)
    return; // 确保网格布局已创建

  // 更新温度信息
  char temp_buf[32];
  snprintf(temp_buf, sizeof(temp_buf), "今天 %d°C ~ %d°C", today.low_temp,
           today.high_temp);
  lv_label_set_text(weather_cache.today_temp_label, temp_buf); // 今天温度

  snprintf(temp_buf, sizeof(temp_buf), "明天 %d°C ~ %d°C", nextday1.low_temp,
           nextday1.high_temp);
  lv_label_set_text(weather_cache.nextday1_temp_label, temp_buf); // 明天温度

  // 更新白天天气信息
  lv_img_set_src(weather_cache.today_day_icon, get_weather_icon(today.day_code));
  lv_label_set_text(weather_cache.today_day_label, today.day_weather); // 今天白天
  lv_obj_set_style_margin_left(weather_cache.today_day_label, 5, 0);

  lv_img_set_src(weather_cache.nextday1_day_icon, get_weather_icon(nextday1.day_code));
  lv_label_set_text(weather_cache.nextday1_day_label, nextday1.day_weather); // 明天白天
  lv_obj_set_style_margin_left(weather_cache.nextday1_day_label, 5, 0);

  // 更新夜晚天气信息
  lv_img_set_src(weather_cache.today_night_icon, get_weather_icon(today.night_code));
  lv_label_set_text(weather_cache.today_night_label, today.night_weather); // 今天夜晚
  lv_obj_set_style_margin_left(weather_cache.today_night_label, 5, 0);

  lv_img_set_src(weather_cache.nextday1_night_icon, get_weather_icon(nextday1.night_code));
  lv_label_set_text(weather_cache.nextday1_night_label, nextday1.night_weather); // 明天夜晚
  lv_obj_set_style_margin_left(weather_cache.nextday1_night_label, 5, 0);
}

void update_weather(WeathersStruct &data) {
  update_weather2(data.today, data.nextday1);
}