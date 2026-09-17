/**
 * @file boot_uart_prog.h
 * @brief UART encoding for boot_img_prog (BOOT_UART_FLASH).
 */
#ifndef BOOT_UART_PROG_H
#define BOOT_UART_PROG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Poll for one UART command frame. No-op if BOOT_UART_FLASH=0. */
void boot_uart_prog_poll(void);

/**
 * Boot window: look for ASCII "ESSFLASH" for window_ms.
 * @return 1 if seen (encoding already locked UART).
 */
int boot_uart_wait_essflash(uint32_t window_ms);

#ifdef __cplusplus
}
#endif

#endif /* BOOT_UART_PROG_H */
