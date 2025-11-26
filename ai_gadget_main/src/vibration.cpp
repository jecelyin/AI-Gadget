#include "vibration.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "SC7A20.h"
#include "rtc_task.h"

// 引入 FreeRTOS 定时器库
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

static const char *TAG = "VIBRATION";

volatile bool vibrationDetected = false;

// 定义定时器句柄
static TimerHandle_t screen_timer = NULL;

// 定时器回调函数：时间到了自动关屏
static void screen_off_callback(TimerHandle_t xTimer) {
    gpio_set_level(LCD_EN_PIN, 0); // 假设 0 是关闭屏幕
    ESP_LOGI(TAG, "Timeout reached. Screen turned OFF.");
}

static void wake_screen() {
    // 1. 马上点亮屏幕
    gpio_set_level(LCD_EN_PIN, 1); 
    ESP_LOGI(TAG, "Screen wake-up triggered by vibration. Screen turned ON.");

    // 2. 计算延时时间 (单位: 毫秒)
    uint32_t timeout_ms = 0;

    // 判断时间段 (RTC_Task::T_HOUR 需确保已由 RTC 任务更新)
    if (RTC_Task::T_HOUR >= 23 || RTC_Task::T_HOUR < 7) {
        // 夜间时间段 (23:00 - 06:59)，30秒后关闭
        timeout_ms = 30 * 1000;
        ESP_LOGI(TAG, "Night mode detected. Screen will turn off in 30s.");
    } else {
        // 白天时间段，1小时后关闭
        timeout_ms = 60 * 60 * 1000;
        ESP_LOGI(TAG, "Day mode detected. Screen will turn off in 1h.");
    }

    // 3. 设置并启动/重置定时器
    if (screen_timer != NULL) {
        // xTimerChangePeriod 会自动启动定时器（如果它原本是停止的），或者重置计时（如果它正在运行）
        // portMAX_DELAY 表示如果队列满则一直等待，但在中断外调用通常用 0 或少量等待
        if (xTimerChangePeriod(screen_timer, pdMS_TO_TICKS(timeout_ms), 0) != pdPASS) {
            ESP_LOGW(TAG, "Failed to start/reset screen timer");
        }
    }
}

// 中断处理函数
static void IRAM_ATTR handleInterrupt(void *arg) {
    vibrationDetected = true;
    // 注意：不要在 ISR 中做复杂的逻辑或打印，只设置标志位
}

// 初始化振动传感器和中断
void vibration_setup() {
    // 初始化 SC7A20 到振动检测模式
    SC7A20_Init(SC7A20_MODE_VIBRATION);

    // 配置中断引脚
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << GYRO_INT1_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,         // CHANGE 触发
    };
    gpio_config(&io_conf);

    // 安装 ISR 服务 (如果其他模块已经安装过，这里可能会报错，需根据实际项目调整)
    // gpio_install_isr_service(0); 
    gpio_isr_handler_add(GYRO_INT1_PIN, handleInterrupt, NULL);

    // --- 创建软件定时器 ---
    // 参数: 名字, 周期(tick), 自动重载(pdFALSE=单次), ID, 回调函数
    screen_timer = xTimerCreate("ScreenTimer", pdMS_TO_TICKS(60000), pdFALSE, (void*)0, screen_off_callback);
    
    if (screen_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create screen timer!");
    }

    ESP_LOGI(TAG, "Vibration sensor initialized.");
}

// 主循环中的逻辑
void vibration_loop() {
    if (vibrationDetected) {
        vibrationDetected = false;
        ESP_LOGI(TAG, "Vibration detected!");
        
        // 调用唤醒屏幕逻辑
        wake_screen();
    }

    // 可选：打印加速度数据（如有需要）
    /*
    if (SC7A20_NewDataReady(&sc7a20)) {
        SC7A20_Measure(&sc7a20);
        ESP_LOGI(TAG, "X: %.2f g | Y: %.2f g | Z: %.2f g",
                 sc7a20.acc_x, sc7a20.acc_y, sc7a20.acc_z);
    }
    */
}