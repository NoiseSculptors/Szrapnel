
cmake_minimum_required(VERSION 3.16)

add_firmware(
  NAME DAC_compare_sample_rate
  SRCS DAC_compare_sample_rate.c
  LIBS m
)

