
cmake_minimum_required(VERSION 3.16)

add_firmware(
  NAME DAC_simple_sine_q31
  SRCS DAC_simple_sine_q31.c
  LIBS m lib::cmsis-dsp
)

