#include "sgp4x_task.h"
#include "esp_log.h"
#include <driver/i2c_master.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <data_struct.h>
#include "uart_cmd.h"
#include "cmd.h"
#include "main_board_api.h"
#include <sgp4x.h>
#include <sensirion_gas_index_algorithm.h>

static const char* TAG = "SGP4x";

// I2C configuration
#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                  /*!< I2C master i2c port number */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */

namespace SGP4x_Task {
    sgp4x_config_t dev_cfg = I2C_SGP41_CONFIG_DEFAULT;
    sgp4x_handle_t dev_hdl;
    bool dev_self_tested  = false;
    bool dev_conditioned  = false;
    GasIndexAlgorithmParams voc_params = {0};
    GasIndexAlgorithmParams nox_params = {0};


    void setup() {
        // initialize i2c device configuration

        
        /* initialize gas index parameters */
        GasIndexAlgorithm_init(&voc_params, GasIndexAlgorithm_ALGORITHM_TYPE_VOC);
        GasIndexAlgorithm_init(&nox_params, GasIndexAlgorithm_ALGORITHM_TYPE_NOX);
        //
        // init device
        sgp4x_init(i2c0_bus_hdl, &dev_cfg, &dev_hdl);
        if (dev_hdl == NULL) {
            ESP_LOGE(TAG, "sgp4x handle init failed");
            assert(dev_hdl);
        }
    }

    void updateVOCandNOX(float temperature, float humidity) {
        ESP_LOGI(TAG, "Updating VOC and NOX...");
        // uint16_t defaultRh = 0x8000;  // 50% RH
        // uint16_t defaultT = 0x6666;   // 25°C
        SGP41Struct data = {0};
        ESP_LOGI(TAG, "######################## SGP4X - START #########################");
        
        /* handle sensor */
        if(dev_self_tested == false) {
            sgp4x_self_test_result_t self_test_result;
            esp_err_t result = sgp4x_execute_self_test(dev_hdl, &self_test_result);
            if(result != ESP_OK) {
                ESP_LOGE(TAG, "sgp4x device self-test failed (%s)", esp_err_to_name(result));
            } else {
                ESP_LOGI(TAG, "VOC Pixel:   %d", self_test_result.pixels.voc_pixel_failed);
                ESP_LOGI(TAG, "NOX Pixel:   %d", self_test_result.pixels.nox_pixel_failed);
            }
            dev_self_tested = true;
        }
        /* conditioning validation */
        if(dev_conditioned == false) {
            for(int i = 0; i < 10; i++) {
                uint16_t sraw_voc; 
                esp_err_t result = sgp4x_execute_conditioning(dev_hdl, &sraw_voc);
                if(result != ESP_OK) {
                    ESP_LOGE(TAG, "sgp4x device conditioning failed (%s)", esp_err_to_name(result));
                } else {
                    ESP_LOGI(TAG, "SRAW VOC: %u", sraw_voc);
                }
                vTaskDelay(pdMS_TO_TICKS(1000)); // 1-second * 10 iterations = 10-seconds
            }
            dev_conditioned = true;
        } else {
            /* measure signals and process gas algorithm */
            uint16_t sraw_voc; uint16_t sraw_nox;
            int32_t voc_index; int32_t nox_index;
            // esp_err_t result = sgp4x_measure_signals(dev_hdl, &sraw_voc, &sraw_nox);
            esp_err_t result = sgp4x_measure_compensated_signals(dev_hdl, temperature, humidity, &sraw_voc, &sraw_nox);
            if(result != ESP_OK) {
                ESP_LOGE(TAG, "sgp4x device conditioning failed (%s)", esp_err_to_name(result));
            } else {
                GasIndexAlgorithm_process(&voc_params, sraw_voc, &voc_index);
                GasIndexAlgorithm_process(&nox_params, sraw_nox, &nox_index);

                ESP_LOGI(TAG, "SRAW VOC: %u | VOC Index: %li", sraw_voc, voc_index);
                ESP_LOGI(TAG, "SRAW NOX: %u | NOX Index: %li", sraw_nox, nox_index);
                data.voc = voc_index;
                data.nox = nox_index;
                SEND_COMMAND(CMD_VOC_NOX, data);
            }
        }
        
        //
        ESP_LOGI(TAG, "######################## SGP4X - END ###########################");
    }

}