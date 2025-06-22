/*******************************************************************************
* Project               :   Can_Fd_Interrupt_47827A_440_090
* Filename              :   Tja1153_Config.c
* Origin Date           :   Nov 16, 2021
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
#include "tja1153_cfg.h"

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
const CanTrcv_tja115x_DevicePinType TJA1153_a_EN =
{
  .gpioBase = CAN0_EN_PORT,
  .pinPortIdx = CAN0_EN_PIN,
};
const CanTrcv_tja115x_DevicePinType TJA1153_a_STB =
{
  .gpioBase = CAN0_STB_PORT,
  .pinPortIdx = CAN0_STB_PIN,
};

const CanTrcv_tja115x_DeviceConfigType TJA1153_ConfigSet[MAX_TJA1153_INSTANCE] =
{
  {
    .EN  = TJA1153_a_EN,
    .STB = TJA1153_a_STB,
    .hwType = TJA115x_HWTYPE_TJA1153
  }
};

CanTrcv_tja115x_CanComParamsType TJA1153_paraType[MAX_TJA1153_INSTANCE] =
{
  {
    .instance = 0,
    .mb_idx   = 6,
    .timeout_ms = 100,
  }
};

CanTrcv_tja115x_CommandsType TJA1153_ComType[MAX_TJA1153_INSTANCE] =
{
  {
    /* Filter element 0_0. */
    .elements[0] =  TJA115x_CMD10_FET(TJA115x_CMD10_FET_STD) | \
                    TJA115x_CMD10_SFC(TJA115x_CMD10_SFC_CLASSIC) | \
                    TJA115x_CMD10_TPL1(TJA115x_CMD10_TPL1_EN) | \
                    TJA115x_CMD10_BBL1(TJA115x_CMD10_BBL1_DIS) | \
                    TJA115x_CMD10_SFID1(0U) | \
                    TJA115x_CMD10_TPL2(TJA115x_CMD10_TPL2_DIS) | \
                    TJA115x_CMD10_BBL2(TJA115x_CMD10_BBL2_DIS) | \
                    TJA115x_CMD10_SFID2(2047U),
    /* Filter element 0_1. */
    .elements[1] =  TJA115x_CMD10_FET(TJA115x_CMD10_FET_EXT) | \
                    TJA115x_CMD10_EFC(TJA115x_CMD10_EFC_MASK) | \
                    TJA115x_CMD10_EFID(4294967295U),
    /* Filter element 0_2. */
    .elements[2] =  TJA115x_CMD10_FET(TJA115x_CMD10_FET_EXT) | \
                    TJA115x_CMD10_EFC(TJA115x_CMD10_EFC_TPL) | \
                    TJA115x_CMD10_EFID(0U),
    /* CMD40. */
    .cmd40 =  TJA115x_CMD40_TMPE(TJA115x_CMD40_TMPE_DIS) | \
              TJA115x_CMD40_FLDTH(TJA115x_CMD40_FLDTH_600) | \
              TJA115x_CMD40_FLDMAX(TJA115x_CMD40_FLDMAX_OFF) | \
              TJA115x_CMD40_FLDL(TJA115x_CMD40_FLDL_25),
    /* CMD50. */
    .cmd50 =  TJA115x_CMD50_CONFIG_EN_TO(TJA115x_CMD50_CONFIG_EN_TO_8) | \
              TJA115x_CMD50_BBLM(TJA115x_CMD50_BBLM_DIS) | \
              TJA115x_CMD50_LGEF(TJA115x_CMD50_LGEF_NORM) | \
              TJA115x_CMD50_SEC_TO(TJA115x_CMD50_SEC_TO_2) | \
              TJA115x_CMD50_CAN_SP(TJA115x_CMD50_CAN_SP_75) | \
              TJA115x_CMD50_CANFD_SP(TJA115x_CMD50_CANFD_SP_75) | \
              TJA115x_CMD50_CAN_CANFD_BR(TJA115x_CMD50_CAN_CANFD_BR_500_2000),
    /* CMD60. */
    .cmd60 =  TJA115x_CMD60_LCLREMn(TJA115x_CMD60_LCLREMn_CONFIG_LOC) | \
              TJA115x_CMD60_CONFIG_ID(416940273U),
    /* Number of elements. */
    .numElements = 3U,
    /* Hardware version. */
    .hwVersion = TJA115x_HW_VER_0,
  }
};

/******************************************************************************
* Function Local Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/

/*************** END OF FUNCTIONS ***************************************************************************/
/*************** END OF FILES ***************************************************************************/
/** @} */
