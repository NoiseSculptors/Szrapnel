
cmake_minimum_required(VERSION 3.19)

add_firmware(
  NAME encoders 
  SRCS encoders.c
  LIBS lib::printf
)

