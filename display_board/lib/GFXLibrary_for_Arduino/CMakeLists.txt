
get_filename_component(dir ${CMAKE_CURRENT_LIST_FILE} PATH)
FILE(GLOB_RECURSE sources ${dir}/src/*.cpp
                        ${dir}/src/canvas/*.cpp
                        ${dir}/src/databus/*.cpp
                        ${dir}/src/display/*.cpp
                        ${dir}/src/font/*.cpp
)

idf_component_register(SRCS ${sources}      
                    INCLUDE_DIRS "src" "src/canvas" "src/databus" "src/display" "src/font"
                    REQUIRES driver arduino-esp32 esp32_display_panel
)
