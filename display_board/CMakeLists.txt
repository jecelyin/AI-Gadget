

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIRS ${SOURCE_DIR}
                ${SOURCE_DIR}/ui
                ${SOURCE_DIR}/ui/widget)


file(GLOB SOURCES
    ${SOURCE_DIR}/*.cc
    ${SOURCE_DIR}/*.cpp
)
file(GLOB UI_SOURCES
    ${SOURCE_DIR}/ui/*.c
    ${SOURCE_DIR}/ui/*.cpp
    ${SOURCE_DIR}/ui/fonts/*.c
    ${SOURCE_DIR}/ui/images/*.c
    ${SOURCE_DIR}/ui/widget/*.c
)
list(APPEND SOURCES ${UI_SOURCES})

# file(GLOB GFX_SOURCES
#     ${SOURCE_DIR}/GFXLibrary/*.cpp
#     ${SOURCE_DIR}/GFXLibrary/font/*.cpp
#     ${SOURCE_DIR}/GFXLibrary/display/*.cpp
#     ${SOURCE_DIR}/GFXLibrary/databus/*.cpp
#     ${SOURCE_DIR}/GFXLibrary/canvas/*.cpp
# )
# list(APPEND INCLUDE_DIRS ${SOURCE_DIR}/GFXLibrary/
#     ${SOURCE_DIR}/GFXLibrary/font
#     ${SOURCE_DIR}/GFXLibrary/display
#     ${SOURCE_DIR}/GFXLibrary/databus
#     ${CMAKE_CURRENT_SOURCE_DIR}/GFXLibrary/canvas
# )
# list(APPEND SOURCES ${GFX_SOURCES})

set(MY_DEPS driver
        nvs_flash
        esp_psram
        esp_event
        json
        common
        StreamProtocol
        FileMeta
        lvgl
        arduino-esp32
        GFXLibrary_for_Arduino
        # Adafruit_BusIO
        # Adafruit_xCA9554
    )


idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDE_DIRS}
                    REQUIRES ${MY_DEPS}
                    WHOLE_ARCHIVE
                    )
