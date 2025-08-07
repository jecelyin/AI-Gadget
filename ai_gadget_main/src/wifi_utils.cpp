#include "wifi_utils.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"

static const char* TAG = "wifi_utils";

namespace wifi_utils {
    static bool wifi_connected = false;

    static void wifi_event_handler(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data) {
        if (event_base == WIFI_EVENT) {
            if (event_id == WIFI_EVENT_STA_CONNECTED) {
                wifi_connected = true;
                ESP_LOGI(TAG, "Connected to AP");
            } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
                wifi_connected = false;
                ESP_LOGI(TAG, "Disconnected from AP");
            }
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            wifi_connected = true;
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        }
    }

    bool init() {
        ESP_LOGI(TAG, "Initializing WiFi event handlers");
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                                        &wifi_event_handler, NULL, NULL));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                                        &wifi_event_handler, NULL, NULL));
        return true;
    }
    bool isWifiConnected(void) {
        if (!wifi_connected) {
            wifi_ap_record_t ap_info;
            esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
            if (ret == ESP_OK) {
                wifi_connected = true;
            }
        }
        return wifi_connected;
    }


}  // namespace utils