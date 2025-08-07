#include "stream_protocol.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

#define TAG "STREAM_PROTOCOL"

// 发送命令帧
void stream_send_cmd(uart_port_t uart_num, uint8_t cmd, const uint8_t *data, uint16_t length) {
    // 计算校验和
    uint16_t checksum = cmd ^ length;
    for (int i = 0; i < length; i++) {
        checksum ^= data[i];
    }

    // 创建发送缓冲区
    uint16_t buff_size = 6 + length; // Header(1) + CMD(1) + Length(2) + Data(length) + Checksum(2)
    uint8_t *buffer = (uint8_t *)malloc(buff_size);
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate send buffer");
        return;
    }

    // 填充缓冲区
    buffer[0] = 0xAA;               // Header
    buffer[1] = cmd;                // CMD
    buffer[2] = length & 0xFF;      // Data length 低字节
    buffer[3] = (length >> 8) & 0xFF; // Data length 高字节

    // 复制数据到缓冲区
    memcpy(&buffer[4], data, length);

    // 将校验和写入缓冲区
    buffer[4 + length] = checksum & 0xFF;       // 校验和低字节
    buffer[5 + length] = (checksum >> 8) & 0xFF; // 校验和高字节

    // 发送数据
    int sent = uart_write_bytes(uart_num, (const char *)buffer, buff_size);
    if (sent != buff_size) {
        ESP_LOGE(TAG, "UART send failed, expected %d, sent %d", buff_size, sent);
    }
    
    // 等待发送完成
    uart_wait_tx_done(uart_num, pdMS_TO_TICKS(1000));
    
    free(buffer);
}

// 确保读取到帧头
static bool ensure_header(uart_port_t uart_num, char *invalid_msg, size_t invalid_msg_size) {
    uint8_t head;
    int bytes_read = 0;
    
    while (1) {
        bytes_read = uart_read_bytes(uart_num, &head, 1, pdMS_TO_TICKS(100));
        if (bytes_read == 1) {
            if (head == 0xAA) {
                return true; // 找到头部
            } else {
                // 记录无效数据
                if (invalid_msg && invalid_msg_size > 3) { // 确保有足够空间
                    snprintf(invalid_msg, invalid_msg_size, "%02X", head);
                }
            }
        } else if (bytes_read == -1) {
            ESP_LOGE(TAG, "UART read error");
            return false;
        } else {
            // 超时
            return false;
        }
    }
}

// 读取命令帧
bool stream_read_cmd(uart_port_t uart_num, uint8_t *cmd, uint8_t **data, uint16_t *length, char *error, size_t error_size) {
    char invalid_msg[64] = {0};
    
    // 检查 Header
    if (!ensure_header(uart_num, invalid_msg, sizeof(invalid_msg))) {
        if (strlen(invalid_msg) > 0) {
            snprintf(error, error_size, "Invalid header data: %s", invalid_msg);
        }
        return false;
    }

    // 读取命令和长度
    uint8_t header[3]; // cmd + length(2 bytes)
    int bytes_read = uart_read_bytes(uart_num, header, 3, pdMS_TO_TICKS(100));
    if (bytes_read != 3) {
        snprintf(error, error_size, "Read CMD timeout, got %d bytes", bytes_read);
        return false;
    }

    *cmd = header[0];
    *length = header[1] | (header[2] << 8);
    ESP_LOGI(TAG, "Received CMD: %02X, Length: %d", *cmd, *length);
    if (*length > 4096) { // 假设最大长度为4096
        snprintf(error, error_size, "Data length too large: %d", *length);
        return false;
    }
    if (*length > 0){
        // 分配数据缓冲区
        *data = (uint8_t *)malloc(*length);
        if (*data == NULL) {
            snprintf(error, error_size, "Memory allocation failed for %d bytes", *length);
            return false;
        }

        // 读取数据
        bytes_read = uart_read_bytes(uart_num, *data, *length, pdMS_TO_TICKS(200));
        if (bytes_read != *length) {
            free(*data);
            snprintf(error, error_size, "Read data timeout, expected %d got %d", *length, bytes_read);
            return false;
        }
    }
    // 读取校验和
    uint8_t checksum_buf[2];
    bytes_read = uart_read_bytes(uart_num, checksum_buf, 2, pdMS_TO_TICKS(100));
    if (bytes_read != 2) {
        free(*data);
        snprintf(error, error_size, "Read checksum timeout");
        return false;
    }

    // 计算校验和
    uint16_t received_checksum = checksum_buf[0] | (checksum_buf[1] << 8);
    uint16_t checksum = *cmd ^ *length;
    for (int i = 0; i < *length; i++) {
        checksum ^= (*data)[i];
    }

    // 验证校验和
    if (checksum != received_checksum) {
        free(*data);
        snprintf(error, error_size, "Checksum error, expected %04X got %04X", checksum, received_checksum);
        return false;
    }

    return true;
}