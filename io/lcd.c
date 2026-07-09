
#include "delay.h"
#include "dma.h"
#include "dmamux.h"
#include "gpio.h"
#include "i2c.h"
#include "io.h"
#include "nvic.h"
#include "printf.h"
#include "rcc.h"
#include "rng.h"
#include "rng.h"
#include "spi.h"
#include "serial.h"
#include "syscall.h"
#include "systick.h"
#include "timer.h"
#include <string.h>

#define PRINTF_BUFSZ (20*10)+1 // 20x10 (8x8px) characters

uint16_t fb[WIDTH*HEIGHT] __attribute__((section(".dma")));
uint8_t ramwr[1] __attribute__((section(".dma")));
static uint8_t cmd[4] __attribute__((section(".dma"))); 
static uint8_t parameter[4] __attribute__((section(".dma"))); 
volatile uint32_t lcd_busy;
extern uint8_t font_8x8_linux[256*8];

__attribute__((section(".itcm"),used))
uint16_t *lcd_get_fb(void)
{
    return fb;
}

__attribute__((section(".itcm"),used))
void lcd_flush_fb(void)
{
    ramwr[0] = 0x2c;
    *GPIOD_BSRR = 0x2000000; // CS low - sending command
    lcd_dma_send(ramwr, 1);    // sending one byte - ramwr
    delay_us(10);
    *GPIOD_BSRR = 0x200;     // CS high - sending data
    lcd_dma_send((uint8_t*)fb, 25600);
    delay_ms(2);
}

__attribute__((section(".itcm"),used))
void lcd_flush_fb_async(void)
{
    ramwr[0] = 0x2c;
    *GPIOD_BSRR = 0x2000000; // CS low - sending command
    lcd_dma_send(ramwr, 1);    // sending one byte - ramwr
    delay_us(1);
    *GPIOD_BSRR = 0x200;     // CS high - sending data
    lcd_dma_send((uint8_t*)fb, 25600);
}

__attribute__((section(".itcm"),used))
static void lcd_command(uint8_t n)
{
    /* DC low */
    *GPIOD_BSRR = 0x2000000;
    cmd[0] = n;
    lcd_dma_send(cmd, 1);
}

__attribute__((section(".itcm"),used))
static void lcd_data(uint8_t *arr, uint16_t n)
{
    /* DC high */
    *GPIOD_BSRR = 0x200; 
    lcd_dma_send(arr, n);
}

void lcd_led_brightness(uint8_t pct)
{
    static uint8_t initialized;
    if(!initialized){
        *TIM12_PSC   = 1199;        // prescaler for 2kHz
        *TIM12_ARR   = 99;         // set period
        *TIM12_CCMR1 &= ~(7U << 4); // clear OC1M[6:4]
        *TIM12_CCMR1 |= (6U << 4);
        *TIM12_CCER  |= (1U << 0);
        *TIM12_CR1   |= (1U << 0);  // CEN: start counter
        initialized = 1;
    }
    /* P-MOSFET is used so
           0=max light
           20=20% duty PWM
           50=50% duty PWM
           100=turned off LED */
    *TIM12_CCR1  = 100u-pct;
}

static void lcd_spi_init(void)
{
    /* sharing the same PLL2_P with I2S/DAC */
    pll_2_start(PLLSRC_HSE, 16, 181, 3968, 1, 128, 128, 26000000u);

    *RCC_D2CCIP1R &= ~(7u<<12);
    *RCC_D2CCIP1R |= (1u<<12); // PLL2Q as clock source for SPI123

    *SPI2_CFG1 =
        (0 << 28) |     // 30:28 baud rate, 0:max speed, 7:slowest
        (8-1);          // bits per frame (n-1)
    *SPI2_CFG2 =
        (1u << 30) |    // SSOM output management in master mode
        (1u << 29) |    // SSOE  hardware NSS output
        (1u << 22) |    // MASTER
        (1u << 17) ;    // COMM  half-duplex
    *SPI2_CR1 =
        (1<<11);        // HDDIR half-duplex, transmitter only
}

static void lcd_pwr(uint8_t state)
{
    /* P-MOSFET is used so D10 low turns on the LCD */
    if(state)
        *GPIOD_BSRR = 0x4000000; // erase D10 
    else
        *GPIOD_BSRR = 0x400;     // set D10 
}

static void lcd_init_dma(void){

    uint32_t dma_request_spi2_tx_dma = 40;

    // Clocks
    *RCC_AHB1ENR |= (1u<<0); // DMA1EN

    // DMAMUX: route spi2_tx_dma to DMA1 Stream0
    *DMAMUX1_C0CR = 0; // clear first
    *DMAMUX1_C0CR = (dma_request_spi2_tx_dma & 0x7F);

    // Make sure stream disabled
    *DMA1_S0CR &= ~1u;
    while (*DMA1_S0CR & 1u) {}

    // Addresses
    *DMA1_S0PAR  = (uint32_t)SPI2_TXDR; // peripheral = SPI->TXDR

    // CR: M2P | MINC | 8/8-bit | priority | (optional) TCIE/TEIE
    *DMA1_S0CR = (1u<<10) // MINC=1
        | (1u<<6); // DIR = Memory to peripheral
}


#define DELAY 1
void io_lcd_init(void)
{

    *RCC_AHB4ENR |= (1u<<GPIOBEN)| (1u<<GPIODEN);
/********** Display ***********************************************************/
    *RCC_APB1LENR  |=  (1u<<14)| // SPI2EN
                       (1u<<6);  // TIM12EN

 // *RCC_APB1LRSTR |=  (1u<<14); // SPI2RST
 // *RCC_APB1LRSTR &= ~(1u<<14); // SPI2RST

    gpio_ctrl(GPIOD, GPIO_MODE, GPIO8|GPIO9|GPIO10, MODE_OUTPUT); // EN RESET DC
    gpio_ctrl(GPIOB, GPIO_MODE, GPIO12|GPIO13|GPIO14|GPIO15, MODE_AF);
    gpio_ctrl(GPIOB, GPIO_AFRH, GPIO12|GPIO13|GPIO15, AF5); // SPI2 NSS SCK MOSI
    gpio_ctrl(GPIOB, GPIO_AFRH, GPIO14, AF2); // TIM12_CH1
    gpio_ctrl(GPIOB, GPIO_OSPEED, GPIO12|GPIO13|GPIO15, OSPEED_MEDIUM);
    
    lcd_init_dma();
    lcd_spi_init();

    /* LCD hard reset */
    *GPIOD_ODR &= ~(1<<8);
    delay_ms(10);
    *GPIOD_ODR |= (1<<8);
    delay_ms(120);

    // lcd_pwr(0); delay_ms(100); lcd_pwr(1); // power cycle
    // lcd_command(0x01); delay_ms(150);    // SWRESET
    lcd_command(0x11); delay_ms(150);    // SLEEPOUT

    lcd_command(0x3A);       // COLMOD
    delay_ms(DELAY);
    parameter[0]=0x05;
    lcd_data(parameter,1);   // COLMOD = 65K 16bit/pixesvel
    delay_ms(DELAY);

    lcd_command(0x36);       // MADCTL
    delay_ms(DELAY);
    //                 MV
    //              MY | RGB
    //               | | |
    uint8_t madctl=0b01100000;                
    //                | | |
    //               MX | MH
    //                  ML

    parameter[0] = madctl;

    lcd_data(parameter, 1); 
    delay_ms(DELAY);

    uint16_t width = WIDTH-1;
    uint16_t height = HEIGHT-1;
    uint8_t offset = 24;

    // caset
    parameter[0]=0;                        // X Address Start 15:8
    parameter[1]=0;                   // X Address Start  7:0
    parameter[2]=(uint8_t)(width>>8);      // X Address End   15:8
    parameter[3]=(uint8_t)(width & 0xff);  // X Address End 7:0

    lcd_command(0x2A); 
    delay_ms(DELAY);

    lcd_data(parameter, 4); 
    delay_ms(DELAY);

    // raset
    parameter[0]=0;                        // Y Address Start 15:8
    parameter[1]=offset;                   // Y Address Start  7:0
    parameter[2]=(uint8_t)(height>>8);     // Y Address End 15:8
    parameter[3]=(uint8_t)(height& 0xff)+offset;  // Y Address End  7:0

    lcd_command(0x2B); 
    delay_ms(DELAY);

    lcd_data(parameter, 4); 
    delay_ms(DELAY);

    lcd_command(0x29); delay_ms(20);    // DISPLAY ON
    delay_ms(DELAY);

    lcd_led_brightness(100);
    lcd_flush_fb();
}

__attribute__((section(".itcm"),used))
void lcd_dma_send(uint8_t *arr, uint16_t n)
{
    // disable TXDMA
    *SPI2_CFG1  &= ~(1u<<15);  // TXDMAEN disable
    *SPI2_CR1 &= ~1;       // SPI disable

    *DMA1_LIFCR = (1u<<5)  // CTCIF0
        |(1u<<3); // CTEIF0

    *DMA1_S0M0AR = (uint32_t)arr;
    *DMA1_S0NDTR = n;

    /* trigger transfer */
    *SPI2_CFG1 |= (1<<15); // TXDMAEN enable
    *DMA1_S0CR |= (1<<0);  // enable
    *SPI2_CR1 |= 1;        // SPI enable
    *SPI2_CR1 |= (1<<9);   // CSTART
}

/* rgb888 -> rgb565
   r:0001111100000000
   b:0000000011111000 
   g:1110000000000111 */
__attribute__((section(".itcm"),used))
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((g & 0x1C) << 3 | (r >> 3)) << 8 | (b & 0xF8) | g >> 5;
}

__attribute__((section(".itcm"), used))
int lcd_printf(int x, int y, int xmul, int ymul,
               uint16_t text_color,
               uint16_t bg_color, const char *fmt, ...)
{
    if (xmul < 1) xmul = 1;
    if (ymul < 1) ymul = 1;

    char buf[PRINTF_BUFSZ];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n < 0)
        return n;

    buf[sizeof(buf) - 1] = '\0';

    if (buf[0] == '\0')
        return n;

    int x_origin = x;
    const int glyph_w = 8 * xmul;
    const int glyph_h = 8 * ymul;

    for (const char *p = buf; *p; p++) {
        if (*p == '\n' || x + glyph_w > WIDTH) {
            y += glyph_h;
            x = x_origin;
            if (y + glyph_h > HEIGHT)
                break;
            if (*p == '\n')
                continue;
        }

        const uint8_t *g = font_8x8_linux + ((uint8_t)*p) * 8;

        for (int r = 0; r < 8; r++) {
            uint8_t font_byte = g[r];

            for (int ry = 0; ry < ymul; ry++) {
                uint16_t *dst = &fb[(y + r * ymul + ry) * WIDTH + x];

                if (xmul == 1) {
                    for (int c = 0; c < 8; c++) {
                        dst[c] = (font_byte & (0x80 >> c)) ? text_color : bg_color;
                    }
                } else {
                    for (int c = 0; c < 8; c++) {
                        uint16_t color = (font_byte & (0x80 >> c)) ? text_color : bg_color;
                        uint16_t *px = dst + c * xmul;
                        for (int rx = 0; rx < xmul; rx++) {
                            px[rx] = color;
                        }
                    }
                }
            }
        }

        x += glyph_w;
    }

    return n;
}

__attribute__((section(".itcm"),used))
void lcd_pixel(uint8_t x, uint8_t y, uint16_t color)
{
	fb[(y*WIDTH)+x] = color;
}

__attribute__((section(".itcm"),used))
void lcd_clear(uint16_t color)
{
    for(int i=0;i<WIDTH*HEIGHT;i++)
        fb[i]=color;
}

__attribute__((section(".itcm"),used))
static void lcd_clear_flush(uint16_t color)
{
    for(int i=0;i<WIDTH*HEIGHT;i++)
        fb[i]=color;
    lcd_flush_fb();
}

__attribute__((section(".itcm"),used))
inline void lcd_draw_waveform(
    const int32_t* audio_buf,   // Interleaved L/R signed 32-bit samples stored in uint32_t
    int32_t audio_frames         // Number of stereo frames
)
{
    const uint16_t fgL = rgb565(255, 255, 0);
    const uint16_t fgR = rgb565(0, 255, 255);
    const uint16_t bg = rgb565(0, 64, 0);

    lcd_clear(bg);

    const int32_t mid_y_l = HEIGHT / 4;
    const int32_t mid_y_r = (3 * HEIGHT) / 4;
    const int32_t max_amplitude = (HEIGHT / 4) - 2;  // bigger waveform

    // Fixed-point step for mapping audio frames to screen width
    const uint32_t fp_step = ((uint32_t)audio_frames << 16) / WIDTH;
    uint32_t fp_index = 0;

    for (int32_t x = 0; x < WIDTH; x++) {
        const int32_t sample_idx = fp_index >> 16;

        // Interleaved stereo: [L0, R0, L1, R1, ...]
        const int32_t sample_l = (int32_t)audio_buf[sample_idx * 2];
        const int32_t sample_r = (int32_t)audio_buf[sample_idx * 2 + 1];

        const int32_t h_l = (((sample_l >> 16) * max_amplitude) >> 15);
        const int32_t h_r = (((sample_r >> 16) * max_amplitude) >> 15);

        const int32_t y_l = mid_y_l + h_l;
        const int32_t y_r = mid_y_r + h_r;

        const int32_t draw_x = WIDTH - 1 - x;

        if ((unsigned)y_l < (unsigned)HEIGHT) {
            fb[y_l * WIDTH + draw_x] = fgL;
        }

        if ((unsigned)y_r < (unsigned)HEIGHT) {
            fb[y_r * WIDTH + draw_x] = fgR;
        }

        fp_index += fp_step;
    }

}

