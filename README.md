
# Szrapnel | Hackable synth kit

## What It Is

Szrapnel is for people who like to build, hack, experiment, and explore sound
through hardware and firmware.

It is designed for **creative coders, synth hackers, experimental musicians,
and embedded developers** who want a flexible tool they can learn from and
shape over time.

Szrapnel is built to be open, practical, and hackable. It gives you a platform
for signal generation, sound experiments, custom firmware, and hands-on
exploration without forcing you into a closed workflow.

In simple terms: it is a tool for people who want to make things, change
things, and understand how they work.

Open.  Experimental.  Built for makers, musicians, and curious programmers
exploring sound and DSP.

---

## Getting started

If you are new to embedded audio projects, start with the example programs
that:

- test basic hardware features
- show how the board is connected
- introduce sound synthesis and audio output

These examples are the best way to learn how the system works before making
your own changes.

---

## Compiling

Szrapnel is built with CMake and an ARM embedded toolchain.

### 1) Install the required STM32 pack

If you are developing for **AXI RAM**, install
[pyOCD](https://github.com/pyocd/pyOCD) and the STM32H750 device
pack first:

```bash
pip install pyocd
pyocd pack install STM32H750VBTx
```

This pack is needed for uploading to **AXI memory** when developing and running
code from AXI memory.

**AXI memory** is a fast system RAM region, and also the largest contiguous
region on the chip.

### 2) Build

From your home directory, run:

```bash
cmake -S /path/to/szrapnel -B /path/to/szrapnel/build -G Ninja \
  -DPART=h750 \
  -DRUN=axi \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DCMAKE_COLOR_DIAGNOSTICS=ON \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build /path/to/szrapnel/build --config Debug -- -j <no_of_threads>
```

Replace `/path/to/szrapnel` with the path where you cloned the project.
Replace `-DRUN=axi` with `-DRUN=flash` to run code from flash.
Replace `-DCMAKE_BUILD_TYPE=Debug` with `-DCMAKE_BUILD_TYPE=Release` for builds
without debugging.

### 3) Upload with a debug probe

If you use a debugging probe, connect it to the **4 isolated pins** on the board:

- **SWCLK**
- **GND**
- **SWDIO**
- **NRST**

When looking from the front, the pins are arranged left to right as:

```
                 1 2 3 4
   o o    o o    o o o o    o o o o o o o o o o o o o o

   1:SWCLK
   2:GND
   3:SWDIO
   4:NRST
```

This setup has been tested with:
- [dap42](https://github.com/devanlai/dap42) — bluepill-based free/open-source solution
- **ST-Link V2**

Then upload the generated binary with:

```bash
pyocd load /path/to/szrapnel/build/examples/<an_example>/*.bin --target stm32h750vbtx --base 0x24000000
```

---

## No debug probe? Use DFU over USB

If you do not have a debugging probe, you can still upload firmware using the board’s **DFU mode**:

1. Press the **DFU** button
2. Reset the device
3. Release the **DFU** button
4. Upload the firmware over USB using DFU software

Recommended free/open-source tool you can use:
- **dfu-util** — Linux, macOS, Windows

---

## Notes for beginners

If you are not sure where to begin:
1. build and run the simplest example(s) first
2. try the hardware test examples
3. move on to the audio synthesis examples
4. only then start modifying firmware or adding your own code

That approach will help you understand the signal path, the hardware, and the
firmware workflow step by step.

---

## Philosophy

Szrapnel is designed to be:
- open
- practical
- hackable
- fast
- simple 
- educational
- fun to experiment with

