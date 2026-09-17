#ifndef ESSFW_H
#define ESSFW_H

#include <stddef.h>
#include <stdint.h>
#include "app_metadata.h"
#include "sec_boot_layout.h"

/*
 * Slot header = Arm China 1-image manifest + app_metadata_t.
 * img_num=1, no ext-prog. Cipher info is always present (Arm position:
 * after img digest, before pubkey). On S32K312 it is reserved zeros and
 * img_info[0] must be 0. S32K358 fills AES-CTR-128 and decrypts flash→SRAM.
 *
 *   manifest_header | img_header | sha256 | cipher | app_metadata | pubkey | sig
 * Pubkey in the file must match Boot .rodata (App trust root). No OTP hash.
 */

#define SEC_BOOT_MNFST_MAGIC          (0xA1BC2FD8UL)
#define SEC_BOOT_MNFST_LAYOUT_VER    (1U)
#define SEC_BOOT_DGST_SCH_SHA256      (3U)
#define SEC_BOOT_SIG_ECDSA_256_SHA256 (6U)
#define SEC_BOOT_CIPHER_AES_CTR128    (2U)
#define SEC_BOOT_ECDSA256_PUBKEY_LEN  (65U)
#define SEC_BOOT_ECDSA256_SIG_LEN     (64U)
#define SEC_BOOT_AES_IV_LEN           (16U)
#define SEC_BOOT_AES128_KEY_LEN       (16U)

#define SEC_BOOT_IMG_ENC_FLAG         (1UL)
#define SEC_BOOT_CIPHER_BLOB_BIT      (1UL << 2)

/* flags: sec_boot | pubkey SHA-256 | ECDSA_256_SHA256 | img SHA-256 */
#define ESSFW_MNFST_FLAGS \
    (1UL | (SEC_BOOT_DGST_SCH_SHA256 << 3) | \
     ((uint32_t)SEC_BOOT_SIG_ECDSA_256_SHA256 << 8) | \
     ((uint32_t)SEC_BOOT_DGST_SCH_SHA256 << 16))

/* S32K358: same as above plus cipher-blob + AES-CTR-128 in bits 24–31. */
#define ESSFW_MNFST_FLAGS_ENC \
    (ESSFW_MNFST_FLAGS | SEC_BOOT_CIPHER_BLOB_BIT | \
     ((uint32_t)SEC_BOOT_CIPHER_AES_CTR128 << 24))

typedef struct {
    uint32_t magic;
    uint32_t mnfst_layout_ver;
    uint32_t mnfst_content_ver;
    uint32_t mnfst_size;
    uint32_t flags;
    uint32_t img_num;
} sec_boot_manifest_header_t;

typedef struct {
    uint32_t img_info;
    uint32_t img_ver;
    uint32_t img_static_addr;
    uint32_t img_load_addr;
    uint32_t img_size;
    uint32_t img_entry;
} sec_boot_img_header_t;

/* Arm cipher_key_info for AES-CTR-128, always 36 B. Unused on S32K312. */
typedef struct {
    uint32_t blob_len;
    uint8_t  iv[SEC_BOOT_AES_IV_LEN];
    uint8_t  keyblob[SEC_BOOT_AES128_KEY_LEN];
} sec_boot_cipher_info_t;

typedef struct __attribute__((packed)) {
    sec_boot_manifest_header_t mnfst;
    sec_boot_img_header_t      img;
    uint8_t                   sha256[32];
    sec_boot_cipher_info_t    cipher;
    app_metadata_t             meta;
    uint8_t                   pubkey[SEC_BOOT_ECDSA256_PUBKEY_LEN];
    uint8_t                   sig_r_s[SEC_BOOT_ECDSA256_SIG_LEN];
} essfw_hdr_t;

#define ESSFW_SIGNED_SIZE  ((uint32_t)offsetof(essfw_hdr_t, sig_r_s))

typedef struct {
    uint32_t slot_id;
    uint32_t load_addr;
    uint32_t image_size;
} boot_img_info_t;

enum {
    BOOT_VF_OK = 0,
    BOOT_VF_BAD_HDR = 1,
    BOOT_VF_SLOT = 2,
    BOOT_VF_SIZE = 3,
    BOOT_VF_SP = 4,
    BOOT_VF_HASH = 5,
    BOOT_VF_SIG = 6,
    BOOT_VF_ROLLBACK = 7,
    BOOT_VF_ENC = 8
};

extern const uint8_t boot_app_pubkey_uncompressed[65];
extern const uint8_t boot_app_enc_key[16];

int boot_target_can_decrypt(uint32_t img_size);
int boot_img_decrypt(const essfw_hdr_t *hdr);

int boot_verify_slot(uint32_t slot_id, boot_img_info_t *out);
int boot_verify_preferred(boot_img_info_t *out);

#endif /* ESSFW_H */
