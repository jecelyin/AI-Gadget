#include "SC7A20.h"
#include "esp_log.h"
#include <driver/i2c_master.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "SC7A20";

// I2C configuration (should match your project's config)
// #define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL
// #define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
// #define SC7A20_I2C_ADDR             0x19  // Default I2C address

// Register definitions (unchanged from original)
#define WHO_AM_I_REG       0x0F
#define CTRL_REG1          0x20
#define CTRL_REG2          0x21
#define CTRL_REG3          0x22
#define CTRL_REG4          0x23
#define CTRL_REG5          0x24
#define CTRL_REG6          0x25
#define ADDR_STATUS_REG    0x27
#define OUT_X_L_REG        0x28
#define OUT_X_H_REG        0x29
#define OUT_Y_L_REG        0x2A
#define OUT_Y_H_REG        0x2B
#define OUT_Z_L_REG        0x2C
#define OUT_Z_H_REG        0x2D
#define INT1_CFG           0x30
#define INT1_SRC           0x31
#define INT1_THS           0x32
#define INT1_DURATION      0x33
#define CLICK_CFG          0x38
#define CLICK_SRC          0x39
#define CLICK_THS          0x3A
#define TIME_LIMIT         0x3B
#define TIME_LATENCY       0x3C
#define TIME_WINDOW        0x3D
#define TEMP_CFG           0x1F
#define CHIP_ID            0x11

static float g_measure_range = 2.0f;  // Default range ±2g
extern i2c_master_bus_handle_t  i2c0_bus_hdl;
static i2c_master_dev_handle_t dev_handle = NULL;

static esp_err_t i2c_master_init() {
    // Add device (SC7A20 sensor)
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SC7A20_I2C_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t ret = i2c_master_bus_add_device(i2c0_bus_hdl, &dev_cfg, &dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE("I2C", "Failed to add device: %s", esp_err_to_name(ret));
        return ret;
    }
    return ESP_OK;
}

static esp_err_t i2c_write(uint8_t reg, uint8_t value) {
    uint8_t write_buf[2] = {reg, value};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
}

static esp_err_t i2c_read(uint8_t reg, uint8_t *data, uint8_t len) {
    // Write register address first
    esp_err_t ret = i2c_master_transmit(dev_handle, &reg, 1, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        return ret;
    }
    // Read data
    return i2c_master_receive(dev_handle, data, len, pdMS_TO_TICKS(1000));
}

static int16_t convert_12bit_complement(uint8_t msb, uint8_t lsb) {
    int16_t temp = (msb << 8) | lsb;
    temp = temp >> 4;  // Keep only top 12 bits
    if (temp & 0x0800) { // If negative
        temp |= 0xF000;  // Sign extend
    }
    return temp;
}

void SC7A20_Init(SC7A20_Mode mode) {
    uint8_t temp;
    
    // Initialize I2C
    ESP_ERROR_CHECK(i2c_master_init());

    // Check device ID
    ESP_ERROR_CHECK(i2c_read(WHO_AM_I_REG, &temp, 1));
    if (temp != CHIP_ID) {
        ESP_LOGE(TAG, "SC7A20 Not Found! (ID: 0x%02X)", temp);
        return;
    }

    if (mode == SC7A20_MODE_VIBRATION) {
        ESP_LOGI(TAG, "Initializing Vibration Mode...");
        SC7A20_SetCTRL_REG1(ODR_50HZ, 1, 0x07);
        ESP_ERROR_CHECK(i2c_write(CTRL_REG2, 0x01));
        ESP_ERROR_CHECK(i2c_write(CTRL_REG3, INT1_AOI1_ON_INT1));
        ESP_ERROR_CHECK(i2c_write(CTRL_REG4, 0x88));
        ESP_ERROR_CHECK(i2c_write(TEMP_CFG, 0x01));
        ESP_ERROR_CHECK(i2c_write(INT1_CFG, 0x2A));
        SC7A20_SetInterruptThreshold(128.0, RANGE_4G);
        ESP_ERROR_CHECK(i2c_write(INT1_DURATION, 0x01));
    }
    else if (mode == SC7A20_MODE_SINGLE_CLICK) {
        ESP_LOGI(TAG, "Initializing Single Click Mode...");
        ESP_ERROR_CHECK(i2c_write(CTRL_REG1, 0x47));
        ESP_ERROR_CHECK(i2c_write(CTRL_REG4, 0x88));
        ESP_ERROR_CHECK(i2c_write(CTRL_REG2, 0x31));
        ESP_ERROR_CHECK(i2c_write(CTRL_REG3, 0x40));
        ESP_ERROR_CHECK(i2c_write(CTRL_REG6, 0x00));
        ESP_ERROR_CHECK(i2c_write(INT1_CFG, 0x2A));
        ESP_ERROR_CHECK(i2c_write(INT1_THS, 0x05));
        ESP_ERROR_CHECK(i2c_write(INT1_DURATION, 0x00));
    }

    ESP_LOGI(TAG, "SC7A20 Initialized");
}

bool SC7A20_NewDataReady() {
    uint8_t status;
    if (i2c_read(ADDR_STATUS_REG, &status, 1) != ESP_OK) {
        return false;
    }
    return status & 0x08;  // Check ZYXDA bit
}

void SC7A20_ReadAccelerationRaw(int16_t *acc_x, int16_t *acc_y, int16_t *acc_z) {
    uint8_t data[6];
    if (i2c_read(OUT_X_L_REG, data, 6) == ESP_OK) {
        *acc_x = convert_12bit_complement(data[1], data[0]);
        *acc_y = convert_12bit_complement(data[3], data[2]);
        *acc_z = convert_12bit_complement(data[5], data[4]);
    }
}

void SC7A20_ReadAcceleration(float *acc_x, float *acc_y, float *acc_z) {
    int16_t acc_x_raw, acc_y_raw, acc_z_raw;
    SC7A20_ReadAccelerationRaw(&acc_x_raw, &acc_y_raw, &acc_z_raw);
    
    *acc_x = acc_x_raw * g_measure_range / 2048.0f;
    *acc_y = acc_y_raw * g_measure_range / 2048.0f;
    *acc_z = acc_z_raw * g_measure_range / 2048.0f;
}

void SC7A20_SetInterruptThreshold(float threshold_mg, uint8_t range) {
    uint8_t ths = 0;
    float lsb_value = 0;

    switch (range) {
        case RANGE_2G: lsb_value = 16.0; break;
        case RANGE_4G: lsb_value = 32.0; break;
        case RANGE_8G: lsb_value = 64.0; break;
        case RANGE_16G: lsb_value = 128.0; break;
        default:
            ESP_LOGE(TAG, "Invalid range!");
            return;
    }

    ths = (uint8_t)(threshold_mg / lsb_value);
    if (ths > 127) ths = 127;

    ESP_ERROR_CHECK(i2c_write(INT1_THS, ths));
    ESP_LOGD(TAG, "Interrupt threshold set to %.1f mg", threshold_mg);
}

void SC7A20_SetInterruptDuration(uint16_t duration_ms, uint16_t odr) {
    float odr_period = 0;

    switch (odr) {
        case 1:    odr_period = 1000.0; break; // 1Hz
        case 10:   odr_period = 100.0;  break; // 10Hz
        case 25:   odr_period = 40.0;   break; // 25Hz
        case 50:   odr_period = 20.0;   break; // 50Hz
        case 100:  odr_period = 10.0;   break; // 100Hz
        case 200:  odr_period = 5.0;    break; // 200Hz
        case 400:  odr_period = 2.5;    break; // 400Hz
        default:
            ESP_LOGE(TAG, "Invalid ODR!");
            return;
    }

    uint8_t duration = (uint8_t)(duration_ms / odr_period);
    if (duration > 127) duration = 127;

    ESP_ERROR_CHECK(i2c_write(INT1_DURATION, duration));
    ESP_LOGD(TAG, "Interrupt duration set to %d ms", duration_ms);
}

void SC7A20_SetCTRL_REG1(uint8_t odr, uint8_t low_power, uint8_t enable_axes) {
    if (enable_axes == 0) {
        ESP_LOGE(TAG, "Error: At least one axis must be enabled!");
        return;
    }

    uint8_t ctrl_reg1_value = odr | (low_power ? 0x08 : 0x00) | (enable_axes & 0x07);
    ESP_ERROR_CHECK(i2c_write(CTRL_REG1, ctrl_reg1_value));
    ESP_LOGD(TAG, "CTRL_REG1 configured: 0x%02X", ctrl_reg1_value);
}