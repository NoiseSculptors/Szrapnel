
cmake_minimum_required(VERSION 3.19)

add_firmware(
  NAME leds 
  SRCS leds.c
  LIBS lib::printf
)

