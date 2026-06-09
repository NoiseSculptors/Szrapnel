
/* program executes from Flash (0x08000000), data+stack in SRAM */
ENTRY(Reset_Handler)

/* Stack in SRAM */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);
PROVIDE(__StackTop   = _estack);
PROVIDE(__StackLimit = ORIGIN(SRAM));

SECTIONS
{
  /* Vector table must be at Flash base so the boot ROM finds it */
  .isr_vector ORIGIN(FLASH) :
  {
    . = ALIGN(128);
    KEEP(*(.isr_vector))
    . = ALIGN(128);
  } > FLASH

  /* Code and const data live in Flash */
  .text :
  {
    . = ALIGN(8);
    *(.text*)
    *(.glue_7*) *(.glue_7t*)

    KEEP(*(.init))
    KEEP(*(.fini))

    *(.rodata*)

    /* Unwind info (if enabled) */
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    *(.ARM.exidx*)

    . = ALIGN(8);
  } > FLASH

  /* C++ static ctor/dtor arrays (if used) */
  .preinit_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } > FLASH

  .init_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } > FLASH

  .fini_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } > FLASH

  /* Initialized data: VMA in SRAM, LMA in FLASH. Startup copies it. */
  .data :
  {
    . = ALIGN(8);
    __data_start__ = .;
    *(.data*)
    __data_end__ = .;
    . = ALIGN(8);
  } > SRAM AT > FLASH

  /* Copy source for .data */
  __data_load_start__ = LOADADDR(.data);

  /* Zero-init */
  .bss (NOLOAD) :
  {
    . = ALIGN(8);
    __bss_start__ = .;
    *(.bss*) *(COMMON)
    __bss_end__ = .;
    . = ALIGN(8);
  } > SRAM

  /* Heap starts after BSS by default */
  PROVIDE(__heap_start__ = .);
}

/* Symbols used by startup */
PROVIDE(__VectorTable = ORIGIN(FLASH));

