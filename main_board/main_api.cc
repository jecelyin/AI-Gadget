#include "main_api.h"
#include <esp_log.h>
#include "application.h"

static const char* TAG = "MainApi";


void voice_start() {
    ESP_LOGI(TAG, "voice, start listening");
    Application::GetInstance().StartListening();
}

void voice_end() {
    ESP_LOGI(TAG, "voice, stop listening");
    Application::GetInstance().StopListening();
}
