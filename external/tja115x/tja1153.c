/*******************************************************************************
* Project               :   Tja1153_S32K344EVB_TestPrj
* Filename              :   Tja1153.c
* Origin Date           :   Nov 18, 2021
* Version               :   
* Compiler              :   
* Target                :   
* Notes                 :   
* Author                :   nxf65056
*
*   (c) Copyright 2020 NXP Semiconductors
*   All Rights Reserved.
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
*****************************************************************************/
/*************** SOURCE REVISION LOG *****************************************
*
*    Date    Version   Author         Description 
*  TODO
*
*******************************************************************************/
/** @file 
 *  @brief This is the source file for TODO: WHAT DO I DO? 
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include "tja1153.h"

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/

/******************************************************************************
* Function Local Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
Std_ReturnType Tja1153_Init(uint8_t instance)
{
    Std_ReturnType status = E_NOT_OK;
    CanTrcv_tja115x_ErrorCodeType errorCode = TJA115x_ERR_INVALID;
    status = TJA115x_DRV_Init(instance, &TJA1153_ConfigSet[instance]);

    status = TJA115x_DRV_UpdateCanCommand(tja115x_drv_send); //Install the send API

    errorCode = TJA115x_DRV_EnterConfigVanilla(&TJA1153_paraType[instance]); //transmit CAN message for baudrate auto-detection.
    //tja1153_status = TJA115x_DRV_EnterConfigLocal(TJA115x_CMD60_CONFIG_ID_DEFAULT, &tja1153_paraType, 0);

    // ID filter, 0x40, 0x50, 0x60 register are all configured in this line
    errorCode = TJA115x_DRV_ConfigureDevice(TJA115x_CMD60_CONFIG_ID_DEFAULT, &TJA1153_paraType[instance], &TJA1153_ComType[instance]);
                          
    errorCode = TJA115x_DRV_LeaveConfig(TJA115x_CMD60_CONFIG_ID_DEFAULT, &TJA1153_paraType[instance], TJA115x_CLOSE_VOLATILE);
    
    status = TJA115x_DRV_SetMode(instance, TJA115x_TRCVMODE_NORMAL);

    return status;
}


/*************** END OF FUNCTIONS ***************************************************************************/
/*************** END OF FILES ***************************************************************************/
/** @} */
