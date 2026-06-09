
cmake_minimum_required(VERSION 3.19)
project(app_io C)


add_library(app_io STATIC
                              buttons.c
                              dac.c
                              encoders.c
                              fonts/font_6x10.c
                              fonts/font_8x8.c
                              io.c
                              lcd.c
                              leds.c
                              serial.c
                              )
add_library(lib::io ALIAS app_io)

target_link_libraries(app_io PUBLIC lib::printf)

target_include_directories(app_io PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(app_io PRIVATE -ffunction-sections -fdata-sections)

