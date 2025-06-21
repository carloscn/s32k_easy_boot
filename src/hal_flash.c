/**
 * @file hal_flash.c
 * @brief Hardware Abstraction Layer (HAL) implementation for flash memory operations
 *
 * @version 1.0.1
 * @date 2025-06-21
 *
 * @details
 * This source file implements the flash memory operations defined in hal_flash.h,
 * including initialization, read, write, erase, and resource deallocation. It uses
 * the NXP S32K3 C40_Ip driver for low-level flash operations. Error handling is based
 * on return values defined in hal_err.h, with specific flash error codes.
 *
 * @history
 *  - v1.0.0, 2025-06-21, Initial implementation based on NXP C40_Ip driver
 *  - v1.0.1, 2025-06-21, Updated to use specific flash error codes from hal_err.h
 */

#include "C40_Ip.h"
#include "hal_error.h"
#include "hal_flash.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SECTOR_SIZE (8192u) /* Sector size for S32K312, adjust for other MCUs */
#define MASTER_ID (0U) /* Master ID for C40_Ip operations */

/*******************************************************************************
 * Local Variables
 ******************************************************************************/

static bool is_initialized = false; /* Tracks flash module initialization state */

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

/**
 * @brief Validates if an address range is within a flash block
 *
 * @param addr Start address of the range
 * @param size Size of the range in bytes
 * @return boolean TRUE if valid, FALSE otherwise
 */
static bool hal_flash_is_valid_address(uint32_t addr, uint32_t size)
{
    uint32_t end_addr;

    if (size == 0u || addr > UINT32_MAX - size) {
        return false;
    }

    if ((addr % 4U != 0U) || (size % 4U != 0U)) {
        return false;
    }

    end_addr = addr + size - 1u;

    if (addr >= C40_IP_CODE_BLOCK_0_BASE_ADDR && end_addr <= C40_IP_CODE_BLOCK_0_END_ADDR) {
        return true;
    }
    if (addr >= C40_IP_CODE_BLOCK_1_BASE_ADDR && end_addr <= C40_IP_CODE_BLOCK_1_END_ADDR) {
        return true;
    }
    if (addr >= C40_IP_DATA_BLOCK_BASE_ADDR && end_addr <= C40_IP_DATA_BLOCK_END_ADDR) {
        return true;
    }
    return false;
}

/**
 * @brief Initializes the flash memory interface
 *
 * @return int32_t Returns HAL_ERR_SUCCESS on success, or HAL_ERR_FLASH_INIT_FAILED on failure
 */
int32_t hal_flash_init(void)
{
	C40_Ip_StatusType status = C40_Ip_Init(&C40_Ip_InitCfg);
	if (status != C40_IP_STATUS_SUCCESS) {
		return HAL_ERR_FLASH_INIT_FAILED;
	}
	is_initialized = true;

	return HAL_ERR_SUCCESS;
}

/**
 * @brief Frees resources associated with the flash memory interface
 */
void hal_flash_free(void)
{
	is_initialized = false;
	/* No deinitialization required for C40_Ip driver */
}

/**
 * @brief Erases one or more sectors of the flash memory starting at the specified address
 *
 * @param addr The starting address in flash memory to begin erasing
 * @param num_sectors The number of sectors to erase
 * @return int32_t Returns HAL_ERR_SUCCESS on success, or one of:
 *                 - HAL_ERR_INVALID_PARAM for invalid inputs
 *                 - HAL_ERR_NOT_INITIALIZED if module is not initialized
 *                 - HAL_ERR_FLASH_SECTOR_PROTECTED if a sector cannot be unlocked
 *                 - HAL_ERR_FLASH_ERASE_FAILED for erase failures
 */
int32_t hal_flash_erase_sector(uint32_t addr, uint32_t num_sectors)
{
    if (num_sectors == 0u) {
        return HAL_ERR_INVALID_PARAM;
    }

    if (is_initialized == false) {
    	return HAL_ERR_NOT_INITIALIZED;
    }

    uint32_t size = num_sectors * SECTOR_SIZE;
    if (!hal_flash_is_valid_address(addr, size)) {
        return HAL_ERR_INVALID_PARAM;
    }

    C40_Ip_StatusType status;
    C40_Ip_VirtualSectorsType start_sector = C40_Ip_GetSectorNumberFromAddress(addr);
    C40_Ip_VirtualSectorsType sector;
    uint32_t i;

    for (i = 0u; i < num_sectors; i++) {
        sector = start_sector + i;

        if (C40_Ip_GetLock(sector) == C40_IP_STATUS_SECTOR_PROTECTED) {
            status = C40_Ip_ClearLock(sector, MASTER_ID);
            if (status != C40_IP_STATUS_SUCCESS) {
                return HAL_ERR_FLASH_SECTOR_PROTECTED;
            }
        }

        C40_Ip_MainInterfaceSectorErase(sector, MASTER_ID);
        do {
            status = C40_Ip_MainInterfaceSectorEraseStatus();
        } while (status == C40_IP_STATUS_BUSY);

        if (status != C40_IP_STATUS_SUCCESS) {
            return HAL_ERR_FLASH_ERASE_FAILED;
        }
    }

    return HAL_ERR_SUCCESS;
}

/**
 * @brief Writes data to the flash memory at the specified address
 *
 * @param addr The starting address in flash memory to write to
 * @param data Pointer to the data buffer to write
 * @param size The size in bytes of the data to write
 * @return int32_t Returns HAL_ERR_SUCCESS on success, or one of:
 *                 - HAL_ERR_INVALID_PARAM for invalid inputs
 *                 - HAL_ERR_NOT_INITIALIZED if module is not initialized
 *                 - HAL_ERR_FLASH_SECTOR_PROTECTED if a sector cannot be unlocked
 *                 - HAL_ERR_FLASH_WRITE_FAILED for write or verification failures
 */
int32_t hal_flash_write(uint32_t addr, const uint8_t *data, uint32_t size)
{
    if (data == NULL || !hal_flash_is_valid_address(addr, size)) {
        return HAL_ERR_INVALID_PARAM;
    }

    if (is_initialized == false) {
    	return HAL_ERR_NOT_INITIALIZED;
    }

    C40_Ip_StatusType status;
    C40_Ip_VirtualSectorsType start_sector = C40_Ip_GetSectorNumberFromAddress(addr);
    C40_Ip_VirtualSectorsType end_sector = C40_Ip_GetSectorNumberFromAddress(addr + size - 1u);
    uint32_t sector_count = (end_sector - start_sector) + 1u;

    if (hal_flash_erase_sector(addr, sector_count) != HAL_ERR_SUCCESS) {
        return HAL_ERR_FLASH_ERASE_FAILED;
    }

    C40_Ip_MainInterfaceWrite(addr, size, data, MASTER_ID);
    do {
        status = C40_Ip_MainInterfaceWriteStatus();
    } while (status == C40_IP_STATUS_BUSY);

    if (status != C40_IP_STATUS_SUCCESS) {
        return HAL_ERR_FLASH_WRITE_FAILED;
    }

    status = C40_Ip_Compare(addr, size, data);
    if (status != C40_IP_STATUS_SUCCESS) {
        return HAL_ERR_FLASH_VERIFY_FAILED;
    }

    return HAL_ERR_SUCCESS;
}

/**
 * @brief Reads data from the flash memory at the specified address
 *
 * @param addr The starting address in flash memory to read from
 * @param data Pointer to the buffer where data will be stored
 * @param size The size in bytes of data to read
 * @return int32_t Returns HAL_ERR_SUCCESS on success, or one of:
 *                 - HAL_ERR_INVALID_PARAM for invalid inputs
 *                 - HAL_ERR_NOT_INITIALIZED if module is not initialized
 *                 - HAL_ERR_FLASH_READ_FAILED for read failures
 */
int32_t hal_flash_read(uint32_t addr, uint8_t *data, uint32_t size)
{
    if (data == NULL || !hal_flash_is_valid_address(addr, size)) {
        return HAL_ERR_INVALID_PARAM;
    }

    if (is_initialized == false) {
    	return HAL_ERR_NOT_INITIALIZED;
    }

    C40_Ip_StatusType status = C40_Ip_Read(addr, size, data);
    if (status != C40_IP_STATUS_SUCCESS) {
        return HAL_ERR_FLASH_READ_FAILED;
    }

    return HAL_ERR_SUCCESS;
}



/*******************************************************************************
 * EOF
 ******************************************************************************/
