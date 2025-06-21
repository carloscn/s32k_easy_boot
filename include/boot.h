#ifndef BOOT_H_
#define BOOT_H_

// Application start address (in flash)
#define APP_START_ADDRESS 0x00440000U
#define EASY_BOOT_START_ADDR 0x00400000U
#define APP_METADATA_ADDR 0x005CFFF0U
#define APP_METADATA_MAGIC 0xAABBCCDDU
#define APP_NAME_MAX_LEN 16U
#define APP_VERSION_MAX_LEN 12U

typedef struct
{
    uint32_t magic;                         // 0x00: Identifier
    char     app_name[APP_NAME_MAX_LEN];    // 0x04: Null-terminated app name
    char     version[APP_VERSION_MAX_LEN];  // 0x14: Null-terminated version
    char    *build_timestamp;               // 0x24: Null-terminated build date and time string
    uint32_t flash_start_addr;              // 0x28: App binary flash base address
    uint32_t image_size;                    // 0x2C: Size in bytes for CRC coverage
    uint32_t crc32;                         // 0x30: CRC32 over image (excluding metadata)
} app_metadata_t;
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
 * @brief Start execution of the user application from flash.
 *
 * This function performs the necessary steps to safely jump from
 * the bootloader to the application code located at a fixed flash
 * address (APP_START_ADDRESS).
 *
 * Steps performed:
 * 1. Disable interrupts to avoid unexpected behavior during jump.
 * 2. Read and validate the application's initial stack pointer (MSP).
 * 3. Read the application reset vector (initial PC) from vector table.
 * 4. Disable and clear all NVIC interrupts.
 * 5. Set the SCB->VTOR register to point to the application's vector table.
 * 6. Set MSP and PSP to the application's initial stack pointer.
 * 7. Jump to the application's reset handler.
 *
 * If validation fails, the function will enter a failure loop with an LED indication.
 *
 * @note APP_START_ADDRESS must be defined elsewhere as the base address of the application in flash.
 * @note This function does not return on success.
 */
void boot_app(void);

#endif /* BOOT_H_ */
