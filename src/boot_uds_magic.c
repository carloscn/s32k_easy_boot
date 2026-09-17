/**
 * @file boot_uds_magic.c
 * @brief Programming-entry SRAM flag (App 10 02 / FLASH → Boot).
 */
#include "boot_uds_magic.h"

typedef struct {
    uint32_t word;
    uint32_t crc8; /* CRC-8/SMBUS of the 4 magic bytes, stored in low 8 bits */
} boot_uds_magic_t;

static volatile boot_uds_magic_t s_boot_uds_magic
    __attribute__((section(".boot_uds_magic")));

static uint8_t crc8_smbus(uint32_t word)
{
    uint8_t crc = 0U;
    uint8_t i;
    uint8_t b;
    const uint8_t *p = (const uint8_t *)&word;

    for (i = 0U; i < 4U; i++) {
        crc ^= p[i];
        for (b = 0U; b < 8U; b++) {
            if (0U != (crc & 0x80U)) {
                crc = (uint8_t)((crc << 1) ^ 0x07U);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

static int word_is_valid(uint32_t word, uint32_t crc)
{
    if ((BOOT_PROG_MAGIC_UDS != word) && (BOOT_PROG_MAGIC_UART != word)) {
        return 0;
    }
    if ((uint32_t)crc8_smbus(word) != (crc & 0xFFU)) {
        return 0;
    }
    return 1;
}

int boot_uds_magic_is_set(void)
{
    return word_is_valid(s_boot_uds_magic.word, s_boot_uds_magic.crc8);
}

void boot_uds_magic_set(void)
{
    uint32_t word = BOOT_PROG_MAGIC_UDS;

    s_boot_uds_magic.word = word;
    s_boot_uds_magic.crc8 = (uint32_t)crc8_smbus(word);
}

void boot_uds_magic_set_uart(void)
{
    uint32_t word = BOOT_PROG_MAGIC_UART;

    s_boot_uds_magic.word = word;
    s_boot_uds_magic.crc8 = (uint32_t)crc8_smbus(word);
}

void boot_uds_magic_clear(void)
{
    s_boot_uds_magic.word = 0U;
    s_boot_uds_magic.crc8 = 0U;
}

uint32_t boot_uds_magic_take(void)
{
    uint32_t word = s_boot_uds_magic.word;
    uint32_t crc = s_boot_uds_magic.crc8;

    boot_uds_magic_clear();
    if (0 == word_is_valid(word, crc)) {
        return 0U;
    }
    return word;
}
