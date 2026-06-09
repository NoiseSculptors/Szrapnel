
cmake_minimum_required(VERSION 3.19)

add_firmware(
  NAME lcd_gradient
  SRCS lcd_gradient.c
  LIBS lib::printf
)

