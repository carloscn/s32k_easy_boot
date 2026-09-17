/**
 * @file boot_uds_enter.c
 * @brief SW6 / USER_SW1 (PTB19) long-press to enter Boot UDS.
 */
#include "boot_uds.h"
#include "osal_utils.h"
#include "Siul2_Dio_Ip.h"
#include "Siul2_Port_Ip_Cfg.h"

#ifndef BOARD_S32K312EVB
#define BOARD_S32K312EVB 1
#endif

#ifndef BOOT_UDS_BTN_HOLD_MS
#define BOOT_UDS_BTN_HOLD_MS (2000U)
#endif

#ifndef BOOT_UDS_BTN_SAMPLE_MS
#define BOOT_UDS_BTN_SAMPLE_MS (20U)
#endif

static int s_enter_requested;

static int user_sw1_pressed(void)
{
    return (0U != Siul2_Dio_Ip_ReadPin(USER_SW1_PORT, USER_SW1_PIN)) ? 1 : 0;
}

void boot_uds_poll_button_early(void)
{
#if BOARD_S32K312EVB
    uint32_t held_ms = 0U;

    if (0 == user_sw1_pressed()) {
        return;
    }

    while (0 != user_sw1_pressed()) {
        osal_utils_delay_ms(BOOT_UDS_BTN_SAMPLE_MS);
        held_ms += BOOT_UDS_BTN_SAMPLE_MS;
        if (held_ms >= BOOT_UDS_BTN_HOLD_MS) {
            s_enter_requested = 1;
            return;
        }
    }
#else
    /* Production boards: no user key. */
#endif
}

int boot_uds_enter_requested(void)
{
    return s_enter_requested;
}
