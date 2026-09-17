/**
 * Image-decrypt hook. S32K312 SRAM is 96 KB — too small for an 800 KB App,
 * so this target never decrypts (SB-D12).
 *
 * S32K358 (~1.12 MB SRAM): implement AES-CTR-128 here:
 *   ciphertext @ hdr->img.img_static_addr (slot flash)
 *   plaintext  @ hdr->img.img_load_addr   (SRAM)
 *   jump load_addr, not XIP flash.
 * Hash in the header is of the plaintext (Arm order).
 * Key: boot_app_enc_key in Boot .rodata; IV/keyblob in hdr->cipher.
 * Call mbedtls_aes_crypt_ctr — no pal_*, no hal_crypto.
 */

#include "essfw.h"
#include "osal_log.h"

#ifndef SECBOOT_DECRYPT_TO_RAM
#if defined(CPU_S32K358)
#define SECBOOT_DECRYPT_TO_RAM 1
#else
#define SECBOOT_DECRYPT_TO_RAM 0
#endif
#endif

#ifndef SECBOOT_DECRYPT_SRAM_MAX
#define SECBOOT_DECRYPT_SRAM_MAX  (0x0011C000UL) /* 1.12 MiB, S32K358 */
#endif

int boot_target_can_decrypt(uint32_t img_size)
{
#if SECBOOT_DECRYPT_TO_RAM
    return ((img_size > 0U) && (img_size <= SECBOOT_DECRYPT_SRAM_MAX)) ? 1 : 0;
#else
    (void)img_size;
    return 0;
#endif
}

int boot_img_decrypt(const essfw_hdr_t *hdr)
{
    if (hdr == NULL) {
        return BOOT_VF_BAD_HDR;
    }
    if (boot_target_can_decrypt(hdr->img.img_size) == 0) {
        osal_log_info("[SECBOOT] decrypt-to-RAM not available on this MCU\r\n");
        return BOOT_VF_ENC;
    }

#if SECBOOT_DECRYPT_TO_RAM
    /*
     * TODO(S32K358):
     *  - require hdr->mnfst.flags == ESSFW_MNFST_FLAGS_ENC
     *  - require hdr->cipher.blob_len == 16
     *  - memcmp(hdr->cipher.keyblob, boot_app_enc_key, 16)
     *  - mbedtls_aes_crypt_ctr(flash static → SRAM load)
     *  - I/D-cache maintenance on the SRAM window
     */
    (void)boot_app_enc_key;
    osal_log_info("[SECBOOT] decrypt-to-RAM hook not filled in yet\r\n");
    return BOOT_VF_ENC;
#else
    return BOOT_VF_ENC;
#endif
}
