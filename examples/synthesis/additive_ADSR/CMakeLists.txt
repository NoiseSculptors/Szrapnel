
cmake_minimum_required(VERSION 3.19)

add_firmware(
  NAME additive_ADSR 
  SRCS additive_ADSR.c
)

target_link_libraries(additive_ADSR PRIVATE
    lib::ns_audio
    m
)

