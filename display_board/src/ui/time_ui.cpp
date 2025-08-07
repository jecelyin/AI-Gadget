#include "time_ui.h"
#include "resources.h"
#include <stdio.h>
#include "weather_layout.h"


const char* weekDays[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};

char battery_value[16] = "";           // Default battery value
char date_text[32] = "2024-11-18 星期一";   // Default date text
char time_text[6] = "00:00";               // Default time text
char temp_value[16] = "25°";                // Default temperature value
char humid_value[16] = "40%";               // Default humidity value
uint16_t voc_value = 1;             // Default VOC value
uint16_t nox_value = 1;             // Default NOx value;

lv_obj_t *battery_icon;
lv_obj_t *battery_label;
lv_obj_t *date_label;
lv_obj_t *time_label;
lv_obj_t *temp_label;
lv_obj_t *humid_label;
lv_obj_t *voc_label;
lv_obj_t *nox_label;
lv_obj_t *footer_label;

const char* get_air_quality_text(uint16_t voc);

void time_ui_page(lv_obj_t *parent) {
    // lv_obj_set_size(parent, LV_PCT(100), LV_PCT(100)); // Full-screen container
    // Create a parent container to center all content
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES); // Full-screen container
    // lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_style_align(container, LV_ALIGN_CENTER, 0);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    // Battery icon and label
    lv_obj_t *battery_layout = lv_obj_create(container);
    lv_obj_set_size(battery_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Auto-size
    lv_obj_set_layout(battery_layout, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(battery_layout, LV_FLEX_FLOW_ROW); // Horizontal layout
    lv_obj_set_flex_align(battery_layout, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    battery_icon = lv_img_create(battery_layout);
    lv_img_set_src(battery_icon, &ic_medium_battery); // Use binary image C array
    battery_label = lv_label_create(battery_layout);
    lv_label_set_text(battery_label, battery_value);
    lv_obj_set_style_margin_left(battery_label, 5, 0); 

    // Weather label
    build_weather_layout(container);

    // Time label
    time_label = lv_label_create(container);
    lv_obj_set_style_text_font(time_label, &SF_UI_Text_b_120, 0);
    lv_label_set_text(time_label, time_text);

    // Date label
    date_label = lv_label_create(container);
    lv_obj_set_style_text_font(date_label, &SF_UI_Text_b_28, 0);
    lv_label_set_text(date_label, date_text);

    // Create a flex container for temperature and humidity
    lv_obj_t *temp_humid_row = lv_obj_create(container);
    lv_obj_set_size(temp_humid_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Auto-size based on content
    lv_obj_set_layout(temp_humid_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(temp_humid_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(temp_humid_row, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Temperature layout (image + label)
    lv_obj_t *temp_layout = lv_obj_create(temp_humid_row);
    lv_obj_set_size(temp_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Auto-size
    lv_obj_set_layout(temp_layout, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(temp_layout, LV_FLEX_FLOW_ROW); // Vertical layout for temp icon and value
    lv_obj_set_flex_align(temp_layout, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *temp_img = lv_img_create(temp_layout);
    lv_img_set_src(temp_img, &ic_temperature); // Use binary image C array
    temp_label = lv_label_create(temp_layout);
    lv_label_set_text(temp_label, temp_value);

    // Humidity layout (image + label)
    lv_obj_t *humid_layout = lv_obj_create(temp_humid_row);
    lv_obj_set_style_margin_left(humid_layout, 15, 0);
    lv_obj_set_size(humid_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Auto-size
    lv_obj_set_layout(humid_layout, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(humid_layout, LV_FLEX_FLOW_ROW); // Vertical layout for humidity icon and value
    lv_obj_set_flex_align(humid_layout, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *humid_img = lv_img_create(humid_layout);
    lv_img_set_src(humid_img, &ic_humidity); // Use binary image C array
    humid_label = lv_label_create(humid_layout);
    lv_obj_set_style_margin_left(humid_label, 5, 0);
    lv_label_set_text(humid_label, humid_value);

    // Create a flex container for VOC and NOx
    lv_obj_t *voc_nox_row = lv_obj_create(container);
    lv_obj_set_size(voc_nox_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Auto-size based on content
    lv_obj_set_layout(voc_nox_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(voc_nox_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(voc_nox_row, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *voc_img = lv_img_create(voc_nox_row);
    lv_img_set_src(voc_img, &ic_voc); 
    // VOC label
    voc_label = lv_label_create(voc_nox_row);
    lv_obj_set_style_margin_left(voc_label, 5, 0);

    lv_obj_t *nox_img = lv_img_create(voc_nox_row);
    lv_obj_set_style_margin_left(nox_img, 15, 0);
    lv_img_set_src(nox_img, &ic_nox); 
    // NOx label
    nox_label = lv_label_create(voc_nox_row);
    lv_obj_set_style_margin_left(nox_label, 5, 0);

    // Footer (e.g., small "1000" text)
    footer_label = lv_label_create(container);
    lv_label_set_text(footer_label, "");
    lv_obj_set_style_text_align(footer_label, LV_TEXT_ALIGN_CENTER, 0);

    // Adjust spacing between rows
    lv_obj_set_style_pad_row(container, 15, 0); // Add vertical spacing between elements
}

void time_ui_update_datetime(DateTimeStruct dt) {
    // snprintf(date_text, strlen(date_text), "%04d-%02d-%02d %s",
    //          dt.year,
    //          dt.month,
    //          dt.day,
    //          weekDays[dt.week]);
    // lv_label_set_text(date_label, date_text);
    // snprintf(time_text, strlen(time_text), "%02d:%02d", dt.hour, dt.min);
    // lv_label_set_text(time_label, time_text);
    lv_label_set_text_fmt(date_label, "%04d-%02d-%02d %s", dt.year,dt.month,dt.day,weekDays[dt.week]);
    lv_label_set_text_fmt(time_label, "%02d:%02d", dt.hour, dt.min);
}

/**
 * @brief VOC: VOC指数是由检测到的空气中乙醇当量换算得出。测量范围为0-1000ppm乙醇当量。
0-100：优，无需通风
100-200：良好，无需通风
200-300：轻度污染，建议开窗通风
300-400：中度污染，建议开窗通风
400-500：重度污染，建议猛烈通风
 * 
 * @param data 
 */
void time_ui_update_air(SGP41Struct data) {
    voc_value = data.voc;
    nox_value = data.nox;
    lv_label_set_text_fmt(voc_label,  "%d (%s)",  voc_value, get_air_quality_text(voc_value));
    lv_label_set_text_fmt(nox_label, "%d (%s)", nox_value, get_air_quality_text(nox_value));
}

const char* get_air_quality_text(uint16_t voc) {
    if (voc < 100) {
        // snprintf(buffer, size, "🟢空气清新");
        return "空气清新";
    } else if (voc < 200) {
        // snprintf(buffer, size, "🟡轻微污染");
        return "轻微污染";
    } else if (voc < 300) {
        // snprintf(buffer, size, "🟠有异味");
        return "有异味";
    } else if (voc < 400) {
        // snprintf(buffer, size, "🔴空气不佳");
        return "空气不佳";
    } else {
        // snprintf(buffer, size, "🟣污染严重");
        return "污染严重";
    }
}

void time_ui_update_temp_humidity(Sht4xStruct data) {
    LV_LOG("temperature: %.2f", data.temperature);
    float temp = ((int)(data.temperature * 10)) / 10.0f;  // 23.986 → 23.9
    float humidity = ((int)(data.humidity * 10)) / 10.0f;  // 23.986 → 23.9
    lv_label_set_text_fmt(temp_label, "%.1f°", temp);
    lv_label_set_text_fmt(humid_label, "%.1f%%", humidity);
}

void time_ui_update_battery(float voltage) {
    lv_label_set_text_fmt(battery_label, "%.2fv", voltage);
}

void time_ui_update_footer(const char* text) {
    lv_label_set_text(footer_label, text);
}