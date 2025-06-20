#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "S32K312.h"
#include "S32K312_SCB.h"
#include "S32K312_NVIC.h"
#include "leds_ctrl.h"
#include "osal_log.h"
#include "boot.h"

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
    if (!version_buffer || buf_size == 0)
        return -1;

    const app_metadata_t *meta = get_app_metadata();
    if (!meta)
        return -1;

    strncpy(version_buffer, meta->version, buf_size - 1);
    version_buffer[buf_size - 1] = '\0';
    return 0;
}

int32_t boot_read_app_name(char *name_buffer, size_t buf_size)
{
    if (!name_buffer || buf_size == 0)
        return -1;

    const app_metadata_t *meta = get_app_metadata();
    if (!meta)
        return -1;

    strncpy(name_buffer, meta->app_name, buf_size - 1);
    name_buffer[buf_size - 1] = '\0';
    return 0;
}

int32_t boot_print_app_info(void)
{
    char version[16] = {0};
    char app_name[20] = {0};

    const app_metadata_t *meta = get_app_metadata();
    if (!meta)
    {
        osal_log_info((const char *)"Invalid app metadata\r\n");
        return -1;
    }

    if (boot_read_version(version, sizeof(version)) != 0)
        snprintf(version, sizeof(version), "Unknown");

    if (boot_read_app_name(app_name, sizeof(app_name)) != 0)
        snprintf(app_name, sizeof(app_name), "Unknown");

    char line_buf[64];

    osal_log_info((const char *)"\r\n===== Application Info =====\r\n");

    snprintf(line_buf, sizeof(line_buf), "App Name       : %s\r\n", app_name);
    osal_log_info((const char *)line_buf);

    snprintf(line_buf, sizeof(line_buf), "Version        : %s\r\n", version);
    osal_log_info((const char *)line_buf);

    snprintf(line_buf, sizeof(line_buf), "Build Time     : %lu\r\n", (unsigned long)meta->build_timestamp);
    osal_log_info((const char *)line_buf);

    snprintf(line_buf, sizeof(line_buf), "Flash Start    : 0x%08" PRIX32 "\r\n", meta->flash_start_addr);
    osal_log_info((const char *)line_buf);

    snprintf(line_buf, sizeof(line_buf), "Image Size     : %lu bytes\r\n", (unsigned long)meta->image_size);
    osal_log_info((const char *)line_buf);

    snprintf(line_buf, sizeof(line_buf), "CRC32          : 0x%08" PRIX32 "\r\n", meta->crc32);
    osal_log_info((const char *)line_buf);

    osal_log_info((const char *)"==============================\r\n");

    return 0;
}
