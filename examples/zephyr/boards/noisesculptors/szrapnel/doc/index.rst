.. zephyr:board:: szrapnel

Overview
********

The Szrapnel board is a maker-friendly breakout platform based on the
**STMicroelectronics STM32H750VB** microcontroller (Arm® Cortex®-M7 core).

Unlike traditional evaluation boards, the Szrapnel provides **maximum pin
exposure and minimal pre-wired peripherals**, giving developers freedom to build
custom add-ons, connect salvaged hardware, and design their own
application-specific user experience. Almost all available MCU pins are broken
out to dual headers, each paired with a dedicated GND pin for robust
signal integrity.

Key features:

- STM32H750VB microcontroller in LQFP100 package
- ARM® 32-bit Cortex®-M7 CPU with FPU, running up to 480 MHz
- 128 Kbytes Flash memory
- 1 Mbyte SRAM
- **16 MByte (128 Mbit) external QSPI Flash**
- **26 MHz Seiko-Epson TCXO** for precision timing
- Nearly all pins (≈ 72, incl. SWD) broken out, QSPI pins reserved
- Dedicated GND next to each GPIO header pin
- USB-C device port
- No onboard user LEDs or user buttons – intended for expansion
- External SWD debug interface required (ST-LINK, J-Link, etc.)

Hardware
********

The Szrapnel board exposes most of the STM32H750VB I/Os, allowing users to
add displays, sensors, or custom boards directly to the header rows.

Why a GND next to each pin?
===========================

Each GPIO header pin is paired with a dedicated GND pin. This design choice:

- Minimizes **ground loops**
- Improves **signal integrity** for high-speed peripherals
- Simplifies prototyping with jumper wires or salvaged components
- Enables the Szrapnel to serve as a **mainboard** with robust
  daughterboard connections

Supported Features
==================

.. zephyr:board-supported-hw::

Connections and IOs
===================

Pin Exposure
------------

- ~72 I/O pins exposed (excluding QSPI)
- SWDIO / SWCLK available for external debug probe
- GND adjacent to every GPIO pin
- QSPI Flash wired to MCU (not broken out)

System Clock
------------

By default, the board uses the **26 MHz TCXO**, providing high-accuracy
clocking for USB, timers, and communication peripherals.

Programming and Debugging
*************************

.. zephyr:board-supported-runners::

The Szrapnel does not include an onboard debug probe.  
An **external SWD programmer/debugger** is required (e.g. ST-LINK, J-Link, etc.).

Flashing
========

Example with ST-LINK:

.. code-block:: console

   $ west flash --runner stlink

Example with J-Link:

.. code-block:: console

   $ west flash --runner jlink

Alternatively, flashing **without a hardware probe** is also possible using the
board’s built-in USB DFU bootloader (selected by pressing the BOOTSEL button).

Debugging
=========

Debugging requires an external probe connected to SWD pins. Example:

.. zephyr-app-commands::
   :zephyr-app: samples/hello_world
   :board: szrapnel
   :goals: debug

References
**********

- `Noise Sculptors website <https://noisesculptors.com/products/szrapnel>`_
- `STM32H750VB website <https://www.st.com/en/microcontrollers-microprocessors/stm32h750vb.html>`_
- `STM32H750 reference manual <https://www.st.com/resource/en/reference_manual/rm0433-stm32h742-stm32h743753-and-stm32h750-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.pdf>`_
- `STM32CubeProgrammer <https://www.st.com/en/development-tools/stm32cubeprog.html>`_
