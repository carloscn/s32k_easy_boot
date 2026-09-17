/**
 * @file boot_uds_magic.h
 * @brief SRAM programming-entry flag shared by App and Boot.
 *
 * Lives in .boot_uds_magic, placed before __sram_bss_start so a software
 * reset does not ECC-wipe it (startup only zeros from BSS on non-POR).
 * Power-on still clears the whole SRAM, so the flag cannot survive a
 * power cycle — that is intended.
 */
#ifndef BOOT_UDS_MAGIC_H
#define BOOT_UDS_MAGIC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** App/Boot UDS 10 02 → stay in Boot, lock UDS encoding. */
#define BOOT_PROG_MAGIC_UDS  (0xAA55F10Au)
/** App UART "FLASH" / Boot ESSFLASH → stay in Boot, lock UART encoding. */
#define BOOT_PROG_MAGIC_UART (0xAA55F10Bu)

/** True if any valid programming-entry magic is present. */
int boot_uds_magic_is_set(void);

/** Write F10A before a software reset into Boot (UDS 10 02). */
void boot_uds_magic_set(void);

/** Write F10B before a software reset into Boot (UART FLASH). */
void boot_uds_magic_set_uart(void);

/** Clear after Boot consumes it (so 11 01 can jump App). */
void boot_uds_magic_clear(void);

/**
 * Read and clear. Returns 0 if none,
 * BOOT_PROG_MAGIC_UDS, or BOOT_PROG_MAGIC_UART.
 */
uint32_t boot_uds_magic_take(void);

#ifdef __cplusplus
}
#endif

#endif /* BOOT_UDS_MAGIC_H */
