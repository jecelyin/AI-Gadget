# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(PROJECT_VER "1.6.2")

# Add this line to disable the specific warning
add_compile_options(-Wno-missing-field-initializers -DLV_LVGL_H_INCLUDE_SIMPLE=1 -Wno-error=format -Wno-format)

# 通过环境变量选择环境
# export ESP_ENV=main_board
# idf.py build
# if(NOT DEFINED ENV{ESP_ENV})
#     set(ESP_ENV "main_board")  # 默认开发环境
# else()
#     set(ESP_ENV $ENV{ESP_ENV})
# endif()
set(ESP_ENV "main_board")
# set(ESP_ENV "display_board")

if(ESP_ENV STREQUAL "main_board")
    # 主板配置
    set(EXTRA_COMPONENT_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/${ESP_ENV}
        ${CMAKE_CURRENT_SOURCE_DIR}/components
        ${CMAKE_CURRENT_SOURCE_DIR}/ai_gadget_main
    )
else()

    # 显示板配置
    add_compile_options(#-DLV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${ESP_ENV}/include/lv_conf.h"
                        -DBOARD_HAS_PSRAM
                        -mfix-esp32-psram-cache-issue
                        -DCONFIG_AUTOSTART_ARDUINO
                        -DLV_CONF_INCLUDE_SIMPLE
                        -DLV_LVGL_H_INCLUDE_SIMPLE)
    set(EXTRA_COMPONENT_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/${ESP_ENV}
        ${CMAKE_CURRENT_SOURCE_DIR}/components
    )
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(xiaozhi)

