#include "wifi_board.h"
#include "audio_codecs/no_audio_codec.h"
#include "display/oled_display.h"
#include "system_reset.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "iot/thing_manager.h"
#include "led/single_led.h"
#include "assets/lang_config.h"
#include "driver/touch_pad.h"

#include <wifi_station.h>
#include <esp_log.h>
#include <driver/i2c_master.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#ifdef SH1106
#include <esp_lcd_panel_sh1106.h>
#endif

#define TAG "CompactWifiBoard"

LV_FONT_DECLARE(font_puhui_14_1);
LV_FONT_DECLARE(font_awesome_14_1);

#define GPIO_BIT_MASK ((1ULL << AMP_GAIN_PIN) | (1ULL << AMP_SD_PIN) | (1ULL << SD_EN_PIN) | \
                       (1ULL << MIC_EN_PIN) | (1ULL << VCC_EN_PIN) | (1ULL << LCD_EN_PIN))
#define TOUCH_PAD_GPIO4_CHANNEL TOUCH_PAD_NUM4 // GPIO4 对应 TOUCH_PAD_NUM3（ESP32-S3）
#define TOUCH_BUTTON_WATERPROOF_ENABLE 1
#define TOUCH_BUTTON_DENOISE_ENABLE 1
#define TOUCH_CHANGE_CONFIG 0

i2c_master_bus_handle_t i2c0_bus_hdl;


class CompactWifiBoard : public WifiBoard
{
private:
    // i2c_master_bus_handle_t display_i2c_bus_;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    Display *display_ = nullptr;
    Button boot_button_;
    Button touch_button_;
    // Button volume_up_button_;
    // Button volume_down_button_;

    void ScanI2cDevices()
    {

        ESP_LOGI(TAG, "Scanning I2C bus...");
        printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
        printf("00:         ");

        // 3. 扫描所有有效地址 (0x08~0x77)
        for (uint8_t addr = 0x08; addr < 0x78; addr++) {
            if (addr % 16 == 0) {
                printf("\n%02X:", addr & 0xF0);
            }

            // 4. 尝试与设备通信
            i2c_device_config_t dev_cfg = {
                .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                .device_address = addr,
                .scl_speed_hz = 100000,
            };

            i2c_master_dev_handle_t dev_handle;
            esp_err_t ret = i2c_master_bus_add_device(i2c0_bus_hdl, &dev_cfg, &dev_handle);
            
            if (ret == ESP_OK) {
                printf(" %02X", addr);  // 发现设备
                i2c_master_bus_rm_device(dev_handle);
            } else {
                printf(" --");         // 无设备
            }
        }

        printf("\n\nScan completed.\n");
    }

    void InitializeDisplayI2c()
    {
        i2c_master_bus_config_t bus_config = {
            .i2c_port = (i2c_port_t)0,
            .sda_io_num = I2C_SDA_PIN,
            .scl_io_num = I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c0_bus_hdl));
        // ScanI2cDevices();

    }

    void InitializeSsd1306Display()
    {
        // SSD1306 config
        //         esp_lcd_panel_io_i2c_config_t io_config = {
        //             .dev_addr = 0x3C,
        //             .on_color_trans_done = nullptr,
        //             .user_ctx = nullptr,
        //             .control_phase_bytes = 1,
        //             .dc_bit_offset = 6,
        //             .lcd_cmd_bits = 8,
        //             .lcd_param_bits = 8,
        //             .flags = {
        //                 .dc_low_on_data = 0,
        //                 .disable_control_phase = 0,
        //             },
        //             .scl_speed_hz = 400 * 1000,
        //         };

        //         ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2(i2c0_bus_hdl, &io_config, &panel_io_));

        //         ESP_LOGI(TAG, "Install SSD1306 driver");
        //         esp_lcd_panel_dev_config_t panel_config = {};
        //         panel_config.reset_gpio_num = -1;
        //         panel_config.bits_per_pixel = 1;

        //         esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        //             .height = static_cast<uint8_t>(DISPLAY_HEIGHT),
        //         };
        //         panel_config.vendor_config = &ssd1306_config;

        // #ifdef SH1106
        //         ESP_ERROR_CHECK(esp_lcd_new_panel_sh1106(panel_io_, &panel_config, &panel_));
        // #else
        //         ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(panel_io_, &panel_config, &panel_));
        // #endif
        //         ESP_LOGI(TAG, "SSD1306 driver installed");

        //         // Reset the display
        //         ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
        //         if (esp_lcd_panel_init(panel_) != ESP_OK) {
        //             ESP_LOGE(TAG, "Failed to initialize display");
        display_ = new NoDisplay();
        //     return;
        // }

        // // Set the display to on
        // ESP_LOGI(TAG, "Turning display on");
        // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));

        // display_ = new OledDisplay(panel_io_, panel_, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y,
        //     {&font_puhui_14_1, &font_awesome_14_1});
    }

    static void touchsensor_filter_set(touch_filter_mode_t mode)
    {
        /* Filter function */
        touch_filter_config_t filter_info = {
            .mode = mode,           // Test jitter and filter 1/4.
            .debounce_cnt = 1,      // 1 time count.
            .noise_thr = 0,         // 50%
            .jitter_step = 4,       // use for jitter mode.
            .smh_lvl = TOUCH_PAD_SMOOTH_IIR_2,
        };
        touch_pad_filter_set_config(&filter_info);
        touch_pad_filter_enable();
        ESP_LOGI(TAG, "touch pad filter init");
    }
    // 触摸中断回调函数
    /*
    Handle an interrupt triggered when a pad is touched.
    Recognize what pad has been touched and save it in a table.
    */
    static void touchsensor_interrupt_cb(void *arg)
    {
        // int task_awoken = pdFALSE;
        // touch_event_t evt;
        static uint8_t guard_mode_flag = 0;
        uint32_t intr_mask = touch_pad_read_intr_status_mask();
        uint32_t pad_status = touch_pad_get_status();
        uint32_t pad_num = touch_pad_get_current_meas_channel();

        // xQueueSendFromISR(que_touch, &evt, &task_awoken);
        // if (task_awoken == pdTRUE) {
        //     portYIELD_FROM_ISR();
        // }
        if (intr_mask & TOUCH_PAD_INTR_MASK_ACTIVE) {
            /* if guard pad be touched, other pads no response. */
            if (pad_num == TOUCH_PAD_GPIO4_CHANNEL) {
                guard_mode_flag = 1;
                ESP_LOGW(TAG, "TouchSensor [%"PRIu32"] be activated, enter guard mode", pad_num);
            } else {
                if (guard_mode_flag == 0) {
                    ESP_LOGI(TAG, "TouchSensor [%"PRIu32"] be activated, status mask 0x%"PRIu32"", pad_num, pad_status);
                } else {
                    ESP_LOGW(TAG, "In guard mode. No response");
                }
            }
        }
        if (intr_mask & TOUCH_PAD_INTR_MASK_INACTIVE) {
            /* if guard pad be touched, other pads no response. */
            if (pad_num == TOUCH_PAD_GPIO4_CHANNEL) {
                guard_mode_flag = 0;
                ESP_LOGW(TAG, "TouchSensor [%"PRIu32"] be inactivated, exit guard mode", pad_num);
            } else {
                if (guard_mode_flag == 0) {
                    ESP_LOGI(TAG, "TouchSensor [%"PRIu32"] be inactivated, status mask 0x%"PRIu32, pad_num, pad_status);
                }
            }
        }
        if (intr_mask & TOUCH_PAD_INTR_MASK_SCAN_DONE) {
            ESP_LOGI(TAG, "The touch sensor group measurement is done [%"PRIu32"].", pad_num);
        }
        if (intr_mask & TOUCH_PAD_INTR_MASK_TIMEOUT) {
            /* Add your exception handling in here. */
            ESP_LOGI(TAG, "Touch sensor channel %"PRIu32" measure timeout. Skip this exception channel!!", pad_num);
            touch_pad_timeout_resume(); // Point on the next channel to measure.
        }
    }

    // 初始化触摸传感器
    void touch_button_init()
    {
        // 初始化触摸模块
        touch_pad_init();
        touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
        touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

        // 配置 GPIO4 为触摸通道
        touch_pad_config(TOUCH_PAD_GPIO4_CHANNEL); // 初始阈值设为 0（后续动态设置）
#if TOUCH_BUTTON_DENOISE_ENABLE
        /* Denoise setting at TouchSensor 0. */
        touch_pad_denoise_t denoise = {
            /* The bits to be cancelled are determined according to the noise level. */
            .grade = TOUCH_PAD_DENOISE_BIT4,
            /* By adjusting the parameters, the reading of T0 should be approximated to the reading of the measured channel. */
            .cap_level = TOUCH_PAD_DENOISE_CAP_L4,
        };
        touch_pad_denoise_set_config(&denoise);
        touch_pad_denoise_enable();
        ESP_LOGI(TAG, "Denoise function init");
#endif

#if TOUCH_BUTTON_WATERPROOF_ENABLE
        /* Waterproof function */
        touch_pad_waterproof_t waterproof = {
            .guard_ring_pad = TOUCH_PAD_GPIO4_CHANNEL, // If no ring pad, set 0;
            /* It depends on the number of the parasitic capacitance of the shield pad.
               Based on the touch readings of T14 and T0, estimate the size of the parasitic capacitance on T14
               and set the parameters of the appropriate hardware. */
            .shield_driver = TOUCH_PAD_SHIELD_DRV_L2,
        };
        touch_pad_waterproof_set_config(&waterproof);
        touch_pad_waterproof_enable();
        ESP_LOGI(TAG, "touch pad waterproof init");
#endif

        /* Filter setting */
        touchsensor_filter_set(TOUCH_PAD_FILTER_IIR_16);
        touch_pad_timeout_set(true, TOUCH_PAD_THRESHOLD_MAX);
        /* Register touch interrupt ISR, enable intr type. */
        touch_pad_isr_register(touchsensor_interrupt_cb, NULL, (touch_pad_intr_mask_t)TOUCH_PAD_INTR_MASK_ALL);
        /* If you have other touch algorithm, you can get the measured value after the `TOUCH_PAD_INTR_MASK_SCAN_DONE` interrupt is generated. */
        touch_pad_intr_enable((touch_pad_intr_mask_t)(TOUCH_PAD_INTR_MASK_ACTIVE | TOUCH_PAD_INTR_MASK_INACTIVE | TOUCH_PAD_INTR_MASK_TIMEOUT));

        /* Enable touch sensor clock. Work mode is "timer trigger". */
        touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
        touch_pad_fsm_start();

        ESP_LOGI(TAG, "Touch button initialized (GPIO4)");
    }

    void InitializeButtons()
    {
        boot_button_.OnClick([this]()
                             {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState(); });
        touch_button_.OnPressDown([this]() {
            ESP_LOGI(TAG, "Touch button pressed down, start listening");
            Application::GetInstance().StartListening();
        });
        touch_button_.OnPressUp([this]() {
            ESP_LOGI(TAG, "Touch button released, stop listening");
            Application::GetInstance().StopListening();
        });

        // volume_up_button_.OnClick([this]() {
        //     auto codec = GetAudioCodec();
        //     auto volume = codec->output_volume() + 10;
        //     if (volume > 100) {
        //         volume = 100;
        //     }
        //     codec->SetOutputVolume(volume);
        //     GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        // });

        // volume_up_button_.OnLongPress([this]() {
        //     GetAudioCodec()->SetOutputVolume(100);
        //     GetDisplay()->ShowNotification(Lang::Strings::MAX_VOLUME);
        // });

        // volume_down_button_.OnClick([this]() {
        //     auto codec = GetAudioCodec();
        //     auto volume = codec->output_volume() - 10;
        //     if (volume < 0) {
        //         volume = 0;
        //     }
        //     codec->SetOutputVolume(volume);
        //     GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        // });

        // volume_down_button_.OnLongPress([this]() {
        //     GetAudioCodec()->SetOutputVolume(0);
        //     GetDisplay()->ShowNotification(Lang::Strings::MUTED);
        // });
    }

    // 物联网初始化，添加对 AI 可见设备
    void InitializeIot()
    {
        auto &thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker"));
        thing_manager.AddThing(iot::CreateThing("Lamp"));
    }

    void InitializeGPIO()
    {
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = GPIO_BIT_MASK;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);

        gpio_set_level(LCD_EN_PIN, 1);   // Enable LCD
        gpio_set_level(AMP_GAIN_PIN, 1); // Enable amplifier gain
        gpio_set_level(AMP_SD_PIN, 1);   // Enable amplifier active
        gpio_set_level(MIC_EN_PIN, 0);   // Enable microphone
        gpio_set_level(VCC_EN_PIN, 1);   // Enable VCC
    }

public:
    CompactWifiBoard() : // boot_button_(BOOT_BUTTON_GPIO),
                         // touch_button_(TOUCH_BUTTON_GPIO),
                         // volume_up_button_(VOLUME_UP_BUTTON_GPIO),
                         // volume_down_button_(VOLUME_DOWN_BUTTON_GPIO) {
                        boot_button_(BOOT_BUTTON_GPIO),
                        touch_button_(TOUCH_BUTTON_GPIO)
    {
        InitializeGPIO();
        InitializeDisplayI2c();
        InitializeSsd1306Display();
        
        InitializeButtons();
        InitializeIot();
        // touch_button_init();

    }

    // virtual Led* GetLed() override {
    //     static SingleLed led(BUILTIN_LED_GPIO);
    //     return &led;
    // }

    virtual AudioCodec *GetAudioCodec() override
    {
#ifdef AUDIO_I2S_METHOD_SIMPLEX
        static NoAudioCodecSimplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                               AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
#else
        static NoAudioCodecDuplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                              AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN);
#endif
        return &audio_codec;
    }

    virtual Display *GetDisplay() override
    {
        return display_;
    }
};

DECLARE_BOARD(CompactWifiBoard);
