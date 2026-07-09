
cmake_minimum_required(VERSION 3.19)

add_firmware(
  NAME hc_sr04 
  SRCS hc_sr04.c
  LIBS lib::ns_audio
)

