#include "rtc_task.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "cmd.h"
#include "uart_cmd.h"
#include "data_struct.h"
#include <time.h>
#include <sys/time.h>
#include "wifi_utils.h"

#define TAG "RTC_Task"
#define RTC_UPDATE_INTERVAL_MS (10 * 1000)

namespace RTC_Task
{
    static bool sntp_initialized = false;
    static time_t epochNowTime;

#if USE_RTC_DS3231
#include "ds3231.h" // 你需要提供/集成 DS3231 驱动
    extern ds3231_dev_t rtc_dev;
#endif

    static void updateRTCInfo(void)
    {
        ESP_LOGI(TAG, "Updating RTC Info...");
        struct tm timeinfo;

#if USE_RTC_DS3231
        float temperature;
        ds3231_get_temperature(&rtc_dev, &temperature);
        ESP_LOGI(TAG, "RTC Temperature: %.2f °C", temperature);

        if (!ds3231_get_time(&rtc_dev, &timeinfo))
        {
            ESP_LOGE(TAG, "Failed to get time from RTC");
            return;
        }
#else
        time(&epochNowTime);
        localtime_r(&epochNowTime, &timeinfo);
#endif

        DateTimeStruct data = {
            .year = static_cast<uint16_t>(timeinfo.tm_year + 1900),
            .month = static_cast<uint8_t>(timeinfo.tm_mon + 1),
            .day = static_cast<uint8_t>(timeinfo.tm_mday),
            .hour = static_cast<uint8_t>(timeinfo.tm_hour),
            .min = static_cast<uint8_t>(timeinfo.tm_min),
            .sec = static_cast<uint8_t>(timeinfo.tm_sec),
            .week = static_cast<uint8_t>(timeinfo.tm_wday),
            .is_pm = (timeinfo.tm_hour >= 12)};

        SEND_COMMAND(CMD_DATE_TIME, data);
    }

    static void initialize_sntp(void)
    {
        if (sntp_initialized)
            return;

        ESP_LOGI(TAG, "Initializing SNTP");
        // ✅ 设置时区为东八区（中国标准时间 CST）
        setenv("TZ", "CST-8", 1);
        tzset();
        esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "ntp1.aliyun.com"); // 国内NTP服务器
        esp_sntp_init();

        sntp_initialized = true;
    }

    static void sync_time_with_ntp(void)
    {
        initialize_sntp();

        int retry = 0;
        const int retry_count = 10;
        struct tm timeinfo;

        while (retry < retry_count)
        {
            time_t now;
            time(&now);
            localtime_r(&now, &timeinfo);
            if (timeinfo.tm_year > (2016 - 1900))
            {
                epochNowTime = now;
                ESP_LOGI(TAG, "NTP Time Sync success: %s", asctime(&timeinfo));
#if USE_RTC_DS3231
                ds3231_set_time(&rtc_dev, &timeinfo);
#endif
                return;
            }
            ESP_LOGW(TAG, "Waiting for NTP sync...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            retry++;
        }
        ESP_LOGE(TAG, "NTP sync failed.");
    }

    void setup(void)
    {
#if USE_RTC_DS3231
        ds3231_init_desc(&rtc_dev, I2C_NUM_0, (gpio_num_t)CONFIG_SDA_GPIO, (gpio_num_t)CONFIG_SCL_GPIO);
        if (!ds3231_is_available(&rtc_dev))
        {
            ESP_LOGE(TAG, "DS3231 not found!");
            return;
        }
#endif
    }

    void rtc_sync(void)
    {
        ESP_LOGI(TAG, "RTC sync...");
        if (!wifi_utils::isWifiConnected())
        {
            ESP_LOGW(TAG, "WiFi not connected, skipping RTC sync");
            return;
        }

        sync_time_with_ntp();
        updateRTCInfo();
    }
}