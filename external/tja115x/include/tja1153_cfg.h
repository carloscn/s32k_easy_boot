/****************************************************************************
* Project               :   Can_Fd_Interrupt_47827A_440_090
* Filename              :   Tja1153_Config.h
* Origin Date           :   Nov 16, 2021
* Version               :   X.X.X
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
*************** INTERFACE CHANGE LIST ****************************************/
/*************** HEADER REVISION LOG *****************************************
*    Date    Version   Author         Description 
*  TODO
*
*****************************************************************************/
/** @file 
 *  @brief This module TODO: WHAT DO I DO?
 * 
 *  This is the header file for the definition TODO: MORE ABOUT ME!
 */
#ifndef TJA1153_CONFIG_H_
#define TJA1153_CONFIG_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include "CanTrcv_tja115x_Ip.h"
#include "Siul2_Port_Ip.h"
/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants
*******************************************************************************/
#define MAX_TJA1153_INSTANCE 1


/******************************************************************************
* Macros
*******************************************************************************/

	
/******************************************************************************
* Typedefs
*******************************************************************************/

/******************************************************************************
* GLOBAL Variables
*******************************************************************************/
extern const CanTrcv_tja115x_DevicePinType TJA1153_a_EN;
extern const CanTrcv_tja115x_DevicePinType TJA1153_a_STB;

extern const CanTrcv_tja115x_DeviceConfigType TJA1153_ConfigSet[MAX_TJA1153_INSTANCE];

extern CanTrcv_tja115x_CanComParamsType TJA1153_paraType[MAX_TJA1153_INSTANCE];

extern CanTrcv_tja115x_CommandsType TJA1153_ComType[MAX_TJA1153_INSTANCE];

/******************************************************************************
* Function Prototypes
*******************************************************************************/

#endif /* TJA1153_CONFIG_H_ */
/** @} */


