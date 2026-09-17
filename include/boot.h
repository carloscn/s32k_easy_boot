#ifndef BOOT_H_
#define BOOT_H_

#include <stddef.h>
#include <stdint.h>
#include "app_metadata.h"
#include "sec_boot_layout.h"

#define APP_START_ADDRESS     ((uint32_t)SEC_BOOT_SLOT_A_IMAGE_ADDR)
#define EASY_BOOT_START_ADDR  0x00400000U

/**
 * @brief Read the app version string from metadata.
 * @param[out] version_buffer Buffer to store the version string.
 * @param[in] buf_size Size of version_buffer (should be >= 12).
 * @return 0 on success, -1 if metadata invalid or parameters invalid.
 */
int32_t boot_read_version(char *version_buffer, size_t buf_size);

/**
 * @brief Read the app name string from metadata.
 * @param[out] name_buffer Buffer to store the app name.
 * @param[in] buf_size Size of name_buffer (should be >= 16).
 * @return 0 on success, -1 if metadata invalid or parameters invalid.
 */
int32_t boot_read_app_name(char *name_buffer, size_t buf_size);

/**
 * @brief Print all application metadata info via osal_log_info.
 * @return 0 on success, -1 if metadata invalid.
 */
int32_t boot_print_board_info(void);

/**
 * @brief Reads the CPUID register from the S32K3 SCB and stores it as a hexadecimal string.
 *
 * This function retrieves the CPUID register value from the System Control Block (SCB)
 * using the NXP SDK's S32_SCB pointer, which contains the implementer, variant, architecture,
 * part number, and revision of the Cortex-M7 core. The value is formatted into an 8-character
 * hexadecimal string (e.g., "41FC2701") and stored in the provided buffer.
 *
 * @param id_buffer Pointer to a character buffer where the CPUID string will be stored.
 * @param buf_size Size of the id_buffer in bytes. Must be at least 9 bytes to hold the
 *                 8-character ID plus null terminator.
 *
 * @return int32_t 0 on success, -1 on failure (e.g., invalid buffer or register access).
 *
 * @note The function relies on the NXP SDK's S32_SCB pointer, which maps to address 0xE000E000.
 *       Verify this against the S32K3 reference manual for the specific device.
 * @warning If buf_size is less than 9, the output will be truncated and null-terminated.
 */
int32_t boot_read_device_id(char *id_buffer, size_t buf_size);

/**
 * @brief Table of SCB Register Used for CPUID
 *
 * The following register in the System Control Block (SCB) is used to store the
 * CPU identification information.
 *
 * | Offset | Register | Width (bits) | Access | Reset Value  | Description                     |
 * |--------|----------|--------------|--------|--------------|---------------------------------|
 * | 0xD00  | CPUID    | 32           | RO     | N/A          | CPUID Base Register containing implementer, variant, architecture, part number, and revision |
 *
 * @note Reset value is not applicable as CPUID is a read-only register set by hardware.
 *       Consult the ARM Cortex-M7 Technical Reference Manual for bit field details.
 */

/**
 * Cortex-M XIP jump: VTOR + MSP + Reset_Handler at vector_table.
 * Does not return on success.
 */
void boot_jump_xip(uint32_t vector_table);

/**
 * Jump via signed slot (boot_verify_preferred). If no valid header exists and
 * SECBOOT_LEGACY_APP_FALLBACK is 1, falls back to boot_app_legacy().
 */
void boot_app(void);

/**
 * Unsigned jump to APP_START_ADDRESS (slot A image / vector table at 0x00442000).
 * Used only while slot headers are not yet programmed.
 */
void boot_app_legacy(void);

#endif /* BOOT_H_ */
