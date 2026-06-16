
#include "delay.h"
#include "dma.h"
#include "dmamux.h"
#include "gpio.h"
#include "io.h"
#include "nvic.h"
#include "nvic_exti.h"
#include "init.h"
#include "printf.h"
#include "rcc.h"
#include "rng.h"
#include "spi_i2s.h"

/* PCM5102A is connected to:
   A4  I2S1_WS
   A5  I2S1_CK
   A7  I2S1_SDO
*/

static volatile uint8_t free0 = 0;
static volatile uint8_t free1 = 0;
static inline void sev(void) { __asm__ volatile ("sev" ::: "memory"); }
static inline void wfe(void) { __asm__ volatile ("wfe" ::: "memory"); }

#if 0
static volatile uint8_t next_cpu_buffer = 0; // 0 = CPU fills buf0, 1 = CPU fills buf1
static volatile uint8_t buffer_ready_to_fill = 1; // Start flagged as ready!
#endif

/* DMA buffers */
__attribute__((aligned(32))) static int32_t dma_buf0[SAMPLES];
__attribute__((aligned(32))) static int32_t dma_buf1[SAMPLES];

// DMA: DBM + circular ping-pong
static void io_dac_dma_init(void)
{
    // Prime both halves (silence to start)
    for(size_t i = 0;i<(sizeof dma_buf0/sizeof dma_buf0[0]);++i)dma_buf0[i]=0;
    for(size_t i = 0;i<(sizeof dma_buf1/sizeof dma_buf1[0]);++i)dma_buf1[i]=0;

    *RCC_AHB1ENR |= (1u<<0); // DMA1EN 0

    // Route DMAMUX1 Channel 5 to SPI1_TX (request ID 38)
    // DMAMUX channel index equals DMA stream index for DMA1.
#define SPI1_TX_DMA 38u // page 695 in RM0433
    *DMAMUX1_C5CR = 0; // reset chan
    *DMAMUX1_C5CR = (SPI1_TX_DMA & 0x7Fu); // DMAREQ_ID[6:0], no sync

    // Make sure stream disabled
    *DMA1_S5CR &= ~1u;
    //Bit 0 EN: Stream enable/flag stream ready when read low
    while (*DMA1_S5CR & 1u) {}

    // Addresses
    *DMA1_S5PAR  = (uint32_t)SPI1_TXDR;
    *DMA1_S5M0AR = (uint32_t)dma_buf0;
    *DMA1_S5M1AR = (uint32_t)dma_buf1;
    *DMA1_S5NDTR = SAMPLES; // words per HALF

    // FIFO: enable, full threshold
    *DMA1_S5FCR = (1u<<2)  // DMDIS
                | (3u<<0); // FTH

    uint32_t cr = 0;
    cr |= (1u<<18)  // DBM (double buffer)
        | (3u<<16)  // PL = Very High
        | (2u<<13)  // MSIZE=32
        | (2u<<11)  // PSIZE=32
        | (1u<<10)  // MINC
        | (1u<<8)   // CIRC
        | (1u<<6)   // DIR M2P
        | (1u<<4)   // TCIE
        | (1u<<2);  // TEIE

    *DMA1_S5CR = cr;

    // Enable SPI TX DMA requests
    #define SPI_CFG1_TXDMAEN (1u<<15)
    *SPI1_CFG1 |= SPI_CFG1_TXDMAEN;

//  NVIC_SetPriority(DMA1_Stream5_IRQn, 5);
    irq_enable(IRQ_DMA_STR5);

    // Enable DMA stream, then enable + start I2S
    *DMA1_S5CR |= 1u;
    *SPI1_CR1  |= (1u<<0);  // SPE
    *SPI1_CR1  |= (1u<<9);  // CSTART
}

// ======= Wait for a free half, fill it, and go idle again =======
static inline void wait_for_free_half(int *idx_out, int32_t **ptr_out)
{
    for (;;) {
        if (free0){
            *idx_out = 0;
            *ptr_out = dma_buf0;
            return;
        }
        if (free1){
            *idx_out = 1;
            *ptr_out = dma_buf1;
            return;
        }
        wfe(); // sleep until DMA IRQ does SEV
    }
}

void io_dac_init(void)
{
    pll_2_start(PLLSRC_HSE, 16, 181, 3968, 1, 128, 128, 26000000u);

    delay_ms(1);

    /* start gpiox clocks */
    *RCC_AHB4ENR |= (1<<GPIOAEN);

    gpio_ctrl(GPIOA, GPIO_OSPEED, GPIO4|GPIO5|GPIO7, OSPEED_MEDIUM);
    gpio_ctrl(GPIOA, GPIO_MODE,   GPIO4|GPIO5|GPIO7, MODE_AF);
    gpio_ctrl(GPIOA, GPIO_AFRL,   GPIO4|GPIO5|GPIO7, AF5);

    /* start spi1 clock */
#define SPI1EN 12
    *RCC_APB2ENR |= (1<<SPI1EN);

    /* disable I2S */
    *SPI1_CR1 &= ~(1<<0); // SPE

#define SPI123SEL 12
    /* erase SPI123SEL bits */
    *RCC_D2CCIP1R &= ~(0b111<<SPI123SEL);
    /* 1:pll2_p_ck as SPI(I2S)1,2,3 kernel clock */
    *RCC_D2CCIP1R |= (1u<<SPI123SEL);

#define I2SMOD  0
#define I2SCFG  1 /* 010: master - transmit */
#define I2SSTD  4 /* 0:I2S Philips, 1: MSB justified, 2: LSB, 3: PCM standard */
#define PCMSYNC 7
#define DATALEN 8
#define CHLEN   10
#define I2SDIV  16  
    /* enable I2S mode, Philips mode,  */
    *SPI1_I2SCFGR = 0;
    *SPI1_I2SCFGR |=
#if   SAMPLING_FREQ == 48
                     (48 << I2SDIV)   |
#elif SAMPLING_FREQ == 96
                     (24 << I2SDIV)   |
#elif SAMPLING_FREQ == 192
                     (12 << I2SDIV)   |
#elif SAMPLING_FREQ == 384
                     (6 << I2SDIV)   |
#else
#  error "Unsupported SAMPLING_FREQ"
#endif
                     (0b1<<CHLEN)    | /* 0-16bit 1-32bit */
                     (0b10<<DATALEN) | /* 0-16bit 1-24bit 2-32bit */
                     (0b0<<PCMSYNC)  |
                     (0b00<<I2SSTD)  |
                     (0b010<<I2SCFG) |
                     (0b1<<I2SMOD);

    /* enable I2S */
    *SPI1_CR1 |= (1<<0); // SPE

#define CSTART 9
    *SPI1_CR1 |= (1<<CSTART);


    io_dac_dma_init();

}

void IRQ_DMA_STR5_Handler(void)
{
#if 1
    uint32_t hisr = *DMA1_HISR;

#define DMA_HISR_TCIF5   11
#define DMA_HIFCR_CTCIF5 11

    if (hisr & (1<<DMA_HISR_TCIF5)) {
        *DMA1_HIFCR |= (1<<DMA_HIFCR_CTCIF5);  // clear TC

        // After TC, CT selects the NEXT target → the OTHER half just finished.
        uint32_t ct = (*DMA1_S5CR >> 19) & 1u;
        if(ct)
            free0 = 1;
        else
            free1 = 1;

        sev(); // wake main loop if it's in WFE
    }
#endif
#if 0
    uint32_t hisr = *DMA1_HISR;

    if (hisr & (1 << 11)) { // TCIF5
        *DMA1_HIFCR |= (1 << 11);  // Clear TC flag
        buffer_ready_to_fill = 1;
        sev(); // Wake up main loop
    }
#endif
}

static void SCB_CleanDCache_by_Addr(uint32_t *addr, int32_t dsize)
{
    // The Cortex-M7 L1 cache line size is always 32 bytes
    const uint32_t cache_line_size = 32u;

    uint32_t start_addr = (uint32_t)addr;
    uint32_t end_addr   = start_addr + (uint32_t)dsize;

    // Align the starting address downward to the nearest 32-byte boundary
    start_addr &= ~(cache_line_size - 1u);

    // Loop through the memory range, cleaning one cache line at a time
    while (start_addr < end_addr) {
        // DCCMVAC register address is 0xE000EF68
        // Writing the address to it flushes that cache line to RAM
        *(volatile uint32_t *)0xE000EF68u = start_addr;

        start_addr += cache_line_size;
    }

    // Memory Barrier to ensure the cache operations finish before DMA starts
    __asm__ volatile ("dsb ish" ::: "memory");
    __asm__ volatile ("isb"     ::: "memory");
}

void dac_dma_loop(){
    for (;;) {
#if 0
0       while (!buffer_ready_to_fill) {
            wfe();
        }
        buffer_ready_to_fill = 0;

        int32_t *p = (next_cpu_buffer == 0) ? dma_buf0 : dma_buf1;

        audio_fill_buffer(p, SAMPLES);

     // MANDATORY FOR STM32H7: Push data from L1 Cache down to RAM so DMA can see it
     // If you don't have a dedicated function, use the CMSIS standard macro:
     // SCB_CleanDCache_by_Addr((uint32_t*)p, SAMPLES * sizeof(int32_t));
        next_cpu_buffer = !next_cpu_buffer;
#endif
#if 1
        int idx; int32_t *p;
        wait_for_free_half(&idx, &p);

        audio_fill_buffer(p, SAMPLES);

        // If buffers are cacheable, clean only this half. If MPU made them
        // non-cacheable, this is a no-op.  CLEAN_DCACHE_RANGE(p, BUF_WORDS *
        // sizeof(int32_t));

        // Release the flag
        if (idx == 0) free0 = 0; else free1 = 0;
#endif
    }
}

