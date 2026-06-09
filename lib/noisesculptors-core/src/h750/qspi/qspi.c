
#include "delay.h"
#include "gpio.h"
#include "init.h"
#include "qspi.h"
#include "rcc.h"
#include <stdint.h>

#define COMMON_BIT_SETTINGS ( \
                (0x2u<<12) /* 13:12 ADSIZE = 2:24-bit address         */ \
               |(0x3u<<10) /* 11:10 ADMODE = 3:address on 4 lines     */ \
               |(0x3u<<8)  /* 9:8   IMODE  = 3:instruction on 4 lines */ \
              )

enum {
    READ_STATUS_REGISTER_1  = 0x05,
    READ_STATUS_REGISTER_2  = 0x35,
    READ_STATUS_REGISTER_3  = 0x15,
    WRITE_STATUS_REGISTER_1 = 0x01,
    WRITE_STATUS_REGISTER_2 = 0x31,
    WRITE_STATUS_REGISTER_3 = 0x11,
    WRITE = 0,
    READ  = 1,
    SPI_MODE = 0,
    QPI_MODE = 1,
};

static uint8_t pll2_enabled = 0;

static void qspi_wait_busy(void)
{
    while(*QUADSPI_SR & (1<<5)) // BUSY
        __asm__("nop");
}

/* only for up to 4 bytes (uint8_t) */
static uint32_t qspi_cmd_u32(uint32_t mode,
                             uint32_t rw,
                             uint8_t  instr,
                             uint32_t value,
                             size_t   len)
{
    qspi_wait_busy();

    if (len) {
        *QUADSPI_DLR = (uint32_t)(len - 1u);  // N+1 data bytes
    }

    /* FMODE: 0=indirect write, 1=indirect read */
    uint32_t fmode = rw;  // WRITE=0, READ=1

    /* IMODE/DMODE depend on SPI vs QPI */
    uint32_t imode;
    uint32_t dmode;

    if (mode == SPI_MODE) {
        imode = 0x1u;                  // instruction on 1 line
        dmode = (len ? 0x1u : 0x0u);   // data on 1 line (if any)
    } else { // QPI_MODE
        imode = 0x3u;                  // instruction on 4 lines
        dmode = (len ? 0x3u : 0x0u);   // data on 4 lines (if any)
    }

    *QUADSPI_CCR =
         ( fmode << 26)  /* 27:26 FMODE   0:indirect write, 1:indirect read */
        |( dmode << 24)  /* 25:24 DMODE   0:no data, 1:1-line, 3:4-lines */
        |( imode << 8 )  /* 9:8   IMODE   1:1-line instr, 3:4-line instr */
        | instr;         /* 7:0   INSTR   opcode */

    uint32_t dr = 0;

    if (len) {
        if (rw == READ) {
            dr = *QUADSPI_DR;   // len<=4 ... single read is enough
        } else {
            *QUADSPI_DR = value;
        }
    }

    qspi_wait_busy();

    return dr;
}

void qspi_write_instruction(uint8_t instruction)
{
    qspi_cmd_u32(QPI_MODE, WRITE,instruction, 0, 1);
}

uint32_t qspi_read_status_register(uint32_t status_reg)
{
    return (uint8_t)((qspi_cmd_u32(QPI_MODE, READ, status_reg, 0, 1) & 0xff));
}

void qspi_write_status_register(uint32_t status_reg, uint8_t value)
{
    qspi_cmd_u32(QPI_MODE, WRITE, status_reg, value, 1);
}

void init_qspi(uint8_t prescaler)
{

/*

https://www.st.com/resource/en/errata_sheet/es0445-stm32h745xig-stm32h755xi-stm32h747xig-stm32h757xi-device-errata-stmicroelectronics.pdf

2.8.5 QUADSPI memory failure when using HCLK quadspi_ker_ck clock as QUADSPI CLK
Description

When using HCLK as kernel clock for the QUADSPI peripheral and PRESCALER[7:0]
bit = 0 in the QUADSPI_CR register, memories sensitive to clock duty cycle fail
at high speed because the clock does not respect the 50% duty cycle.

Workaround

- When using HCLK as kernel clock for the QUADSPI peripheral, use a clock
prescale that respects the 50% duty cycle.

Or

- Use an alternative kernel clock source, for example, pll1_q_ck or pll2_r_ck
with QSPISEL[1:0] bits in the RCC_D1CCIPR register.

 */
    if(prescaler == 0){ /* do not continue on prescaler == 0, hangs even on
                           pll2_r_ck even if not mentioned in the document */
        for(;;);
    }

    *RCC_CR &= ~(1<<26); /* erase PLL2ON */

    /* QSPISEL clock source:
        0:rcc_hclk3 (default)
        1:pll1_q_ck
        2:pll2_r_ck
        3:per_ck */
    *RCC_D1CCIPR &= ~(3u<<4);

    if(pll2_enabled){
        /* pll2 to 208MHz */
        pll_2_start(PLLSRC_HSE, 2, 16, 0u, 104, 104, 1, 26000000u);
        *RCC_D1CCIPR |= (2u<<4);
    }

    *RCC_AHB4ENR |= (0x1u<<GPIOBEN) | (0x1u<<GPIOCEN) | (0x1u<<GPIOEEN);

    *RCC_AHB3ENR |= (0x1u << 14); // QSPIEN

    qspi_wait_busy();

    /*
    PB2   QSPI_CLK  AF9
    PC11  BK2_NCS  AF9
    PE7   BK2_IO0  AF10
    PE8   BK2_IO1  AF10
    PE9   BK2_IO2  AF10
    PE10  BK2_IO3  AF10
    */

    *GPIOB_MODER &= ~(0x3u<<(MODER2));
    *GPIOB_MODER |=  (0x2u<<(MODER2));
    *GPIOB_OSPEEDR |=(0x1u<<(OSPEED2));
    *GPIOB_AFRL  &= ~(0xFu<<(AFR2));
    *GPIOB_AFRL  |=  (0x9u<<(AFR2));

    *GPIOC_MODER &= ~(0x3u<<(MODER11));
    *GPIOC_MODER |=  (0x2u<<(MODER11));
    *GPIOC_OSPEEDR |=(0x1u<<(OSPEED11));
    *GPIOC_AFRH  &= ~(0xFu<<(AFR11));
    *GPIOC_AFRH  |=  (0x9u<<AFR11);

    *GPIOE_MODER &= ~((0x3u<<(MODER7))|(0x3u<<(MODER8))|(0x3u<<(MODER9))|(0x3u<<(MODER10)));
    *GPIOE_MODER |=   (0x2u<<(MODER7))|(0x2u<<(MODER8))|(0x2u<<(MODER9))|(0x2u<<(MODER10));
    *GPIOE_OSPEEDR |= (0x1u<<(OSPEED7))|(0x1u<<(OSPEED8))|(0x1u<<(OSPEED9))|(0x1u<<(OSPEED10));
    *GPIOE_AFRL  &= ~(0xFu<<AFR7);
    *GPIOE_AFRL  |=  (0xAu<<AFR7);
    *GPIOE_AFRH  &= ~((0xFu<<AFR8)|(0xFu<<AFR9)|(0xFu<<AFR10));
    *GPIOE_AFRH  |=    (0xAu<<AFR8)|(0xAu<<AFR9)|(0xAu<<AFR10);

    *QUADSPI_DCR |=
        (0x17<<16) /* 20:16 FSIZE   (2^(0x17+1))/1024/1024 = 16MB (spi flash size) */
      | (0x7u<<8)  /* 10:8  CSHT    chip select high time */
      | (0x1u<<0); /* 0     CKMODE  1 CLK stays high while NCS high (mode 3) */

    *QUADSPI_CR =
      (prescaler<<24)  /* 31:24 PRESCALER clock prescaler  */
           |(0x1fu<<8) /* 12:8  FTHRES FIFO threshold flag setting */
                       /*       level for in indirect mode  */
           |(0x1u<<7)  /*  7    FSEL 0:BK1 1:BK2 selection */
           |(0x0u<<6)  /*  6    DFM dual flash mode */
           |(0x1u<<4)  /*  4    SSHIFT 0: no shift 1: half cycle shift */
           |(0x0u<<3)  /*  3    TCEN enable timeout mode for memory */
                       /*       mapped mode to lower flash consumption */
           |(0x0u<<1)  /*  1    ABORT 1: abort requested */
           |(0x1u<<0); /*  0    EN QUADSPI enabled  */

    // Reset the device
    qspi_cmd_u32(SPI_MODE,WRITE,0x66,0,0); // 0x66 Enable Reset
    qspi_cmd_u32(SPI_MODE,WRITE,0x99,0,0); // 0x99 Reset Device

    // Enter QPI mode - first write to status register 2
    // then write 0x38
    uint8_t sr2 = qspi_cmd_u32(SPI_MODE,READ,READ_STATUS_REGISTER_2,0,1);
    qspi_cmd_u32(SPI_MODE,WRITE,WRITE_STATUS_REGISTER_2,sr2|0x02,1);
    qspi_cmd_u32(SPI_MODE,WRITE,0x38,0,0);
    qspi_cmd_u32(SPI_MODE,WRITE,0xC0,0x0,1);
}

void init_qspi_80MHz(void)
{
    pll2_enabled = 0;
    init_qspi(2);
}

void init_qspi_pll2_104MHz(void)
{
    pll2_enabled = 1;
    init_qspi(1);
}

void init_qspi_pll2(uint8_t prescaler)
{
    pll2_enabled = 1;
    init_qspi(prescaler);
}

void qspi_memory_map_mode(void)
{
    qspi_wait_busy();

    *QUADSPI_CCR =
            (0x3u<<26)  /* 27:26 FMODE   functional mode (3: memory mapped) */
           |(0x2u<<18)  /* 22:18 DCYC    number of dummy cycles */
           |(0x3u<<24)  /* 25:24 DMODE   data mode (3: data on 4 lines) */
           | COMMON_BIT_SETTINGS
           | 0xEB;     

    qspi_wait_busy();
}

int32_t qspi_read(uint32_t addr, void *buf, size_t len)
{
    if (addr + len > (16 * 1024 * 1024UL)) {
        return -1;
    }

    uint32_t *dst = (uint32_t *)buf;

    qspi_wait_busy();

    *QUADSPI_CCR =
        (0x1u<<28)  /* 28    SIOO    1:instruction sent only on 1st command */
       |(0x1u<<26)  /* 27:26 FMODE   1:read */
       |(0x3u<<24)  /* 25:24 DMODE   data mode (3: data on 4 lines) */
       |(0x2u<<18)  /* 22:18 DCYC    number of dummy cycles */
       | COMMON_BIT_SETTINGS
       | 0xEB;

    while (len) {

        uint32_t chunk = (len > 0x10000u) ? 0x10000u : len;

        *QUADSPI_DLR = chunk - 1u;
        *QUADSPI_AR  = addr;

        /* Read chunk bytes from DR */
        for (uint32_t i = 0; i < chunk; ++i) {
            /* Blocking read */
            dst[i] = *QUADSPI_DR ;
        }

        dst  += chunk;
        addr += chunk;
        len  -= chunk;
    }

    qspi_wait_busy();

    return 0;
}

/* Wait until flash is not busy (SR1.BUSY == 0) */
static void qspi_wait_chip_busy(void)
{
    for (;;) {
        uint8_t sr1 =
            (uint8_t)qspi_cmd_u32(QPI_MODE, READ, READ_STATUS_REGISTER_1, 0, 1);
        if ((sr1 & 0x01u) == 0u) {   /* BUSY == 0 */
            break;
        }
    }
}

/* Write Enable / Disable in QPI mode */
static inline void qspi_write_enable_qpi(void)
{
    /* 06h = WREN */
    qspi_cmd_u32(QPI_MODE, WRITE, 0x06u, 0, 0);
    /* Optional: wait until flash is ready again */
    qspi_wait_chip_busy();
}

static inline void qspi_write_disable_qpi(void)
{
    /* 04h = WRDI */
    qspi_cmd_u32(QPI_MODE, WRITE, 0x04u, 0, 0);
    qspi_wait_chip_busy();
}

/* Erase one block at addr with given opcode: 20h/52h/D8h (QPI 4-4-4) */
static void qspi_erase_block_qpi(uint8_t opcode, uint32_t addr)
{
    qspi_write_enable_qpi();

    qspi_wait_busy();   /* wait for QSPI peripheral idle */

    *QUADSPI_CCR =
         (0x0u<<26)  /* 27:26 FMODE = 0:indirect write */
        |(0x0u<<24)  /* 25:24 DMODE = 0:no data */
        | COMMON_BIT_SETTINGS
        | opcode;    /* 7:0   INSTR  = 20h/52h/D8h */

    *QUADSPI_AR = addr;

    /* Wait for erase to finish (can be long) */
    qspi_wait_chip_busy();
}

/* Smart erase using 64k / 32k / 4k as appropriate.
*/
static void qspi_erase_range(uint32_t addr, uint32_t size)
{
    if (size == 0u) {
        return;
    }

    // Compute [start, end) region that must be erased
    uint32_t start = addr;
    uint32_t end   = addr + size;

    // Align to 4k sector boundaries
    uint32_t cur   = start & ~0xFFFu;                 // down
    uint32_t end_aligned = (end + 0xFFFu) & ~0xFFFu;  // up

    while (cur < end_aligned) {
        uint32_t remaining = end_aligned - cur;

        // Prefer largest blocks when aligned and big enough
        if (((cur & 0xFFFFu) == 0u) && (remaining >= 0x10000u)) {
            // 64 KB block erase (D8h)
            qspi_erase_block_qpi(0xD8u, cur);
            cur += 0x10000u;
        }
        else if (((cur & 0x7FFFu) == 0u) && (remaining >= 0x8000u)) {
            // 32 KB block erase (52h)
            qspi_erase_block_qpi(0x52u, cur);
            cur += 0x8000u;
        }
        else {
            // 4 KB sector erase (20h)
            qspi_erase_block_qpi(0x20u, cur);
            cur += 0x1000u;
        }
    }
}

/* Page program (max 256 bytes) in QPI mode (4-4-4, opcode 02h).
   'data' is byte pointer; we pack 4 bytes into each 32-bit DR write.
*/
static void qspi_page_program_qpi(uint32_t addr, const uint8_t *data, uint32_t len)
{
    qspi_write_enable_qpi();

    qspi_wait_busy();

    *QUADSPI_DLR = len - 1u;

    *QUADSPI_CCR =
         (0x0u<<26)  /* 27:26 FMODE  = 0:indirect write */
        |(0x3u<<24)  /* 25:24 DMODE  = 3:data on 4 lines */
        | COMMON_BIT_SETTINGS
        | 0x02u;     /* 7:0   INSTR  = 02h Page Program */

    *QUADSPI_AR = addr;

    /* Use 32-bit writes to DR wherever possible */
    uint32_t full_words = len >> 2;      /* len / 4 */
    uint32_t rem_bytes  = len & 0x3u;    /* len % 4 */
    const uint8_t *p    = data;

    /* Full 32-bit words: pack 4 bytes -> 1 DR write */
    for (uint32_t i = 0u; i < full_words; ++i) {
        uint32_t v =
              (uint32_t)p[0]
            | ((uint32_t)p[1] << 8)
            | ((uint32_t)p[2] << 16)
            | ((uint32_t)p[3] << 24);
        *QUADSPI_DR = v;
        p += 4;
    }

    /* Tail bytes (1 to 3): pack into one extra word */
    if (rem_bytes != 0u) {
        uint32_t v = 0u;
        uint32_t shift = 0u;
        while (rem_bytes-- != 0u) {
            v |= (uint32_t)(*p++) << shift;
            shift += 8u;
        }
        *QUADSPI_DR = v;
    }

    qspi_wait_busy();

    qspi_wait_chip_busy();
}

/* Top-level: erase-then-write arbitrary range in QPI mode.
   src is 32-bit pointer in API, but we treat it as bytes internally.
*/
void qspi_write_flash_qpi(uint32_t addr, const uint32_t *src, uint32_t size)
{
    const uint8_t *src8 = (const uint8_t *)src;

    /* 1. Erase range first (4k/32k/64k mix) */
    qspi_erase_range(addr, size);

    /* 2. Program page by page (256-byte pages) */
    while (size > 0u) {

        uint32_t page_off  = addr & 0xFFu;        /* offset within 256-byte page */
        uint32_t page_left = 256u - page_off;     /* remaining bytes in this page */
        uint32_t chunk     = (size < page_left) ? size : page_left;

        qspi_page_program_qpi(addr, src8, chunk);

        addr  += chunk;
        src8  += chunk;
        size  -= chunk;
    }

    /* 3. Disable write for safety */
    qspi_write_disable_qpi();
}

