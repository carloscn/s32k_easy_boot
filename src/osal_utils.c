#include "osal_utils.h"
#include <string.h>
#include <stdio.h>

size_t osal_utils_uint8_array_to_hex(const uint8_t *data, size_t length, char *output, size_t output_size)
{
    size_t required_size = length * 3; // 2 chars per byte + 1 space, minus 1 for no trailing space
    if (required_size >= output_size) {
        return 0; // Buffer too small
    }

    size_t pos = 0;
    for (size_t i = 0; i < length; i++) {
        pos += snprintf(output + pos, output_size - pos, "%02X", data[i]);
        if (i < length - 1) {
            output[pos++] = ' ';
        }
    }
    output[pos] = '\0'; // Ensure null termination

    return pos;
}

// Define core clock frequency (Hz)
// Adjust based on your S32K312 clock configuration (e.g., 120000000 for 120 MHz, 30000000 for 30 MHz)
#define CORE_CLOCK_HZ 120000000UL // Default: 120 MHz, change to 30000000UL if confirmed 30 MHz

/**
 * Delay for a specified number of microseconds using a loop.
 * Tuned for S32K312 with a configurable core clock (default 120 MHz).
 * Uses inline assembly for predictable cycle counts.
 * Note: Delay is approximate; calibrate with GPIO and oscilloscope for precision.
 * @param us: Number of microseconds to delay.
 */
void osal_utils_delay_us(size_t us)
{
    // Calculate cycles per microsecond: CORE_CLOCK_HZ / 1,000,000
    const size_t cycles_per_us = CORE_CLOCK_HZ / 1000000UL; // e.g., 120 for 120 MHz, 30 for 30 MHz
    // Estimated cycles per loop: 4 NOPs (4 cycles) + loop overhead (subs: 1, bne: 2) ≈ 7 cycles
    // Adjusted to ~1.75 cycles to account for 4x speedup (7 / 4)
    const size_t cycles_per_loop = 2; // Approximate, tuned for observed 4x speedup
    const size_t loops_per_us = cycles_per_us / cycles_per_loop; // e.g., 120 / 2 = 60 loops per µs

    // Total loops needed
    size_t loops = us * loops_per_us;

    // Inline assembly loop with NOPs for consistent timing
    __asm volatile (
        "1: \n"
        "nop \n"           // 1 cycle
        "nop \n"           // 1 cycle
        "nop \n"           // 1 cycle
        "nop \n"           // 1 cycle
        "subs %0, %0, #1 \n" // 1 cycle (decrement)
        "bne 1b \n"        // 2 cycles (branch if not zero)
        : "+r" (loops)     // Input/output: loops (modified)
        :
        : "cc"             // Clobber condition codes
    );
}

void osal_utils_delay_ms(size_t ms)
{
    // Convert milliseconds to microseconds and call osal_delay_us
    for (size_t i = 0; i < ms; i++) {
        osal_utils_delay_us(1000); // 1 ms = 1000 µs
    }
}
