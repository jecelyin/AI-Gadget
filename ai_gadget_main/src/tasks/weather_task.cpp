#include "weather_task.h"
#include <data_struct.h>
#include <uart_cmd.h>
#include <cmd.h>
#include "weather.h"

namespace Weather_Task
{
  weather_config_t config = {
      .api_key = CONFIG_WEATHER_API_KEY,
      .api_host = NULL,    // 和风天气需要配置host
      .city = NULL, // city为NULL自动根据IP地址获取位置,也可以指定城市
      // .city = "北京",
      .type = WEATHER_XINZHI  //可更改为api配置WEATHER_AMAP或WEATHER_XINZHI
  };

  void setWeatherData(WeatherStruct &day, weather_info_t *info) {
    day.low_temp = 0;
    day.high_temp = info->temperature;
    day.day_code = info->code;
    strcpy(day.day_weather, info->weather);
    day.night_code = info->code;
    strcpy(day.night_weather, info->weather);
  }

  bool updateWeather()
  {
    // if (WiFi.status() != WL_CONNECTED) {
    //   Serial.println(F("WiFi not connected! Weather update failed!"));
    //   return false;
    // }
    WeathersStruct data;
    weather_info_t *info = weather_get(&config);
    if (info) {
        weather_print_info(info); // 打印天气信息
        setWeatherData(data.today, info);      // 设置 today
        setWeatherData(data.nextday1, info);   // 设置 nextday1
        SEND_COMMAND(CMD_WEATHER, data);
        weather_info_free(info);
    }


    return true;
  }

  void setup()
  {
    // 配置心知天气请求信息
    // weatherNow.config(reqUserKey, reqLocation, reqUnit);
    // forecast.config(reqUserKey, reqLocation, reqUnit);
  }
}