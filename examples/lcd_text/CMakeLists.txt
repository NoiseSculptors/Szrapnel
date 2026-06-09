
cmake_minimum_required(VERSION 3.19)

add_firmware(
  NAME lcd_text
  SRCS lcd_text.c
  LIBS m lib::printf
)

