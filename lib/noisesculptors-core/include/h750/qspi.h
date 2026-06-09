
#ifndef QSPI_H
#define QSPI_H

#include "memory.h"
#include <stddef.h>
#include <stdint.h>

int32_t qspi_read(uint32_t addr, void *buf, size_t len);
uint32_t qspi_read_status_register(uint32_t status_reg);
void init_qspi(uint8_t prescaler);
void init_qspi_80MHz(void);
void init_qspi_pll2(uint8_t prescaler);
void init_qspi_pll2_104MHz(void);
void qspi_memory_map_mode(void);
void qspi_write_flash_qpi(uint32_t addr, const uint32_t *src, uint32_t size);
void qspi_write_instruction(uint8_t instruction);
void qspi_write_status_register(uint32_t status_reg, uint8_t value);

#define QUADSPI_CR      (volatile uint32_t *)(QUADSPI + 0x00)
#define QUADSPI_DCR     (volatile uint32_t *)(QUADSPI + 0x04)
#define QUADSPI_SR      (volatile uint32_t *)(QUADSPI + 0x08)
#define QUADSPI_FCR     (volatile uint32_t *)(QUADSPI + 0x0C)
#define QUADSPI_DLR     (volatile uint32_t *)(QUADSPI + 0x10)
#define QUADSPI_CCR     (volatile uint32_t *)(QUADSPI + 0x14)
#define QUADSPI_AR      (volatile uint32_t *)(QUADSPI + 0x18)
#define QUADSPI_ABR     (volatile uint32_t *)(QUADSPI + 0x1C)
#define QUADSPI_DR      (volatile uint32_t *)(QUADSPI + 0x20)
#define QUADSPI_PSMKR   (volatile uint32_t *)(QUADSPI + 0x24)
#define QUADSPI_PSMAR   (volatile uint32_t *)(QUADSPI + 0x28)
#define QUADSPI_PIR     (volatile uint32_t *)(QUADSPI + 0x2C)
#define QUADSPI_LPTR    (volatile uint32_t *)(QUADSPI + 0x30)

#endif

