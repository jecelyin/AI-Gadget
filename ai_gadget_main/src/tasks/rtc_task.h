#pragma once
#include <stdint.h>

namespace RTC_Task {
  extern uint8_t T_HOUR;
  extern uint8_t T_MIN;
  void setup();
  void rtc_sync();
}