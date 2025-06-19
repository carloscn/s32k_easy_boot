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
#include "Mcal.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "Clock_Ip.h"
#include "stdint.h"
#include "S32K312_SCB.h"

#define EASY_BOOT_START_ADDR 		0x00400000U
#define AppStartAddress 		 	0x00440000U

static void (*JumpTpApplication)(void) = NULL;
static uint32_t stack_point;
volatile int exit_code = 0;
/* User includes */

/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/

void boot_app(void)
{
	__asm("cpsid i");
	uint32_t JumpAddress = 0;

	stack_point = *(volatile uint32_t *)(AppStartAddress+0x0c);
	JumpAddress = *(volatile uint32_t *)(stack_point+0x04);
	JumpTpApplication = (void (*)(void))JumpAddress;
	S32_SCB->VTOR = *(volatile uint32_t *)(AppStartAddress+0x0c);
	__asm volatile("MSR msp, %0\n":: "r" (stack_point));
	__asm volatile("MSR psp, %0\n":: "r" (stack_point));
	JumpTpApplication();
}

// Simple delay function
void test_delay(uint32_t delay)
{
    static volatile uint32_t delay_timer = 0;
    while (delay_timer < delay) {
        delay_timer++;
    }
    delay_timer = 0;
}

// LED blinking function
void test_led(void)
{
    uint8_t count = 0;

    while (count++ < 5) {
        Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 1U);
        test_delay(4800000);
        Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
        test_delay(4800000);
    }
}


void board_level_init(void)
{
    Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
                       g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);
}

int main(void)
{
    /* Write your code here */
	board_level_init();
    test_led();
    boot_app();
    return 0;
}

/** @} */
