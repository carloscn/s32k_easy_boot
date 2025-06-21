#ifndef HAL_CRC_H
#define HAL_CRC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAL_CRC_USE_HARDWARE
#define HAL_CRC_USE_HARDWARE 1
#endif

/**
 * @brief Initializes the CRC module (hardware/software depending on configuration)
 *        - For hardware: initializes Crc_Ip channel configuration
 *        - For software: optional (no operation)
 */
void hal_crc_init(void);

/**
 * @brief Deinitializes the CRC module and releases any used hardware resources
 *        - For hardware: calls Crc_Ip_Deinit if necessary
 *        - For software: optional (no operation)
 */
void hal_crc_deinit(void);

/**
 * @brief Compute CRC16 using CCITT-FALSE polynomial (0x1021)
 *
 * @param data        Pointer to the data buffer
 * @param length      Number of bytes to compute
 * @param init_value  Initial CRC value (typically 0xFFFF)
 * @return 16-bit CRC result
 */
uint16_t hal_crc16_compute(const uint8_t *data, size_t length, uint16_t init_value);

/**
 * @brief Compute CRC32 using Ethernet polynomial (0x04C11DB7)
 *
 * @param data        Pointer to the data buffer
 * @param length      Number of bytes to compute
 * @param init_value  Initial CRC value (typically 0xFFFFFFFF)
 * @return 32-bit CRC result
 */
uint32_t hal_crc32_compute(const uint8_t *data, size_t length, uint32_t init_value);

#ifdef __cplusplus
}
#endif

#endif /* HAL_CRC_H */
