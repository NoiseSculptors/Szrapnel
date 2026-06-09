
cmake_minimum_required(VERSION 3.16)

# Build trampoline only if we're *not* running from flash
if(RUN STREQUAL "flash")
  return()
endif()

message(STATUS "Building flash trampoline for ${PART}")

set(TRAMP_NAME "trampoline_${PART}")
set(TRAMP_SRC  "${CMAKE_CURRENT_SOURCE_DIR}/trampoline.S")

add_executable(${TRAMP_NAME} ${TRAMP_SRC})

# --- Architecture settings per chip --------------------------
if(PART STREQUAL "h750")
  set(ARCH_FLAGS -mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
  set(APP_BASE  0x24000000)
  set(STACK_TOP 0x20020000)
elseif(PART STREQUAL "l431")
  set(ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
  set(APP_BASE  0x20000000)
  set(STACK_TOP 0x20018000)
elseif(PART STREQUAL "g030")
  set(ARCH_FLAGS -mthumb -mcpu=cortex-m0plus)
  set(APP_BASE  0x20000000)
  set(STACK_TOP 0x20002000)
else()
  message(FATAL_ERROR "Unknown PART '${PART}' for trampoline")
endif()

target_compile_options(${TRAMP_NAME} PRIVATE ${ARCH_FLAGS})
target_compile_definitions(${TRAMP_NAME} PRIVATE
  APP_BASE=${APP_BASE}
  STACK_TOP=${STACK_TOP}
)

target_link_options(${TRAMP_NAME} PRIVATE
    -T${_LD_DIR}/linker_flash.ld
  ${LINKER_DEFS}
  -nostartfiles
  -Wl,--gc-sections
  -Wl,-Map=${TRAMP_NAME}.map
)

# --- Post-build BIN output -----------------------------------
find_program(OBJCOPY NAMES arm-none-eabi-objcopy REQUIRED)
add_custom_command(TARGET ${TRAMP_NAME} POST_BUILD
  COMMAND ${OBJCOPY} -O binary
    $<TARGET_FILE:${TRAMP_NAME}> ${TRAMP_NAME}.bin
  COMMENT "Generating ${TRAMP_NAME}.bin for flashing"
)

