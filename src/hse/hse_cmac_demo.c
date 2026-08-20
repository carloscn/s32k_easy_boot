/**
 * @file hse_cmac_demo.c
 * @brief See hse_cmac_demo.h.
 *
 * Ported from s32k312_provision CMAC path (D-cache clean/invalidate, uint32_t
 * pTagLength, SRAM globals). Vectors match s32k_demo/src/test_cmac.c.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Hse_Ip.h"
#include "hse_cmac_demo.h"
#include "osal_log.h"
#include "S32K312_SCB.h"
#include "Mcal.h"

#define HSE_MU_INSTANCE   (0U)
#define HSE_DCACHE_LINE   (32U)
#define HSE_SYNC_REQ_TIMEOUT (0xFFFFFFFFUL)
#define LOG_BUF_SIZE      (192U)
#define SECOC_CMAC_TRUNC_BYTES (3U)

/* Must match s32k312_provision CMAC_SMOKE_TEST_KEY_HANDLE (NVM CUST AES-128 g0s0). */
#define PROVISIONED_SECOC_KEY_HANDLE \
    GET_KEY_HANDLE(HSE_KEY_CATALOG_ID_NVM, 0U, 0U)

static Hse_Ip_MuStateType s_hse_mu_state;
static hseSrvDescriptor_t s_hse_srv_desc;
static hseKeyInfo_t s_hse_key_info;
static uint8_t s_cmac_tag[16];
static uint32_t s_cmac_tag_len;

/* Same DataToAuthenticator as provision / test_cmac.c (key material lives in HSE NVM). */
static const uint8_t s_secoc_data_to_auth[22] = {
    0x03, 0x09,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t s_cmac_input[sizeof(s_secoc_data_to_auth)];

static const uint8_t s_secoc_expected_mac_trunc[3] = { 0x6A, 0x0E, 0x6D };
static const uint8_t s_secoc_expected_mac_full[16] = {
    0x6A, 0x0E, 0x6D, 0x87, 0xC6, 0xF9, 0x0E, 0x16,
    0x5D, 0x05, 0x8C, 0x2C, 0xF9, 0x06, 0x15, 0xE2
};

static void hse_dcache_clean(const void *addr, size_t len)
{
    if ((NULL == addr) || (0U == len)) {
        return;
    }
    uintptr_t start = (uintptr_t)addr & ~(uintptr_t)(HSE_DCACHE_LINE - 1U);
    uintptr_t end = ((uintptr_t)addr + len + (HSE_DCACHE_LINE - 1U))
                    & ~(uintptr_t)(HSE_DCACHE_LINE - 1U);
    for (uintptr_t a = start; a < end; a += HSE_DCACHE_LINE) {
        S32_SCB->DCCMVAC = (uint32_t)a;
    }
    __asm volatile ("dsb 0xF" ::: "memory");
    __asm volatile ("isb 0xF" ::: "memory");
}

static void hse_dcache_invalidate(const void *addr, size_t len)
{
    if ((NULL == addr) || (0U == len)) {
        return;
    }
    uintptr_t start = (uintptr_t)addr & ~(uintptr_t)(HSE_DCACHE_LINE - 1U);
    uintptr_t end = ((uintptr_t)addr + len + (HSE_DCACHE_LINE - 1U))
                    & ~(uintptr_t)(HSE_DCACHE_LINE - 1U);
    for (uintptr_t a = start; a < end; a += HSE_DCACHE_LINE) {
        S32_SCB->DCIMVAC = (uint32_t)a;
    }
    __asm volatile ("dsb 0xF" ::: "memory");
    __asm volatile ("isb 0xF" ::: "memory");
}

static void log_line(const char *fmt, ...)
{
    char buf[LOG_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    osal_log_info(buf);
}

static const char *hse_rsp_name(hseSrvResponse_t rsp)
{
    switch (rsp) {
    case HSE_SRV_RSP_OK:                 return "OK";
    case HSE_SRV_RSP_INVALID_PARAM:      return "INVALID_PARAM";
    case HSE_SRV_RSP_NOT_SUPPORTED:      return "NOT_SUPPORTED";
    case HSE_SRV_RSP_NOT_ALLOWED:        return "NOT_ALLOWED";
    case HSE_SRV_RSP_NOT_ENOUGH_SPACE:   return "NOT_ENOUGH_SPACE";
    case HSE_SRV_RSP_READ_FAILURE:       return "READ_FAILURE";
    case HSE_SRV_RSP_WRITE_FAILURE:      return "WRITE_FAILURE";
    case HSE_SRV_RSP_INVALID_ADDR:       return "INVALID_ADDR";
    case HSE_SRV_RSP_KEY_EMPTY:          return "KEY_EMPTY";
    case HSE_SRV_RSP_KEY_NOT_AVAILABLE:  return "KEY_NOT_AVAILABLE";
    case HSE_SRV_RSP_KEY_INVALID:        return "KEY_INVALID";
    case HSE_SRV_RSP_VERIFY_FAILED:      return "VERIFY_FAILED";
    case HSE_SRV_RSP_GENERAL_ERROR:      return "GENERAL_ERROR";
    default:
        if (rsp == (hseSrvResponse_t)0xBB55BB55UL) {
            return "NO_RESPONSE(timeout)";
        }
        return "?";
    }
}

static hseSrvResponse_t hse_send_sync(const char *step, hseSrvDescriptor_t *desc)
{
    s_hse_srv_desc = *desc;
    desc = &s_hse_srv_desc;
    hse_dcache_clean(desc, sizeof(s_hse_srv_desc));

    uint8_t channel = Hse_Ip_GetFreeChannel(HSE_MU_INSTANCE);
    if (HSE_IP_INVALID_MU_CHANNEL_U8 == channel) {
        log_line("[hse_cmac] %s: no free MU channel\r\n", step);
        return HSE_SRV_RSP_GENERAL_ERROR;
    }

    Hse_Ip_ReqType req;
    memset(&req, 0, sizeof(req));
    req.eReqType = HSE_IP_REQTYPE_SYNC;
    req.u32Timeout = HSE_SYNC_REQ_TIMEOUT;

    log_line("[hse_cmac] >>> %s ch=%u\r\n", step, (unsigned)channel);
    hseSrvResponse_t rsp = Hse_Ip_ServiceRequest(HSE_MU_INSTANCE, channel, &req, desc);
    Hse_Ip_ReleaseChannel(HSE_MU_INSTANCE, channel);
    log_line("[hse_cmac] <<< %s rsp=0x%08lX (%s)\r\n",
             step, (unsigned long)rsp, hse_rsp_name(rsp));
    return rsp;
}

static hseSrvResponse_t hse_get_key_info(hseKeyHandle_t keyHandle)
{
    memset(&s_hse_key_info, 0, sizeof(s_hse_key_info));
    hse_dcache_clean(&s_hse_key_info, sizeof(s_hse_key_info));

    hseSrvDescriptor_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.srvId = HSE_SRV_ID_GET_KEY_INFO;
    desc.hseSrv.getKeyInfoReq.keyHandle = keyHandle;
    desc.hseSrv.getKeyInfoReq.pKeyInfo = (HOST_ADDR)(uintptr_t)&s_hse_key_info;

    hseSrvResponse_t rsp = hse_send_sync("get_key_info", &desc);
    if (HSE_SRV_RSP_OK == rsp) {
        hse_dcache_invalidate(&s_hse_key_info, sizeof(s_hse_key_info));
        log_line("[hse_cmac] keyInfo: type=0x%02X bitLen=%u flags=0x%04X\r\n",
                 (unsigned)s_hse_key_info.keyType,
                 (unsigned)s_hse_key_info.keyBitLen,
                 (unsigned)s_hse_key_info.keyFlags);
    }
    return rsp;
}

static hseSrvResponse_t hse_cmac_generate(hseKeyHandle_t keyHandle)
{
    memcpy(s_cmac_input, s_secoc_data_to_auth, sizeof(s_cmac_input));
    s_cmac_tag_len = (uint32_t)sizeof(s_cmac_tag);
    hse_dcache_clean(s_cmac_input, sizeof(s_cmac_input));
    hse_dcache_clean(s_cmac_tag, sizeof(s_cmac_tag));
    hse_dcache_clean(&s_cmac_tag_len, sizeof(s_cmac_tag_len));

    hseSrvDescriptor_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.srvId = HSE_SRV_ID_MAC;
    desc.hseSrv.macReq.accessMode = HSE_ACCESS_MODE_ONE_PASS;
    desc.hseSrv.macReq.authDir = HSE_AUTH_DIR_GENERATE;
    desc.hseSrv.macReq.sgtOption = HSE_SGT_OPTION_NONE;
    desc.hseSrv.macReq.macScheme.macAlgo = HSE_MAC_ALGO_CMAC;
    desc.hseSrv.macReq.macScheme.sch.cmac.cipherAlgo = HSE_CIPHER_ALGO_AES;
    desc.hseSrv.macReq.keyHandle = keyHandle;
    desc.hseSrv.macReq.inputLength = (uint32_t)sizeof(s_cmac_input);
    desc.hseSrv.macReq.pInput = (HOST_ADDR)(uintptr_t)&s_cmac_input[0];
    desc.hseSrv.macReq.pTagLength = (HOST_ADDR)(uintptr_t)&s_cmac_tag_len;
    desc.hseSrv.macReq.pTag = (HOST_ADDR)(uintptr_t)&s_cmac_tag[0];

    hseSrvResponse_t rsp = hse_send_sync("cmac_generate", &desc);
    if (HSE_SRV_RSP_OK == rsp) {
        hse_dcache_invalidate(s_cmac_tag, sizeof(s_cmac_tag));
        hse_dcache_invalidate(&s_cmac_tag_len, sizeof(s_cmac_tag_len));
    }
    return rsp;
}

static hseSrvResponse_t hse_cmac_verify(hseKeyHandle_t keyHandle)
{
    memcpy(s_cmac_input, s_secoc_data_to_auth, sizeof(s_cmac_input));
    hse_dcache_clean(s_cmac_input, sizeof(s_cmac_input));
    hse_dcache_clean(s_cmac_tag, sizeof(s_cmac_tag));
    hse_dcache_clean(&s_cmac_tag_len, sizeof(s_cmac_tag_len));

    hseSrvDescriptor_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.srvId = HSE_SRV_ID_MAC;
    desc.hseSrv.macReq.accessMode = HSE_ACCESS_MODE_ONE_PASS;
    desc.hseSrv.macReq.authDir = HSE_AUTH_DIR_VERIFY;
    desc.hseSrv.macReq.sgtOption = HSE_SGT_OPTION_NONE;
    desc.hseSrv.macReq.macScheme.macAlgo = HSE_MAC_ALGO_CMAC;
    desc.hseSrv.macReq.macScheme.sch.cmac.cipherAlgo = HSE_CIPHER_ALGO_AES;
    desc.hseSrv.macReq.keyHandle = keyHandle;
    desc.hseSrv.macReq.inputLength = (uint32_t)sizeof(s_cmac_input);
    desc.hseSrv.macReq.pInput = (HOST_ADDR)(uintptr_t)&s_cmac_input[0];
    desc.hseSrv.macReq.pTagLength = (HOST_ADDR)(uintptr_t)&s_cmac_tag_len;
    desc.hseSrv.macReq.pTag = (HOST_ADDR)(uintptr_t)&s_cmac_tag[0];

    return hse_send_sync("cmac_verify", &desc);
}

void hse_cmac_demo_run(void)
{
    const hseKeyHandle_t keyHandle = PROVISIONED_SECOC_KEY_HANDLE;

    osal_log_info("\r\n[hse_cmac] --- provisioned-key CMAC check begin ---\r\n");
    log_line("[hse_cmac] expect NVM keyHandle=0x%08lX (cat=NVM grp=0 slot=0)\r\n",
             (unsigned long)keyHandle);

    if (HSE_IP_STATUS_SUCCESS != Hse_Ip_Init(HSE_MU_INSTANCE, &s_hse_mu_state)) {
        osal_log_info("[hse_cmac] Hse_Ip_Init failed - aborting\r\n");
        return;
    }

    hseStatus_t status = Hse_Ip_GetHseStatus(HSE_MU_INSTANCE);
    log_line("[hse_cmac] HSE status=0x%04X INIT_OK=%u INSTALL_OK=%u\r\n",
             (unsigned)status,
             (unsigned)((0U != (status & HSE_STATUS_INIT_OK)) ? 1U : 0U),
             (unsigned)((0U != (status & HSE_STATUS_INSTALL_OK)) ? 1U : 0U));

    if (0U == (status & HSE_STATUS_INIT_OK)) {
        osal_log_info("[hse_cmac] HSE FW not ready (INIT_OK clear) - run s32k312_provision first\r\n");
        return;
    }
    if (0U == (status & HSE_STATUS_INSTALL_OK)) {
        osal_log_info("[hse_cmac] key catalogs not formatted (INSTALL_OK clear) - run provision\r\n");
        return;
    }

    hseSrvResponse_t rsp = hse_get_key_info(keyHandle);
    if (HSE_SRV_RSP_OK != rsp) {
        log_line("[hse_cmac] NVM SecOC key missing/unusable (0x%08lX %s)\r\n",
                 (unsigned long)rsp, hse_rsp_name(rsp));
        osal_log_info("[hse_cmac] Re-flash/run s32k312_provision so it imports SecOC key to NVM g0s0\r\n");
        return;
    }

    rsp = hse_cmac_generate(keyHandle);
    log_line("[hse_cmac] CMAC generate done, tagLen=%lu\r\n", (unsigned long)s_cmac_tag_len);
    if (HSE_SRV_RSP_OK != rsp) {
        log_line("[hse_cmac] CMAC generate FAILED (0x%08lX %s) - key may be wrong type/flags\r\n",
                 (unsigned long)rsp, hse_rsp_name(rsp));
        return;
    }

    log_line("[hse_cmac] tag = %02X %02X %02X %02X %02X %02X %02X %02X "
             "%02X %02X %02X %02X %02X %02X %02X %02X\r\n",
             s_cmac_tag[0], s_cmac_tag[1], s_cmac_tag[2], s_cmac_tag[3],
             s_cmac_tag[4], s_cmac_tag[5], s_cmac_tag[6], s_cmac_tag[7],
             s_cmac_tag[8], s_cmac_tag[9], s_cmac_tag[10], s_cmac_tag[11],
             s_cmac_tag[12], s_cmac_tag[13], s_cmac_tag[14], s_cmac_tag[15]);

    if (0 != memcmp(s_cmac_tag, s_secoc_expected_mac_trunc, SECOC_CMAC_TRUNC_BYTES)) {
        log_line("[hse_cmac] trunc MAC MISMATCH got %02X %02X %02X expect %02X %02X %02X\r\n",
                 s_cmac_tag[0], s_cmac_tag[1], s_cmac_tag[2],
                 s_secoc_expected_mac_trunc[0], s_secoc_expected_mac_trunc[1],
                 s_secoc_expected_mac_trunc[2]);
        osal_log_info("[hse_cmac] FAIL - provisioned key material does not match SecOC test vector\r\n");
        return;
    }
    osal_log_info("[hse_cmac] trunc MAC matches mbedtls/provision reference (6A 0E 6D)\r\n");

    if (0 != memcmp(s_cmac_tag, s_secoc_expected_mac_full, sizeof(s_secoc_expected_mac_full))) {
        osal_log_info("[hse_cmac] full 16 B MAC differs from last provision capture (trunc OK)\r\n");
    } else {
        osal_log_info("[hse_cmac] full 16 B MAC matches provision capture\r\n");
    }

    rsp = hse_cmac_verify(keyHandle);
    if (HSE_SRV_RSP_OK == rsp) {
        osal_log_info("[hse_cmac] CMAC verify PASSED - NVM key provisioning confirmed\r\n");
    } else {
        log_line("[hse_cmac] CMAC verify FAILED (0x%08lX %s)\r\n",
                 (unsigned long)rsp, hse_rsp_name(rsp));
    }

    osal_log_info("[hse_cmac] --- provisioned-key CMAC check end ---\r\n");
}
