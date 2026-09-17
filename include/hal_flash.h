/**
 * @file hal_flash.h
 * @brief Hardware Abstraction Layer (HAL) interface for flash memory operations
 */
#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** C40 code-flash sector size on S32K312. */
#define HAL_FLASH_SECTOR_SIZE     (8192u)
/** C40 main-interface program: 8-byte alignment, max 128 B per quad-page. */
#define HAL_FLASH_PROGRAM_ALIGN   (8u)
#define HAL_FLASH_PROGRAM_MAX     (128u)

int32_t hal_flash_init(void);
void hal_flash_free(void);

/**
 * Erase the covering sectors then program. Unsafe for UDS 0x36.
 * Kept for test_flash.c.
 */
int32_t hal_flash_write(uint32_t addr, const uint8_t *data, uint32_t size);

int32_t hal_flash_read(uint32_t addr, uint8_t *data, uint32_t size);
int32_t hal_flash_erase_sector(uint32_t addr, uint32_t num_sectors);

/**
 * Program already-erased flash. Does not erase.
 * addr and size must be 8-byte aligned. Splits at 128-byte quad-pages.
 */
int32_t hal_flash_program(uint32_t addr, const uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* HAL_FLASH_H */
