
cmake_minimum_required(VERSION 3.19)
project(io C)

add_library(io STATIC buttons.c
                      dac.c
                      encoders.c
                      fonts/font_8x8.c
                      io.c
                      lcd.c
                      leds.c
                      serial.c
)

add_library(lib::io ALIAS io)

target_link_libraries(io PUBLIC lib::printf)

target_include_directories(io PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)

target_compile_options(io PRIVATE -ffunction-sections -fdata-sections)

add_subdirectory(drivers)

