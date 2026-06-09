
cmake_minimum_required(VERSION 3.19)

add_firmware(
  NAME lcd_display
  SRCS lcd_display.c
  LIBS lib::printf
)

