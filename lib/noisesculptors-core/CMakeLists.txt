
cmake_minimum_required(VERSION 3.16)

project(noisesculptors_core C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# -----------------------------------------------------------------------------
# Public knobs consumed from the top-level (keep them as CACHE so they're visible)
# -----------------------------------------------------------------------------
set(PART ""    CACHE STRING "Exact MCU part (e.g. g030, g070, l431, h750)")
set(RUN  ""    CACHE STRING "Run profile for the chosen family (flash|sram|axi)")
# MPU is an include-layout alias; default to the MCU family (set below)
set(MPU  ""    CACHE STRING "Alias used by include layout; defaults to MCU family")

if(PART STREQUAL "")
  message(FATAL_ERROR "Set -DPART=<g030|g070|l431|h750|...> at configure time")
endif()

# -----------------------------------------------------------------------------
# PART -> MCU family (folder) + arch flags
#   * g030/g070/... -> MCU=g0x0
#   * l431 -> MCU=l431
#   * h750 -> MCU=h750
# -----------------------------------------------------------------------------
string(TOLOWER "${PART}" _part_lc)
if(_part_lc MATCHES "^g0[0-9][0-9]$")
  set(MCU "g0x0")
  set(ARCH_FLAGS -mthumb -mcpu=cortex-m0plus)
  set(VALID_RUNS flash sram)
elseif(_part_lc STREQUAL "l432")
  set(MCU "l431")
  set(ARCH_FLAGS -mthumb -mcpu=cortex-m4)
  # L431 has single-precision FPU
  set(FPU_FLAGS  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fsingle-precision-constant)
  set(VALID_RUNS flash sram)
elseif(_part_lc STREQUAL "h750")
  set(MCU "h750")
  set(ARCH_FLAGS -mthumb -mcpu=cortex-m7)
  set(FPU_FLAGS  -mfpu=fpv5-sp-d16 -mfloat-abi=hard -fsingle-precision-constant)
  set(VALID_RUNS flash axi)
else()
  message(FATAL_ERROR "Unknown PART='${PART}' -> cannot deduce MCU family")
endif()

# Default MPU alias to MCU unless explicitly set
if(MPU STREQUAL "")
  set(MPU "${MCU}" CACHE STRING "Alias for include layout" FORCE)
endif()

# Validate RUN choice for the family
list(FIND VALID_RUNS "${RUN}" _rid)
if(_rid EQUAL -1)
  message(FATAL_ERROR "RUN='${RUN}' invalid for PART='${PART}' (family ${MCU}). Valid: [${VALID_RUNS}]")
endif()

set(CPUFLAGS ${ARCH_FLAGS} ${FPU_FLAGS})

# -----------------------------------------------------------------------------
# Linker + Memory map
#   Layout/linker is per-family: ldscripts/<MCU>/linker_<RUN>.ld
#   Memory map is per-part:      ldscripts/<MCU>/memory_map_<PART>.ld
# -----------------------------------------------------------------------------
set(_LD_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/${MCU} CACHE PATH "Linker part dir")
set(_LDS_FILE ${_LD_DIR}/linker_${RUN}.ld)

# Pass explicit memory map for this PART if it exists
set(MEMMAP "${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/${MCU}/memory_map_${PART}.ld")
if(EXISTS "${MEMMAP}")
  set(LINKER_DEFS -Wl,-T${MEMMAP})
endif()

# Allow either memory_map_<part>.ld or memory_map_<PART>.ld (case-insensitive)
set(_MEM_CANDIDATES
  ${_LD_DIR}/memory_map_${_part_lc}.ld
  ${_LD_DIR}/memory_map_${PART}.ld
)
set(_MEM_FILE "")
foreach(_c IN LISTS _MEM_CANDIDATES)
  if(EXISTS "${_c}")
    set(_MEM_FILE "${_c}")
    break()
  endif()
endforeach()
if(_MEM_FILE STREQUAL "")
  message(FATAL_ERROR "Missing per-part memory map for PART='${PART}' under ${_LD_DIR} (expected memory_map_${_part_lc}.ld)")
endif()

# -----------------------------------------------------------------------------
# Source discovery
# -----------------------------------------------------------------------------
set(STARTUP startup/${MCU}/startup.c)

file(GLOB_RECURSE SRCS_MCU
  CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/${MCU}/*.c
)
set(CORE_SRCS ${SRCS_MCU} ${CMAKE_CURRENT_SOURCE_DIR}/${STARTUP})

# -----------------------------------------------------------------------------
# Include paths
# -----------------------------------------------------------------------------
set(PUBLIC_INC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/${MPU}
)

set(MCU_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/${MCU})
file(GLOB MCU_INC_DIRS LIST_DIRECTORIES true ${MCU_SRC_DIR}/*)
foreach(_p IN LISTS MCU_INC_DIRS)
  if(NOT IS_DIRECTORY "${_p}")
    list(REMOVE_ITEM MCU_INC_DIRS "${_p}")
  endif()
endforeach()
list(INSERT MCU_INC_DIRS 0 ${MCU_SRC_DIR})

# -----------------------------------------------------------------------------
# OBJECT "core"
# -----------------------------------------------------------------------------
add_library(noisesculptors_${MCU} OBJECT ${CORE_SRCS})
add_library(noisesculptors::core ALIAS noisesculptors_${MCU})

target_include_directories(noisesculptors_${MCU}
  PUBLIC
    ${PUBLIC_INC}
    ${MCU_INC_DIRS}
)

set(WARN_FLAGS
  -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef
  -Wstrict-prototypes -Wmissing-prototypes -Wredundant-decls
  -Wpointer-arith -Wcast-align -Wwrite-strings -Wstrict-overflow
)

target_compile_options(noisesculptors_${MCU}
  PUBLIC
    ${CPUFLAGS}
    ${WARN_FLAGS}
    -O3
    -flto
    -ffunction-sections
    -fdata-sections
    -fno-builtin
)

target_link_options(noisesculptors_${MCU}
  PUBLIC
    -flto
    ${CPUFLAGS}
)

# Startup/profile defines: startup-only are PRIVATE; profile defs PUBLIC
target_compile_definitions(noisesculptors_${MCU}
  PRIVATE
    __STARTUP_CLEAR_BSS
    __START=main
    __NO_SYSTEM_INIT
  PUBLIC
    $<$<STREQUAL:${RUN},flash>:RUN_FROM_FLASH>
    $<$<STREQUAL:${RUN},axi>:RUN_AXI>
    $<$<STREQUAL:${RUN},sram>:RUN_SRAM>
)

add_subdirectory(startup/common)

# -----------------------------------------------------------------------------
# Export absolute paths so apps can link with two -T scripts (memmap first)
# -----------------------------------------------------------------------------
set_property(TARGET noisesculptors_${MCU}
  PROPERTY NOISESCULPTORS_LDSCRIPT "${_LDS_FILE}")
set_property(TARGET noisesculptors_${MCU}
  PROPERTY NOISESCULPTORS_MEMMAP  "${_MEM_FILE}")

