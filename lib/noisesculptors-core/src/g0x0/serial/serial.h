
#ifndef SERIAL_G0_H
#define SERIAL_G0_H

#include "init.h"

/* Pick exactly one of these four */
// #define USART1_PA10_RX_PA9_TX  /* AF1 on PA9/PA10 */
#define USART1_PB7_RX_PB6_TX   /* AF0 on PB6/PB7 */
// #define USART2_PA3_RX_PA2_TX   /* AF1 on PA2/PA3 */
// #define USART2_PA15_RX_PA14_TX /* AF1 on PA14/PA15 */

/* Public, H7-like simple API */
void init_serial(const clock_info_t *ci, unsigned baud);
void serial_write_char(int c);
int  serial_read_char(void);

/* If you want to explicitly use more than one at once, these are exposed: */
void init_usart1_pa10_pa9(const clock_info_t *ci, unsigned baud);
void init_usart1_pb7_pb6(const clock_info_t *ci, unsigned baud);
void init_usart2_pa3_pa2 (const clock_info_t *ci, unsigned baud);
void init_usart2_pa15_pa14(const clock_info_t *ci, unsigned baud);

void usart1_write_char(int c);
int  usart1_read_char(void);
void usart2_write_char(int c);
int  usart2_read_char(void);

#endif

