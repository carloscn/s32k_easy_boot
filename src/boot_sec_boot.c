/**
 * Jump after boot_verify_preferred: Cortex-M XIP from the verified slot.
 * App trust root is the ECDSA P-256 pubkey in easy_boot .rodata.
 */

#include <stdint.h>
#include <stdio.h>
#include "S32K312.h"
#include "S32K312_SCB.h"
#include "S32K312_NVIC.h"
#include "osal_log.h"
#include "boot.h"
#include "boot_uds.h"
#include "boot_uds_magic.h"
#include "boot_img_prog.h"
#include "boot_uart_prog.h"
#include "essfw.h"
#include "leds_ctrl.h"

#ifndef SECBOOT_LEGACY_APP_FALLBACK
#define SECBOOT_LEGACY_APP_FALLBACK 1
#endif

void boot_jump_xip(uint32_t vector_table)
{
    uint32_t sp;
    uint32_t reset;
    void (*entry)(void);

    __asm volatile("cpsid i" ::: "memory");

    sp = *(volatile uint32_t *)vector_table;
    reset = *(volatile uint32_t *)(vector_table + 4U);
    if (((sp & 0x7U) != 0U) || (sp == 0U) || ((reset & 1U) == 0U)) {
        osal_log_info("[SECBOOT] bad vector table, refuse jump\r\n");
        leds_ctrl_boot_led_blink_failure();
        while (1) {
        }
    }

    S32_NVIC->ICER[0] = 0xFFFFFFFFU;
    S32_NVIC->ICPR[0] = 0xFFFFFFFFU;
    S32_SCB->VTOR = vector_table;

    /* Instruction cache may hold old flash lines after a previous download. */
    S32_SCB->ICIALLU = 0U;
    __asm volatile("dsb" ::: "memory");
    __asm volatile("isb" ::: "memory");

    __asm volatile("msr msp, %0" : : "r"(sp) : "memory");
    entry = (void (*)(void))reset;
    entry();
    while (1) {
        leds_ctrl_boot_led_blink_critical_failure();
    }
}

void boot_app(void)
{
    boot_img_info_t info;
    char line[96];
    int rc;
    uint32_t magic;
    int stay = 0;

    magic = boot_uds_magic_take();
    if (BOOT_PROG_MAGIC_UDS == magic) {
        (void)boot_prog_lock_enc(BOOT_PROG_ENC_UDS);
        osal_log_info("[SECBOOT] UDS via 10 02 magic\r\n");
        stay = 1;
    } else if (BOOT_PROG_MAGIC_UART == magic) {
#if BOOT_UART_FLASH
        (void)boot_prog_lock_enc(BOOT_PROG_ENC_UART);
        osal_log_info("[SECBOOT] UART via FLASH magic\r\n");
        stay = 1;
#else
        osal_log_info("[SECBOOT] UART magic ignored (BOOT_UART_FLASH=0)\r\n");
#endif
    }

    if (0 != boot_uds_enter_requested()) {
        osal_log_info("[SECBOOT] UDS via SW6/PTB19\r\n");
        stay = 1;
    }

#if BOOT_UART_FLASH
    if (0 == stay) {
        if (0 != boot_uart_wait_essflash(2000U)) {
            osal_log_info("[SECBOOT] UART via ESSFLASH\r\n");
            stay = 1;
        }
    }
#endif

    if (0 != stay) {
        boot_uds_run();
    }

    osal_log_info("[SECBOOT] verifying App (ECDSA P-256 + SHA-256, pubkey in Boot)\r\n");
    rc = boot_verify_preferred(&info);
    if (rc == BOOT_VF_OK) {
        (void)snprintf(line, sizeof(line),
                      "[SECBOOT] ok  slot=%lu entry=0x%08lX size=%lu\r\n",
                      (unsigned long)info.slot_id,
                      (unsigned long)info.load_addr,
                      (unsigned long)info.image_size);
        osal_log_info(line);
        boot_jump_xip(info.load_addr);
    }

#if SECBOOT_LEGACY_APP_FALLBACK
    osal_log_info("[SECBOOT] no signed slot; legacy unsigned jump 0x00442000\r\n");
    boot_app_legacy();
#else
    osal_log_info("[SECBOOT] no valid App, stay in Boot UDS\r\n");
    (void)boot_prog_lock_enc(BOOT_PROG_ENC_UDS);
    boot_uds_run();
#endif
}
