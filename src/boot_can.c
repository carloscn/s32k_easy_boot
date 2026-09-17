/**
 * @file boot_can.c
 * @brief FlexCAN0 polling driver + TJA1043 EN/STB for Boot UDS.
 *
 * Mailbox map matches the App: 0 = TX, 1 = phys RX (0x7E0), 2 = func (0x7DF).
 * ISO-TP N_PDUs (≤8 B) are sent as classic CAN. No FlexCAN NVIC — Boot IntCtrl
 * only has LPUART6 + PIT0; RX is FlexCAN_Ip_MainFunctionRead().
 */
#include "boot_can.h"
#include "FlexCAN_Ip.h"
#include "FlexCAN_Ip_Sa_PBcfg.h"
#include "Siul2_Dio_Ip.h"
#include "Siul2_Port_Ip_Cfg.h"

#include <string.h>

#define BOOT_CAN_INST       INST_FLEXCAN_0
#define BOOT_CAN_MB_TX      (0U)
#define BOOT_CAN_MB_RX_PHYS (1U)
#define BOOT_CAN_MB_RX_FUNC (2U)

static Flexcan_Ip_MsgBuffType s_rx_phys_mb;
static Flexcan_Ip_MsgBuffType s_rx_func_mb;
static boot_can_rx_cb_t s_rx_cb;
static void *s_rx_user;
static uint8_t s_inited;

static void tja1043_normal_mode(void)
{
    Siul2_Dio_Ip_WritePin(CAN0_EN_PORT, CAN0_EN_PIN, 1U);
    Siul2_Dio_Ip_WritePin(CAN0_STB_PORT, CAN0_STB_PIN, 1U);
}

static void fill_tx_info(Flexcan_Ip_DataInfoType *info, uint8_t len, int use_fd)
{
    (void)memset(info, 0, sizeof(*info));
    info->msg_id_type = FLEXCAN_MSG_ID_STD;
    info->data_length = (uint32)len;
    info->fd_enable = use_fd ? TRUE : FALSE;
    info->fd_padding = 0U;
    info->enable_brs = use_fd ? TRUE : FALSE;
    info->is_remote = FALSE;
    info->is_polling = TRUE;
}

static void fill_rx_info(Flexcan_Ip_DataInfoType *info)
{
    (void)memset(info, 0, sizeof(*info));
    info->msg_id_type = FLEXCAN_MSG_ID_STD;
    info->data_length = BOOT_CAN_MAX_DLC;
    info->fd_enable = TRUE;
    info->enable_brs = TRUE;
    info->is_remote = FALSE;
    info->is_polling = TRUE;
}

static int arm_rx_mb(uint8_t mb, uint32_t id, Flexcan_Ip_MsgBuffType *buf)
{
    Flexcan_Ip_DataInfoType info;

    fill_rx_info(&info);
    if (FLEXCAN_STATUS_SUCCESS != FlexCAN_Ip_ConfigRxMb(BOOT_CAN_INST, mb, &info, id)) {
        return -1;
    }
    if (FLEXCAN_STATUS_SUCCESS != FlexCAN_Ip_Receive(BOOT_CAN_INST, mb, buf, TRUE)) {
        return -1;
    }
    return 0;
}

static void deliver_rx(const Flexcan_Ip_MsgBuffType *mb)
{
    boot_can_frame_t frame;

    if ((NULL == mb) || (NULL == s_rx_cb)) {
        return;
    }
    frame.id = mb->msgId;
    frame.dlc = mb->dataLen;
    if (frame.dlc > BOOT_CAN_MAX_DLC) {
        frame.dlc = BOOT_CAN_MAX_DLC;
    }
    (void)memcpy(frame.data, mb->data, frame.dlc);
    s_rx_cb(&frame, s_rx_user);
}

void CAN_ISR_Callback(uint8 instance, Flexcan_Ip_EventType eventType,
                      uint32 buffIdx, const Flexcan_Ip_StateType *flexcanState)
{
    (void)flexcanState;

    if (BOOT_CAN_INST != instance) {
        return;
    }
    if (FLEXCAN_EVENT_RX_COMPLETE != eventType) {
        return;
    }

    if (BOOT_CAN_MB_RX_PHYS == buffIdx) {
        deliver_rx(&s_rx_phys_mb);
        (void)FlexCAN_Ip_Receive(BOOT_CAN_INST, BOOT_CAN_MB_RX_PHYS, &s_rx_phys_mb, TRUE);
    } else if (BOOT_CAN_MB_RX_FUNC == buffIdx) {
        deliver_rx(&s_rx_func_mb);
        (void)FlexCAN_Ip_Receive(BOOT_CAN_INST, BOOT_CAN_MB_RX_FUNC, &s_rx_func_mb, TRUE);
    }
}

int boot_can_init(void)
{
    if (0U != s_inited) {
        return 0;
    }

    tja1043_normal_mode();

    if (FLEXCAN_STATUS_SUCCESS != FlexCAN_Ip_Init(BOOT_CAN_INST, &FlexCAN_State0, &FlexCAN_Config0)) {
        return -1;
    }
    if (FLEXCAN_STATUS_SUCCESS != FlexCAN_Ip_SetStartMode(BOOT_CAN_INST)) {
        return -1;
    }
    if (0 != arm_rx_mb(BOOT_CAN_MB_RX_PHYS, BOOT_CAN_UDS_PHYS_REQ_ID, &s_rx_phys_mb)) {
        return -1;
    }
    if (0 != arm_rx_mb(BOOT_CAN_MB_RX_FUNC, BOOT_CAN_UDS_FUNC_REQ_ID, &s_rx_func_mb)) {
        return -1;
    }

    s_inited = 1U;
    return 0;
}

void boot_can_set_rx_cb(boot_can_rx_cb_t cb, void *user)
{
    s_rx_cb = cb;
    s_rx_user = user;
}

int boot_can_send(uint32_t id, const uint8_t *data, uint8_t len)
{
    Flexcan_Ip_DataInfoType info;
    Flexcan_Ip_StatusType st;

    if ((0U == s_inited) || (NULL == data) || (len > BOOT_CAN_MAX_DLC)) {
        return -1;
    }

    fill_tx_info(&info, len, (len > 8U) ? 1 : 0);
    st = FlexCAN_Ip_Send(BOOT_CAN_INST, BOOT_CAN_MB_TX, &info, id, data);
    if (FLEXCAN_STATUS_BUSY == st) {
        return 1; /* no space */
    }
    if (FLEXCAN_STATUS_SUCCESS != st) {
        return -1;
    }
    return 0;
}

void boot_can_poll(void)
{
    if (0U == s_inited) {
        return;
    }
    FlexCAN_Ip_MainFunctionRead(BOOT_CAN_INST, BOOT_CAN_MB_RX_PHYS);
    FlexCAN_Ip_MainFunctionRead(BOOT_CAN_INST, BOOT_CAN_MB_RX_FUNC);
}
