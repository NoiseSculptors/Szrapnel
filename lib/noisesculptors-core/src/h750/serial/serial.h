
#ifndef SERIAL_H
#define SERIAL_H

#include "init.h"
#include <stddef.h>

/*
Used by qspi on Calibrator750 so choose some other pair:
PC11 QUADSPI_BK2_NCS
PE7  QUADSPI_BK2_IO0
PE8  QUADSPI_BK2_IO1

Used on Rocket Calibrator prototype
USART3 PB11_RX PB10_TX ... RX TX 3V3 GND
*/

void init_lpuart1_pa10_pa9(const clock_info_t *, unsigned);
void init_lpuart1_pb7_pb6(const clock_info_t *, unsigned);
void init_uart4_pa1_pa0(const clock_info_t *, unsigned);
void init_uart4_pb8_pb9(const clock_info_t *, unsigned);
void init_uart4_pc11_pc10(const clock_info_t *, unsigned);
void init_uart4_pd0_pd1(const clock_info_t *, unsigned);
void init_uart5_pb12_pb13(const clock_info_t *, unsigned);
void init_uart5_pb5_pb6(const clock_info_t *, unsigned);
void init_uart5_pd2_pc12(const clock_info_t *, unsigned);
void init_uart7_pb3_pb4(const clock_info_t *, unsigned);
void init_uart7_pe7_pe8(const clock_info_t *, unsigned);
void init_uart7_pa8_pa15(const clock_info_t *, unsigned);
void init_uart8_pe0_pe1(const clock_info_t *, unsigned);
void init_usart1_pa10_pa9(const clock_info_t *, unsigned);
void init_usart1_pb15_pb14(const clock_info_t *, unsigned);
void init_usart1_pb7_pb6(const clock_info_t *, unsigned);
void init_usart2_pa3_pa2(const clock_info_t *, unsigned);
void init_usart2_pd6_pd5(const clock_info_t *, unsigned);
void init_usart3_pb11_pb10(const clock_info_t *, unsigned);
void init_usart3_pc11_pc10(const clock_info_t *, unsigned);
void init_usart3_pd9_pd8(const clock_info_t *, unsigned);
void init_usart6_pc7_pc6(const clock_info_t *, unsigned);

void lpuart1_write_char(int c);
void uart4_write_char(int c);
void uart5_write_char(int c);
void uart7_write_char(int c);
void uart8_write_char(int c);
void usart1_write_char(int c);
void usart2_write_char(int c);
void usart3_write_char(int c);
void usart6_write_char(int c);

int lpuart1_read_char(void);
int uart4_read_char(void);
int uart5_read_char(void);
int uart7_read_char(void);
int uart8_read_char(void);
int usart1_read_char(void);
int usart2_read_char(void);
int usart3_read_char(void);
int usart6_read_char(void);

#endif

