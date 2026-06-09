/* trampoline.S — flash-resident forwarder into RAM app
 * - Do NOT modify MSP; we keep the MSP loaded from this trampoline's flash vector.
 * - Read only PC from APP_BASE[1], set VTOR=APP_BASE, then branch.
 * - Optional: small sanity check on PC (non-zero, Thumb).
 *
 * Configurable via -D defines from CMake:
 *   APP_BASE  : RAM app vector base (default 0x24000000 for H750 AXI; 0x20000000 for G0)
 */

.syntax unified
.thumb

/* SCB registers */
.set SCB_VTOR,    0xE000ED08
.set SCB_ICIALLU, 0xE000EF50

/* ---- minimal vector table in flash (for this trampoline) ----
 * word0: MSP value used by core on reset (adjust per part/linker)
 * word1: this Reset_Handler | 1 (Thumb)
 */
.section .isr_vector, "a", %progbits
.align  2
.word  STACK_TOP
.word  Reset_Handler + 1

.text
.thumb
.align 2
.global Reset_Handler
.type   Reset_Handler, %function
Reset_Handler:
    /* r0 := APP_BASE (RAM vector) */
    ldr   r0, =APP_BASE

    /* Load PC from APP_BASE[1] */
    ldr   r2, [r0, #4]

    /* Sanity: if zero, trap */
    cmp   r2, #0
    beq   .Ltrap

    /* Ensure Thumb bit set (in case image forgot) */
    lsls  r3, r2, #31        /* test bit0 by shifting left 31 -> N flag if bit0=1 */
    bne   .Lthumb_ok
    adds  r2, #1
.Lthumb_ok:

    /* VTOR := APP_BASE so exceptions go to RAM app */
    ldr   r3, =SCB_VTOR
    str   r0, [r3]
    dsb   sy
    isb

#ifdef __CORTEX_M7
    /* Invalidate I-cache (M7 only) */
    ldr   r3, =SCB_ICIALLU
    movs  r4, #0
    str   r4, [r3]
    dsb   sy
    isb
#endif

    /* Jump to RAM app Reset_Handler (keep current MSP) */
    bx    r2

.Ltrap:
    /* Hang if PC invalid */
    b     .Ltrap

.size Reset_Handler, .-Reset_Handler

