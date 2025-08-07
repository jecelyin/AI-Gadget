#include "player.h"
#include "audio_player_proxy.h"
#include "app_sdcard.h"
#include <esp_log.h>
#include "prefs.h"
#include <config.h>
#include <data_struct.h>
#include <uart_cmd.h>
#include "FS.h"


#define TAG "Player"
#define AUDIO_ROOT "/Audio"
String last_path;
String last_audio;
uint16_t last_position;
uint16_t last_duration;
AudioPlayer player;
static fs::FS SD = fs::FS();
bool is_pause = false;

static bool is_player_available() { return is_sdcard_mounted(); }

static void save_audio_position() {
  if (player.isPlaying()) {
    pref_set_player_last_position(player.getDuration());
  }
}

void player_init() {
  // pinMode(AMP_SD_PIN, OUTPUT);
  // digitalWrite(AMP_SD_PIN, HIGH);

  // pinMode(AMP_GAIN_PIN, OUTPUT);
  // digitalWrite(AMP_GAIN_PIN, HIGH);

  // player.getAudio()->setVolume(21); // default 0...21
  // player.begin();
}

void player_loop() {
  player.loop();
}

void player_stop() {
  save_audio_position();
  player.end();
}


static void send_file_list(String &path, uint16_t page) {
  BinaryPacker s;
  String full_path = AUDIO_ROOT;
  full_path += path;
  s.writeString(full_path);
  s.writeUint16(page);

  listFiles(full_path.c_str(), s, path.length() > 1, page);
  // log_d("send file list, s.available = %d, s.size = %d, s.offset = %d", s.available(), s.size(), s.getOffset());
  SEND_BUFFER(s, CMD_PLAYER_FILES_RESP);
}

void handle_player_get_last(uint8_t *src, uint16_t length) {
  if (!is_player_available())
    return;
  last_path = pref_get_player_last_path();
  last_audio = pref_get_player_last_audio();
  last_position = pref_get_player_last_position();
  last_duration = pref_get_player_last_duration();

  BinaryPacker s;
  s.writeString(last_path);
  s.writeString(last_audio);
  s.writeUint16(last_position);
  s.writeUint16(last_duration);
  SEND_BUFFER(s, CMD_PLAYER_LAST_RESP);

  send_file_list(last_path, 1);
}

void handle_player_load(uint8_t *src, uint16_t length) {
  ESP_LOGI(TAG, "handle_player_load: src=%p, length=%d", src, length);
  if (!is_player_available()){
    ESP_LOGE(TAG, "player load: inavailable!");
    return;
  }
  if (last_audio.isEmpty()) {
    ESP_LOGE(TAG, "player load: last audio empty!");
    return;
  }
  String file = AUDIO_ROOT + last_path + "/" + last_audio;
  player.begin();
  player.playFile(file.c_str(), last_position);
}

void handle_player_load_and_play(uint8_t *src, uint16_t length) {
  ESP_LOGI(TAG, "handle_player_load_and_play: src=%p, length=%d", src, length);
  if (!is_player_available()) {
    ESP_LOGE(TAG, "player load and play: inavailable!");
    return;
  }
  if (is_pause) {
    ESP_LOGD(TAG, "player load and play: resume from pause");
    player.resume();
    is_pause = false;
    return;
  }
  PlayerPlayStruct data;
  decode_data(data, src, length);
  ESP_LOGD(TAG, "player load and play: name=%s, last_path=%s", data.name, last_path.c_str());
  char file[255] = {0};
  sprintf(file, "%s%s/%s", AUDIO_ROOT, last_path.c_str(), data.name);
  if (!file_exists(file)) {
    ESP_LOGE(TAG, "player load and play: file not exists: %s", file);
    return;
  }
  ESP_LOGD(TAG, "player load and play: connecttoFS: %s", file);
  player.begin();
  player.playFile(file);
  last_audio = data.name;
  pref_set_player_last_path(last_path.c_str());
  pref_set_player_last_audio(data.name);
  pref_set_player_last_duration(player.getPosition());
}

void handle_player_play(uint8_t *src, uint16_t length) {
  ESP_LOGI(TAG, "handle_player_play: src=%p, length=%d", src, length);
  if (!is_player_available())
    return;
  if (!player.isInitialized()) {
    ESP_LOGI(TAG, "player play: not initialized!");
    player.begin();
  }
  ESP_LOGD(TAG, "player: play");
  // player.stop();
  if (is_pause) {
    ESP_LOGD(TAG, "player load and play: resume from pause");
    player.resume();
    is_pause = false;
    return;
  }
  String last_path = pref_get_player_last_path();
  if (last_path.isEmpty()) {
    return;
  }
  String name = pref_get_player_last_audio();
  char file[255] = {0};
  sprintf(file, "%s%s/%s", AUDIO_ROOT, last_path.c_str(), name.c_str());
  if (!file_exists(file)) {
    ESP_LOGE(TAG, "player load and play: file not exists: %s", file);
    return;
  }
  uint16_t pos = pref_get_player_last_position();
  ESP_LOGD(TAG, "player load and play: connecttoFS: %s, pos: %d", file, pos);
  player.begin();
  player.playFile(file, pos);
}

void handle_player_pause(uint8_t *src, uint16_t length) {
  ESP_LOGI(TAG, "handle_player_pause: src=%p, length=%d", src, length);
  if (!is_player_available() || !player.isInitialized())
    return;
  ESP_LOGD(TAG, "player: pause");
  save_audio_position();
  player.pause();
  // player.end();
  is_pause = true;
}

void handle_player_seek(uint8_t *src, uint16_t length) {
  ESP_LOGI(TAG, "handle_player_seek: src=%p, length=%d", src, length);
  if (!is_player_available() || !player.isInitialized())
    return;
  uint32_t pos = 0;
  UNPACK_UINT32(pos, src);
  // Serial.print("seek to position: ");
  // Serial.println(pos);
  player.seek(pos);
  save_audio_position();
}

void handle_player_open_dir(uint8_t *src, uint16_t length) {
  ESP_LOGI(TAG, "handle_player_open_dir: src=%p, length=%d", src, length);
  if (!is_player_available())
    return;
  String dir(src, length);
  if (dir == "..") {
    dir = getParentPath(last_path);
    if (dir == "/") {
      dir = "";
    }
  } else {
    if (last_path.endsWith("/")) {
      dir = last_path + dir;
    } else {
      dir = last_path + "/" + dir;
    }
  }
  last_path = dir;

  send_file_list(dir, 1);
}

void handle_player_file_list(uint8_t *src, uint16_t length) {
  PlayerPageDataStruct data;
  decode_data(data, src, length);
  send_file_list(last_path, data.page);
}
