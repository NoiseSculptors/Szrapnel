
/* program executes from Flash (0x08000000), data+stack in DTCM */
ENTRY(Reset_Handler)

/* Stack in DTCM */
_estack = ORIGIN(DTCM) + LENGTH(DTCM);
PROVIDE(__StackTop   = _estack);
PROVIDE(__StackLimit = ORIGIN(DTCM));

SECTIONS
{
  /* Vector table must be at Flash base so bootloader can find it */
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
    *(.init) *(.fini)
    *(.rodata*)
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    *(.ARM.exidx*)
    . = ALIGN(8);
  } > FLASH

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

  /* Initialized data section:
     LMA = Flash (load), VMA = DTCM (run)
     The startup code will copy it. */
  .data :
  {
    . = ALIGN(8);
    __data_start__ = .;
    *(.data*)
    __data_end__ = .;
    . = ALIGN(8);
  } > DTCM AT > FLASH

  /* Define copy source (LMA) symbol for startup */
  __data_load_start__ = LOADADDR(.data);

  /* Zero-init */
  .bss (NOLOAD) :
  {
    . = ALIGN(8);
    __bss_start__ = .;
    *(.bss*) *(COMMON)
    __bss_end__ = .;
    . = ALIGN(8);
  } > DTCM

  /* Optional heap (can place in AXI if large buffers needed) */
  PROVIDE(__heap_start__ = ORIGIN(AXI));

  /* Any function tagged section(".itcm") will land here. */
  .itcm :
  {
    . = ALIGN(16);
    __itcm_start__ = .;
    *(.itcm) *(.itcm.*)
    __itcm_end__ = .;
    . = ALIGN(16);
  } > ITCM AT > FLASH
  __itcm_load__ = LOADADDR(.itcm);

  /* For initialized variables placed with section(".dtcm_data") */
  .dtcm_data :
  {
    . = ALIGN(8);
    __dtcm_data_start__ = .;
    *(.dtcm_data) *(.dtcm_data.*)
    __dtcm_data_end__ = .;
    . = ALIGN(8);
  } > DTCM AT > FLASH
  __dtcm_data_load__ = LOADADDR(.dtcm_data);

  /* For zero-init variables placed with section(".dtcm_bss") */
  .dtcm_bss (NOLOAD) :
  {
    . = ALIGN(8);
    __dtcm_bss_start__ = .;
    *(.dtcm_bss) *(.dtcm_bss.*)
    __dtcm_bss_end__ = .;
    . = ALIGN(8);
  } > DTCM
}

/* Symbols used by startup */
PROVIDE(__VectorTable = ORIGIN(FLASH));

