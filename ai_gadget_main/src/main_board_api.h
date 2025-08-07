#pragma once
#include <driver/i2c_master.h>
#include <driver/i2s_std.h>

extern i2c_master_bus_handle_t  i2c0_bus_hdl;

struct main_api_t {
    void (*voice_start)();
    void (*voice_end)();
    i2s_chan_handle_t (*get_i2s_tx_handle)();
    i2s_std_config_t* (*get_i2s_std_config)();
};

extern main_api_t* main_api;