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
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GYRO_INT1_PIN, handleInterrupt, NULL);

    ESP_LOGI(TAG, "Vibration sensor initialized.");
}

// 主循环中的逻辑
void vibration_loop() {
    if (vibrationDetected) {
        vibrationDetected = false;
        ESP_LOGI(TAG, "Vibration detected!");
        
        // 调用唤醒屏幕逻辑
        gpio_set_level(LCD_EN_PIN, 1);
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