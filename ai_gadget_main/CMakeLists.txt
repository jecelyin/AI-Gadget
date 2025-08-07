set(SOURCES "")
set(INCLUDE_DIRS "src")


# 定义一个函数来收集目录
function(collect_include_dirs base_dir out_var)
    file(GLOB_RECURSE dirs LIST_DIRECTORIES true "${base_dir}/*")
    set(collected_dirs "")
    foreach(dir ${dirs})
        if(IS_DIRECTORY ${dir})
            file(RELATIVE_PATH rel_dir ${CMAKE_CURRENT_SOURCE_DIR} ${dir})
            list(APPEND collected_dirs ${rel_dir})
        endif()
    endforeach()
    list(REMOVE_DUPLICATES collected_dirs)
    set(${out_var} ${collected_dirs} PARENT_SCOPE)
endfunction()

# 使用函数
collect_include_dirs(${CMAKE_CURRENT_SOURCE_DIR}/src BOARD_INCLUDE_DIRS)
list(APPEND INCLUDE_DIRS ${BOARD_INCLUDE_DIRS})
message(STATUS "Collected include directories: ${INCLUDE_DIRS}")
# 收集所有的源文件
file(GLOB_RECURSE BOARD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)
list(APPEND SOURCES ${BOARD_SOURCES})
message(STATUS "Collected source files: ${SOURCES}")

set(MY_DEPS driver
        sdmmc
        fatfs
        nvs_flash
        spi_flash
        esp_adc
        esp_wifi
        esp_psram
        json
        common
        FileMeta
        SC7A20
        weather
        sensirion_gas_index_algorithm
        StreamProtocol
        esp_audio_player
    )

idf_component_register(SRCS ${SOURCES}
                EMBED_FILES ${LANG_SOUNDS} ${COMMON_SOUNDS}
                INCLUDE_DIRS ${INCLUDE_DIRS}
                REQUIRES ${MY_DEPS}
                WHOLE_ARCHIVE
                )
