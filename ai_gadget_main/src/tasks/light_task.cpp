#include "light_task.h"
#include <data_struct.h>
#include <uart_cmd.h>
#include <cmd.h>
#include "app_io.h"
#include "esp_log.h"

namespace Light_Task
{
  static const char *TAG = "Light_Task";
  void updateLightSensorValue()
  {
    // 更新光敏电阻的值
    // Serial.println("Updating Light Sensor Value...");
    uint16_t data = readLightValue();
    // ESP_LOGI(TAG, "Current light sensor value: %d", data);
    SEND_COMMAND(CMD_LIGHT, data);
  }

}