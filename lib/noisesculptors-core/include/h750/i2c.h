#ifndef I2C_H
#define I2C_H

#include "init.h"
#include <stdint.h>

void i2c_write_bytes(uint32_t i2c_periph,
                     uint8_t addr,
                     const uint8_t *data,
                     uint16_t n);

void i2c_write_header_and_bytes_long(uint32_t i2c_base,
                                     uint8_t addr,
                                     uint8_t header,
                                     const uint8_t *data,
                                     uint32_t n);

void init_i2c1_pb6_pb7(const clock_info_t *);
void init_i2c1_pb8_pb9(const clock_info_t *);
void init_i2c2_pb10_pb11(const clock_info_t *);
void init_i2c3_pa8_pc9(const clock_info_t *);
void init_i2c4_pd12_pd13(const clock_info_t *);
void init_i2c4_pb6_pd7(const clock_info_t *);
void init_i2c4_pb8_pd9(const clock_info_t *);

#define I2C_CR1(ADDR)      ((volatile uint32_t *)(ADDR + 0x00))
#define I2C_CR2(ADDR)      ((volatile uint32_t *)(ADDR + 0x04))
#define I2C_OAR1(ADDR)     ((volatile uint32_t *)(ADDR + 0x08))
#define I2C_OAR2(ADDR)     ((volatile uint32_t *)(ADDR + 0x0C))
#define I2C_TIMINGR(ADDR)  ((volatile uint32_t *)(ADDR + 0x10))
#define I2C_TIMEOUTR(ADDR) ((volatile uint32_t *)(ADDR + 0x14))
#define I2C_ISR(ADDR)      ((volatile uint32_t *)(ADDR + 0x18))
#define I2C_ICR(ADDR)      ((volatile uint32_t *)(ADDR + 0x1C))
#define I2C_PECR(ADDR)     ((volatile uint32_t *)(ADDR + 0x20))
#define I2C_RXDR(ADDR)     ((volatile uint32_t *)(ADDR + 0x24))
#define I2C_TXDR(ADDR)     ((volatile uint32_t *)(ADDR + 0x28))

#define I2C1_CR1           (volatile uint32_t *)(I2C1 + 0x00)
#define I2C1_CR2           (volatile uint32_t *)(I2C1 + 0x04)
#define I2C1_OAR1          (volatile uint32_t *)(I2C1 + 0x08)
#define I2C1_OAR2          (volatile uint32_t *)(I2C1 + 0x0C)
#define I2C1_TIMINGR       (volatile uint32_t *)(I2C1 + 0x10)
#define I2C1_TIMEOUTR      (volatile uint32_t *)(I2C1 + 0x14)
#define I2C1_ISR           (volatile uint32_t *)(I2C1 + 0x18)
#define I2C1_ICR           (volatile uint32_t *)(I2C1 + 0x1C)
#define I2C1_PECR          (volatile uint32_t *)(I2C1 + 0x20)
#define I2C1_RXDR          (volatile uint32_t *)(I2C1 + 0x24)
#define I2C1_TXDR          (volatile uint32_t *)(I2C1 + 0x28)

#define I2C2_CR1           (volatile uint32_t *)(I2C2 + 0x00)
#define I2C2_CR2           (volatile uint32_t *)(I2C2 + 0x04)
#define I2C2_OAR1          (volatile uint32_t *)(I2C2 + 0x08)
#define I2C2_OAR2          (volatile uint32_t *)(I2C2 + 0x0C)
#define I2C2_TIMINGR       (volatile uint32_t *)(I2C2 + 0x10)
#define I2C2_TIMEOUTR      (volatile uint32_t *)(I2C2 + 0x14)
#define I2C2_ISR           (volatile uint32_t *)(I2C2 + 0x18)
#define I2C2_ICR           (volatile uint32_t *)(I2C2 + 0x1C)
#define I2C2_PECR          (volatile uint32_t *)(I2C2 + 0x20)
#define I2C2_RXDR          (volatile uint32_t *)(I2C2 + 0x24)
#define I2C2_TXDR          (volatile uint32_t *)(I2C2 + 0x28)

#define I2C3_CR1           (volatile uint32_t *)(I2C3 + 0x00)
#define I2C3_CR2           (volatile uint32_t *)(I2C3 + 0x04)
#define I2C3_OAR1          (volatile uint32_t *)(I2C3 + 0x08)
#define I2C3_OAR2          (volatile uint32_t *)(I2C3 + 0x0C)
#define I2C3_TIMINGR       (volatile uint32_t *)(I2C3 + 0x10)
#define I2C3_TIMEOUTR      (volatile uint32_t *)(I2C3 + 0x14)
#define I2C3_ISR           (volatile uint32_t *)(I2C3 + 0x18)
#define I2C3_ICR           (volatile uint32_t *)(I2C3 + 0x1C)
#define I2C3_PECR          (volatile uint32_t *)(I2C3 + 0x20)
#define I2C3_RXDR          (volatile uint32_t *)(I2C3 + 0x24)
#define I2C3_TXDR          (volatile uint32_t *)(I2C3 + 0x28)

#define I2C4_CR1           (volatile uint32_t *)(I2C4 + 0x00)
#define I2C4_CR2           (volatile uint32_t *)(I2C4 + 0x04)
#define I2C4_OAR1          (volatile uint32_t *)(I2C4 + 0x08)
#define I2C4_OAR2          (volatile uint32_t *)(I2C4 + 0x0C)
#define I2C4_TIMINGR       (volatile uint32_t *)(I2C4 + 0x10)
#define I2C4_TIMEOUTR      (volatile uint32_t *)(I2C4 + 0x14)
#define I2C4_ISR           (volatile uint32_t *)(I2C4 + 0x18)
#define I2C4_ICR           (volatile uint32_t *)(I2C4 + 0x1C)
#define I2C4_PECR          (volatile uint32_t *)(I2C4 + 0x20)
#define I2C4_RXDR          (volatile uint32_t *)(I2C4 + 0x24)
#define I2C4_TXDR          (volatile uint32_t *)(I2C4 + 0x28)

#endif
