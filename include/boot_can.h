/**
 * @file boot_can.h
 * @brief Bare-metal FlexCAN0 + TJA1043 for Boot UDS (polling, no FreeRTOS).
 */
#ifndef BOOT_CAN_H
#define BOOT_CAN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOOT_CAN_UDS_PHYS_REQ_ID  (0x7E0U)
#define BOOT_CAN_UDS_PHYS_RESP_ID (0x7E8U)
#define BOOT_CAN_UDS_FUNC_REQ_ID  (0x7DFU)
#define BOOT_CAN_MAX_DLC          (64U)

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[BOOT_CAN_MAX_DLC];
} boot_can_frame_t;

typedef void (*boot_can_rx_cb_t)(const boot_can_frame_t *frame, void *user);

int boot_can_init(void);
void boot_can_set_rx_cb(boot_can_rx_cb_t cb, void *user);
int boot_can_send(uint32_t id, const uint8_t *data, uint8_t len);
void boot_can_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* BOOT_CAN_H */
