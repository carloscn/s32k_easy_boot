/****************************************************************************
* Project               :   Tja1153_S32K344EVB_TestPrj
* Filename              :   Tja1153.h
* Origin Date           :   Nov 18, 2021
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
#ifndef CANTRCV_43_TJA115X_TS_T40D11M8I0R0_TJA1153_H_
#define CANTRCV_43_TJA115X_TS_T40D11M8I0R0_TJA1153_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include "CanTrcv_tja115x_Ip.h"
#include "tja1153_cfg.h"

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/


/******************************************************************************
* Configuration Constants
*******************************************************************************/


/******************************************************************************
* Macros
*******************************************************************************/


	
/******************************************************************************
* Typedefs
*******************************************************************************/

/******************************************************************************
* GLOBAL Variables
*******************************************************************************/


/******************************************************************************
* Function Prototypes
*******************************************************************************/
Std_ReturnType Tja1153_Init(uint8_t instance);
Std_ReturnType Tm_BusyWait1us16bit(uint8 WaitingTimeMin);
#endif /* CANTRCV_43_TJA115X_TS_T40D11M8I0R0_TJA1153_H_ */
/** @} */


