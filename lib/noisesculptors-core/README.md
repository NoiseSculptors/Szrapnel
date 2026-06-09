# Noise Sculptors Core

**Noise Sculptors Core** provides low-level firmware modules shared between multiple Noise Sculptors products.

It implements only a **subset** of each MCU’s functionality — just enough to initialize clocks, handle timing (DWT / delay), interrupts, basic I/O, and serial communication, etc.

At this stage it serves as a **common core** for Noise Sculptors boards, projects and prototypes.

---

## Supported Microcontrollers

**STM32H750** Used by *Calibrator750*, *Rocket Calibrator*, and related projects.
Coming soon - **STM32L431** used by *Herztick*.

Additional Cortex-M families may be added later as the ecosystem grows.

---

## Structure

```

include/750/         # Public headers
src/h750/            # Core source files
startup/h750/        # Startup code (flash / SRAM)
ldscripts/h750/      # Linker scripts and memory maps

````

Each supported MCU family has its own subfolder (e.g. `h750/`, `l431/`).

---

## Building

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake -DPART=h750 -DRUN=axi
cmake --build build
```

---

## Use in Products

Add this repository as a **submodule** to your product (explained in product's ```README.md```).

---

## Usage note for STM32H750
### TCM (Tightly Coupled Memory) Sections — `.itcm`, `.dtcm_data`, `.dtcm_bss`

The STM32H7 has two *tightly coupled* on-core RAM blocks that run at **zero
wait states** and are directly connected to the Cortex-M7 core:

| Region   | Address Range | Purpose         | Typical Use                                     |
| :------- | :------------ | :-------------- | :---------------------------------------------- |
| **ITCM** | `0x00000000`  | Instruction TCM | Fastest possible code execution                 |
| **DTCM** | `0x20000000`  | Data TCM        | Fastest data access (stack, buffers, variables) |

Unlike AXI SRAM, these memories are **not cached** — the core fetches directly,
with deterministic latency.  They’re ideal for real-time DSP loops, interrupt
handlers, or other time-critical code and data.

#### Why special sections?

Because the compiler and linker don’t know which functions or variables should
live in these tiny but ultra-fast memories, we explicitly mark them using GCC
section attributes:

```c
/* places the function in ITCM, running at full CPU speed. */
__attribute__((section(".itcm")))
void synth_render_block(int32_t *dst, unsigned frames);

/* places initialized variables in DTCM (copied from Flash at startup). */
__attribute__((section(".dtcm_data")))
float g_env_coeffs[8] = { ... };

/* places zero-initialized buffers in DTCM (cleared at startup). */
__attribute__((section(".dtcm_bss")))
int32_t mix_buffer[1024];
```

Unmarked functions and data live in **AXI SRAM**, which is larger but slightly
slower and shared with DMA. **AXI SRAM** accesses are typically accelerated by
the M7’s L1 instruction/data caches.

---

© Noise Sculptors [https://noisesculptors.com](https://noisesculptors.com)

