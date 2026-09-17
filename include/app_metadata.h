#ifndef APP_METADATA_H_
#define APP_METADATA_H_

#include <stdint.h>

/*
 * ESS-STD-003 identity. Same layout as s32k312_freertos/include/app_metadata.h.
 * Slot header (essfw_hdr_t) embeds this struct — do not invent a parallel version.
 *
 * Fields through crc32 are the original Boot ABI; the rest are F187/F191/F1A1/F1A0.
 */
/* 0x005CFFF0 abolished. Identity is in the slot header; this is last-resort only. */
#define APP_METADATA_ADDR       0x005CFFF0U
#define APP_METADATA_MAGIC      0xAABBCCDDU

#define APP_NAME_MAX_LEN            16U
#define APP_VERSION_MAX_LEN         12U  /* F195 wire length */
#define APP_PRODUCT_FAMILY_MAX_LEN  16U
#define APP_PART_NUMBER_MAX_LEN     16U  /* F187 */
#define APP_HW_VERSION_MAX_LEN      16U  /* F191 */
#define APP_RXSWIN_MAX_LEN          32U  /* F1A0 */

/* F195 / F180 core: MM.mm.pp — never "v" or SemVer. */
#define APP_SW_VERSION_EXAMPLE      "00.01.00"

typedef struct {
    uint32_t magic;
    char     app_name[APP_NAME_MAX_LEN];
    char     version[APP_VERSION_MAX_LEN];
    char    *build_timestamp;
    uint32_t flash_start_addr;
    uint32_t image_size;
    uint32_t crc32;
    char     product_family[APP_PRODUCT_FAMILY_MAX_LEN];
    char     part_number[APP_PART_NUMBER_MAX_LEN];
    char     hw_version[APP_HW_VERSION_MAX_LEN];
    uint32_t anti_rollback_counter;
    char     rxswin[APP_RXSWIN_MAX_LEN];
} app_metadata_t;

#endif /* APP_METADATA_H_ */
