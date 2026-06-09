
cmake_minimum_required(VERSION 3.31)

# ---- printf -----------------------------------------------------------------
add_library(lib_printf STATIC printf/printf.c)
add_library(lib::printf ALIAS lib_printf)
target_include_directories(lib_printf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/printf)
target_link_libraries(lib_printf PUBLIC noisesculptors::core)
# Suppress known benign warnings
set_source_files_properties(${CMAKE_SOURCE_DIR}/lib/printf/printf.c
  PROPERTIES COMPILE_OPTIONS "-Werror;-Wno-double-promotion")

