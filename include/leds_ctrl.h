#ifndef LEDS_CTRL_H_
#define LEDS_CTRL_H_

// Color enumeration for LED selection
typedef enum {
    LED_RED,      // Red LED only
    LED_GREEN,    // Green LED only
    LED_BLUE,     // Blue LED only
    LED_YELLOW,   // Red + Green
    LED_MAGENTA,  // Red + Blue
    LED_CYAN,     // Green + Blue
    LED_WHITE     // All LEDs (Red + Green + Blue)
} LedColor;

void leds_ctrl_boot_led_blink(void);
void leds_ctrl_set_brightness(LedColor color, uint8_t duty_cycle, uint32_t period_ms);
void leds_ctrl_boot_led_blink_failure(void);
void leds_ctrl_boot_led_blink_critical_failure(void);

#endif /* LEDS_CTRL_H_ */
