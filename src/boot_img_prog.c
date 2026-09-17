/**
 * @file boot_img_prog.c
 * @brief Idle-slot erase / program / verify. Flash only via hal_flash.
 */
#include "boot_img_prog.h"
#include "hal_flash.h"
#include "hal_error.h"
#include "osal_log.h"

#include <string.h>

/* osal_log_set_enabled() is in osal_log.h */

#define BOOT_PROG_ERASE_CHUNK     (8U)
#define BOOT_PROG_ALIGN           (8U)
#define BOOT_PROG_PAD_FF          (0xFFU)

typedef enum {
    BOOT_PROG_ST_IDLE = 0,
    BOOT_PROG_ST_ERASED,
    BOOT_PROG_ST_TRANSFER,
    BOOT_PROG_ST_CLOSED,
    BOOT_PROG_ST_VERIFIED
} boot_prog_st_t;

static boot_prog_enc_t s_enc;
static boot_prog_st_t s_st;
static uint8_t s_erased;
static uint8_t s_flash_ready;
static uint8_t s_idle_slot = 0xFFU;
static uint32_t s_header;
static uint32_t s_begin_addr;
static uint32_t s_expect_size;
static uint32_t s_written;
static uint8_t s_pad[BOOT_PROG_ALIGN];
static uint32_t s_pad_n;

static uint32_t s_erase_addr;
static uint32_t s_erase_left;
static uint8_t s_erase_active;

extern void osal_log_set_enabled(int on);

static uint32_t slot_header(uint32_t slot)
{
    return (0U == slot) ? SEC_BOOT_SLOT_A_HEADER_ADDR : SEC_BOOT_SLOT_B_HEADER_ADDR;
}

static uint32_t slot_image(uint32_t slot)
{
    return (0U == slot) ? SEC_BOOT_SLOT_A_IMAGE_ADDR : SEC_BOOT_SLOT_B_IMAGE_ADDR;
}

static int flash_ready(void)
{
    if (0U != s_flash_ready) {
        return 1;
    }
    if (HAL_ERR_SUCCESS != hal_flash_init()) {
        osal_log_info("[prog] flash init FAIL\r\n");
        return 0;
    }
    s_flash_ready = 1U;
    return 1;
}

static void reset_xfer(void)
{
    s_begin_addr = 0U;
    s_expect_size = 0U;
    s_written = 0U;
    s_pad_n = 0U;
    (void)memset(s_pad, (int)BOOT_PROG_PAD_FF, sizeof(s_pad));
}

static void fail_to_idle(void)
{
    s_st = BOOT_PROG_ST_IDLE;
    s_erased = 0U;
    s_erase_active = 0U;
    reset_xfer();
}

static void refresh_idle(void)
{
    boot_img_info_t info;
    int va;
    int vb;

    (void)memset(&info, 0, sizeof(info));
    va = boot_verify_slot(0U, &info);
    vb = boot_verify_slot(1U, &info);

    /* A OK → idle B; only B OK → idle A; both bad → idle A (IMG-D2). */
    if (BOOT_VF_OK == va) {
        s_idle_slot = 1U;
    } else if (BOOT_VF_OK == vb) {
        s_idle_slot = 0U;
    } else {
        s_idle_slot = 0U;
    }
    s_header = slot_header(s_idle_slot);
}

boot_prog_enc_t boot_prog_enc(void)
{
    return s_enc;
}

int boot_prog_lock_enc(boot_prog_enc_t enc)
{
    if ((BOOT_PROG_ENC_NONE == enc) || (BOOT_PROG_ENC_NONE == s_enc) || (s_enc == enc)) {
        s_enc = enc;
#if BOOT_UART_FLASH
        if (BOOT_PROG_ENC_UART == s_enc) {
            osal_log_set_enabled(0);
        }
#endif
        return 0;
    }
    return -1;
}

void boot_prog_unlock_enc(void)
{
    s_enc = BOOT_PROG_ENC_NONE;
    osal_log_set_enabled(1);
}

boot_prog_err_t boot_prog_query(boot_prog_query_t *out)
{
    if (NULL == out) {
        return BOOT_PROG_STATE;
    }
    refresh_idle();
    out->idle_slot = s_idle_slot;
    out->header_addr = s_header;
    out->image_addr = slot_image(s_idle_slot);
    out->max_payload = BOOT_PROG_SLOT_SPAN;
    return BOOT_PROG_OK;
}

static boot_prog_err_t erase_chunk(uint32_t nsec)
{
    int32_t rc;

    if (0U == nsec) {
        return BOOT_PROG_OK;
    }
    if (0 == flash_ready()) {
        return BOOT_PROG_IO;
    }
    rc = hal_flash_erase_sector(s_erase_addr, nsec);
    if (HAL_ERR_SUCCESS != rc) {
        return BOOT_PROG_IO;
    }
    s_erase_addr += nsec * HAL_FLASH_SECTOR_SIZE;
    s_erase_left -= nsec;
    return BOOT_PROG_OK;
}

boot_prog_err_t boot_prog_erase_idle_step(int *done)
{
    uint32_t n;
    boot_prog_err_t err;

    if (NULL != done) {
        *done = 0;
    }
    if (BOOT_PROG_ST_TRANSFER == s_st) {
        return BOOT_PROG_STATE;
    }

    if (0U == s_erase_active) {
        refresh_idle();
        s_erase_addr = s_header;
        s_erase_left = BOOT_PROG_SLOT_SPAN / HAL_FLASH_SECTOR_SIZE;
        s_erase_active = 1U;
        s_erased = 0U;
        s_st = BOOT_PROG_ST_IDLE;
        reset_xfer();
    }

    n = s_erase_left;
    if (n > BOOT_PROG_ERASE_CHUNK) {
        n = BOOT_PROG_ERASE_CHUNK;
    }
    err = erase_chunk(n);
    if (BOOT_PROG_OK != err) {
        fail_to_idle();
        return err;
    }
    if (0U != s_erase_left) {
        return BOOT_PROG_BUSY;
    }

    s_erase_active = 0U;
    s_erased = 1U;
    s_st = BOOT_PROG_ST_ERASED;
    if (NULL != done) {
        *done = 1;
    }
    osal_log_info("[prog] idle slot erased\r\n");
    return BOOT_PROG_OK;
}

boot_prog_err_t boot_prog_erase_idle(void)
{
    int done = 0;
    boot_prog_err_t err;

    do {
        err = boot_prog_erase_idle_step(&done);
    } while (BOOT_PROG_BUSY == err);

    return err;
}

boot_prog_err_t boot_prog_begin(uint32_t addr, uint32_t size)
{
    refresh_idle();

    if (0U == s_erased) {
        return BOOT_PROG_STATE;
    }
    if ((BOOT_PROG_ST_ERASED != s_st) && (BOOT_PROG_ST_IDLE != s_st)) {
        /* Allow restart only from erased/idle, not mid-transfer. */
        if (BOOT_PROG_ST_TRANSFER == s_st) {
            return BOOT_PROG_STATE;
        }
    }
    if ((addr != s_header) || (0U != (addr % BOOT_PROG_ALIGN))) {
        return BOOT_PROG_BAD_ADDR;
    }
    if ((size < SEC_BOOT_SLOT_HEADER_SIZE) || (size > BOOT_PROG_SLOT_SPAN)) {
        return BOOT_PROG_BAD_ADDR;
    }
    if (0 == flash_ready()) {
        return BOOT_PROG_IO;
    }

    reset_xfer();
    s_begin_addr = addr;
    s_expect_size = size;
    s_st = BOOT_PROG_ST_TRANSFER;
    return BOOT_PROG_OK;
}

static boot_prog_err_t flush_aligned(const uint8_t *data, uint32_t n)
{
    uint32_t dest = s_begin_addr + s_written;
    int32_t rc;

    if (0U == n) {
        return BOOT_PROG_OK;
    }
    rc = hal_flash_program(dest, data, n);
    if (HAL_ERR_SUCCESS != rc) {
        fail_to_idle();
        return BOOT_PROG_IO;
    }
    s_written += n;
    return BOOT_PROG_OK;
}

boot_prog_err_t boot_prog_write(const uint8_t *buf, uint32_t n)
{
    uint32_t room;
    uint32_t take;
    boot_prog_err_t err;

    if (BOOT_PROG_ST_TRANSFER != s_st) {
        return BOOT_PROG_STATE;
    }
    if ((0U == n) || (NULL == buf)) {
        return (0U == n) ? BOOT_PROG_OK : BOOT_PROG_STATE;
    }
    if ((s_written + s_pad_n + n) > s_expect_size) {
        fail_to_idle();
        return BOOT_PROG_STATE;
    }

    /* Fill remainder to 8 B, then program 8-aligned chunks. */
    if (s_pad_n > 0U) {
        take = BOOT_PROG_ALIGN - s_pad_n;
        if (take > n) {
            take = n;
        }
        (void)memcpy(&s_pad[s_pad_n], buf, take);
        s_pad_n += take;
        buf += take;
        n -= take;
        if (s_pad_n == BOOT_PROG_ALIGN) {
            err = flush_aligned(s_pad, BOOT_PROG_ALIGN);
            if (BOOT_PROG_OK != err) {
                return err;
            }
            s_pad_n = 0U;
        }
    }

    room = n - (n % BOOT_PROG_ALIGN);
    if (room > 0U) {
        err = flush_aligned(buf, room);
        if (BOOT_PROG_OK != err) {
            return err;
        }
        buf += room;
        n -= room;
    }
    if (n > 0U) {
        (void)memcpy(s_pad, buf, n);
        s_pad_n = n;
    }
    return BOOT_PROG_OK;
}

boot_prog_err_t boot_prog_finish(void)
{
    boot_prog_err_t err;

    if (BOOT_PROG_ST_TRANSFER != s_st) {
        return BOOT_PROG_STATE;
    }
    if ((s_written + s_pad_n) != s_expect_size) {
        fail_to_idle();
        return BOOT_PROG_STATE;
    }
    if (s_pad_n > 0U) {
        while (s_pad_n < BOOT_PROG_ALIGN) {
            s_pad[s_pad_n] = BOOT_PROG_PAD_FF;
            s_pad_n++;
        }
        err = flush_aligned(s_pad, BOOT_PROG_ALIGN);
        if (BOOT_PROG_OK != err) {
            return err;
        }
        /* Padding is not part of expect_size; written overshoots by pad. */
        s_written = s_expect_size;
        s_pad_n = 0U;
    }
    s_st = BOOT_PROG_ST_CLOSED;
    s_erased = 0U;
    return BOOT_PROG_OK;
}

boot_prog_err_t boot_prog_verify(int *out_vf)
{
    boot_img_info_t info;
    int vf;

    if (NULL != out_vf) {
        *out_vf = BOOT_VF_BAD_HDR;
    }
    if ((BOOT_PROG_ST_CLOSED != s_st) && (BOOT_PROG_ST_VERIFIED != s_st)
        && (BOOT_PROG_ST_ERASED != s_st) && (BOOT_PROG_ST_IDLE != s_st)) {
        /* Mid-transfer: refuse. Closed/idle/erased: allow (re-check). */
        if (BOOT_PROG_ST_TRANSFER == s_st) {
            return BOOT_PROG_STATE;
        }
    }

    refresh_idle();
    (void)memset(&info, 0, sizeof(info));
    vf = boot_verify_slot(s_idle_slot, &info);
    if (NULL != out_vf) {
        *out_vf = vf;
    }
    if (BOOT_VF_OK != vf) {
        s_st = BOOT_PROG_ST_IDLE;
        return BOOT_PROG_VERIFY;
    }
    s_st = BOOT_PROG_ST_VERIFIED;
    return BOOT_PROG_OK;
}

boot_prog_err_t boot_prog_commit(void)
{
    (void)s_st;
    return BOOT_PROG_UNSUPPORTED;
}

int boot_prog_has_valid_slot(void)
{
    boot_img_info_t info;
    int va;
    int vb;

    (void)memset(&info, 0, sizeof(info));
    va = boot_verify_slot(0U, &info);
    vb = boot_verify_slot(1U, &info);
    return ((BOOT_VF_OK == va) || (BOOT_VF_OK == vb)) ? 1 : 0;
}
