
# RAM-Boot Trampoline — Boot apps directly from SRAM without flash wear

Tiny flash image that *forwards reset* to an application whose vector table lives in SRAM (default `0x24000000`, AXI SRAM on STM32H750).
On reset, the trampoline:

1. Reads the **MSP** from the RAM app’s vector table.
2. Sets **VTOR = APP_BASE** so all subsequent interrupts/exceptions use the RAM app’s vectors.
3. Invalidates the **I-cache** (so instructions are fetched from RAM).
4. Loads **MSP** and **branches** to the RAM app’s `Reset_Handler` (Thumb).

---

This trampoline gives you a *valid* flash image that simply hands execution to your real app in RAM.

---

## Flash the trampoline

For STM32H750

```sh
st-flash write startup/common/trampoline_h750.bin 0x8000000
```

or STM32G030

```sh
st-flash write startup/common/trampoline_g030.bin 0x8000000
```

This writes the tiny forwarder into flash at `0x08000000`.

---

## Load and run your RAM app

Once the trampoline is flashed, you can download your app to AXI SRAM through
patched pyocd (explained in the source code):

```sh
# Example: load a Zephyr RAM image to 0x24000000
pyocd load /path/to/zephyr.bin --target stm32h750xx --base 0x24000000
# Reset or run; on reset, the trampoline ideally jumps into the RAM app
```

## Important

To be able to "flash" to 0x24000000 memory address, please add the following
address region to pyocd: 

```bash
 pyocd/target/builtin/target_STM32H750xx.py
```

```diff
   #DTCM
   RamRegion(   start=0x20000000, length=0x20000,
             is_cachable=False,
             access="rw"),
+  #ram
+  RamRegion(   start=0x24000000, length=0x80000,
+            is_cachable=False,
+            access="rwx"),
   #sram1
   RamRegion(   start=0x30000000, length=0x20000,
             is_powered_on_boot=False),
```

On hardware reset:

* CPU vectors to flash, lands in trampoline `Reset_Handler`
* Trampoline switches VTOR to `0x24000000` and jumps to your app

