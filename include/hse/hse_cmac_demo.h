/**
 * @file hse_cmac_demo.h
 * @brief Boot-time check that the SecOC AES-128 key was provisioned into HSE NVM.
 *
 * Uses the same key handle / DataToAuthenticator / expected truncated MAC as
 * s32k312_provision CMAC smoke test and s32k_demo/src/test_cmac.c.
 * Does NOT install HSE FW, format catalogs, or import keys - that is provision's job.
 */

#ifndef HSE_CMAC_DEMO_H
#define HSE_CMAC_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Verify provisioned NVM SecOC key via HSE CMAC generate + verify.
 * @details Expects HSE FW already installed and key catalogs formatted, with
 *          the SecOC test key in NVM CUST AES-128 group 0 slot 0
 *          (GET_KEY_HANDLE(NVM, 0, 0)). Safe to call every boot before app jump.
 */
void hse_cmac_demo_run(void);

#ifdef __cplusplus
}
#endif

#endif /* HSE_CMAC_DEMO_H */
