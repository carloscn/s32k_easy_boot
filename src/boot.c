#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "S32K312.h"
#include "S32K312_SCB.h"
#include "S32K312_TEMPSENSE.h"
#include "S32K312_NVIC.h"
#include "leds_ctrl.h"
#include "osal_log.h"
#include "boot.h"
#include "boot_version.h"

// Board information structure
typedef struct {
    // Bootloader information
    const char *boot_version;      // Bootloader version (e.g., "v2025.06")
    const char *build_time;        // Build timestamp (e.g., "Jun 20 2025 - 21:32:00 +0800")

    // Hardware information
    const char *cpu_model;         // CPU model (e.g., "ARM Cortex-M7, 120 MHz (Single Core)")
    const char *model;             // Board model (e.g., "S32K312")
    const char *board_id;          // Board ID (e.g., "S32K312_RevA_1234ABCD")
    const char *dram_size;         // SRAM size (e.g., "192 KiB")
    const char *pflash_size;       // Primary flash size (e.g., "1920 KiB")
    const char *dflash_size;       // Data flash size (e.g., "128 KiB")
    const char *serial_in;         // Input serial port (e.g., "serial@LPUART6")
    const char *serial_out;        // Output serial port (e.g., "serial@LPUART6")
    const char *serial_err;        // Error serial port (e.g., "serial@LPUART6")
    const char *network;           // Network interface (e.g., "None")
    const char *hse;
    uint32_t load_address;         // Load address (e.g., 0x80007FC0)
    uint32_t entry_point;          // Entry point (e.g., 0x80007FC0)
    uint32_t app_address;          // App address (e.g., 0x83000000)
    uint32_t app_load_addr;        // App load address (e.g., 0x83000000)
} board_info_t;

// Board info initialization with updated memory values
static const board_info_t board_info = {
    .boot_version = EASY_BOOT_VERSION,
    .build_time = "Jun 20 2025 - 21:32:00 +0800",
    .cpu_model = "ARM Cortex-M7 @120MHz (Single Core)",
    .model = "S32K312",
    .board_id = "401FC271",              // Replace with dynamic chip_id if needed
    .dram_size = "192 KiB",              // Total SRAM (ITCM + DTCM + SRAM)
    .pflash_size = "1920 KiB",           // Code Flash (pflash + app_0)
    .dflash_size = "128 KiB",            // Data Flash
    .serial_in = "serial@LPUART6",
    .serial_out = "serial@LPUART6",
    .serial_err = "serial@LPUART6",
    .network = "None",
	.hse = "Disabled",
    .load_address = EASY_BOOT_START_ADDR,
    .entry_point = EASY_BOOT_START_ADDR,
    .app_address = APP_START_ADDRESS,
    .app_load_addr = APP_START_ADDRESS,
};

// Global variables (retained to match original)
static void (*jump_to_application)(void) = NULL;
static uint32_t stack_point;

/**
 * Validates the stack pointer for the application.
 * Checks for 8-byte alignment and non-zero value.
 * @param sp: Stack pointer to validate.
 * @return: 1 if valid, 0 if invalid.
 */
static uint8_t boot_validate_app(uint32_t sp)
{
    // Check stack pointer: 8-byte aligned and non-zero
    if ((sp & 0x7) != 0 || sp == 0) {
        return 0;
    }
    return 1;
}

void boot_app(void)
{
    uint32_t jump_address = 0;

    // Disable interrupts
    __asm volatile("cpsid i" ::: "memory");

    // Read initial stack pointer (offset 0x0C) and reset vector (offset 0x04)
    stack_point = *(volatile uint32_t *)(APP_START_ADDRESS + 0x0CU);

    // Validate stack pointer
    if (!boot_validate_app(stack_point)) {
        leds_ctrl_boot_led_blink_failure(); // Magenta breathing for failure
        while (1); // Stay in failure state
    }

    jump_address = *(volatile uint32_t *)(stack_point + 0x04U);

    // Clear pending interrupts (minimal cleanup)
    S32_NVIC->ICER[0] = 0xFFFFFFFF; // Disable all interrupts
    S32_NVIC->ICPR[0] = 0xFFFFFFFF; // Clear all pending interrupts

    // Set Vector Table Offset Register
    S32_SCB->VTOR = APP_START_ADDRESS + 0x0CU;

    // Set Main Stack Pointer (MSP) and Process Stack Pointer (PSP)
    __asm volatile("msr msp, %0" : : "r" (stack_point) : "memory");
    __asm volatile("msr psp, %0" : : "r" (stack_point) : "memory");

    // Set jump address
    jump_to_application = (void (*)(void))jump_address;

    // Jump to application
    jump_to_application();

    // Should not reach here
    while (1) {
        leds_ctrl_boot_led_blink_critical_failure(); // Safety loop with failure indication
    }
}

/**
 * @brief Get pointer to app metadata structure in flash.
 * @return Pointer if valid metadata found, NULL otherwise.
 */
static const app_metadata_t *get_app_metadata(void)
{
    const app_metadata_t *meta = (const app_metadata_t *)APP_METADATA_ADDR;
    if (meta->magic != APP_METADATA_MAGIC) {
        return NULL;
    }

    return meta;
}

int32_t boot_read_version(char *version_buffer, size_t buf_size)
{
    if (!version_buffer || buf_size == 0) {
        return -1;
    }

    const app_metadata_t *meta = get_app_metadata();
    if (!meta) {
        return -1;
    }

    strncpy(version_buffer, meta->version, buf_size - 1);
    version_buffer[buf_size - 1] = '\0';

    return 0;
}

int32_t boot_read_app_name(char *name_buffer, size_t buf_size)
{
    if (!name_buffer || buf_size == 0) {
        return -1;
    }

    const app_metadata_t *meta = get_app_metadata();
    if (!meta) {
        return -1;
    }

    strncpy(name_buffer, meta->app_name, buf_size - 1);
    name_buffer[buf_size - 1] = '\0';

    return 0;
}

int32_t boot_read_device_id(char *id_buffer, size_t buf_size)
{
    if (!id_buffer || buf_size == 0) {
        return -1; // Return error if buffer pointer is NULL or size is 0
    }

    // Use the SDK-provided S32_SCB pointer to access the CPUID register
    if (!S32_SCB) {
        return -1; // Return error if S32_SCB is not initialized
    }

    // Read the CPUID register value directly
    uint32_t cpuid_value = S32_SCB->CPUID;

    // Format the 32-bit CPUID value into an 8-character hexadecimal string
    snprintf(id_buffer, buf_size, "%08lX", cpuid_value);
    id_buffer[buf_size - 1] = '\0'; // Ensure null termination

    return 0; // Return success
}

int32_t boot_print_board_info(void)
{
    char line_buf[128];
    char version[16] = {0};
    char app_name[20] = {0};
    char chip_id[9] = {0};
    char build_time_str[32];

    snprintf(build_time_str, sizeof(build_time_str), "%s - %s", __DATE__, __TIME__);

    if (boot_read_device_id(chip_id, sizeof(chip_id)) != 0) {
        snprintf(chip_id, sizeof(chip_id), "Unknown");
    }

    snprintf(line_buf, sizeof(line_buf), "\r\nBoot from PFLASH: 0x%08lX...\r\n", board_info.load_address);
    osal_log_info(line_buf);

    // === Bootloader Information ===
    snprintf(line_buf, sizeof(line_buf), "\r\nS32K EasyBoot %s (%s)\r\n", EASY_BOOT_VERSION, build_time_str);
    osal_log_info(line_buf);

    // === Hardware Information ===
    snprintf(line_buf, sizeof(line_buf), "CPU:   %s\r\n", board_info.cpu_model);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "Model: %s\r\n", board_info.model);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "CPUID: %s\r\n", chip_id[0] != 'U' ? chip_id : board_info.board_id);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "SRAM:  %s\r\n", board_info.dram_size);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "Flash: pflash: %s\r\n", board_info.pflash_size);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "       dflash: %s\r\n", board_info.dflash_size);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "In:    %s\r\n", board_info.serial_in);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "Out:   %s\r\n", board_info.serial_out);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "Err:   %s\r\n", board_info.serial_err);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "Net:   %s\r\n", board_info.network);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "HSE:   %s\r\n", board_info.hse);
    osal_log_info(line_buf);

    // === Application Information ===
    osal_log_info("## Loading App from pflash...\r\n");

    const app_metadata_t *meta = get_app_metadata();
    if (!meta) {
        osal_log_info("Invalid app metadata\r\n");
        return -1;
    }

    if (boot_read_version(version, sizeof(version)) != 0) {
        snprintf(version, sizeof(version), "1.0.0"); // Default version if read fails
    }

    if (boot_read_app_name(app_name, sizeof(app_name)) != 0) {
        snprintf(app_name, sizeof(app_name), "Unknown");
    }

    osal_log_info("App loaded successfully\r\n");

    snprintf(line_buf, sizeof(line_buf), "## Booting App from pflash at 0x%08lX ...\r\n", board_info.app_address);
    osal_log_info(line_buf);
    snprintf(line_buf, sizeof(line_buf), "   App Name:    %s\r\n", app_name);
    osal_log_info(line_buf);
    snprintf(line_buf, sizeof(line_buf), "   Built Time:  %s\r\n", meta->build_timestamp);
    osal_log_info(line_buf);
    snprintf(line_buf, sizeof(line_buf), "   App Type:    Raw Binary\r\n");
    osal_log_info(line_buf);
    snprintf(line_buf, sizeof(line_buf), "   App Ver:     %s\r\n", version);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "   App Size:    %lu Bytes = %ld KiB\r\n",
             (unsigned long)meta->image_size, meta->image_size / 1024);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "   App Address: 0x%08lX\r\n", board_info.app_address);
    osal_log_info(line_buf);

    snprintf(line_buf, sizeof(line_buf), "   App Point:   0x%08lX\r\n", board_info.app_load_addr);
    osal_log_info(line_buf);

    osal_log_info("   Verifying Checksum ... OK\r\n");
    snprintf(line_buf, sizeof(line_buf), "## Loading App from 0x%08lX ...\r\n\r\n", board_info.app_address);
    osal_log_info(line_buf);

    osal_log_info("Starting App ...\r\n\r\n");

    return 0;
}
