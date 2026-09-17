/**
 * @file boot_uart_prog.c
 * @brief UART command encoding → boot_img_prog (proposal §6).
 *
 * Frame: STX AA 55 | cmd u8 | seq u8 | len u16 BE | payload | crc8(SMBUS of cmd..payload)
 */
#include "boot_uart_prog.h"
#include "boot_img_prog.h"
#include "boot_uds.h"
#include "hal_uart.h"
#include "hal_error.h"
#include "osal_log.h"
#include "osal_utils.h"

#include <string.h>

extern HAL_UART lpuart6;

#if BOOT_UART_FLASH

#define UART_STX0          (0xAAU)
#define UART_STX1          (0x55U)
#define UART_CMD_ACK       (0x00U)
#define UART_CMD_QUERY     (0x01U)
#define UART_CMD_ERASE     (0x02U)
#define UART_CMD_BEGIN     (0x03U)
#define UART_CMD_DATA      (0x04U)
#define UART_CMD_END       (0x05U)
#define UART_CMD_VERIFY    (0x06U)
#define UART_CMD_COMMIT    (0x07U)
#define UART_CMD_ENTER     (0x10U)
#define UART_CMD_RESET     (0x11U)
#define UART_MAX_PAYLOAD   (256U)
#define UART_MAX_FRAME     (6U + UART_MAX_PAYLOAD + 1U)

static uint8_t s_rx[UART_MAX_FRAME];
static uint32_t s_rx_n;
static uint8_t s_line[16];
static uint32_t s_line_n;
static uint8_t s_ascii_mode = 1U; /* ESSFLASH / ENTER ASCII until first binary STX */

static uint8_t crc8_smbus(const uint8_t *p, uint32_t n)
{
    uint8_t crc = 0U;
    uint32_t i;
    uint8_t b;

    for (i = 0U; i < n; i++) {
        crc ^= p[i];
        for (b = 0U; b < 8U; b++) {
            if (0U != (crc & 0x80U)) {
                crc = (uint8_t)((crc << 1) ^ 0x07U);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

static uint32_t be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static void put_be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)((v >> 24) & 0xFFU);
    p[1] = (uint8_t)((v >> 16) & 0xFFU);
    p[2] = (uint8_t)((v >> 8) & 0xFFU);
    p[3] = (uint8_t)(v & 0xFFU);
}

static void uart_send(const uint8_t *data, uint32_t n)
{
    (void)hal_uart_transmit_it(&lpuart6, (uint8_t *)data, n);
}

static void send_ack(uint8_t seq, uint8_t err, const uint8_t *extra, uint32_t extra_n)
{
    uint8_t frame[16];
    uint32_t n = 0U;
    uint8_t crc;

    frame[n++] = UART_STX0;
    frame[n++] = UART_STX1;
    frame[n++] = UART_CMD_ACK;
    frame[n++] = seq;
    frame[n++] = (uint8_t)(((1U + extra_n) >> 8) & 0xFFU);
    frame[n++] = (uint8_t)((1U + extra_n) & 0xFFU);
    frame[n++] = err;
    if ((NULL != extra) && (extra_n > 0U) && ((n + extra_n) < sizeof(frame))) {
        (void)memcpy(&frame[n], extra, extra_n);
        n += extra_n;
    }
    crc = crc8_smbus(&frame[2], n - 2U);
    frame[n++] = crc;
    uart_send(frame, n);
}

static uint8_t map_err(boot_prog_err_t e)
{
    return (uint8_t)e;
}

static void handle_cmd(uint8_t cmd, uint8_t seq, const uint8_t *pl, uint32_t len)
{
    boot_prog_query_t q;
    boot_prog_err_t err;
    uint8_t extra[16];
    uint32_t extra_n = 0U;
    int vf = 0;
    uint32_t addr;
    uint32_t size;

    if (UART_CMD_ENTER == cmd) {
        if (0 != boot_prog_lock_enc(BOOT_PROG_ENC_UART)) {
            send_ack(seq, (uint8_t)BOOT_PROG_STATE, NULL, 0U);
            return;
        }
        s_ascii_mode = 0U;
        send_ack(seq, (uint8_t)BOOT_PROG_OK, NULL, 0U);
        return;
    }

    if (BOOT_PROG_ENC_UART != boot_prog_enc()) {
        /* SW6 first-wins: ENTER not yet seen. */
        send_ack(seq, (uint8_t)BOOT_PROG_STATE, NULL, 0U);
        return;
    }

    switch (cmd) {
    case UART_CMD_QUERY:
        err = boot_prog_query(&q);
        if (BOOT_PROG_OK == err) {
            extra[0] = (uint8_t)q.idle_slot;
            put_be32(&extra[1], q.header_addr);
            put_be32(&extra[5], q.image_addr);
            put_be32(&extra[9], q.max_payload);
            extra_n = 13U;
        }
        send_ack(seq, map_err(err), extra, extra_n);
        break;
    case UART_CMD_ERASE:
        err = boot_prog_erase_idle();
        send_ack(seq, map_err(err), NULL, 0U);
        break;
    case UART_CMD_BEGIN:
        if (len < 8U) {
            send_ack(seq, (uint8_t)BOOT_PROG_STATE, NULL, 0U);
            break;
        }
        addr = be32(&pl[0]);
        size = be32(&pl[4]);
        err = boot_prog_begin(addr, size);
        send_ack(seq, map_err(err), NULL, 0U);
        break;
    case UART_CMD_DATA:
        err = boot_prog_write(pl, len);
        send_ack(seq, map_err(err), NULL, 0U);
        break;
    case UART_CMD_END:
        err = boot_prog_finish();
        send_ack(seq, map_err(err), NULL, 0U);
        break;
    case UART_CMD_VERIFY:
        err = boot_prog_verify(&vf);
        extra[0] = (uint8_t)vf;
        send_ack(seq, map_err(err), extra, 1U);
        break;
    case UART_CMD_COMMIT:
        err = boot_prog_commit();
        send_ack(seq, map_err(err), NULL, 0U);
        break;
    case UART_CMD_RESET:
        send_ack(seq, (uint8_t)BOOT_PROG_OK, NULL, 0U);
        osal_utils_delay_ms(20U);
        boot_mcu_system_reset();
        break;
    default:
        send_ack(seq, (uint8_t)BOOT_PROG_STATE, NULL, 0U);
        break;
    }
}

static int try_byte(uint8_t *b)
{
    int32_t rc = hal_uart_receive(&lpuart6, b, 1U, 2U);

    if (HAL_ERR_SUCCESS == rc) {
        return 1;
    }
    return 0;
}

static void feed_ascii(uint8_t b)
{
    if ((b == (uint8_t)'\r') || (b == (uint8_t)'\n')) {
        if (s_line_n >= 8U) {
            /* ESSFLASH */
            if ((s_line[0] == (uint8_t)'E' || s_line[0] == (uint8_t)'e') &&
                (0 == memcmp(s_line, "ESSFLASH", 8) ||
                 0 == memcmp(s_line, "essflash", 8))) {
                if (0 == boot_prog_lock_enc(BOOT_PROG_ENC_UART)) {
                    s_ascii_mode = 0U;
                    uart_send((const uint8_t *)"OK\r\n", 4U);
                }
            }
        }
        s_line_n = 0U;
        return;
    }
    if (s_line_n < (sizeof(s_line) - 1U)) {
        if ((b >= (uint8_t)'a') && (b <= (uint8_t)'z')) {
            b = (uint8_t)(b - (uint8_t)'a' + (uint8_t)'A');
        }
        s_line[s_line_n++] = b;
    } else {
        s_line_n = 0U;
    }
}

static void feed_bin(uint8_t b)
{
    uint32_t need;
    uint32_t len;
    uint8_t crc;

    if (s_rx_n < UART_MAX_FRAME) {
        s_rx[s_rx_n++] = b;
    } else {
        s_rx_n = 0U;
        return;
    }

    if (1U == s_rx_n) {
        if (s_rx[0] != UART_STX0) {
            s_rx_n = 0U;
        }
        return;
    }
    if (2U == s_rx_n) {
        if (s_rx[1] != UART_STX1) {
            s_rx_n = (s_rx[1] == UART_STX0) ? 1U : 0U;
            if (1U == s_rx_n) {
                s_rx[0] = UART_STX0;
            }
        }
        return;
    }
    if (s_rx_n < 6U) {
        return;
    }
    len = ((uint32_t)s_rx[4] << 8) | (uint32_t)s_rx[5];
    if (len > UART_MAX_PAYLOAD) {
        s_rx_n = 0U;
        return;
    }
    need = 6U + len + 1U;
    if (s_rx_n < need) {
        return;
    }
    crc = crc8_smbus(&s_rx[2], 4U + len);
    if (crc != s_rx[6U + len]) {
        s_rx_n = 0U;
        return;
    }
    handle_cmd(s_rx[2], s_rx[3], &s_rx[6], len);
    s_rx_n = 0U;
}

void boot_uart_prog_poll(void)
{
    uint8_t b;
    uint32_t n = 0U;

    while ((n < 32U) && (0 != try_byte(&b))) {
        n++;
        if ((1U == s_ascii_mode) && (BOOT_PROG_ENC_UART != boot_prog_enc())) {
            if (b == UART_STX0) {
                s_ascii_mode = 0U;
                s_rx_n = 0U;
                feed_bin(b);
            } else {
                feed_ascii(b);
            }
        } else {
            s_ascii_mode = 0U;
            feed_bin(b);
        }
    }
}

int boot_uart_wait_essflash(uint32_t window_ms)
{
    uint32_t waited = 0U;
    uint8_t b;

    osal_log_info("[SECBOOT] UART flash: send ESSFLASH within 2s, or FLASH from App\r\n");

    s_ascii_mode = 1U;
    s_line_n = 0U;
    while (waited < window_ms) {
        if (0 != try_byte(&b)) {
            feed_ascii(b);
            if (BOOT_PROG_ENC_UART == boot_prog_enc()) {
                return 1;
            }
        } else {
            osal_utils_delay_ms(10U);
            waited += 10U;
        }
    }
    return (BOOT_PROG_ENC_UART == boot_prog_enc()) ? 1 : 0;
}

#else /* !BOOT_UART_FLASH */

void boot_uart_prog_poll(void)
{
}

int boot_uart_wait_essflash(uint32_t window_ms)
{
    (void)window_ms;
    return 0;
}

#endif /* BOOT_UART_FLASH */
