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

/* Including necessary configuration files. */
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

#define BOOT_WEC_MSG ("Hello, this is the bootloader! \r\n")
#define BOOT_APP_MSG ("Bootloader is booting App...\r\n")

void board_level_init(void)
{
    // 1. Initialize clock
    Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);

    // 2. Initialize ports
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
                       g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);

    // 3. Initialize interrupt controller
    IntCtrl_Ip_Init(&IntCtrlConfig_0);

    // 4. Initialize LPUART6
    Lpuart_Uart_Ip_Init(LPUART_INSTANCE, &Lpuart_Uart_Ip_xHwConfigPB_6);
}

int main(void)
{
	board_level_init();

	osal_log_info((const char *)BOOT_WEC_MSG);
	boot_print_app_info();
	leds_ctrl_boot_led_blink();
	osal_log_info((const char *)BOOT_APP_MSG);
    boot_app();

    return 0;
}

/** @} */
