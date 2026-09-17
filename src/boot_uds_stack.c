/**
 * @file boot_uds_stack.c
 * @brief Bare-metal iso14229 + isotp-c poll loop for Boot programming.
 */
#include "boot_uds.h"
#include "boot_can.h"
#include "boot_img_prog.h"
#include "boot_uart_prog.h"
#include "boot.h"
#include "essfw.h"
#include "iso14229_port.h"
#include "osal_log.h"
#include "leds_ctrl.h"
#include "Siul2_Dio_Ip.h"
#include "Siul2_Port_Ip_Cfg.h"

#include <string.h>

/* DWT CYCCNT @ assumed 120 MHz (same as osal_utils). Wrap ~9.9 h. */
#define BOOT_UDS_CORE_HZ     (120000000UL)
#define BOOT_UDS_CYCLES_MS   (BOOT_UDS_CORE_HZ / 1000UL)
#define DWT_CTRL             (*(volatile uint32_t *)0xE0001000UL)
#define DWT_CYCCNT           (*(volatile uint32_t *)0xE0001004UL)
#define SCB_DEMCR            (*(volatile uint32_t *)0xE000EDFCUL)
#define DEMCR_TRCENA         (1UL << 24)
#define DWT_CTRL_CYCCNTENA   (1UL)

#define BOOT_UDS_S3_MS           (60000U)
#define BOOT_UDS_P2_MS           (3000U)
#define BOOT_UDS_P2_STAR_MS      (8000U)
#define BOOT_PROG_UNLOCK_TO_MS   (30000U)

extern UDSErr_t boot_uds_app_fn(UDSServer_t *srv, UDSEvent_t event, void *arg);

static UDSServer_t s_srv;
static UDSISOTpC_t s_tp;

uint32_t UDSMillis(void)
{
    return DWT_CYCCNT / BOOT_UDS_CYCLES_MS;
}

uint32_t isotp_user_get_us(void)
{
    return UDSMillis() * 1000U;
}

void isotp_user_debug(const char *message, ...)
{
    (void)message;
}

int isotp_user_send_can(const uint32_t arbitration_id, const uint8_t *data,
                        const uint8_t size, void *arg)
{
    int rc;

    (void)arg;
    if ((NULL == data) || (size > 8U)) {
        return ISOTP_RET_ERROR;
    }
    rc = boot_can_send(arbitration_id, data, size);
    if (0 == rc) {
        return ISOTP_RET_OK;
    }
    if (1 == rc) {
        return ISOTP_RET_NOSPACE;
    }
    return ISOTP_RET_ERROR;
}

static void boot_uds_on_can_rx(const boot_can_frame_t *frame, void *user)
{
    UDSISOTpC_t *tp = (UDSISOTpC_t *)user;
    uint8_t dlc = frame->dlc;

    if (dlc > 8U) {
        dlc = 8U;
    }

    if (frame->id == tp->phys_sa) {
        isotp_on_can_message(&tp->phys_link, frame->data, dlc);
    } else if (frame->id == tp->func_sa) {
        if (ISOTP_RECEIVE_STATUS_IDLE != tp->phys_link.receive_status) {
            return;
        }
        isotp_on_can_message(&tp->func_link, frame->data, dlc);
    }
}

static void dwt_millis_init(void)
{
    SCB_DEMCR |= DEMCR_TRCENA;
    DWT_CYCCNT = 0U;
    DWT_CTRL |= DWT_CTRL_CYCCNTENA;
}

static void uds_led_heartbeat(void)
{
    static uint32_t s_last;
    static uint8_t s_on;
    uint32_t now = UDSMillis();

    if ((now - s_last) < 250U) {
        return;
    }
    s_last = now;
    s_on = (uint8_t)(0U == s_on);
    if (BOOT_PROG_ENC_UART == boot_prog_enc()) {
        Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, s_on);
        Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, 0U);
    } else {
        Siul2_Dio_Ip_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, s_on);
        Siul2_Dio_Ip_WritePin(LED_RED_PORT, LED_RED_PIN, 0U);
    }
    Siul2_Dio_Ip_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, 0U);
}

static void maybe_jump_unlocked(uint32_t t0)
{
    boot_img_info_t info;
    int rc;

    if (BOOT_PROG_ENC_NONE != boot_prog_enc()) {
        return;
    }
    if ((UDSMillis() - t0) < BOOT_PROG_UNLOCK_TO_MS) {
        return;
    }
    if (0 == boot_prog_has_valid_slot()) {
        return;
    }
    osal_log_info("[SECBOOT] programming idle timeout → App\r\n");
    rc = boot_verify_preferred(&info);
    if (BOOT_VF_OK == rc) {
        boot_jump_xip(info.load_addr);
    }
}

void boot_uds_run(void)
{
    static const UDSISOTpCConfig_t cfg = {
        .source_addr = BOOT_CAN_UDS_PHYS_REQ_ID,
        .target_addr = BOOT_CAN_UDS_PHYS_RESP_ID,
        .source_addr_func = BOOT_CAN_UDS_FUNC_REQ_ID,
        .target_addr_func = UDS_TP_NOOP_ADDR,
    };
    uint32_t last_log = 0U;
    uint32_t t0;

    dwt_millis_init();
    t0 = UDSMillis();

    (void)memset(&s_tp, 0, sizeof(s_tp));
    (void)memset(&s_srv, 0, sizeof(s_srv));

    if (0 != boot_can_init()) {
        osal_log_info("[SECBOOT] CAN init FAIL\r\n");
        leds_ctrl_boot_led_blink_failure();
        while (1) {
        }
    }

    if (UDS_OK != UDSISOTpCInit(&s_tp, &cfg)) {
        osal_log_info("[uds] tp init FAIL\r\n");
        while (1) {
        }
    }
    if (UDS_OK != UDSServerInit(&s_srv)) {
        osal_log_info("[uds] srv init FAIL\r\n");
        while (1) {
        }
    }

    /* SW6 / magic already left us in Boot: start in Programming so 0x31/0x34
     * work without a second 10 02 (BOOT_UDS_APP_FLASHING §4). */
    s_srv.sessionType = UDS_LEV_DS_PRGS;
    s_srv.p2_ms = BOOT_UDS_P2_MS;
    s_srv.p2_star_ms = BOOT_UDS_P2_STAR_MS;
    s_srv.s3_ms = BOOT_UDS_S3_MS;
    s_srv.s3_session_timeout_timer = UDSMillis() + s_srv.s3_ms;
    s_srv.tp = &s_tp.hdl;
    s_srv.fn = boot_uds_app_fn;

    boot_can_set_rx_cb(boot_uds_on_can_rx, &s_tp);

    osal_log_info("[SECBOOT] UDS listen phys=0x7E0 resp=0x7E8 func=0x7DF\r\n");

    for (;;) {
        boot_can_poll();
        UDSServerPoll(&s_srv);
        boot_uart_prog_poll();
        uds_led_heartbeat();
        maybe_jump_unlocked(t0);
        if ((UDSMillis() - last_log) >= 5000U) {
            last_log = UDSMillis();
            if (BOOT_PROG_ENC_UART != boot_prog_enc()) {
                osal_log_info("[SECBOOT] UDS listening\r\n");
            }
        }
    }
}
