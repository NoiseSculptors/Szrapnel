
#include "dma.h"
#include "dmamux.h"
#include "gpio.h"
#include "init.h"
#include "printf.h"
#include "rcc.h"
#include "spi_i2s.h"
#include "serial.h"
#include "io.h"
#include <math.h>
#include <stdint.h>

/* PCM5102A is connected to:
   A4  I2S1_WS
   A5  I2S1_CK
   A7  I2S1_SDO
*/

#if SAMPLING_FREQ == 48
#define I2S_SAMPLE_RATE 48000.0f
#elif SAMPLING_FREQ == 96
#define I2S_SAMPLE_RATE 96000.0f
#elif SAMPLING_FREQ == 192
#define I2S_SAMPLE_RATE 192000.0f
#elif SAMPLING_FREQ == 384
#define I2S_SAMPLE_RATE 384000.0f
#endif

#define AUDIO_FRAMES    SAMPLING_FREQ * 50 // per half-buffer (stereo frames)
#define CHANNELS 2
#define TX_WORDS        (AUDIO_FRAMES * CHANNELS)

// Align to cache line; put in AXI SRAM (D2) in your linker if possible.
// On H7 with D-cache ON, either put this in a non-cacheable region or
// do cache maintenance (CleanDCache_by_Addr) when (re)filling.

// double-buffer: [0..TX_WORDS-1]=H1, [TX_WORDS..2*TX_WORDS-1]=H2
 int32_t i2s_txbuf[TX_WORDS*2];

static inline int32_t attenuate(float x, float amp)
{
    // x in [-1..1], amp in [0..1]
    float y = x * amp * 8388607.0f;             // 24-bit peak = 2^23−1
    if (y >  8388607.0f) y =  8388607.0f;
    if (y < -8388608.0f) y = -8388608.0f;
    return ((int32_t)lrintf(y));
}

static void fill_buffer(int32_t *dst, unsigned frames /* L+R frames */,
                        float freq_hz, float amp)
{
    const float two_pi = 6.28318530718f;
    static float phaseSin = 0.0f; // radians
    static float phaseSaw = 0.0f; // cycles
    const float stepSin = two_pi * freq_hz / I2S_SAMPLE_RATE;
    const float stepSaw = freq_hz / I2S_SAMPLE_RATE;

    for (unsigned i = 0; i < frames; ++i) {
        float sL = sinf(phaseSin);
        float sR = (2.0f * phaseSaw) - 1.0f;

        dst[2*i + 0] = attenuate(sL, amp); // Left
        dst[2*i + 1] = attenuate(sR, amp); // Right

        phaseSin += stepSin;  if (phaseSin >= two_pi) phaseSin -= two_pi;
        phaseSaw += stepSaw;  if (phaseSaw >= 1.0f)   phaseSaw -= 1.0f;
    }
}

// DMA/DMAMUX low-level start
static void i2s1_dma_start_tx(int32_t *buf, unsigned words_total)
{
  // 1) Enable clocks: DMAMUX1 + DMA1 (AHB1)
  #ifndef DMAMUX1EN
  #define DMAMUX1EN 0
  #endif
  #ifndef DMA1EN
  #define DMA1EN    0
  #endif
  *RCC_AHB1ENR |= (1u<<DMAMUX1EN);
  *RCC_AHB1ENR |= (1u<<DMA1EN);

  // 2) Route DMAMUX1 Channel 5 to SPI1_TX (request ID 38)
  //    DMAMUX channel index equals DMA stream index for DMA1.
  #define SPI1_TX_DMA 38u // page 695 in RM0433
  *DMAMUX1_C5CR = 0; // reset chan
  *DMAMUX1_C5CR = (SPI1_TX_DMA & 0x7Fu); // DMAREQ_ID[6:0], no sync

  // 3) Configure DMA1 Stream 5 as M2P, circular, 32->32 bit, inc memory
  //    Clear and wait EN=0
  *DMA1_S5CR &= ~1u;
  while (*DMA1_S5CR & 1u) {}

  // Peripheral address: SPI1_TXDR
  *DMA1_S5PAR  = (uint32_t)SPI1_TXDR;

  // Memory address
  *DMA1_S5M0AR = (uint32_t)buf;

  // Number of 32-bit words
  *DMA1_S5NDTR = words_total;

  // FIFO: enable, full threshold (optional but good for steady burst)
  // FCR bits: DMDIS=bit2 (1=enable FIFO), FTH[1:0]=3 (full)
  *DMA1_S5FCR = (1u<<2) | (3u<<0);

  // CR fields (DMA v2 stream):
  // EN(0)=0, DMEIE(1)=0, TEIE(2)=1, HTIE(3)=0, TCIE(4)=0 (no IRQ; pure
  // circular) PFCTRL(5)=0 (DMA is flow ctrl), DIR(7:6)=01 (M2P), CIRC(8)=1,
  // PINC(9)=0, MINC(10)=1 PSIZE(12:11)=10 (32-bit), MSIZE(14:13)=10 (32-bit)
  // PL(17:16)=3 (very high) CHSEL(27:25)=0 (unused with DMAMUX)
  uint32_t cr = 0;
  cr |= (1u<<2);              // TEIE
  cr |= (1u<<8);              // CIRC
  cr |= (1u<<10);             // MINC
  cr |= (1u<<11) | (1u<<12);  // PSIZE=32b (bit12), MSIZE=32b (bit14:13 ->
                              // set bits 13+14)
  cr |= (1u<<13) | (1u<<14);
  cr |= (1u<<16) | (1u<<17);  // PL=VeryHigh
  cr |= (1u<<6);              // DIR = 01 (M2P)
  *DMA1_S5CR = cr;

  // 4) Enable TX-DMA in SPI (TXDMAEN in CFG1), then enable SPI and start I2S
  #define SPI_CFG1_TXDMAEN (1u<<15)
  *SPI1_CFG1 |= SPI_CFG1_TXDMAEN;

  // Enable stream
  *DMA1_S5CR |= 1u;

}

int main(void)
{
    init_clock();

    /* gives 24.5760MHz on my scope, fin: 13MHz, VCO: ca.196.6MHz*/
    // pll_2_start(PLLSRC_HSE, 2, 15, 1011, 2, 0, 0, 26000000u);

    /* without using FRACN, ca 24.578MHz */
    pll_2_start(PLLSRC_HSE, 16, 121, 0, 2, 0, 0, 26000000u);

    io_dac_init();
    
    // Prefill the circular buffer with 1 kHz test tone
    fill_buffer(&i2s_txbuf[0], TX_WORDS, 1000.0f, 0.5f);
    
    i2s1_dma_start_tx(i2s_txbuf, TX_WORDS); // stream the buffer circularly
    
    while(1){};
    
    return 0;
}

