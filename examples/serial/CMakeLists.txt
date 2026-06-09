
cmake_minimum_required(VERSION 3.19)

add_firmware(
  NAME serial
  SRCS serial.c
  #  LIBS lib::printf
)

