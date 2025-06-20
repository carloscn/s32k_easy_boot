#include <stdint.h>
#include "S32K312.h"
#include "S32K312_SCB.h"
#include "S32K312_NVIC.h"
#include "leds_ctrl.h"
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

/**
 * Jumps to the application at AppStartAddress.
 * Disables interrupts, sets up stack and VTOR, and jumps to the reset vector.
 */
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
