
cmake_minimum_required(VERSION 3.16)

add_firmware(
  NAME DAC_simple_without_dma
  SRCS DAC_simple_without_dma.c
  LIBS m lib::printf
)

