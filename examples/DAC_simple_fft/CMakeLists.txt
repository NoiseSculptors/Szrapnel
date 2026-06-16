
cmake_minimum_required(VERSION 3.16)

add_firmware(
  NAME DAC_simple_fft 
  SRCS DAC_simple_fft.c
  LIBS lib::cmsis-dsp
)

