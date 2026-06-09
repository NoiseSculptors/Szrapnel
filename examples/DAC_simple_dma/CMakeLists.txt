
cmake_minimum_required(VERSION 3.16)

add_firmware(
  NAME DAC_simple_dma
  SRCS DAC_simple_dma.c
  LIBS m lib::printf
)

