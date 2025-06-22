/**
* @file CanTrcv_tja115x_Ip.h
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

#ifndef CANTRCV_TJA115X_IP_H
#define CANTRCV_TJA115X_IP_H

#if defined(__cplusplus)
extern "C" {
#endif

/**
* @page misra_violations MISRA-C:2012 violations
*
* @section [global]
* Violates MISRA 2012 Advisory Rule 2.5, global macro not referenced
* The default configuration ID is defined to be used by the application code.
*/

#include "CanTrcv_tja115x_Ip_Regs.h"
//#include "CanTrcv_tja115x_Ip_Cfg.h"
//#include "Tja1153_Config.h"
//#include "Dio.h"

/**
* @addtogroup CanTrcv_tja115x TJA115x TRCV Driver
* @ingroup trcv_tja115x
* @{
*/

/*==================================================================================================
*                               SOURCE FILE VERSION INFORMATION
==================================================================================================*/

/*==================================================================================================
*                                      FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
*                                           CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       DEFINES AND MACROS
==================================================================================================*/
/* Maximum number of messages sent for auto bit rate detection */
#define CANTRCV_TJA115X_ENTERVANILLA_TRIES 2U

/* Maximum number of devices */
#define CANTRCV_TJA115X_MAX_DEVICES_NUM    (16U)
#define CANTRCV_TJA115X_INVALID_DEVICE_IDX (127U)

/* Maximum number of filter elements */
#define CANTRCV_TJA115X_ELEMENTS_COUNT     (16U)

#define CANTRCV_TJA115X_AUTOBRDET_CANID    (0x555U)

#define CANTRCV_TJA115X_STATE_MASK         (0x03U)
#define CANTRCV_TJA115X_PRGCNT_MAX         (127U)

#define USE_IPV_CANTRCV_TJA115X            STD_ON

#ifdef USE_IPV_CANTRCV_TJA115X
#include "Siul2_Dio_Ip.h"
#else
#include "Dio.h"
#endif

/* Calculations were done for 160MHz CPU frequency */
#define CPU_FREQUENCY_FACTOR 160

/*==================================================================================================
                                             ENUMS
==================================================================================================*/
/**
* @brief Error codes
*
* @implements CanTrcv_tja115x_ErrorCodeType_Class
*/
typedef enum
{
    TJA115x_NOERROR = 0,     /**< No error - successful operation */
    TJA115x_ERR_NOACK,       /**< CAN message is not acknowleged */
    TJA115x_ERR_INVALID,     /**< Invalid parameter(s) */
    TJA115x_ERR_BAUDRATE,    /**< Failure during vanilla baudrate detection */
    TJA115x_ERR_REM_REQ,     /**< Failure to perform or initiate remote configuration request*/
    TJA115x_ERR_REM_CONF,    /**< Failure to confirm remote configuration request */
    TJA115x_ERR_LOC_REQ,     /**< Failure to perform or initiate local configuration request*/
    TJA115x_ERR_LOC_INIT,    /**< Failure to get the device into Standby due to init fails */
    TJA115x_ERR_MODE,        /**< Failure to confirm configuration mode, i.e.: TJA115x is not in expected mode */
    TJA115x_ERR_ELEMENT,     /**< Failure to transmit filter element command */
    TJA115x_ERR_WRONG_ELM,   /**< Filter element register verification failure */
    TJA115x_ERR_COMMAND,     /**< Failure to transmit non-FET command register */
    TJA115x_ERR_WRONG_CMD,   /**< Command (non-FET) register verification failure */
    TJA115x_ERR_LEAVE        /**< Failure to leave configuration mode */
} CanTrcv_tja115x_ErrorCodeType;

/**
* @brief Leave configuration modes
*
* @implements CanTrcv_tja115x_LeaveModeType_Class
*/
typedef enum
{
    TJA115x_LEAVE_VOLATILE  = 0x71U, /**< Development mode without lock, NO MTP programming. */
    TJA115x_LEAVE_OPEN_LOCK = 0x72U, /**< Programming configuration into nonvolatile
                                          and set 'open lock bit' */
    TJA115x_LEAVE_FULL_LOCK = 0x74U, /**< Programming configuration into nonvolatile
                                          and set 'full lock bit' */
    TJA115x_CLOSE_VOLATILE  = 0xF1U, /**< Same as TJA115x_LEAVE_VOLATILE with immediate
                                          close of configuration window */
    TJA115x_CLOSE_OPEN_LOCK = 0xF2U, /**< Same as TJA115x_LEAVE_OPEN_LOCK with immediate
                                          close of configuration window */
    TJA115x_CLOSE_FULL_LOCK = 0xF4U  /**< Same as TJA115x_LEAVE_FULL_LOCK with immediate
                                          close of configuration window */
} CanTrcv_tja115x_LeaveModeType;

/**
* @brief TJA115x Transceiver modes
*
* @implements     CanTrcv_tja115x_TrcvModeType_Class
*/
typedef enum
{
    TJA115x_TRCVMODE_NORMAL     = 0x00U,
    TJA115x_TRCVMODE_STANDBY    = 0x01U,
    TJA115x_TRCVMODE_SLEEP      = 0x02U,
    TJA115x_TRCVMODE_LISTENONLY = 0x03U
} CanTrcv_tja115x_TrcvModeType;

/**
* @brief TJA115x Hardware type
*/
typedef enum
{
    TJA115x_HWTYPE_TJA1152 = 0x00U,
    TJA115x_HWTYPE_TJA1153 = 0x01U
} CanTrcv_tja115x_HwType;

/**
* @brief TJA115x Hardware version
*/
typedef enum
{
    TJA115x_HW_VER_0 = 0U,
    TJA115x_HW_VER_1 = 1U
} CanTrcv_tja115x_HwVersionType;

/**
* @brief CAN message type
*/
typedef enum
{
    TJA115x_CANMSG_STD = 0U,
    TJA115x_CANMSG_EXT = 1U
} CanTrcv_tja115x_CanMsgType;

/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
/**
* @brief TJA115x Device configuration description - register based.
*
* This form of the configuration allows to optimise the required
* storage space to store the device configuration. The required
* storage is purely limited to the space used by the device
* registers which is e.g.: helpful during firmware updates.
*
* @implements CanTrcv_tja115x_CommandsType_Class
*
*/
typedef struct
{
    uint32 elements[CANTRCV_TJA115X_ELEMENTS_COUNT];
    uint32 cmd40;
    uint32 cmd50;
    uint32 cmd60;
    uint8  numElements;
    CanTrcv_tja115x_HwVersionType   hwVersion;
} CanTrcv_tja115x_CommandsType;

/**
* @brief TJA115x Single PIN description
*/
typedef struct
{
#ifdef USE_IPV_CANTRCV_TJA115X
    Siul2_Dio_Ip_GpioType* gpioBase; /**< GPIO base pointer. */
#else
    Dio_PortType port;               /**< Port number. */
#endif
    uint8        pinPortIdx;         /**< Port pin number. */
} CanTrcv_tja115x_DevicePinType;

/**
* @brief TJA115x CAN communication configuration
*/
typedef struct
{
    uint8  instance;   /**< Used CAN instance for configuration. */
    uint8  mb_idx;     /**< Index of the message buffer used for transmission. */
    uint32 timeout_ms; /**< A timeout for the transmission in milliseconds. */
} CanTrcv_tja115x_CanComParamsType;

/**
* @brief TJA115x Driver configuration description
*
* @implements CanTrcv_tja115x_DeviceConfigType_Class
*/
typedef struct
{
    CanTrcv_tja115x_DevicePinType  EN;     /**< Enable output pin. This pin is not used for TJA1152 */
    CanTrcv_tja115x_DevicePinType  STB;    /**< Standby output pin. This pin is negated for TJA1153 */
#ifdef USE_IPV_CANTRCV_TJA115X
    CanTrcv_tja115x_DevicePinType  WAKE;   /**< Local wake-up output pin. */
#endif
    CanTrcv_tja115x_HwType hwType; /**< Hardware type */
} CanTrcv_tja115x_DeviceConfigType;

/**
* @brief          CanTrcv_tja115x_DeviceConfigCommandsType
* @details        The CanTrcv_tja115x_DeviceConfigCommandsType structure contains an
*                 entire device configuration and the state in which the
*                 device is left after the operations.
*
* @implements     CanTrcv_tja115x_DeviceConfigCommandsType_Class
*/
typedef struct
{
    CanTrcv_tja115x_CommandsType  ConfigCommands;
    CanTrcv_tja115x_LeaveModeType LeaveMode;
} CanTrcv_tja115x_DeviceConfigCommandsType;

/**
* @brief TJA115x CAN transmission function definition.
*
* @param CanComParams is the CAN parameters needed for transmission.
* @param CanId CAN identifier
* @param CanMsgType Type of message ID (standard or extended)
* @param Dlc Data Length Code (max 8)
* @param Payload Pointer to allocated payload data
*
* @return TJA115x_NOERROR on successful transmission.
* @return TJA115x_ERR_NOACK on missing acknowledgment.
*/
typedef CanTrcv_tja115x_ErrorCodeType (*tja115x_can_send)(const uint32 CanId,
                                                          const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                          const CanTrcv_tja115x_CanMsgType CanMsgType,
                                                          const uint32 Dlc,
                                                          uint8* const Payload);

/*==================================================================================================
*                                 GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/
#ifdef CANTRCV_TJA115X_CONFIG_EXT
/**
* @brief   Export Post-Build configurations.
*/
CANTRCV_TJA115X_CONFIG_EXT
#endif

/*==================================================================================================
*                                    FUNCTION PROTOTYPES
==================================================================================================*/
/**
* @brief Initialize transceiver driver instance.
*
* @param Instance is the driver instance number to initialize.
* @param Cfg Pointer to allocated driver configuration description.
*
* @return STATUS_SUCCESS or STATUS_ERROR in case of invalid parameter.
*/
Std_ReturnType TJA115x_DRV_Init(const uint8 Instance,
                          const CanTrcv_tja115x_DeviceConfigType* const Cfg);

/**
* @brief Change the transceiver operation mode.
*
* @param Instance is the driver instance number.
* @param Mode is the new mode to set.
*
* @return STATUS_SUCCESS or STATUS_ERROR in case of invalid parameter.
*/
Std_ReturnType  TJA115x_DRV_SetMode(const uint8 Instance, const CanTrcv_tja115x_TrcvModeType Mode);

/**
* @brief Get the transceiver operation mode.
*
* @param Instance is the driver instance number.
* @param Mode (output parameter) will hold the mode of the transceiver.
*
* @return STATUS_SUCCESS or STATUS_ERROR in case of invalid parameter.
*/
Std_ReturnType TJA115x_DRV_GetMode(const uint8 Instance, CanTrcv_tja115x_TrcvModeType* Mode);

/**
* @brief DeInitialize transceiver driver instance. The driver will
*        leave the transceiver in the STANDBY mode.
*
* @param Instance is the driver instance number to de-initialize.
*
* @return STATUS_SUCCESS or STATUS_ERROR in case of invalid parameter.
*/
Std_ReturnType TJA115x_DRV_DeInit(const uint8 Instance);

/**
* @brief Update CAN communication method for the transceiver.
*
* The CAN communication method can be configured to use
* a user defined CAN transmission function. If this
* method is not called, the driver will use a default method.
*
* @param Fct is the function to transmit a CAN message (@see
* tja115x_can_send for details).
*
* @return STATUS_SUCCESS or STATUS_ERROR in case of invalid
* parameters.
*/
Std_ReturnType TJA115x_DRV_UpdateCanCommand(const tja115x_can_send Fct);

/**
* @brief Enter configuration mode on vanilla device, i.e.: transmit
* CAN message for baudrate auto-detection.
*
* @param CanComParams is the CAN parameters needed for transmission.
*
* @return TJA115x_NOERROR on success or TJA115x_ERR_BAUDRATE
*/
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_EnterConfigVanilla(const CanTrcv_tja115x_CanComParamsType* CanComParams);

/**
* @brief Enter configuration mode on remote device, i.e.: transmit
* CAN messages to enter configuration mode: request only.
* To confirm the request, wait at least 100ms and call TJA115x_DRV_ConfirmConfigRemote.
*
* @param CanComParams is the CAN parameters needed for transmission.
* @param CanId CAN ID of the configuration message command
*
* @return TJA115x_NOERROR on success or TJA115x_ERR_REM_REQ.
*/
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_EnterConfigRemote(const uint32 CanId,
                                                            const CanTrcv_tja115x_CanComParamsType* CanComParams);

/**
* @brief Enter configuration mode on remote device, i.e.: transmit
* CAN messages to enter configuration mode: confirmation only.
* Between requesting configuration mode (TJA115x_DRV_EnterConfigRemote)
* and confirmation there should be at least 100ms waiting time.
*
* @param CanComParams is the CAN parameters needed for transmission.
* @param CanId CAN ID of the configuration message command
*
* @return TJA115x_NOERROR on success or TJA115x_ERR_REM_CONF or TJA115x_ERR_MODE.
*/
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_ConfirmConfigRemote(const uint32 CanId,
                                                              const CanTrcv_tja115x_CanComParamsType* CanComParams);

/**
* @brief Enter configuration mode on local device, i.e.: transmit
* CAN messages to enter configuration mode.
*
* @param CanComParams is the CAN parameters needed for transmission.
* @param CanId CAN ID of the configuration message command
* @param Instance is the CAN instance number to address transceiver.
*
* @return TJA115x_NOERROR on success or TJA115x_ERR_LOC_REQ or TJA115x_ERR_LOC_INIT or TJA115x_ERR_MODE.
*/
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_EnterConfigLocal(const uint32 CanId,
                                                           const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                           const uint8 Instance);
/**
* @brief Transmit message command to leave configuration mode.
*
* @param CanId Configuration message CAN ID
* @param CanComParams is the CAN parameters needed for transmission.
* @param LeaveMode Leave configuration mode (@see CanTrcv_tja115x_LeaveModeType)
*
* @return TJA115x_NOERROR on success or TJA115x_ERR_LEAVE on command ACK failure
*/
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_LeaveConfig(const uint32 CanId,
                                                      const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                      const CanTrcv_tja115x_LeaveModeType LeaveMode);

/**
* @brief Transmit provided configuration description.
*
* Preliminary: target device must be in configuration mode (@see
* TJA115x_DRV_EnterConfigVanilla, TJA115x_DRV_EnterConfigRemote or
* TJA115x_DRV_EnterConfigLocal)
*
* @param CanId CAN ID of the configuration message command
* @param CanComParams is the CAN parameters needed for transmission.
* @param Cfg Pointer to allocated and configured device configuration description.
*
* @return TJA115x_NOERROR on success or another TJA115x_ErrorCode
*/
CanTrcv_tja115x_ErrorCodeType TJA115x_DRV_ConfigureDevice(const uint32 CanId,
                                                          const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                          const CanTrcv_tja115x_CommandsType* Cfg);

/**
* @brief Initialize device configuration description with default values.
*
* @param Cfg Pointer to allocated device configuration description
* @param HwVersion
*
* @return TJA115x_NOERROR
*/
Std_ReturnType TJA115x_DRV_GetDefaultConfiguration(CanTrcv_tja115x_CommandsType* const Cfg,
                                             const CanTrcv_tja115x_HwVersionType HwVersion);

CanTrcv_tja115x_ErrorCodeType tja115x_drv_send(const uint32 CanId,
                                                      const CanTrcv_tja115x_CanComParamsType* CanComParams,
                                                      const CanTrcv_tja115x_CanMsgType CanMsgType,
                                                      const uint32 DataLen,
                                                      const uint8* const Data);
#ifdef __cplusplus
}
#endif


Std_ReturnType Tm_BusyWait1us16bit(uint8 WaitingTimeMin);

#endif /* CANTRCV_TJA115H_IP_H */

/** @} */
