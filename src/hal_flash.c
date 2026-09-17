/**
 * @file hal_flash.c
 * @brief Flash HAL — C40_Ip. program() does not erase (UDS 0x36).
 */
#include "C40_Ip.h"
#include "hal_error.h"
#include "hal_flash.h"

#include <string.h>

#define SECTOR_SIZE (HAL_FLASH_SECTOR_SIZE)
#define MASTER_ID (0U)
#define HAL_DCACHE_LINE (32u)
/* Cortex-M7 SCB: don't depend on S32_SCB member names. */
#define SCB_DCCMVAC (*(volatile uint32_t *)0xE000EF68UL)
#define SCB_DCIMVAC (*(volatile uint32_t *)0xE000EF5CUL)

static bool is_initialized = false;

/* C40 DATA registers are filled by the CPU; keep a cache-line-aligned copy
 * anyway so a future DMA path and D-cache invalidate of the destination
 * stay in one place. Lives in .bss (cacheable SRAM), not DTCM. */
static uint8_t s_prog_buf[HAL_FLASH_PROGRAM_MAX]
    __attribute__((aligned(32)));

static void dcache_clean(const void *addr, uint32_t len)
{
    uint32_t p;
    uint32_t end;

    if ((NULL == addr) || (0U == len)) {
        return;
    }
    p = (uint32_t)addr & ~(HAL_DCACHE_LINE - 1U);
    end = (uint32_t)addr + len;
    while (p < end) {
        SCB_DCCMVAC = p;
        p += HAL_DCACHE_LINE;
    }
    __asm volatile("dsb" ::: "memory");
    __asm volatile("isb" ::: "memory");
}

static void dcache_invalidate(uint32_t addr, uint32_t len)
{
    uint32_t p;
    uint32_t end;

    if (0U == len) {
        return;
    }
    p = addr & ~(HAL_DCACHE_LINE - 1U);
    end = addr + len;
    while (p < end) {
        SCB_DCIMVAC = p;
        p += HAL_DCACHE_LINE;
    }
    __asm volatile("dsb" ::: "memory");
    __asm volatile("isb" ::: "memory");
}

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

static int32_t unlock_sector(uint32_t addr)
{
    C40_Ip_VirtualSectorsType sector = C40_Ip_GetSectorNumberFromAddress(addr);
    C40_Ip_StatusType status;

    if (C40_Ip_GetLock(sector) == C40_IP_STATUS_SECTOR_PROTECTED) {
        status = C40_Ip_ClearLock(sector, MASTER_ID);
        if (status != C40_IP_STATUS_SUCCESS) {
            return HAL_ERR_FLASH_SECTOR_PROTECTED;
        }
    }
    return HAL_ERR_SUCCESS;
}

int32_t hal_flash_init(void)
{
    C40_Ip_StatusType status = C40_Ip_Init(&C40_Ip_InitCfg);
    if (status != C40_IP_STATUS_SUCCESS) {
        return HAL_ERR_FLASH_INIT_FAILED;
    }
    is_initialized = true;

    return HAL_ERR_SUCCESS;
}

void hal_flash_free(void)
{
    is_initialized = false;
}

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
        dcache_invalidate((uint32_t)addr + (i * SECTOR_SIZE), SECTOR_SIZE);
    }

    return HAL_ERR_SUCCESS;
}

int32_t hal_flash_write(uint32_t addr, const uint8_t *data, uint32_t size)
{
    if (data == NULL || !hal_flash_is_valid_address(addr, size)) {
        return HAL_ERR_INVALID_PARAM;
    }

    if (is_initialized == false) {
        return HAL_ERR_NOT_INITIALIZED;
    }

    C40_Ip_VirtualSectorsType start_sector = C40_Ip_GetSectorNumberFromAddress(addr);
    C40_Ip_VirtualSectorsType end_sector = C40_Ip_GetSectorNumberFromAddress(addr + size - 1u);
    uint32_t sector_count = (end_sector - start_sector) + 1u;

    if (hal_flash_erase_sector(addr, sector_count) != HAL_ERR_SUCCESS) {
        return HAL_ERR_FLASH_ERASE_FAILED;
    }

    return hal_flash_program(addr, data, size);
}

int32_t hal_flash_program(uint32_t addr, const uint8_t *data, uint32_t size)
{
    C40_Ip_StatusType status;
    int32_t rc;

    if ((data == NULL) || (0u == size) || !hal_flash_is_valid_address(addr, size)) {
        return HAL_ERR_INVALID_PARAM;
    }
    if (((addr % HAL_FLASH_PROGRAM_ALIGN) != 0U) ||
        ((size % HAL_FLASH_PROGRAM_ALIGN) != 0U)) {
        return HAL_ERR_INVALID_PARAM;
    }
    if (is_initialized == false) {
        return HAL_ERR_NOT_INITIALIZED;
    }

    while (size > 0U) {
        uint32_t off = addr & (HAL_FLASH_PROGRAM_MAX - 1U);
        uint32_t chunk = HAL_FLASH_PROGRAM_MAX - off;

        if (chunk > size) {
            chunk = size;
        }

        rc = unlock_sector(addr);
        if (HAL_ERR_SUCCESS != rc) {
            return rc;
        }

        (void)memcpy(s_prog_buf, data, chunk);
        dcache_clean(s_prog_buf, chunk);

        status = C40_Ip_MainInterfaceWrite(addr, chunk, s_prog_buf, MASTER_ID);
        if (status != C40_IP_STATUS_SUCCESS) {
            return HAL_ERR_FLASH_WRITE_FAILED;
        }
        do {
            status = C40_Ip_MainInterfaceWriteStatus();
        } while (status == C40_IP_STATUS_BUSY);

        if (status != C40_IP_STATUS_SUCCESS) {
            return HAL_ERR_FLASH_WRITE_FAILED;
        }

        dcache_invalidate(addr, chunk);

        addr += chunk;
        data += chunk;
        size -= chunk;
    }

    return HAL_ERR_SUCCESS;
}

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
