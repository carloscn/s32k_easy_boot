#include "osal_utils.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "leds_ctrl.h"

// Breathing effect parameters for boot_led_blink
#define BOOT_BREATHE_CYCLES 3        // Number of breathe cycles
#define BOOT_FADE_STEPS 30           // Steps for fade in/out
#define BOOT_STEP_DELAY_MS 25        // Delay per step (ms)
#define BOOT_PWM_PERIOD_MS 10        // PWM period (ms)
#define BOOT_PAUSE_BETWEEN_CYCLES_MS 50 // Pause between cycles (ms)

// Breathing effect parameters for boot_led_blink_failure and critical_failure
#define FAILURE_FADE_STEPS 40        // Steps for fade in/out
#define FAILURE_STEP_DELAY_MS 50     // Delay per step (ms)
#define FAILURE_PWM_PERIOD_MS 20     // PWM period (ms)

/**
 * Simulates PWM to set LED brightness for the specified color.
 * @param color: LED color (RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE).
 * @param duty_cycle: Duty cycle (0-100%).
 * @param period_ms: PWM period in milliseconds.
 */

void leds_ctrl_set_brightness(LedColor color, uint8_t duty_cycle, uint32_t period_ms)
{
    if (duty_cycle > 100) duty_cycle = 100;
    uint32_t on_time = (period_ms * duty_cycle) / 100;
    uint32_t off_time = period_ms - on_time;

    // Turn selected LEDs on
    if (on_time > 0) {
        switch (color) {
            case LED_RED:
                Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 1U);
                break;
            case LED_GREEN:
                Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 1U);
                break;
            case LED_BLUE:
                Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 1U);
                break;
            case LED_YELLOW:
                Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 1U);
                Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 1U);
                break;
            case LED_MAGENTA:
                Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 1U);
                Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 1U);
                break;
            case LED_CYAN:
                Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 1U);
                Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 1U);
                break;
            case LED_WHITE:
                Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 1U);
                Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 1U);
                Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 1U);
                break;
        }
        osal_utils_delay_ms(on_time);
    }

    // Turn selected LEDs off
    if (off_time > 0) {
        switch (color) {
            case LED_RED:
                Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);
                break;
            case LED_GREEN:
                Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
                break;
            case LED_BLUE:
                Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
                break;
            case LED_YELLOW:
                Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);
                Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
                break;
            case LED_MAGENTA:
                Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);
                Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
                break;
            case LED_CYAN:
                Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
                Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
                break;
            case LED_WHITE:
                Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);
                Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
                Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
                break;
        }
        osal_utils_delay_ms(off_time);
    }
}

/**
 * Bootloader LED blink function to indicate entry.
 * Creates a breathing effect with white LEDs (all on).
 * Total duration within 5 seconds.
 */
void leds_ctrl_boot_led_blink(void)
{
    // Ensure LEDs are off initially
    Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
    Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
    Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);

    // Run breathing cycles
    for (uint8_t cycle = 0; cycle < BOOT_BREATHE_CYCLES; cycle++) {
        // Fade in (0% to 100%)
        for (uint8_t step = 0; step <= BOOT_FADE_STEPS; step++) {
            uint8_t duty_cycle = (step * 100) / BOOT_FADE_STEPS;
            leds_ctrl_set_brightness(LED_WHITE, duty_cycle, BOOT_PWM_PERIOD_MS);
        }
        // Fade out (100% to 0%)
        for (uint8_t step = BOOT_FADE_STEPS; step > 0; step--) {
            uint8_t duty_cycle = ((step - 1) * 100) / BOOT_FADE_STEPS;
            leds_ctrl_set_brightness(LED_WHITE, duty_cycle, BOOT_PWM_PERIOD_MS);
        }
        // Pause between cycles
        Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
        Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
        Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);
        osal_utils_delay_ms(BOOT_PAUSE_BETWEEN_CYCLES_MS);
    }

    // Ensure LEDs are off at the end
    Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
    Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
    Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);
}

/**
 * LED blink function to indicate bootloader failure.
 * Creates a slow breathing effect with white LEDs (~4 s per cycle).
 * Runs indefinitely.
 */
void leds_ctrl_boot_led_blink_failure(void)
{
    // Ensure LEDs are off initially
    Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
    Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
    Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);

    // Run breathing cycles indefinitely
    while (1) {
        // Fade in (0% to 100%)
        for (uint8_t step = 0; step <= FAILURE_FADE_STEPS; step++) {
            uint8_t duty_cycle = (step * 100) / FAILURE_FADE_STEPS;
            leds_ctrl_set_brightness(LED_WHITE, duty_cycle, FAILURE_PWM_PERIOD_MS);
        }
        // Fade out (100% to 0%)
        for (uint8_t step = FAILURE_FADE_STEPS; step > 0; step--) {
            uint8_t duty_cycle = ((step - 1) * 100) / FAILURE_FADE_STEPS;
            leds_ctrl_set_brightness(LED_WHITE, duty_cycle, FAILURE_PWM_PERIOD_MS);
        }
    }
}

/**
 * LED blink function to indicate critical bootloader failure (e.g., flash error).
 * Creates a slow breathing effect with magenta LEDs (~4 s per cycle).
 * Runs indefinitely.
 */
void leds_ctrl_boot_led_blink_critical_failure(void)
{
    // Ensure LEDs are off initially
    Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
    Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
    Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);

    // Run breathing cycles indefinitely
    while (1) {
        // Fade in (0% to 100%)
        for (uint8_t step = 0; step <= FAILURE_FADE_STEPS; step++) {
            uint8_t duty_cycle = (step * 100) / FAILURE_FADE_STEPS;
            leds_ctrl_set_brightness(LED_RED, duty_cycle, FAILURE_PWM_PERIOD_MS);
        }
        // Fade out (100% to 0%)
        for (uint8_t step = FAILURE_FADE_STEPS; step > 0; step--) {
            uint8_t duty_cycle = ((step - 1) * 100) / FAILURE_FADE_STEPS;
            leds_ctrl_set_brightness(LED_RED, duty_cycle, FAILURE_PWM_PERIOD_MS);
        }
    }
}
