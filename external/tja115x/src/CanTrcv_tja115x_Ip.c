/**
* @file CanTrcv_tja115x_Ip.c
*/

/*==================================================================================================
*   Project              : RTD AUTOSAR 4.4
*   Platform             : CORTEXM
*   Peripheral           : 
*   Dependencies         : 
*
*   Autosar Version      : 4.4.0
*   Autosar Revision     : ASR_REL_4_4_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 0.8.0
*   Build Version        : S32_RTD_0_8_0_D2105_ASR_REL_4_4_REV_0000_20210513
*
*   (c) Copyright 2022 NXP Semiconductors
*   All Rights Reserved.
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

/**
* @page misra_violations MISRA-C:2012 violations
*
* @section [global]
* Violates MISRA 2012 Required Rule 1.3, Taking address of near auto variable
* The code is not dynamically linked. An absolute stack address is obtained when
* taking the address of the near auto variable. A source of error in writing
* dynamic code is that the stack segment may be different from the data segment.
*
* @section [global]
* Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and integer type.
* The cast is required to initialize a pointer with an unsigned long define, representing an address.
*
* @section [global]
* Violates MISRA 2012 Required Rule 11.6, Cast from unsigned int to pointer.
* The cast is required to initialize a pointer with an unsigned long define, representing an address.
*
* @section [global]
* Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
* Function is defined for usage by application code.
*
* @section [global]
* Violates MISRA 2012 Mandatory Rule 9.1, Variable may not have been initialized
* Array variabile (baseArr, irqsArr) is initialized at the beginning of the method.
*
* @section [global]
* Violates MISRA 2012 Mandatory Rule 9.1, Symbol not initialized
* Array variabile (payload) is initialized at the beginning of the method.
*
* @section [global]
* Violates MISRA 2012 Required Rule 13.5, side effects on right hand of logical operator, ''&&''
* Evaluating a member of the g_drivers array has no side effects.
* 
*/

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "CanTrcv_tja115x_Ip.h"
#include "tja1153.h"
//#include "TM.h"

#ifdef USE_IPV_CANTRCV_TJA115X
    #include "FlexCAN_Ip.h"
#endif

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
/**
* @brief Internal TJA115x driver structure
*/
typedef struct
{
    CanTrcv_tja115x_DeviceConfigType cfg;           /**< Driver configuration */
    uint8                            mode;          /**< Current transceiver mode */
    boolean                          initialized;   /**< Transceiver is initialized */
} CanTrcv_tja115x_DriverType;

/**
* @brief TJA115x Device life-cycle state
*/
typedef enum
{
    TJA115x_STATE_VANILLA = 0U,
    TJA115x_STATE_OPEN    = 1U,
    TJA115x_STATE_FULL    = 3U
} CanTrcv_tja115x_StateType;

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/
/**
* @brief Internal array of Transceiver units connected to MCU
*/
static volatile CanTrcv_tja115x_DriverType g_drivers[CANTRCV_TJA115X_MAX_DEVICES_NUM];

#ifdef USE_IPV_CANTRCV_TJA115X
/**
* @brief TJA115x CAN transmission default function definition.
*/
/*
static CanTrcv_tja115x_ErrorCodeType tja115x_drv_send(const uint32 CanId,
                                                      const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                      const CanTrcv_tja115x_CanMsgType CanMsgType,
                                                      const uint32 DataLen,
                                                      const uint8* const Data);
*/

/**
* @brief Internal pointer to CAN transmission function.
*/
static volatile tja115x_can_send g_DRV_CAN_Send = tja115x_drv_send;
#else
static volatile tja115x_can_send g_DRV_CAN_Send = NULL_PTR;
#endif

/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
/**
* @brief Hardware specific SetMode function.
*/
static Std_ReturnType TJA1153_SetMode(const uint8 Instance, const CanTrcv_tja115x_TrcvModeType Mode);

/**
* @brief Hardware specific SetMode function.
*/
static Std_ReturnType TJA1152_SetMode(const uint8 Instance, const CanTrcv_tja115x_TrcvModeType Mode);

/**
* @brief Checks the program counter and the device life-cycle.
*/
static CanTrcv_tja115x_ErrorCodeType TJA115x_CheckVersion(const uint32 CanId,
                                                          const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                          const uint8 PrgCnt,
                                                          const CanTrcv_tja115x_StateType State);

/**
* @brief Helper function which sends a command for device configuration.
*/
static CanTrcv_tja115x_ErrorCodeType TJA115x_UpdateCommand(const uint32 CanId,
                                                           const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                           const uint8 Byte,
                                                           const uint32 Reg,
                                                           const uint8 Len);

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/
#ifdef USE_IPV_CANTRCV_TJA115X
/**
* @brief Helper function to transmit HS CAN with standard or extended CAN identifier.
*
* @param CanId        CAN identifier
* @param CanComParams CAN interface parameters
* @param CanMsgType   Type of message ID (standard or extended)
* @param data_len     Data Length Code (max 8)
* @param data         Pointer to allocated payload data
*
* @return TJA115x_NOERROR on successful transmission.
* @return TJA115x_ERR_NOACK on missing acknowledgment.
* @return TJA115x_ERR_INVALID on invalid parameter
*/
CanTrcv_tja115x_ErrorCodeType tja115x_drv_send(const uint32 CanId,
                                                      const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                      const CanTrcv_tja115x_CanMsgType CanMsgType,
                                                      const uint32 DataLen,
                                                      const uint8* const Data)
{
    CanTrcv_tja115x_ErrorCodeType retVal = TJA115x_NOERROR;
    Flexcan_Ip_MsgBuffIdType msg_id_type;

    if (CanMsgType == TJA115x_CANMSG_EXT)
    {
        msg_id_type = FLEXCAN_MSG_ID_EXT;
    }
    else
    {
        msg_id_type = FLEXCAN_MSG_ID_STD;
    }

    /* Set information about the data to be sent
     *  - Message ID
     *  - Bit rate switch disabled
     *  - Flexible data rate disabled
     *  - Use zeros for FD padding
     *  - Standard frame
     *  - Not in polling mode
     */
    Flexcan_Ip_DataInfoType dataInfo =
    {
        .data_length = DataLen,
        .msg_id_type = msg_id_type,
        .enable_brs  = FALSE,
        .fd_enable   = FALSE,
        .fd_padding  = 0U,
        .is_remote = FALSE,
        .is_polling = FALSE
    };

    /* Execute send non-blocking */
    if ((FLEXCAN_STATUS_SUCCESS != FlexCAN_Ip_SendBlocking(CanComParams->instance,
                                                        CanComParams->mb_idx,
                                                        &dataInfo,
                                                        CanId,
                                                        Data,
                                                        CanComParams->timeout_ms)))
    {
        retVal = TJA115x_ERR_NOACK;
    }

    return retVal;
}
#endif

/* See CanTrcv_tja115x_Ip.h for details */
static Std_ReturnType TJA1153_SetMode(const uint8 Instance, const CanTrcv_tja115x_TrcvModeType Mode)
{
#ifdef USE_IPV_CANTRCV_TJA115X
    uint32_t portLevel;
    CanTrcv_tja115x_DevicePinType pinWAKE = g_drivers[Instance].cfg.WAKE;
#endif
    CanTrcv_tja115x_DevicePinType pinEN = g_drivers[Instance].cfg.EN;
    CanTrcv_tja115x_DevicePinType pinSTB_N = g_drivers[Instance].cfg.STB;
    Std_ReturnType status = E_OK;

    switch (Mode)
    {
        case TJA115x_TRCVMODE_NORMAL:
            /* EN=H, STBN=H */
#ifdef USE_IPV_CANTRCV_TJA115X
            Siul2_Dio_Ip_SetPins(pinEN.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinEN.pinPortIdx));
            Siul2_Dio_Ip_SetPins(pinSTB_N.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinSTB_N.pinPortIdx));
#else
            Dio_WriteChannel((pinEN.port) * 16U + pinEN.pinPortIdx, STD_HIGH);
            Dio_WriteChannel((pinSTB_N.port) * 16U + pinSTB_N.pinPortIdx, STD_HIGH);
#endif
            g_drivers[Instance].mode = TJA115x_TRCVMODE_NORMAL;
            break;
#ifdef USE_IPV_CANTRCV_TJA115X
        /* No need for DIO implementation, "Listen only" not available in Autosar */
        case TJA115x_TRCVMODE_LISTENONLY:
            /* EN=L, STBN=H */
            Siul2_Dio_Ip_ClearPins(pinEN.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinEN.pinPortIdx));
            Siul2_Dio_Ip_SetPins(pinSTB_N.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinSTB_N.pinPortIdx));
            g_drivers[Instance].mode = TJA115x_TRCVMODE_LISTENONLY;
            break;
#endif
        case TJA115x_TRCVMODE_STANDBY:
#ifdef USE_IPV_CANTRCV_TJA115X
            if(TJA115x_TRCVMODE_SLEEP != g_drivers[Instance].mode)
            {
#endif
            /* EN=L, STBN=L */
#ifdef USE_IPV_CANTRCV_TJA115X
            Siul2_Dio_Ip_ClearPins(pinEN.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinEN.pinPortIdx));
            Siul2_Dio_Ip_ClearPins(pinSTB_N.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinSTB_N.pinPortIdx));
#else
            Dio_WriteChannel((pinEN.port) * 16U + pinEN.pinPortIdx, STD_LOW);
            Dio_WriteChannel((pinSTB_N.port) * 16U + pinSTB_N.pinPortIdx, STD_LOW);
#endif
#ifdef USE_IPV_CANTRCV_TJA115X
            }
            else
            {
                /* No need for DIO implementation, going to Stanby from Sleep mode not available in Autosar */
                /* Toggle wake pin, STBN=L */
                portLevel = Siul2_Dio_Ip_GetPinsOutput(pinWAKE.gpioBase);
                if((portLevel & (uint32)(1UL << pinWAKE.pinPortIdx)) != 0U)
                {
                    Siul2_Dio_Ip_ClearPins(pinWAKE.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinWAKE.pinPortIdx));
                }
                else
                {
                    Siul2_Dio_Ip_SetPins(pinWAKE.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinWAKE.pinPortIdx));
                }
                Siul2_Dio_Ip_ClearPins(pinSTB_N.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinSTB_N.pinPortIdx));
            }
#endif
            g_drivers[Instance].mode = TJA115x_TRCVMODE_STANDBY;
            break;
        case TJA115x_TRCVMODE_SLEEP:
            /* EN=H, STBN=L */
#ifdef USE_IPV_CANTRCV_TJA115X
            Siul2_Dio_Ip_SetPins(pinEN.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinEN.pinPortIdx));
            Siul2_Dio_Ip_ClearPins(pinSTB_N.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinSTB_N.pinPortIdx));
#else
            Dio_WriteChannel((pinEN.port) * 16U + pinEN.pinPortIdx, STD_HIGH);
            Dio_WriteChannel((pinSTB_N.port) * 16U + pinSTB_N.pinPortIdx, STD_LOW);
#endif
            Tm_BusyWait1us16bit(50); /* Wait 50us = maximum time from issuing go-to-sleep command to entering Sleep mode */
            g_drivers[Instance].mode = TJA115x_TRCVMODE_SLEEP;
            break;
        default:
            status = E_NOT_OK;
            break;
    }

    return status;
}

/* See CanTrcv_tja115x_Ip.h for details */
static Std_ReturnType TJA1152_SetMode(const uint8 Instance, const CanTrcv_tja115x_TrcvModeType Mode)
{
    Std_ReturnType status = E_OK;
    CanTrcv_tja115x_DevicePinType pinSTB = g_drivers[Instance].cfg.STB;

    switch (Mode)
    {
        case TJA115x_TRCVMODE_NORMAL:
            /* STB=L */
#ifdef USE_IPV_CANTRCV_TJA115X
            Siul2_Dio_Ip_ClearPins(pinSTB.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinSTB.pinPortIdx));
#else
            Dio_WriteChannel((pinSTB.port) * 16U + pinSTB.pinPortIdx, STD_LOW);
#endif
            g_drivers[Instance].mode = TJA115x_TRCVMODE_NORMAL;
            break;
        case TJA115x_TRCVMODE_STANDBY:
            /* STB=H */
#ifdef USE_IPV_CANTRCV_TJA115X
            Siul2_Dio_Ip_SetPins(pinSTB.gpioBase, (Siul2_Dio_Ip_PinsChannelType)((uint32)1U << pinSTB.pinPortIdx));
#else
            Dio_WriteChannel((pinSTB.port) * 16U + pinSTB.pinPortIdx, STD_HIGH);
#endif
            g_drivers[Instance].mode = TJA115x_TRCVMODE_STANDBY;
            break;
        default:
            status = E_NOT_OK;
            break;
    }

    return status;
}

/* See TJA115x.h for details */
static CanTrcv_tja115x_ErrorCodeType TJA115x_CheckVersion(const uint32 CanId,
                                                          const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                          const uint8 PrgCnt,
                                                          const CanTrcv_tja115x_StateType State)
{
    CanTrcv_tja115x_ErrorCodeType retVal = TJA115x_ERR_INVALID;

    if (((uint8)State) <= CANTRCV_TJA115X_STATE_MASK)
    {
        uint8 payload[3];

        payload[0] = 0xF0;
        payload[1] = PrgCnt & CANTRCV_TJA115X_PRGCNT_MAX;
        payload[2] = (uint8)State & CANTRCV_TJA115X_STATE_MASK;

        retVal = g_DRV_CAN_Send(CanId, CanComParams, TJA115x_CANMSG_EXT, 3U, payload);
    }

    return retVal;
}

/*!
* @brief Transmit message to update command register
* @param CanId Configuration message CAN ID
* @param Byte Command byte value
* @param Reg Command register value
* @param Len Command register len in bytes
*
* @return TJA115x_NOERROR on successful execution.
* @return TJA115x_ERR_COMMAND on command transmisssion error.
* @return TJA115x_ERR_WRONG_CMD on command verification failure.
*/
static CanTrcv_tja115x_ErrorCodeType TJA115x_UpdateCommand(const uint32 CanId,
                                                           const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                           const uint8 Byte,
                                                           const uint32 Reg,
                                                           const uint8 Len)
{
    CanTrcv_tja115x_ErrorCodeType retval = TJA115x_NOERROR;
    uint8 i;
    uint8 payload[8];
    uint32 NewCanId = CanId;
  
    /* Prepare command payload: command byte and register parameters */
    payload[0] = Byte;
    for (i = 1U; i <= Len; i++)
    {
        payload[i] = (uint8)(0xFFU & (Reg >> (8U * (Len - i))));
    }
  
    if (g_DRV_CAN_Send(CanId, CanComParams, TJA115x_CANMSG_EXT, 1U + Len, payload) != TJA115x_NOERROR)
    {
        retval = TJA115x_ERR_COMMAND;
    }
    else
    {
        /* The new CAN ID is immediately in place */
        if (Byte == 0x60U)
        {
            NewCanId = TJA115x_CMD60_CONFIG_ID(Reg);
        }

        payload[0] |= 0x80U;   /* Register verification request */
        if (g_DRV_CAN_Send(NewCanId, CanComParams, TJA115x_CANMSG_EXT, 1U + Len, payload) != TJA115x_NOERROR)
        {
            retval = TJA115x_ERR_WRONG_CMD;
        }
    }
    return retval;
}

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/
/* See CanTrcv_tja115x_Ip.h for details */
Std_ReturnType TJA115x_DRV_Init(const uint8 Instance,
                          const CanTrcv_tja115x_DeviceConfigType* const Cfg)
{
    Std_ReturnType status = E_NOT_OK;

    if ((Instance < CANTRCV_TJA115X_MAX_DEVICES_NUM) && (Cfg != NULL_PTR) && (g_drivers[Instance].initialized == FALSE))
    {
        g_drivers[Instance].cfg = *Cfg;

        /* initialized needs to be TRUE for the SetMode method */
        g_drivers[Instance].initialized = TRUE;

        /* The transceiver is put into NORMAL mode to clear the internal wake-up flag
         * if set. This allows the driver to accurately read future wake-up events */
        //status = TJA115x_DRV_SetMode(Instance, TJA115x_TRCVMODE_NORMAL);
        status = TJA115x_DRV_SetMode(Instance, TJA115x_TRCVMODE_STANDBY);
        

        if (status != E_OK)
        {
            g_drivers[Instance].initialized = FALSE;
        }
    }

    return status;
}

/* See CanTrcv_tja115x_Ip.h for details */
Std_ReturnType TJA115x_DRV_UpdateCanCommand(const tja115x_can_send Fct)
{
    Std_ReturnType status = E_NOT_OK;

    if (Fct != NULL_PTR)
    {
        status = E_OK;
        g_DRV_CAN_Send = Fct;
    }
    return status;
}

/* See CanTrcv_tja115x_Ip.h for details */
Std_ReturnType TJA115x_DRV_SetMode(const uint8 Instance, const CanTrcv_tja115x_TrcvModeType Mode)
{
    Std_ReturnType retVal = E_NOT_OK;

    if ((Instance < CANTRCV_TJA115X_MAX_DEVICES_NUM) && (g_drivers[Instance].initialized == TRUE))
    {
        switch (g_drivers[Instance].cfg.hwType)
        {
            case TJA115x_HWTYPE_TJA1153:
                retVal = TJA1153_SetMode(Instance, Mode);
                break;
            case TJA115x_HWTYPE_TJA1152:
                retVal = TJA1152_SetMode(Instance, Mode);
                break;
            default:
                retVal = E_NOT_OK;
                break;
        }
    }

    return retVal;
}

/* See CanTrcv_tja115x_Ip.h for details */
Std_ReturnType TJA115x_DRV_GetMode(const uint8 Instance, CanTrcv_tja115x_TrcvModeType* Mode)
{
    Std_ReturnType status = E_NOT_OK;

    if ((Instance < CANTRCV_TJA115X_MAX_DEVICES_NUM) && (Mode != NULL_PTR) && (g_drivers[Instance].initialized == TRUE))
    {
        *Mode = g_drivers[Instance].mode;
        status = E_OK;
    }

    return status;
}

/* See CanTrcv_tja115x_Ip.h for details */
Std_ReturnType TJA115x_DRV_DeInit(const uint8 Instance)
{
    Std_ReturnType status = E_NOT_OK;

    if ((Instance < CANTRCV_TJA115X_MAX_DEVICES_NUM) && (g_drivers[Instance].initialized == TRUE))
    {
        status = TJA115x_DRV_SetMode(Instance, TJA115x_TRCVMODE_STANDBY);
        if (status == E_OK)
        {
            g_drivers[Instance].initialized = FALSE;
        }
    }

    return status;
}

/* See CanTrcv_tja115x_Ip.h for details */
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_EnterConfigVanilla(const CanTrcv_tja115x_CanComParamsType* CanComParams)
{
    CanTrcv_tja115x_ErrorCodeType retVal = TJA115x_ERR_INVALID;
    uint8 i;

    /* Bitrate auto-detection */
    for (i = 0; (i < CANTRCV_TJA115X_ENTERVANILLA_TRIES) && (retVal != TJA115x_NOERROR); i++)
    {
        retVal = g_DRV_CAN_Send(CANTRCV_TJA115X_AUTOBRDET_CANID, CanComParams, TJA115x_CANMSG_STD, 0U, NULL_PTR);
        if (TJA115x_ERR_NOACK == retVal)
        {
            retVal = TJA115x_ERR_BAUDRATE;
        }
        
    }

    return retVal;
}

/* See CanTrcv_tja115x_Ip.h for details */
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_EnterConfigRemote(const uint32 CanId,
                                                            const CanTrcv_tja115x_CanComParamsType* CanComParams)
{
    CanTrcv_tja115x_ErrorCodeType retVal = TJA115x_ERR_REM_REQ;

    /* Request remote configuration: enter configuration mode */
    /* Transmission must be permitted, hence canid is part of local TPL
     * and not past of remote BBL */
    if (g_DRV_CAN_Send(CanId, CanComParams, TJA115x_CANMSG_EXT, 0U, NULL_PTR) == TJA115x_NOERROR)
    {
        retVal = TJA115x_NOERROR;
    }

    return retVal;
}

/* See CanTrcv_tja115x_Ip.h for details */
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_ConfirmConfigRemote(const uint32 CanId,
                                                              const CanTrcv_tja115x_CanComParamsType* CanComParams)
{
    CanTrcv_tja115x_ErrorCodeType retVal = TJA115x_ERR_REM_CONF;

    /* Confirm remote configuration: */
    /* Transmission must be permitted, hence canid is not part of any remote BBL */
    if (g_DRV_CAN_Send(CanId, CanComParams, TJA115x_CANMSG_EXT, 0U, NULL_PTR) == TJA115x_NOERROR)
    {
        /* Check if the device is in configuration mode:
         * Vanilla 127 is not possible, hence cannot be acknowledged in configuration mode */
        if (TJA115x_CheckVersion(CanId, CanComParams, 127U, TJA115x_STATE_VANILLA) == TJA115x_NOERROR)
        {
            retVal = TJA115x_ERR_MODE;
        }
        else
        {
            retVal = TJA115x_NOERROR;
        }
    }

    return retVal;
}

/* See CanTrcv_tja115x_Ip.h for details */
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_EnterConfigLocal(const uint32 CanId,
                                                           const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                           const uint8 Instance)
{
    CanTrcv_tja115x_ErrorCodeType retVal = TJA115x_ERR_LOC_REQ;

    if (g_DRV_CAN_Send(CanId, CanComParams, TJA115x_CANMSG_EXT, 0U, NULL_PTR) == TJA115x_NOERROR)
    {
        /* Set the device in STANDBY mode if the user provided a valid instance ID.
         * This is only needed for vanilla devices. They need to be put 
         * into STANDBY mode for local reconfiguration */
        if (Instance != CANTRCV_TJA115X_INVALID_DEVICE_IDX)
        {
            if (TJA115x_DRV_SetMode(Instance, TJA115x_TRCVMODE_STANDBY) == E_OK)
            {
                retVal = TJA115x_NOERROR;
            }
            else
            {
                retVal = TJA115x_ERR_LOC_INIT;
            }
        }

        /* Check if the device is in configuration mode:
         * Vanilla 127 is not possible, hence cannot be
         * acknowledged in configuration mode */
        if (TJA115x_CheckVersion(CanId, CanComParams, 127, TJA115x_STATE_VANILLA) == TJA115x_NOERROR)
        {
            retVal = TJA115x_ERR_MODE;
        }
        else
        {
            retVal = TJA115x_NOERROR;
        }
    }

    return retVal;
}

/* See CanTrcv_tja115x_Ip.h for details */
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_ConfigureDevice(const uint32 CanId,
                                                          const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                          const CanTrcv_tja115x_CommandsType* Cfg)
{
    CanTrcv_tja115x_ErrorCodeType retVal = TJA115x_NOERROR;
    uint8 i;
    uint8 payload[8];

    if (CANTRCV_TJA115X_ELEMENTS_COUNT < Cfg->numElements)
    {
        retVal = TJA115x_ERR_INVALID;
    }

    for (i = 0U; (i < Cfg->numElements) && (retVal == TJA115x_NOERROR); i++)
    {
        /* Prepare command payload: command byte and register parameters */
        payload[0] = (uint8)0x10U;
        payload[1] = (uint8)i;
        payload[2] = (uint8)(0xFFU & (Cfg->elements[i] >> 24U));
        payload[3] = (uint8)(0xFFU & (Cfg->elements[i] >> 16U));
        payload[4] = (uint8)(0xFFU & (Cfg->elements[i] >> 8U));
        payload[5] = (uint8)(0xFFU & (Cfg->elements[i] >> 0U));

        if (g_DRV_CAN_Send(CanId, CanComParams, TJA115x_CANMSG_EXT, 6U, payload) != TJA115x_NOERROR)
        {
            retVal = TJA115x_ERR_ELEMENT;
        }

        /* Use first element to check if the TJA115x device is in configuation mode - not supported in EAR */
        /* if ((retVal == TJA115x_NOERROR) && (i == 0U)) */
        /* { */
        /*     payload[0] |= 0x80U; */        /* Verification with
                                                 forced/expected failure */
        /*     payload[2] ^= (uint8)0xFFU; */ /* Force failure with unexpected value */
        /*     payload[3] ^= (uint8)0xFFU; */ /* Force failure with unexpected value */
        
        /*     if (g_DRV_CAN_Send(CanId, CanComParams, TJA115x_CANMSG_EXT, 6U, payload) == TJA115x_NOERROR) */
        /*     { */
        /*         retVal = TJA115x_ERR_MODE; */
        /*     } */
               /* Restore expected value for following sucessful verification */
        /*     payload[2] ^= (uint8)0xFFU; */
        /*     payload[3] ^= (uint8)0xFFU; */
        /* } */
    
        payload[0] |= 0x80U; /* Register verification */
        if (retVal == TJA115x_NOERROR)
        {
            if(g_DRV_CAN_Send(CanId, CanComParams, TJA115x_CANMSG_EXT, 6U, payload) != TJA115x_NOERROR)
            {
                retVal = TJA115x_ERR_WRONG_ELM;
            }
        }
    }
  
    /* Update command 0x40 register */
    if (retVal == TJA115x_NOERROR)
    {
        if (Cfg->hwVersion == TJA115x_HW_VER_1)
        {
            retVal = TJA115x_UpdateCommand(CanId, CanComParams, 0x40U, Cfg->cmd40, 2U);
        }
        else if (Cfg->hwVersion == TJA115x_HW_VER_0)
        {
            retVal = TJA115x_UpdateCommand(CanId, CanComParams, 0x40U, Cfg->cmd40, 1U);
        }
        else
        {
            retVal = TJA115x_ERR_INVALID;
        }
    }

    /* Update command 0x50 register */
    if (retVal == TJA115x_NOERROR)
    {
        retVal = TJA115x_UpdateCommand(CanId, CanComParams, 0x50U, Cfg->cmd50, 2U);
    }
  
    /* Update command 0x60 register */
    if (retVal == TJA115x_NOERROR)
    {
        retVal = TJA115x_UpdateCommand(CanId, CanComParams, 0x60U, Cfg->cmd60, 4U);
    }

    return retVal;
}

/* See CanTrcv_tja115x_Ip.h for details */
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_LeaveConfig(const uint32 CanId,
                                                      const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                      const CanTrcv_tja115x_LeaveModeType LeaveMode)
{
    CanTrcv_tja115x_ErrorCodeType retVal = TJA115x_NOERROR;

    uint8 payload[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    /* Apply command to close configuration window (if present) */
    payload[1] |= (uint8)LeaveMode & 0x80U;
    payload[0] = (uint8)LeaveMode & 0x7FU;

    if (g_DRV_CAN_Send(CanId, CanComParams, TJA115x_CANMSG_EXT, 8U, payload) != TJA115x_NOERROR)
    {
        retVal = TJA115x_ERR_LEAVE;
    }

    return retVal;
}

/* See TJA115x.h for details */
Std_ReturnType TJA115x_DRV_GetDefaultConfiguration(CanTrcv_tja115x_CommandsType* const Cfg,
                                             const CanTrcv_tja115x_HwVersionType HwVersion)
{
    Std_ReturnType retVal = E_NOT_OK;

    if(Cfg != NULL_PTR)
    {
        uint8 i;

        retVal = E_OK;

        /* Disable all filters */
        for (i=0; i < CANTRCV_TJA115X_ELEMENTS_COUNT; i++)
        {
            Cfg->elements[i] = 0u;
        }

        /* Align to defaults */
        Cfg->cmd40 =                                                        \
            TJA115x_CMD40_TMPE(TJA115x_CMD40_TMPE_DEFAULT) |                  \
            TJA115x_CMD40_FLDTH(TJA115x_CMD40_FLDTH_DEFAULT) |                \
            TJA115x_CMD40_FLDMAX(TJA115x_CMD40_FLDMAX_DEFAULT) |              \
            TJA115x_CMD40_FLDL(TJA115x_CMD40_FLDL_DEFAULT);

        if (HwVersion == TJA115x_HW_VER_1)
        {
            Cfg->cmd40 |=                                                        \
                TJA115x_CMD40_TXD_GL_SME(TJA115x_CMD40_TXD_GL_SME_DEFAULT) |      \
                TJA115x_CMD40_TXD_GL_EN(TJA115x_CMD40_TXD_GL_EN_DEFAULT) |        \
                TJA115x_CMD40_ENG_ERR_EF(TJA115x_CMD40_ENG_ERR_EF_DEFAULT);
        }

        /* Align to defaults */
        Cfg->cmd50 =                                                        \
            TJA115x_CMD50_CONFIG_EN_TO(TJA115x_CMD50_CONFIG_EN_TO_DEFAULT) |  \
            TJA115x_CMD50_BBLM(TJA115x_CMD50_BBLM_DEFAULT) |                  \
            TJA115x_CMD50_LGEF(TJA115x_CMD50_LGEF_DEFAULT) |                  \
            TJA115x_CMD50_SEC_TO(TJA115x_CMD50_SEC_TO_DEFAULT) |              \
            TJA115x_CMD50_CAN_SP(TJA115x_CMD50_CAN_SP_DEFAULT) |              \
            TJA115x_CMD50_CANFD_SP(TJA115x_CMD50_CANFD_SP_DEFAULT) |          \
            TJA115x_CMD50_CAN_CANFD_BR(TJA115x_CMD50_CAN_CANFD_BR_DEFAULT);

        /* Align to defaults */
        Cfg->cmd60 =                                                        \
            TJA115x_CMD60_LCLREMn(TJA115x_CMD60_LCLREMn_DEFAULT) |            \
            TJA115x_CMD60_CONFIG_ID(TJA115x_CMD60_CONFIG_ID_DEFAULT);

        Cfg->numElements = 0U;
        Cfg->hwVersion = HwVersion;
    }

  return retVal;
}

Std_ReturnType Tm_BusyWait1us16bit(uint8 WaitingTimeMin)
{
    Std_ReturnType eReturnValue = E_OK;
	uint16 counter = (uint16)WaitingTimeMin;
	counter *= CPU_FREQUENCY_FACTOR;
	counter /= 4; /* there are four instructions in one WHILE loop */
	while(counter > 0)
	{
		counter--;
	}
	return eReturnValue;
}
/* End of file: CanTrcv_tja115x.c */
