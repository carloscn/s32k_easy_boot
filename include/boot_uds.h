/**
 * @file boot_uds.h
 * @brief Boot UDS programming session entry and poll loop.
 */
#ifndef BOOT_UDS_H
#define BOOT_UDS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Call after Siul2_Port_Ip_Init(). If SW6/USER_SW1 is held ≥ 2 s,
 * latches an enter-UDS request so boot_app() skips the App jump.
 */
void boot_uds_poll_button_early(void);

/** Non-zero if SW6 long-press already requested a programming session. */
int boot_uds_enter_requested(void);

/**
 * iso14229 listen loop (0x10 / 0x11 / 0x22 F180 / download SIDs).
 * Also polls UART encoding when BOOT_UART_FLASH=1. Does not return
 * except via MCU reset.
 */
void boot_uds_run(void);

/** AIRCR SYSRESETREQ. Does not return. */
void boot_mcu_system_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* BOOT_UDS_H */
