#include "uart_cmd.h"
#include "stream_protocol.h"
#include "app_sdcard.h"
#include "prefs.h"
#include "player/player.h"
// #include "setting.h"
#include "ai/ai.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "UART_CMD";

#define UART_PORT UART_NUM_1
#define GPIO_RX 47
#define GPIO_TX 48
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

static QueueHandle_t uart_queue;

void do_command(uint8_t cmd, uint8_t *data, uint16_t length);

void uart_cmd_setup() {
    ESP_LOGI(TAG, "Initializing UART command interface...");
    
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // 安装UART驱动
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, GPIO_TX, GPIO_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

void uart_cmd_loop() {
    uint8_t cmd;
    uint16_t length;
    uint8_t *data = NULL;
    char error[64] = {0};

    if (stream_read_cmd(UART_PORT, &cmd, &data, &length, error, 64)) {
        ESP_LOGD(TAG, "Received command: 0x%02X", cmd);
        do_command(cmd, data, length);
        free(data);
    } else if (strlen(error) > 0) {
        ESP_LOGE(TAG, "UART read error: %s", error);
    }
}

void send_cmd(uint8_t cmd, const uint8_t *data, uint16_t dataLength) {
    stream_send_cmd(UART_PORT, cmd, data, dataLength);
}

void sendBufferAsCmd(BinaryPacker &s, uint8_t cmd) {
    if (s.available()) {
        stream_send_cmd(UART_PORT, cmd, s.data(), s.size());
    }
}

void update_voice_status(VoiceStatus status) {
    VoiceStatusStruct data = {
        .status = status
    };
    SEND_COMMAND(CMD_VOICE_UPSTATUS, data);
}

static void handle_lcd_log(uint8_t *data, uint16_t length) {
    char msg[length + 1];
    memcpy(msg, data, length);
    msg[length] = '\0';
    ESP_LOGI(TAG, "handle LCD log[LEN=%d]: %s", length, msg);
}

static const CommandHandler handlers[] = {
    {CMD_PLAYER_LAST_REQ, handle_player_get_last},
    {CMD_PLAYER_LOAD_AND_PLAY, handle_player_load_and_play},
    {CMD_PLAYER_PLAY, handle_player_play},
    {CMD_PLAYER_PAUSE, handle_player_pause},
    {CMD_PLAYER_SEEK, handle_player_seek},
    {CMD_PLAYER_OPEN_DIR, handle_player_open_dir},
    {CMD_PLAYER_FILES_REQ, handle_player_file_list},
    // {CMD_SETTING_ENTER, handle_setting_enter},
    // {CMD_SETTING_EXIT, handle_setting_exit},
    {CMD_VOICE_START, handle_voice_start},
    {CMD_VOICE_END, handle_voice_end},
    {CMD_VOICE_CANCEL, handle_voice_cancel},
    {CMD_LCD_LOG, handle_lcd_log}
};

void do_command(uint8_t cmd, uint8_t *src, uint16_t length) {
    for (size_t i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
        if (handlers[i].cmd == cmd) {
            ESP_LOGD(TAG, "Handle cmd: 0x%02X", cmd);
            handlers[i].handler(src, length);
            return;
        }
    }
    ESP_LOGE(TAG, "Unknown cmd: 0x%02X", cmd);
}

// 独立的UART任务处理函数
static void uart_event_task(void *pvParameters) {
    uart_event_t event;
    while (1) {
        if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA:
                    uart_cmd_loop();
                    break;
                case UART_FIFO_OVF:
                    ESP_LOGE(TAG, "UART FIFO overflow");
                    uart_flush_input(UART_PORT);
                    xQueueReset(uart_queue);
                    break;
                case UART_BUFFER_FULL:
                    ESP_LOGE(TAG, "UART buffer full");
                    uart_flush_input(UART_PORT);
                    xQueueReset(uart_queue);
                    break;
                default:
                    ESP_LOGI(TAG, "UART event type: %d", event.type);
                    break;
            }
        }
    }
    vTaskDelete(NULL);
}

void uart_cmd_start_task() {
    xTaskCreate(uart_event_task, "uart_cmd_task", 4096, NULL, 10, NULL);
}