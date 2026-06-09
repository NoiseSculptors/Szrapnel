
/* run entirely from SRAM (0x20000000): code+data+bss+stack in SRAM */
ENTRY(Reset_Handler)

/* Stack at top of SRAM */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);
PROVIDE(__StackTop   = _estack);
PROVIDE(__StackLimit = ORIGIN(SRAM));

SECTIONS
{
  /* Vector table at start of SRAM (VTOR points here) */
  .isr_vector ORIGIN(SRAM) :
  {
    . = ALIGN(128);
    KEEP(*(.isr_vector))
    . = ALIGN(128);
  } > SRAM

  /* Code in SRAM */
  .text :
  {
    . = ALIGN(8);
    *(.text*)
    *(.glue_7*) *(.glue_7t*)

    KEEP(*(.init))
    KEEP(*(.fini))

    *(.rodata*)

    /* Unwind info (if used) */
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    *(.ARM.exidx*)

    . = ALIGN(8);
  } > SRAM

  /* C++ static ctor/dtor arrays (if any) */
  .preinit_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } > SRAM

  .init_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } > SRAM

  .fini_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } > SRAM

  /* Data lives in SRAM already — no LMA copy needed */
  .data :
  {
    . = ALIGN(8);
    __data_start__ = .;
    *(.data*)
    __data_end__ = .;
    . = ALIGN(8);
  } > SRAM

  /* Make load=run so startup’s copy becomes a no-op in SRAM builds */
  __data_load_start__ = __data_start__;

  /* Zero-init */
  .bss (NOLOAD) :
  {
    . = ALIGN(8);
    __bss_start__ = .;
    *(.bss*) *(COMMON)
    __bss_end__ = .;
    . = ALIGN(8);
  } > SRAM

  /* Heap start marker */
  PROVIDE(__heap_start__ = .);
}

/* Handy symbol for VTOR setup */
PROVIDE(__VectorTable = ORIGIN(SRAM));

