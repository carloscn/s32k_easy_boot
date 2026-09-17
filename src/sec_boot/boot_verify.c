/**
 * App slot verify. Trust root is boot_app_pubkey_uncompressed in easy_boot.
 * Log via osal_log_info (same OSAL as the rest of Boot). Crypto is mbedtls.
 * Not a second HAL, and not Arm PAL.
 */

#include <stdio.h>
#include <string.h>
#include "essfw.h"
#include "osal_log.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/ecp.h"

typedef char essfw_signed_size_ok[(ESSFW_SIGNED_SIZE == 313U) ? 1 : -1];
typedef char essfw_hdr_size_ok[(sizeof(essfw_hdr_t) == 377U) ? 1 : -1];
typedef char app_meta_size_ok[(sizeof(app_metadata_t) == 132U) ? 1 : -1];
typedef char mnfst_hdr_size_ok[(sizeof(sec_boot_manifest_header_t) == 24U) ? 1 : -1];
typedef char img_hdr_size_ok[(sizeof(sec_boot_img_header_t) == 24U) ? 1 : -1];
typedef char cipher_size_ok[(sizeof(sec_boot_cipher_info_t) == 36U) ? 1 : -1];

static int f195_mm_mm_pp(const char *v)
{
    unsigned int i;

    if (v == NULL) {
        return 0;
    }
    if ((v[2] != '.') || (v[5] != '.')) {
        return 0;
    }
    for (i = 0U; i < 8U; i++) {
        if ((i == 2U) || (i == 5U)) {
            continue;
        }
        if ((v[i] < '0') || (v[i] > '9')) {
            return 0;
        }
    }
    return 1;
}

static void log_line(const char *msg)
{
    osal_log_info(msg);
}

static int sp_looks_valid(uint32_t sp)
{
    if ((sp & 0x7U) != 0U) {
        return 0;
    }
    if ((sp >= 0x20000000UL) && (sp <= 0x20010000UL)) {
        return 1; /* DTCM */
    }
    if ((sp >= 0x20400000UL) && (sp <= 0x20418000UL)) {
        return 1; /* SRAM */
    }
    return 0;
}

static int ecdsa_p256_verify(const uint8_t *digest, const uint8_t *sig_rs)
{
    mbedtls_ecp_group grp;
    mbedtls_ecp_point Q;
    mbedtls_mpi r;
    mbedtls_mpi s;
    int rc;
    int ok = 0;

    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&Q);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    rc = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    if (rc != 0) {
        goto done;
    }
    rc = mbedtls_ecp_point_read_binary(&grp, &Q, boot_app_pubkey_uncompressed, 65);
    if (rc != 0) {
        goto done;
    }
    rc = mbedtls_mpi_read_binary(&r, sig_rs, 32);
    if (rc != 0) {
        goto done;
    }
    rc = mbedtls_mpi_read_binary(&s, sig_rs + 32, 32);
    if (rc != 0) {
        goto done;
    }
    rc = mbedtls_ecdsa_verify(&grp, digest, 32, &Q, &r, &s);
    ok = (rc == 0);

done:
    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    mbedtls_ecp_point_free(&Q);
    mbedtls_ecp_group_free(&grp);
    return ok ? 0 : -1;
}

int boot_verify_slot(uint32_t slot_id, boot_img_info_t *out)
{
    const essfw_hdr_t *hdr;
    uint32_t hdr_addr;
    uint32_t expect_load;
    uint32_t img_max;
    uint8_t digest[32];
    uint8_t hdr_hash[32];
    uint32_t hash_addr;
    uint32_t sp;
    uint32_t reset;
    char line[96];
    int rc;

    if (slot_id == 0U) {
        hdr_addr = SEC_BOOT_SLOT_A_HEADER_ADDR;
        expect_load = SEC_BOOT_SLOT_A_IMAGE_ADDR;
        img_max = SEC_BOOT_SLOT_A_IMAGE_MAX;
    } else if (slot_id == 1U) {
        hdr_addr = SEC_BOOT_SLOT_B_HEADER_ADDR;
        expect_load = SEC_BOOT_SLOT_B_IMAGE_ADDR;
        img_max = SEC_BOOT_SLOT_B_IMAGE_MAX;
    } else {
        return BOOT_VF_SLOT;
    }

    hdr = (const essfw_hdr_t *)hdr_addr;
    if ((hdr->mnfst.magic != SEC_BOOT_MNFST_MAGIC) ||
        (hdr->mnfst.mnfst_layout_ver != SEC_BOOT_MNFST_LAYOUT_VER) ||
        (hdr->mnfst.img_num != 1U) ||
        (hdr->mnfst.mnfst_size != (uint32_t)sizeof(essfw_hdr_t))) {
        return BOOT_VF_BAD_HDR;
    }
    if (hdr->mnfst.flags == ESSFW_MNFST_FLAGS) {
        if ((hdr->img.img_info & SEC_BOOT_IMG_ENC_FLAG) != 0U) {
            return BOOT_VF_BAD_HDR;
        }
    } else if (hdr->mnfst.flags == ESSFW_MNFST_FLAGS_ENC) {
        if ((hdr->img.img_info & SEC_BOOT_IMG_ENC_FLAG) == 0U) {
            return BOOT_VF_BAD_HDR;
        }
    } else {
        return BOOT_VF_BAD_HDR;
    }
    if ((hdr->img.img_size == 0U) || (hdr->img.img_size > img_max)) {
        return BOOT_VF_SIZE;
    }
    if (hdr->meta.magic != APP_METADATA_MAGIC) {
        return BOOT_VF_BAD_HDR;
    }
    if (f195_mm_mm_pp(hdr->meta.version) == 0) {
        return BOOT_VF_BAD_HDR;
    }
    if (memcmp(hdr->pubkey, boot_app_pubkey_uncompressed, 65) != 0) {
        return BOOT_VF_SIG;
    }

    hash_addr = expect_load;
    if ((hdr->img.img_info & SEC_BOOT_IMG_ENC_FLAG) != 0U) {
        /* Ciphertext stays in the slot; plaintext goes to load_addr (SRAM). */
        if (hdr->img.img_static_addr != expect_load) {
            return BOOT_VF_SLOT;
        }
        rc = boot_img_decrypt(hdr);
        if (rc != BOOT_VF_OK) {
            return rc;
        }
        hash_addr = hdr->img.img_load_addr;
        if ((hash_addr == 0U) || (hdr->img.img_entry != hash_addr)) {
            return BOOT_VF_SLOT;
        }
    } else if ((hdr->img.img_load_addr != expect_load) ||
               (hdr->img.img_static_addr != expect_load) ||
               (hdr->img.img_entry != expect_load)) {
        (void)snprintf(line, sizeof(line),
                      "[SECBOOT] slot %lu load_addr mismatch\r\n",
                      (unsigned long)slot_id);
        log_line(line);
        return BOOT_VF_SLOT;
    }

    if ((hdr->meta.image_size != hdr->img.img_size) ||
        (hdr->meta.flash_start_addr != hdr->img.img_static_addr)) {
        return BOOT_VF_SLOT;
    }

    sp = *(const volatile uint32_t *)hash_addr;
    reset = *(const volatile uint32_t *)(hash_addr + 4U);
    if ((!sp_looks_valid(sp)) || ((reset & 1U) == 0U)) {
        return BOOT_VF_SP;
    }

    if (mbedtls_sha256((const unsigned char *)hash_addr, hdr->img.img_size, digest, 0) != 0) {
        return BOOT_VF_HASH;
    }
    if (memcmp(digest, hdr->sha256, 32) != 0) {
        return BOOT_VF_HASH;
    }

    if (mbedtls_sha256((const unsigned char *)hdr, ESSFW_SIGNED_SIZE, hdr_hash, 0) != 0) {
        return BOOT_VF_SIG;
    }
    if (ecdsa_p256_verify(hdr_hash, hdr->sig_r_s) != 0) {
        return BOOT_VF_SIG;
    }

    /* Data Flash anti-rollback is SB2; until then counter 0 is accepted. */
    if (out != NULL) {
        out->slot_id = slot_id;
        out->load_addr = hash_addr;
        out->image_size = hdr->img.img_size;
    }
    return BOOT_VF_OK;
}

int boot_verify_preferred(boot_img_info_t *out)
{
    int rc;
    char line[80];

    rc = boot_verify_slot(0U, out);
    if (rc == BOOT_VF_OK) {
        log_line("[SECBOOT] slot A verified\r\n");
        return BOOT_VF_OK;
    }
    (void)snprintf(line, sizeof(line), "[SECBOOT] slot A fail %d, try B\r\n", rc);
    log_line(line);

    rc = boot_verify_slot(1U, out);
    if (rc == BOOT_VF_OK) {
        log_line("[SECBOOT] slot B verified\r\n");
        return BOOT_VF_OK;
    }
    (void)snprintf(line, sizeof(line), "[SECBOOT] slot B fail %d\r\n", rc);
    log_line(line);
    return rc;
}
