
cmake_minimum_required(VERSION 3.31)

# ---- CMSIS-DSP---------------------------------------------------------------

set(CMSISCORE "${CMAKE_SOURCE_DIR}/lib/CMSIS_6/CMSIS/Core")
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/CMSIS-DSP)
target_link_libraries(CMSISDSP PUBLIC noisesculptors::core)
target_include_directories(CMSISDSP PUBLIC ${CMSISCORE}/Include)
add_library(lib::cmsis-dsp ALIAS CMSISDSP)

# ---- printf -----------------------------------------------------------------
add_library(printf STATIC printf/printf.c)
add_library(lib::printf ALIAS printf)
target_include_directories(printf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/printf)
target_link_libraries(printf PUBLIC noisesculptors::core)
# Suppress known benign warnings
set_source_files_properties(${CMAKE_SOURCE_DIR}/lib/printf/printf.c
  PROPERTIES COMPILE_OPTIONS "-Werror;-Wno-double-promotion")

# ---- noisesculptors-audio ---------------------------------------------
add_library(ns_audio STATIC noisesculptors-audio/ns_audio.c)
add_library(lib::ns_audio ALIAS ns_audio)
target_include_directories(ns_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/noisesculptors-audio)
target_link_libraries(ns_audio PUBLIC noisesculptors::core)

