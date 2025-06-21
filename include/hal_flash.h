/**
 * @file hal_flash.h
 * @brief Hardware Abstraction Layer (HAL) interface for flash memory operations
 *
 * @version 1.0.1
 * @date 2025-06-21
 *
 * @details
 * This header file provides function prototypes for flash memory operations
 * like initialization, read, write, erase, and resource deallocation. It includes
 * error handling through return values based on error codes defined in
 * hal_err.h.
 *
 * @history
 *  - v1.0.0, 2024-12-23, Initial flash HAL interface definitions
 *  - v1.0.1, 2025-06-21, Added hal_flash_erase_sector function
 */

#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the flash memory interface
 *
 * @return int32_t Returns HAL_SUCCESS on success, or a negative error code on failure
 */
int32_t hal_flash_init(void);

/**
 * @brief Frees resources associated with the flash memory interface
 */
void hal_flash_free(void);

/**
 * @brief Writes data to the flash memory at the specified address
 *
 * @param addr The starting address in flash memory to write to
 * @param data Pointer to the data buffer to write
 * @param size The size in bytes of the data to write
 * @return int32_t Returns HAL_SUCCESS on success, or a negative error code on failure
 */
int32_t hal_flash_write(uint32_t addr, const uint8_t* data, uint32_t size);

/**
 * @brief Reads data from the flash memory at the specified address
 *
 * @param addr The starting address in flash memory to read from
 * @param data Pointer to the buffer where data will be stored
 * @param size The size in bytes of data to read
 * @return int32_t Returns HAL_SUCCESS on success, or a negative error code on failure
 */
int32_t hal_flash_read(uint32_t addr, uint8_t* data, uint32_t size);

/**
 * @brief Erases one or more sectors of the flash memory starting at the specified address
 *
 * @param addr The starting address in flash memory to begin erasing
 * @param num_sectors The number of sectors to erase
 * @return int32_t Returns HAL_SUCCESS on success, or a negative error code on failure
 */
int32_t hal_flash_erase_sector(uint32_t addr, uint32_t num_sectors);

#ifdef __cplusplus
}
#endif

#endif // HAL_FLASH_H
