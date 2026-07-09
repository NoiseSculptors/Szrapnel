
cmake_minimum_required(VERSION 3.16)

add_firmware(
  NAME buttons 
  SRCS buttons.c
  LIBS m lib::cmsis-dsp
)

