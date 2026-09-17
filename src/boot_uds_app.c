/**
 * @file boot_uds_app.c
 * @brief Boot UDS fn: session, reset, F180, A0F1, 0x31 FF00/0202, 0x34/36/37.
 */
#include "iso14229_port.h"
#include "boot_img_prog.h"
#include "boot_uds.h"
#include "build_timestamp.h"
#include "osal_log.h"
#include "S32K312.h"
#include "S32K312_SCB.h"

#include <string.h>

#define UDS_DID_BOOT_SW_ID (0xF180U)
#define UDS_DID_IDLE_SLOT  (0xA0F1U)
#define UDS_F180_LEN       (32U)
#define UDS_SESS_P2_MS     (3000U)
#define UDS_SESS_P2_STAR_MS (8000U)

#define BOOT_RID_ERASE_IDLE  (0xFF00U)
#define BOOT_RID_CHECK_MEM   (0x0202U)
#define BOOT_RID_COMMIT      (0x0203U)

#define BOOT_MCU_AIRCR_VECTKEY     (0x05FAUL << 16)
#define BOOT_MCU_AIRCR_PRIGROUP_M (0x7UL << 8)
#define BOOT_MCU_AIRCR_SYSRESETREQ (1UL << 2)

void boot_mcu_system_reset(void)
{
    S32_SCB->AIRCR = (uint32_t)(BOOT_MCU_AIRCR_VECTKEY |
                                 (S32_SCB->AIRCR & BOOT_MCU_AIRCR_PRIGROUP_M) |
                                 BOOT_MCU_AIRCR_SYSRESETREQ);
    for (;;) {
    }
}

static void u32_to_be(uint32_t v, uint8_t out[4])
{
    out[0] = (uint8_t)((v >> 24) & 0xFFU);
    out[1] = (uint8_t)((v >> 16) & 0xFFU);
    out[2] = (uint8_t)((v >> 8) & 0xFFU);
    out[3] = (uint8_t)(v & 0xFFU);
}

static UDSErr_t nrc_from_prog(boot_prog_err_t e)
{
    switch (e) {
    case BOOT_PROG_OK:
        return UDS_PositiveResponse;
    case BOOT_PROG_BUSY:
        return UDS_NRC_RequestCorrectlyReceived_ResponsePending;
    case BOOT_PROG_DENIED:
        return UDS_NRC_SecurityAccessDenied;
    case BOOT_PROG_BAD_ADDR:
        return UDS_NRC_RequestOutOfRange;
    case BOOT_PROG_IO:
    case BOOT_PROG_VERIFY:
        return UDS_NRC_GeneralProgrammingFailure;
    case BOOT_PROG_UNSUPPORTED:
        return UDS_NRC_ServiceNotSupported;
    case BOOT_PROG_STATE:
    default:
        return UDS_NRC_RequestSequenceError;
    }
}

static int session_is_default(const UDSServer_t *srv)
{
    return (NULL == srv) || (UDS_LEV_DS_DS == srv->sessionType);
}

static UDSErr_t reject_if_default(const UDSServer_t *srv)
{
    if (session_is_default(srv)) {
        return UDS_NRC_ServiceNotSupportedInActiveSession;
    }
    return UDS_PositiveResponse;
}

/** Lock UDS on first diagnostic request; download SIDs fail if UART won. */
static UDSErr_t lock_or_reject_download(void)
{
    if (BOOT_PROG_ENC_UART == boot_prog_enc()) {
        return UDS_NRC_RequestSequenceError;
    }
    if (0 != boot_prog_lock_enc(BOOT_PROG_ENC_UDS)) {
        return UDS_NRC_RequestSequenceError;
    }
    return UDS_PositiveResponse;
}

static UDSErr_t lock_uds_soft(void)
{
    /* Identity / tester present: lock UDS if still unlocked. */
    if (BOOT_PROG_ENC_NONE == boot_prog_enc()) {
        (void)boot_prog_lock_enc(BOOT_PROG_ENC_UDS);
    }
    return UDS_PositiveResponse;
}

static UDSErr_t check_l2(const UDSServer_t *srv)
{
#if BOOT_UDS_L2_SKIP
    (void)srv;
    return UDS_PositiveResponse;
#else
    if ((NULL == srv) || (0U == srv->securityLevel)) {
        return UDS_NRC_SecurityAccessDenied;
    }
    return UDS_PositiveResponse;
#endif
}

static UDSErr_t on_diag_sess(UDSServer_t *srv, UDSDiagSessCtrlArgs_t *args)
{
    (void)srv;
    (void)lock_uds_soft();
    switch (args->type) {
    case UDS_LEV_DS_DS:
        return UDS_PositiveResponse;
    case UDS_LEV_DS_PRGS:
    case UDS_LEV_DS_EXTDS:
        args->p2_ms = UDS_SESS_P2_MS;
        args->p2_star_ms = UDS_SESS_P2_STAR_MS;
        return UDS_PositiveResponse;
    default:
        return UDS_NRC_SubFunctionNotSupported;
    }
}

static UDSErr_t on_ecu_reset(UDSServer_t *srv, UDSECUResetArgs_t *args)
{
    (void)srv;
    if (NULL == args) {
        return UDS_NRC_GeneralReject;
    }
    switch (args->type) {
    case UDS_LEV_RT_HR:
    case UDS_LEV_RT_KOFFONR:
    case UDS_LEV_RT_SR:
        osal_log_info("[uds][11] accept reset (Boot)\r\n");
        return UDS_PositiveResponse;
    default:
        return UDS_NRC_SubFunctionNotSupported;
    }
}

static UDSErr_t on_do_scheduled_reset(uint8_t *reset_type)
{
    (void)reset_type;
    osal_log_info("[uds][11] DoScheduledReset\r\n");
    boot_mcu_system_reset();
    return UDS_PositiveResponse;
}

static UDSErr_t on_rdbi(UDSServer_t *srv, UDSRDBIArgs_t *args)
{
    uint8_t f180[UDS_F180_LEN];
    uint8_t a0f1[13];
    size_t n;
    const char *stamp = BUILD_TIMESTAMP_MADRID;
    boot_prog_query_t q;
    boot_prog_err_t err;

    (void)lock_uds_soft();

    switch (args->dataId) {
    case UDS_DID_BOOT_SW_ID:
        (void)memset(f180, (int)' ', UDS_F180_LEN);
        n = strlen("easy_boot ");
        if (n > UDS_F180_LEN) {
            n = UDS_F180_LEN;
        }
        (void)memcpy(f180, "easy_boot ", n);
        {
            size_t sl = strlen(stamp);
            size_t room = (n < UDS_F180_LEN) ? (UDS_F180_LEN - n) : 0U;
            if (sl > room) {
                sl = room;
            }
            if (sl > 0U) {
                (void)memcpy(&f180[n], stamp, sl);
            }
        }
        return (UDSErr_t)args->copy(srv, f180, UDS_F180_LEN);

    case UDS_DID_IDLE_SLOT:
        err = boot_prog_query(&q);
        if (BOOT_PROG_OK != err) {
            return nrc_from_prog(err);
        }
        /* 13 B, same as UART QUERY / tester kA0F1PayloadLen. No padding. */
        a0f1[0] = (uint8_t)q.idle_slot;
        u32_to_be(q.header_addr, &a0f1[1]);
        u32_to_be(q.image_addr, &a0f1[5]);
        u32_to_be(q.max_payload, &a0f1[9]);
        return (UDSErr_t)args->copy(srv, a0f1, 13U);

    default:
        return UDS_NRC_RequestOutOfRange;
    }
}

static UDSErr_t on_routine(UDSServer_t *srv, UDSRoutineCtrlArgs_t *args)
{
    UDSErr_t gate;
    boot_prog_err_t err;
    int done = 0;
    int vf = 0;
    uint8_t rec;

    gate = reject_if_default(srv);
    if (UDS_PositiveResponse != gate) {
        return gate;
    }
    gate = lock_or_reject_download();
    if (UDS_PositiveResponse != gate) {
        return gate;
    }
    if (UDS_LEV_RCTP_STR != args->ctrlType) {
        return UDS_NRC_SubFunctionNotSupported;
    }

    switch (args->id) {
    case BOOT_RID_ERASE_IDLE:
        gate = check_l2(srv);
        if (UDS_PositiveResponse != gate) {
            return gate;
        }
        err = boot_prog_erase_idle_step(&done);
        return nrc_from_prog(err);

    case BOOT_RID_CHECK_MEM:
        err = boot_prog_verify(&vf);
        rec = (uint8_t)vf;
        if (NULL != args->copyStatusRecord) {
            (void)args->copyStatusRecord(srv, &rec, 1U);
        }
        return nrc_from_prog(err);

    case BOOT_RID_COMMIT:
        err = boot_prog_commit();
        return nrc_from_prog(err);

    default:
        return UDS_NRC_RequestOutOfRange;
    }
}

static UDSErr_t on_download(UDSServer_t *srv, UDSRequestDownloadArgs_t *args)
{
    UDSErr_t gate;
    boot_prog_err_t err;
    uint32_t addr;

    gate = reject_if_default(srv);
    if (UDS_PositiveResponse != gate) {
        return gate;
    }
    gate = lock_or_reject_download();
    if (UDS_PositiveResponse != gate) {
        return gate;
    }
    gate = check_l2(srv);
    if (UDS_PositiveResponse != gate) {
        return gate;
    }
    if (0U != args->dataFormatIdentifier) {
        return UDS_NRC_RequestOutOfRange;
    }
    addr = (uint32_t)(uintptr_t)args->addr;
    args->maxNumberOfBlockLength = (uint16_t)UDS_TP_MTU;
    err = boot_prog_begin(addr, (uint32_t)args->size);
    return nrc_from_prog(err);
}

static UDSErr_t on_xfer(UDSServer_t *srv, UDSTransferDataArgs_t *args)
{
    UDSErr_t gate;
    boot_prog_err_t err;

    (void)srv;
    gate = lock_or_reject_download();
    if (UDS_PositiveResponse != gate) {
        return gate;
    }
    err = boot_prog_write(args->data, args->len);
    return nrc_from_prog(err);
}

static UDSErr_t on_xfer_exit(UDSServer_t *srv, UDSRequestTransferExitArgs_t *args)
{
    UDSErr_t gate;
    boot_prog_err_t err;

    (void)srv;
    (void)args;
    gate = lock_or_reject_download();
    if (UDS_PositiveResponse != gate) {
        return gate;
    }
    err = boot_prog_finish();
    return nrc_from_prog(err);
}

UDSErr_t boot_uds_app_fn(UDSServer_t *srv, UDSEvent_t event, void *arg)
{
    switch (event) {
    case UDS_EVT_DiagSessCtrl:
        return on_diag_sess(srv, (UDSDiagSessCtrlArgs_t *)arg);
    case UDS_EVT_EcuReset:
        return on_ecu_reset(srv, (UDSECUResetArgs_t *)arg);
    case UDS_EVT_DoScheduledReset:
        return on_do_scheduled_reset((uint8_t *)arg);
    case UDS_EVT_ReadDataByIdent:
        return on_rdbi(srv, (UDSRDBIArgs_t *)arg);
    case UDS_EVT_RoutineCtrl:
        return on_routine(srv, (UDSRoutineCtrlArgs_t *)arg);
    case UDS_EVT_RequestDownload:
        return on_download(srv, (UDSRequestDownloadArgs_t *)arg);
    case UDS_EVT_TransferData:
        return on_xfer(srv, (UDSTransferDataArgs_t *)arg);
    case UDS_EVT_RequestTransferExit:
        return on_xfer_exit(srv, (UDSRequestTransferExitArgs_t *)arg);
    case UDS_EVT_SessionTimeout:
        osal_log_info("[uds] S3 timeout → Default\r\n");
        return UDS_PositiveResponse;
    default:
        return UDS_NRC_ServiceNotSupported;
    }
}
