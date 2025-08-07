#include "audio_player_proxy.h"
#include <config.h>
#include "main_board_api.h"
#include <string.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "app_sdcard.h"
#include "audio_player.h"
#include <mp3dec.h>

static const char *TAG = "AudioPlayer";
static i2s_chan_handle_t i2s_tx_chan;
static i2s_std_config_t *i2s_std_cfg;
static QueueHandle_t event_queue;
static FILE *fp = nullptr;
static uint8_t bit_width = 32; // 默认位宽为32位
static uint32_t buffer32[MAX_NCHAN * MAX_NGRAN * MAX_NSAMP]; // 32-bit 缓冲区
static uint8_t s_volume_percent = 100;

/**
 * @link https://github.com/chmorgan/esp-box/blob/master/examples/mp3_demo/main/mp3_demo.c
 */
static void audio_player_callback(audio_player_cb_ctx_t *ctx){
    ESP_LOGI(TAG, "Audio player callback event: %s", event_to_string(ctx->audio_event));
    // wake up the test so it can continue to the next step
    // TEST_ASSERT_EQUAL(xQueueSend(event_queue, &(ctx->audio_event), 0), pdPASS);
}
static esp_err_t audio_mute_function(AUDIO_PLAYER_MUTE_SETTING setting) {
    ESP_LOGI(TAG, "mute setting %d", setting);
    return ESP_OK;
}

// 将 16-bit PCM 数据扩展为 32-bit mono 数据（低 16bit 有效）
static inline void convert_to_32bit_mono(const int16_t *src, uint32_t *dst, size_t samples) {
    for (size_t i = 0; i < samples; i++) {
        dst[i] = (uint32_t)src[i];
    }
}

static esp_err_t bsp_i2s_write(void * audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    // ret = i2s_channel_write(i2s_tx_chan, (char *)audio_buffer, len, bytes_written, timeout_ms);
    if (bit_width == 16) {
        // 计算样本数量（len 是字节数）
        int16_t *in = (int16_t *)audio_buffer;
        size_t sample_count = len / sizeof(int16_t);
        float volume_scale = s_volume_percent / 100.0f;
        memset(buffer32, 0, sizeof(buffer32)); // 清空 32-bit 缓冲区
        // 转换：16-bit PCM → 32-bit 左对齐
        for (size_t i = 0; i < sample_count; ++i) {
            int32_t scaled = in[i] * volume_scale;
            if (scaled > INT16_MAX) scaled = INT16_MAX;
            else if (scaled < INT16_MIN) scaled = INT16_MIN;
            buffer32[i] = scaled << 16;
        }
        // convert_to_32bit_mono(in, buffer32, len);

        ret = i2s_channel_write(i2s_tx_chan, buffer32, sample_count * sizeof(int32_t), bytes_written, timeout_ms);
        *bytes_written /= 2;  // 折算回原始 int16_t 字节数
    }
    else if (bit_width == 32) {
        // 已是 32-bit，直接写入
        ret = i2s_channel_write(i2s_tx_chan, audio_buffer, len, bytes_written, timeout_ms);
    }
    else {
        // 暂不支持其他位宽
        ret = ESP_ERR_INVALID_ARG;
    }
    return ret;
}

static esp_err_t bsp_i2s_reconfig_clk(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    esp_err_t ret = ESP_OK;
    bit_width = (uint8_t)bits_cfg;
    // i2s_std_config_t std_cfg = {
    //     .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(rate),
    //     .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t)bits_cfg, (i2s_slot_mode_t)ch),
    //     .gpio_cfg = BSP_I2S_GPIO_CFG,
    // };
    ret |= i2s_channel_disable(i2s_tx_chan);
    i2s_std_cfg->clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(rate);
    i2s_std_cfg->slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, (i2s_slot_mode_t)ch);
    ret |= i2s_channel_reconfig_std_clock(i2s_tx_chan, &i2s_std_cfg->clk_cfg);
    ret |= i2s_channel_reconfig_std_slot(i2s_tx_chan, &i2s_std_cfg->slot_cfg);
    ret |= i2s_channel_enable(i2s_tx_chan);
    return ret;
}

AudioPlayer::AudioPlayer() {

}

AudioPlayer::~AudioPlayer() {
    end(); // 确保资源被释放
}

bool AudioPlayer::begin() {
    if(m_initialized) return true;
    i2s_tx_chan = main_api->get_i2s_tx_handle();
    i2s_std_cfg = main_api->get_i2s_std_config();
    
    audio_player_config_t config = { .mute_fn = audio_mute_function,
                                     .clk_set_fn = bsp_i2s_reconfig_clk,
                                     .write_fn = bsp_i2s_write,
                                     .priority = 1,
                                     .coreID = 1 };
    ESP_ERROR_CHECK(audio_player_new(config));

    event_queue = xQueueCreate(1, sizeof(audio_player_callback_event_t));
    if (event_queue == NULL) {
        ESP_LOGE("AudioPlayer", "Failed to create event queue");
        return false;
    }

    ESP_ERROR_CHECK(audio_player_callback_register(audio_player_callback, NULL));
    ESP_LOGI("AudioPlayer", "AudioPlayer initialized");

    m_initialized = true;
    return true;
}

void AudioPlayer::loop() {
    if (m_initialized) {
        // m_audio->loop();
    }
}

void AudioPlayer::end() {
    if(!m_initialized) return;
    
    audio_player_delete();
    
    m_initialized = false;
}

bool AudioPlayer::isInitialized() const {
    return m_initialized;
}

bool AudioPlayer::isPlaying() {
    if (!isInitialized()) {
        return false;
    }
    audio_player_state_t state = audio_player_get_state();
    return state == AUDIO_PLAYER_STATE_PLAYING;
}

uint16_t AudioPlayer::getDuration()
{
    return audio_player_get_total_duration();
}

uint16_t AudioPlayer::getPosition()
{
    return audio_player_get_current_position();
}

void AudioPlayer::playFile(const char *filePath, uint16_t position)
{
    std::string file = std::string(MOUNT_POINT) + filePath;
    fp = fopen(file.c_str(), "rb");
    ESP_ERROR_CHECK(audio_player_play(fp, position));
}

void AudioPlayer::stop()
{
    ESP_ERROR_CHECK(audio_player_stop());
}

void AudioPlayer::pause()
{
    ESP_ERROR_CHECK(audio_player_pause());
}

void AudioPlayer::resume()
{
    ESP_ERROR_CHECK(audio_player_resume());
}

/**
 * @param position 以秒为单位
 */
void AudioPlayer::seek(uint16_t position)
{
    ESP_ERROR_CHECK(audio_player_seek(position));
}

void AudioPlayer::setVolume(uint8_t volume_percent)
{
    if (volume_percent > 100) volume_percent = 100;
    s_volume_percent = volume_percent;
}

uint8_t AudioPlayer::getVolume() const
{
    return s_volume_percent;
}
