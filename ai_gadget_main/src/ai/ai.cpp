#include "ai.h"
#include <config.h>
#include "uart_cmd.h"
#include <player.h>
#include <nvs_flash.h>
#include "esp_log.h"
#include "main_board_api.h"
// static AudioManager& audio_manager = AudioManager::GetInstance();


bool keep_listening_ = false;
const char* TAG = "AI";


void ai_init() {

}

void ai_loop() {

}

void handle_voice_start(uint8_t *src, uint16_t length) {
  ESP_LOGI(TAG, "handle_voice_start");
  // digitalWrite(MIC_EN_PIN, HIGH);
  player_stop();
  
  main_api->voice_start();
}

void handle_voice_end(uint8_t *src, uint16_t length) {
  ESP_LOGI(TAG, "handle_voice_end");

  // digitalWrite(MIC_EN_PIN, LOW);
  // Application::GetInstance().StopListening();
  main_api->voice_end();
}

void handle_voice_cancel(uint8_t *src, uint16_t length) {
  ESP_LOGI(TAG, "handle_voice_cancel");
  // digitalWrite(MIC_EN_PIN, LOW);
  // Application::GetInstance().StopListening();
  main_api->voice_end();
}
