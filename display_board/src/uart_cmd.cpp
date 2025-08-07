#include "uart_cmd.h"
#include "ai_ui.h"
#include "data_struct.h"
#include "face_ui.h"
#include "stream_protocol.h"
#include "time_ui.h"
#include "ui_player.h"
#include "weather_layout.h"
#include "BinaryPacker.h"
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "ui_player_crtl.h"
#include "ui_player_presenter.h"
#include "ui_player_list.h"
#include "esp_log.h"
#include "ui_lib.h"
#include "main_ui.h"

#define GPIO_RX   20
#define GPIO_TX   19
#define UART_PORT UART_NUM_1
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

BinaryPacker s;
SemaphoreHandle_t lvglMutex;
QueueHandle_t commandQueue;
static QueueHandle_t uart_queue;

static const char *TAG = "UART_CMD";

void do_command(uint8_t cmd, uint8_t *data, uint16_t length);

void uart_init()
{
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
  // 创建互斥锁
  lvglMutex = xSemaphoreCreateMutex();
  if (lvglMutex == NULL)
  {
    ESP_LOGI(TAG, "Failed to create mutex!");
    while (1)
      ;
  }
  commandQueue = xQueueCreate(10, sizeof(Command));
  if (commandQueue == NULL)
  {
    ESP_LOGI(TAG, "Failed to create queue!");
    while (1)
      ;
  }
}

void uart_task_loop()
{
  uint8_t cmd;
  uint16_t length;
  uint8_t *data = nullptr;
  char error[64] = {0};

  if (stream_read_cmd(UART_PORT, &cmd, &data, &length, error, 64)) {
    // 创建命令结构
    Command command;
    command.cmd = cmd;
    command.length = length;
    command.data = data;

    // 发送命令到队列
    if (xQueueSend(commandQueue, &command, portMAX_DELAY) != pdTRUE)
    {
      ESP_LOGE(TAG, "Failed to send command to queue!");
      if (data != nullptr)
      {
        delete[] data;  // 如果发送失败，释放内存
        data = nullptr; // 防止悬空指针
      }
    }
    // 注意：这里不再释放 data，因为 data 的所有权已经转移到队列中
  } else if (strlen(error) > 0) {
    ESP_LOGE(TAG, "UART read error: %s", error);
  }
}

void uart_lvgl_loop()
{
  Command command;
  if (xQueueReceive(commandQueue, &command, 0) == pdTRUE)
  {
    // 获取互斥锁
    if (xSemaphoreTake(lvglMutex, portMAX_DELAY) == pdTRUE)
    {
      // 执行命令
      // Serial.println("收到命令了");
      do_command(command.cmd, command.data, command.length);

      // 释放互斥锁
      xSemaphoreGive(lvglMutex);
    }

    // 释放数据内存
    if (command.data != nullptr)
    {
      delete[] command.data;
      command.data = nullptr; // 防止悬空指针
    }
  }
}

void handle_date_time(uint8_t *src, uint16_t length)
{
  DateTimeStruct data;
  decode_data(data, src, length);
  time_ui_update_datetime(data);
}

void handle_temp_humidity(uint8_t *src, uint16_t length)
{
  Sht4xStruct data;
  decode_data(data, src, length);
  time_ui_update_temp_humidity(data);
}

void handle_air(uint8_t *src, uint16_t length)
{
  SGP41Struct data;
  decode_data(data, src, length);
  time_ui_update_air(data);
}

void handle_battery(uint8_t *src, uint16_t length)
{
  float voltage = 0;
  decode_data(voltage, src, length);
  time_ui_update_battery(voltage);
}

void handle_wifi_ap(uint8_t *src, uint16_t length)
{
  WiFiStruct data;
  decode_data(data, src, length);
  face_ui_show_wifi_ap_win(data);
}

void handle_setting_url(uint8_t *src, uint16_t length)
{
  String url = "http://" + String(src, length);
  face_ui_show_setting_url_win(url.c_str());
}

void handle_weather(uint8_t *src, uint16_t length)
{
  WeathersStruct data;
  decode_data(data, src, length);
  update_weather(data);
}

void handle_voice_status(uint8_t *src, uint16_t length)
{
  VoiceStatusStruct data;
  decode_data(data, src, length);

  update_voice_status(data);
}

void handle_player_list(uint8_t *src, uint16_t length)
{
  s.load(src, length);
  std::string path = s.readString();
  uint16_t page = s.readUint16();
  uint16_t total_page = s.readUint16();
  uint16_t count = s.readUint16();
  ESP_LOGD(TAG, "receive player list: page=%d, total_page=%d, count=%d", page, total_page, count);

  player_clean_data(page);

  uint16_t i = 0;
  while (i < count && s.available())
  {
    i++;
    uint8_t type = s.readUint8(); // 读取文件类型：文件夹 (0) 或 音频文件 (1)
    FileMeta fileMeta;
    if (type == 0)
    { // 处理文件夹
      std::string folderName = s.readString();
      strncpy(fileMeta.fileName, folderName.c_str(), sizeof(fileMeta.fileName) - 1);
      fileMeta.isFile = false;
    }
    else if (type == 1)
    { // 处理音频文件
      std::string fileName = s.readString();
      uint32_t fileSize = s.readUint32();
      uint32_t duration = s.readUint32();

      strncpy(fileMeta.fileName, fileName.c_str(), sizeof(fileMeta.fileName) - 1);
      fileMeta.isFile = true;
      fileMeta.duration = duration;
      fileMeta.size = fileSize;
    }
    player_add_data(page, fileMeta);
  }
  player_flush_page(page, total_page);
  ui_player_update_list();
  s.clear();
}

void handle_player_last_info(uint8_t *src, uint16_t length)
{
  s.load(src, length);
  std::string last_path = s.readString();
  std::string last_audio = s.readString();
  uint16_t last_position = s.readUint16();
  uint16_t last_duration = s.readUint16();
  ui_player_update_last_info(last_audio.c_str(), last_duration, last_position);
  s.clear();
}

void handle_light(uint8_t *src, uint16_t length)
{
  s.load(src, length);
  char buffer[32]; // 足够大的缓冲区
  snprintf(buffer, sizeof(buffer), "光线：%d%%", s.readUint16());
  time_ui_update_footer(buffer);
}

void handle_set_status(uint8_t *src, uint16_t length)
{
  s.load(src, length);
  std::string status = s.readString();
  ESP_LOGI(TAG, "Set status: %s", status.c_str());
  time_ui_update_footer(status.c_str());
  if (current_ui_page() == AI_UI_PAGE) {
    // 如果当前不是 AI UI 页面，显示为 Toast
    show_toast(status.c_str());
  }
}

void handle_set_notification(uint8_t *src, uint16_t length)
{
  s.load(src, length);
  std::string notification = s.readString();
  ESP_LOGI(TAG, "Set notification: %s", notification.c_str());
  show_toast(notification.c_str());
}

void handle_set_emotion(uint8_t *src, uint16_t length)
{
  s.load(src, length);
  std::string emotion = s.readString();
  ESP_LOGI(TAG, "Set emotion: %s", emotion.c_str());

  ai_ui_set_emotion(emotion.c_str());
  if (current_ui_page() != AI_UI_PAGE) {
    // 如果当前不是 AI UI 页面，显示为 Toast
    show_toast(emotion.c_str());
  }
  
}

void handle_set_chat_message(uint8_t *src, uint16_t length)
{
  s.load(src, length);
  std::string role = s.readString();
  std::string message = s.readString();
  ESP_LOGI(TAG, "Set chat message[%s]: %s", role.c_str(), message.c_str());
  ai_ui_set_chat_message(role.c_str(), message.c_str());
}

// 创建一个包含所有命令处理的数组
CommandHandler handlers[] = {{CMD_DATE_TIME, handle_date_time},
                             {CMD_TEMP_HUMIDITY, handle_temp_humidity},
                             {CMD_VOC_NOX, handle_air},
                             {CMD_BATTERY, handle_battery},
                             {CMD_WIFI_AP, handle_wifi_ap},
                             {CMD_WEATHER, handle_weather},
                             {CMD_LIGHT, handle_light},
                             {CMD_VOICE_UPSTATUS, handle_voice_status},
                             {CMD_PLAYER_LAST_RESP, handle_player_last_info},
                             {CMD_PLAYER_FILES_RESP, handle_player_list},
                             {CMD_SETTING_URL, handle_setting_url},
                             {CMD_SET_STATUS, handle_set_status},
                             {CMD_SET_NOTIFICATION, handle_set_notification},
                             {CMD_SET_EMOTION, handle_set_emotion},
                             {CMD_SET_CHAT_MESSAGE, handle_set_chat_message},
                            };

void do_command(uint8_t cmd, uint8_t *src, uint16_t length)
{
  // 遍历 handlers 数组以找到匹配的 cmd
  for (const auto &handler : handlers)
  {
    if (handler.cmd == cmd)
    {
      handler.handler(src, length); // 调用相应的解码函数
      return;                       // 找到后退出
    }
  }
  // String log = "Unknown cmd: " + String(cmd);
  // send_command(CMD_LCD_LOG, reinterpret_cast<const uint8_t*>(log.c_str()), log.length());
  ESP_LOGE(TAG, "Unknown cmd: 0x%X", cmd);
}

void send_command(uint8_t cmd, const uint8_t *data, uint16_t length)
{
  stream_send_cmd(UART_PORT, cmd, data, length);
}
