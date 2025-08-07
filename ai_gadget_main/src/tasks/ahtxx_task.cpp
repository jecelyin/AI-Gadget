#include "ahtxx_task.h"

#include <data_struct.h>
#include <uart_cmd.h>
#include <cmd.h>
#include <ahtxx.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main_board_api.h"

static const char* TAG = "AHT4x";

// I2C configuration (should match your project's config)
// #define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL
// #define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define SHT40_I2C_ADDR_44           0x44

namespace Ahtxx_Task {
    // SensirionI2cSht4x sht4x;
    static char errorMessage[64];
    static int16_t error;
    #ifdef NO_ERROR
    #undef NO_ERROR
    #endif
    #define NO_ERROR 0

    // initialize i2c device configuration
    //ahtxx_config_t dev_cfg          = I2C_AHT10_CONFIG_DEFAULT;
    ahtxx_config_t dev_cfg          = I2C_AHT20_CONFIG_DEFAULT;
    //ahtxx_config_t dev_cfg          = I2C_AHT21_CONFIG_DEFAULT;
    //ahtxx_config_t dev_cfg          = I2C_AHT25_CONFIG_DEFAULT;
    //ahtxx_config_t dev_cfg          = I2C_AHT30_CONFIG_DEFAULT;
    ahtxx_handle_t dev_hdl;

    void setup() {
        ahtxx_init(i2c0_bus_hdl, &dev_cfg, &dev_hdl);
        if (dev_hdl == NULL) {
            ESP_LOGE(TAG, "ahtxx handle init failed");
            assert(dev_hdl);
        }
        
        // ESP_LOGI(TAG, "SerialNumber: %lu", serialNumber);
    }

    void updateTemperatureAndHumidity(float *ret_temperature, float *ret_humidity) {        
        Sht4xStruct data = {0};
        vTaskDelay(pdMS_TO_TICKS(20));  // Short delay

        ESP_LOGI(TAG, "######################## AHTXX - START #########################");
        ahtxx_reset(dev_hdl);
        vTaskDelay(pdMS_TO_TICKS(20));  // Short delay after reset
        // handle sensor
        float temperature, humidity;
        esp_err_t result = ahtxx_get_measurement(dev_hdl, &temperature, &humidity);
        if(result != ESP_OK) {
            ESP_LOGE(TAG, "ahtxx device read failed (%s)", esp_err_to_name(result));
        } else {
            data.temperature = temperature;
            *ret_temperature = temperature;
            data.humidity = humidity;
            *ret_humidity = humidity;
            ESP_LOGI(TAG, "air temperature:     %.2f °C", temperature);
            ESP_LOGI(TAG, "relative humidity:   %.2f %c", humidity, '%');
        }
        // ahtxx_delete( dev_hdl );
        ESP_LOGI(TAG, "######################## AHTXX - END ###########################");


        ESP_LOGI(TAG, "Temperature: %.2f°C\tHumidity: %.2f%%",
                data.temperature, data.humidity);

        SEND_COMMAND(CMD_TEMP_HUMIDITY, data);
    }

    // void task_main(void* pvParameters) {
    //     setup();
        
    //     while (1) {
    //         updateTemperatureAndHumidity();
    //         vTaskDelay(pdMS_TO_TICKS(2000));  // Update every 2 seconds
    //     }
    // }
}