/*==================================================================================================
* Project : RTD AUTOSAR 4.7
* Platform : CORTEXM
* Peripheral : S32K3XX
* Dependencies : none
*
* Autosar Version : 4.7.0
* Autosar Revision : ASR_REL_4_7_REV_0000
* Autosar Conf.Variant :
* SW Version : 4.0.0
* Build Version : S32K3_RTD_4_0_0_P24_D2405_ASR_REL_4_7_REV_0000_20240515
*
* Copyright 2020 - 2024 NXP
*
* NXP Confidential. This software is owned or controlled by NXP and may only be
* used strictly in accordance with the applicable license terms. By expressly
* accepting such terms or by downloading, installing, activating and/or otherwise
* using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms. If you do not agree to be
* bound by the applicable license terms, then you may not retain, install,
* activate or otherwise use the software.
==================================================================================================*/

/**
*   @file main.c
*
*   @addtogroup main_module main module documentation
*   @{
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "Mcal.h"
#include "Clock_Ip.h"
#include "IntCtrl_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "Lpuart_Uart_Ip.h"
#include "Lpuart_Uart_Ip_Irq.h"
#include "osal_log.h"
#include "osal_utils.h"
#include "leds_ctrl.h"
#include "boot.h"
#include "boot_uds.h"
#include "hal_uart.h"
#include "hse_cmac_demo.h"

HAL_UART lpuart6;

void board_level_init(void)
{
    Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);

    Clock_Ip_InitClock(Clock_Ip_aClockConfig);

    while (CLOCK_IP_PLL_LOCKED != Clock_Ip_GetPllStatus()) { /* Busy wait */ }

    Clock_Ip_DistributePll();

    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
                       g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);

    IntCtrl_Ip_Init(&IntCtrlConfig_0);

    lpuart6.num = LPUART_UART_IP_INSTANCE_USING_6;
    lpuart6.irq = LPUART6_IRQn;
    hal_uart_init(&lpuart6);

    /* SW6/PTB19: sample here so a hold from reset is seen before HSE CMAC. */
    boot_uds_poll_button_early();
}

int main(void)
{
    board_level_init();

    boot_print_board_info();
    if (0 == boot_uds_enter_requested()) {
        hse_cmac_demo_run();
        leds_ctrl_boot_led_blink();
    }
    boot_app();

    return 0;
}

/** @} */
