
/* run entirely from AXI SRAM, stack in DTCM */
ENTRY(Reset_Handler)

/* Stack at top of DTCM */
_estack = ORIGIN(DTCM) + LENGTH(DTCM);
PROVIDE(__StackTop = _estack);
PROVIDE(__StackLimit = ORIGIN(DTCM));

SECTIONS
{
  /* Vector table at start of AXI, keep 128-byte alignment for VTOR */
  .isr_vector ORIGIN(AXI) :
  {
    . = ALIGN(128);
    KEEP(*(.isr_vector))
    . = ALIGN(128);
  } > AXI

  /* Code + const in AXI */
  .text :
  {
    . = ALIGN(8);
    *(.text*)
    *(.glue_7*) *(.glue_7t*)

    /* C runtime bits often end up here */
    KEEP(*(.init))
    KEEP(*(.fini))

    /* Read-only data */
    *(.rodata*)   /* (single occurrence is enough) */

    /* Unwind info (if used) */
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    *(.ARM.exidx*)
    
    . = ALIGN(8);
  } > AXI

  /* C++ static constructors/destructors (if any) */
  .preinit_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } > AXI

  .init_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } > AXI

  .fini_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } > AXI

  /* Data lives in AXI already — no LMA/ROM copy */
  .data :
  {
    . = ALIGN(8);
    __data_start__ = .;
    *(.data*)
    __data_end__ = .;
    . = ALIGN(8);
  } > AXI

  /* Make load=run so startup’s copy becomes a no-op in AXI builds */
  __data_load_start__ = __data_start__;
  
  /* Zero-init */
  .bss (NOLOAD) :
  {
    . = ALIGN(8);
    __bss_start__ = .;
    *(.bss*) *(COMMON)
    __bss_end__ = .;
    . = ALIGN(8);
  } > AXI

  /* Optional heap start marker */
  PROVIDE(__heap_start__ = .);

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

/* Handy symbols for startup */
PROVIDE(__VectorTable = ORIGIN(AXI));

