#include "config.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "adc_cali_schemes.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include "esp_check.h"

static const char *TAG = "app_io";


adc_oneshot_unit_handle_t battery_handle;
adc_cali_handle_t cali_handle = NULL;
adc_unit_t unit_id;
adc_channel_t channel;

static esp_err_t init_adc_calibration();


esp_err_t ADC_init()
{
    esp_err_t ret = ESP_OK;

    ret = adc_oneshot_io_to_channel(LIGHT_ADC_PIN, &unit_id, &channel);
    ESP_RETURN_ON_ERROR(ret, TAG, "adc oneshot failed.");
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit_id,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ret = adc_oneshot_new_unit(&init_config, &battery_handle);
    ESP_RETURN_ON_ERROR(ret, TAG, "adc oneshot unit initialization failed.");
    ret = adc_oneshot_config_channel(battery_handle, channel, &config);
    ESP_RETURN_ON_ERROR(ret, TAG, "adc oneshot channel configuration failed.");
    ret = init_adc_calibration();
    ESP_RETURN_ON_ERROR(ret, TAG, "adc oneshot calibration inittialization failed.");
    return ret;
}


int32_t light_level_read()
{
    int voltage = 0;
    adc_oneshot_get_calibrated_result(battery_handle, cali_handle, channel, &voltage);

    ESP_LOGD(TAG, "Cali voltage: %d", voltage);
    return voltage;
}

static esp_err_t init_adc_calibration()
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit_id,
            .chan = channel,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    cali_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return ret;
}

void app_io_init()
{
  // gpio_reset_pin(LIGHT_ADC_PIN); // 重置引脚状态
  ESP_ERROR_CHECK(ADC_init());
  ESP_LOGI(TAG, "ADC initialized successfully.");

}

uint16_t readLightValue()
{
  uint16_t voltage = light_level_read();
  // ADC 分辨率（默认 12 位）→ 最大 4095
  // if (guangzhaoValue < 100)
  //   guangzhaoValue = 0; // 黑暗环境
  // else if (guangzhaoValue > 4000)
  //   guangzhaoValue = 4095; // 极亮环境

  uint8_t light_percent = 100 - (voltage * 100) / 4095;

  ESP_LOGD("Light", "当前光敏电压值：%dmV 光照强度：%d%%", voltage, light_percent);

  return light_percent;
}

float readBatteryVoltage()
{
  return 3.3;
}

void app_io_deinit()
{
    ESP_ERROR_CHECK(adc_oneshot_del_unit(battery_handle));
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(cali_handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(cali_handle));
#endif
}