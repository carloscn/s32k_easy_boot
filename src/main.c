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
//#include "tja1153.h"
#include "FlexCAN_Ip.h"
#include "hal_uart.h"



#define CORE1_START_ADDR (0x00500000u)
/* User includes (#include below this line is not maintained by Processor Expert) */
#define CAN_MSG_TYPE (CAN_MSG_ID_STD)
#define RX_MAILBOX_ID (1u)
#define TX_MAILBOX_ID (2u)
Flexcan_Ip_MsgBuffType g_RXCANMsg;
#define	RX_FUN_ID (0x7FFu)   /*can tp rx function ID*/
#define	RX_PHY_ID (0x784u)   /*can tp rx phy ID*/
#define	TX_ID (0x7F0u)       /*can tp tx ID*/

extern void CAN0_ORED_0_31_MB_IRQHandler(void);

const Flexcan_Ip_DataInfoType RXCANMsgConfig =
{
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 8u,
    .is_polling = FALSE,
    .is_remote = FALSE
};

Flexcan_Ip_DataInfoType TXCANMsgConfig =
{
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 8u,
    .is_polling = FALSE,
    .is_remote = FALSE
};

HAL_UART lpuart6;
/**
 * @brief
 *
 * @param instance
 * @param eventType
 * @param buffIdx
 * @param flexcanState
 */
void CAN_ISR_Callback(uint8 instance,Flexcan_Ip_EventType eventType,
                      uint32 buffIdx,const Flexcan_Ip_StateType * flexcanState)
{

    if(FLEXCAN_EVENT_RX_COMPLETE == eventType)
    {
        //TP_DriverWriteDataInTP(g_RXCANMsg.msgId, g_RXCANMsg.dataLen, g_RXCANMsg.data);
        FlexCAN_Ip_Receive(INST_FLEXCAN_0, RX_MAILBOX_ID, &g_RXCANMsg, FALSE);
    }
    else if(FLEXCAN_EVENT_TX_COMPLETE == eventType)
    {
        //TP_DoTxMsgSuccesfulCallback();
    }
    else
    {}
}

void board_level_init(void)
{
    // 1. Initialize clock
    Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);

    Clock_Ip_InitClock(Clock_Ip_aClockConfig);

    while (CLOCK_IP_PLL_LOCKED != Clock_Ip_GetPllStatus()) { /* Busy wait */ }

    Clock_Ip_DistributePll();

    // 2. Initialize ports
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
                       g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);

    // 3. Initialize interrupt controller
    IntCtrl_Ip_Init(&IntCtrlConfig_0);

    // 4. Initialize LPUART6
    lpuart6.num = LPUART_UART_IP_INSTANCE_USING_6;
    lpuart6.irq = LPUART6_IRQn;
    hal_uart_init(&lpuart6);

    FlexCAN_Ip_Init(INST_FLEXCAN_0, &FlexCAN_State0, &FlexCAN_Config0);
    FlexCAN_Ip_SetStartMode(INST_FLEXCAN_0);
    FlexCAN_Ip_ConfigRxMb(INST_FLEXCAN_0, RX_MAILBOX_ID, &RXCANMsgConfig, RX_PHY_ID);
    FlexCAN_Ip_Receive(INST_FLEXCAN_0, RX_MAILBOX_ID, &g_RXCANMsg, FALSE);

    // Tja1153_Init(0);
}

int main(void)
{
	board_level_init();

	boot_print_board_info();
	leds_ctrl_boot_led_blink();
    boot_app();

    return 0;
}

/** @} */
