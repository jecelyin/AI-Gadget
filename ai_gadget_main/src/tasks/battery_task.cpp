#include "battery_task.h"
#include <data_struct.h>
#include <uart_cmd.h>
#include <cmd.h>
#include "app_io.h"

namespace Battery_Task
{

  void updateBatteryLevel()
  {
    // 更新电量信息
    float data = readBatteryVoltage();

    SEND_COMMAND(CMD_BATTERY, data);
  }

}