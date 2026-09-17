/**
 * @file boot_img_prog.h
 * @brief Channel-agnostic idle-slot image programming (IMG-D2–D4).
 *
 * No UDS headers, no UART framing. Encoding layers call these functions.
 */
#ifndef BOOT_IMG_PROG_H
#define BOOT_IMG_PROG_H

#include <stddef.h>
#include <stdint.h>
#include "essfw.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BOOT_UART_FLASH
#if defined(BOARD_S32K312EVB) && (BOARD_S32K312EVB)
#define BOOT_UART_FLASH 1
#else
#define BOOT_UART_FLASH 0
#endif
#endif

#ifndef BOOT_UDS_L2_SKIP
#define BOOT_UDS_L2_SKIP 1
#endif

typedef enum {
    BOOT_PROG_OK = 0,
    BOOT_PROG_BUSY,
    BOOT_PROG_DENIED,
    BOOT_PROG_BAD_ADDR,
    BOOT_PROG_IO,
    BOOT_PROG_VERIFY,
    BOOT_PROG_STATE,
    BOOT_PROG_UNSUPPORTED
} boot_prog_err_t;

typedef enum {
    BOOT_PROG_ENC_NONE = 0,
    BOOT_PROG_ENC_UDS,
    BOOT_PROG_ENC_UART
} boot_prog_enc_t;

typedef struct {
    uint32_t idle_slot;     /* 0 = A, 1 = B */
    uint32_t header_addr;
    uint32_t image_addr;
    uint32_t max_payload;   /* 8192 + image max */
} boot_prog_query_t;

/** Header + image of one slot, in 8 KiB sectors (101). */
#define BOOT_PROG_SLOT_SPAN \
    (SEC_BOOT_SLOT_HEADER_SIZE + SEC_BOOT_SLOT_A_IMAGE_MAX)

boot_prog_enc_t boot_prog_enc(void);

/**
 * First-wins encoding lock. Same encoding again is OK.
 * @return 0 on success, -1 if the other encoding is already locked.
 */
int boot_prog_lock_enc(boot_prog_enc_t enc);

void boot_prog_unlock_enc(void);

boot_prog_err_t boot_prog_query(boot_prog_query_t *out);

/** Blocking erase of the idle slot (UART / callers that can wait). */
boot_prog_err_t boot_prog_erase_idle(void);

/**
 * Start or continue idle-slot erase. *done is 1 when the slot is fully erased.
 * Returns BOOT_PROG_BUSY while sectors remain (UDS maps this to NRC 0x78).
 */
boot_prog_err_t boot_prog_erase_idle_step(int *done);

boot_prog_err_t boot_prog_begin(uint32_t addr, uint32_t size);
boot_prog_err_t boot_prog_write(const uint8_t *buf, uint32_t n);
boot_prog_err_t boot_prog_finish(void);

/** checkMemory: boot_verify_slot(idle). Does not change active_slot. */
boot_prog_err_t boot_prog_verify(int *out_vf);

/** P4 stub: always BOOT_PROG_UNSUPPORTED until boot_ctrl_t exists. */
boot_prog_err_t boot_prog_commit(void);

/** True if at least one slot verifies (used for the unlocked-session timeout). */
int boot_prog_has_valid_slot(void);

#ifdef __cplusplus
}
#endif

#endif /* BOOT_IMG_PROG_H */
