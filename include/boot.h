#ifndef BOOT_H_
#define BOOT_H_

// Application start address (in flash)
#define APP_START_ADDRESS 0x00440000U
#define EASY_BOOT_START_ADDR 0x00400000U

void boot_app(void);
void boot_led_blink(void);

#endif /* BOOT_H_ */
