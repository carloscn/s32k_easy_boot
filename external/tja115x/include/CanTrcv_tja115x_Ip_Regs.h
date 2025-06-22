/**
* @file CanTrcv_tja115x_Ip_Regs.h
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

#ifndef CANTRCV_TJA115X_IP_REGS_H
#define CANTRCV_TJA115X_IP_REGS_H

/**
* @page misra_violations MISRA-C:2012 violations
*
* @section [global]
* Violates MISRA 2012 Advisory Rule 2.5, global macro not referenced
* The register header defines macros for all bitfields to be used by the application code.
*
* @section [global]
* Violates MISRA 2012 Advisory Directive 4.9, Function-like macro
* These are generated macros used for accessing the bit-fields from registers.
*
* @section [global]
* Violates MISRA 2012 Required Rule 5.1, identifier clash
* The supported compilers use more than 31 significant characters for identifiers.
*
* @section [global]
* Violates MISRA 2012 Required Rule 5.2, identifier clash
* The supported compilers use more than 31 significant characters for identifiers.
*
* @section [global]
* Violates MISRA 2012 Required Rule 5.4, identifier clash
* The supported compilers use more than 31 significant characters for identifiers.
*
* @section [global]
* Violates MISRA 2012 Required Rule 5.5, identifier clash
* The supported compilers use more than 31 significant characters for identifiers.
* 
*/

#include "Std_Types.h"

/* CMD10 Register Masks
* Configuration of spoofing protection (command byte = 0x10)
*/
		
/* Filter Element Type */
#define TJA115x_CMD10_FET_RMASK	(0x80000000U)
#define TJA115x_CMD10_FET_SHIFT	(31U)
#define TJA115x_CMD10_FET_WIDTH	(1U)
#define	TJA115x_CMD10_FET(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_FET_SHIFT))&TJA115x_CMD10_FET_RMASK)
  
/* Standard Filter Configuration */
#define TJA115x_CMD10_SFC_RMASK	(0x60000000U)
#define TJA115x_CMD10_SFC_SHIFT	(29U)
#define TJA115x_CMD10_SFC_WIDTH	(2U)
#define	TJA115x_CMD10_SFC(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_SFC_SHIFT))&TJA115x_CMD10_SFC_RMASK)
  
/* Transmission Passlist for SFID1 */
#define TJA115x_CMD10_TPL1_RMASK	(0x10000000U)
#define TJA115x_CMD10_TPL1_SHIFT	(28U)
#define TJA115x_CMD10_TPL1_WIDTH	(1U)
#define	TJA115x_CMD10_TPL1(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_TPL1_SHIFT))&TJA115x_CMD10_TPL1_RMASK)
  
/* Bus Blocklist for SFID1 */
#define TJA115x_CMD10_BBL1_RMASK	(0x8000000U)
#define TJA115x_CMD10_BBL1_SHIFT	(27U)
#define TJA115x_CMD10_BBL1_WIDTH	(1U)
#define	TJA115x_CMD10_BBL1(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_BBL1_SHIFT))&TJA115x_CMD10_BBL1_RMASK)
  
/* The first Standard Identifier */
#define TJA115x_CMD10_SFID1_RMASK	(0x7FF0000U)
#define TJA115x_CMD10_SFID1_SHIFT	(16U)
#define TJA115x_CMD10_SFID1_WIDTH	(11U)
#define	TJA115x_CMD10_SFID1(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_SFID1_SHIFT))&TJA115x_CMD10_SFID1_RMASK)
  
/* Transmission Passlist for SFID2 */
#define TJA115x_CMD10_TPL2_RMASK	(0x1000U)
#define TJA115x_CMD10_TPL2_SHIFT	(12U)
#define TJA115x_CMD10_TPL2_WIDTH	(1U)
#define	TJA115x_CMD10_TPL2(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_TPL2_SHIFT))&TJA115x_CMD10_TPL2_RMASK)
  
/* Bus Blocklist for SFID2 */
#define TJA115x_CMD10_BBL2_RMASK	(0x800U)
#define TJA115x_CMD10_BBL2_SHIFT	(11U)
#define TJA115x_CMD10_BBL2_WIDTH	(1U)
#define	TJA115x_CMD10_BBL2(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_BBL2_SHIFT))&TJA115x_CMD10_BBL2_RMASK)
  
/* The second Standard Identifier or Standard Mask */
#define TJA115x_CMD10_SFID2_RMASK	(0x7FFU)
#define TJA115x_CMD10_SFID2_SHIFT	(0U)
#define TJA115x_CMD10_SFID2_WIDTH	(11U)
#define	TJA115x_CMD10_SFID2(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_SFID2_SHIFT))&TJA115x_CMD10_SFID2_RMASK)
  
/* Extended Filter Configuration */
#define TJA115x_CMD10_EFC_RMASK	(0x60000000U)
#define TJA115x_CMD10_EFC_SHIFT	(29U)
#define TJA115x_CMD10_EFC_WIDTH	(2U)
#define	TJA115x_CMD10_EFC(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_EFC_SHIFT))&TJA115x_CMD10_EFC_RMASK)
  
/* The Extended Identifier or the Extended Mask */
#define TJA115x_CMD10_EFID_RMASK	(0x1FFFFFFFU)
#define TJA115x_CMD10_EFID_SHIFT	(0U)
#define TJA115x_CMD10_EFID_WIDTH	(29U)
#define	TJA115x_CMD10_EFID(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD10_EFID_SHIFT))&TJA115x_CMD10_EFID_RMASK)
  
/* CMD10 Register Values
* Configuration of spoofing protection (command byte = 0x10)
*/

/* Filter Element Type */
#define TJA115x_CMD10_FET_STD  (0U) /*! Standard Identifier Filter Type */
#define TJA115x_CMD10_FET_EXT  (1U) /*! Extended Identifier Filter Type */
#define TJA115x_CMD10_FET_DEFAULT	TJA115x_CMD10_FET_STD

/* Standard Filter Configuration */
#define TJA115x_CMD10_SFC_OFF  (0x0U) /*! Filter element is disabled *//* [0b00u] */
#define TJA115x_CMD10_SFC_DUAL  (0x1U) /*! Dual filter ID for SFID1 or SFID2 *//* [0b01u] */
#define TJA115x_CMD10_SFC_CLASSIC  (0x2U) /*! Classic filter: SFID1 = filter, SFID2 = mask *//* [0b10u] */
#define TJA115x_CMD10_SFC_RFU  (0x3U) /*! Reserved *//* [0b11u] */
#define TJA115x_CMD10_SFC_DEFAULT	TJA115x_CMD10_SFC_OFF

/* Transmission Passlist for SFID1 */
#define TJA115x_CMD10_TPL1_DIS  (0U) /*! Do not include SFID1 in the Transmission Passlist */
#define TJA115x_CMD10_TPL1_EN  (1U) /*! Include SFID1 in the Transmission Passlist */
#define TJA115x_CMD10_TPL1_DEFAULT	TJA115x_CMD10_TPL1_DIS

/* Bus Blocklist for SFID1 */
#define TJA115x_CMD10_BBL1_DIS  (0U) /*! Do not include SFID1 in the Bus Blocklist */
#define TJA115x_CMD10_BBL1_EN  (1U) /*! Include SFID1 in the Bus Blocklist */
#define TJA115x_CMD10_BBL1_DEFAULT	TJA115x_CMD10_BBL1_DIS

/* The first Standard Identifier */
#define TJA115x_CMD10_SFID1_DEFAULT  (0x000U)

/* Transmission Passlist for SFID2 */
#define TJA115x_CMD10_TPL2_DIS  (0U) /*! Do not include SFID2 in the Transmission Passlist */
#define TJA115x_CMD10_TPL2_EN  (1U) /*! Include SFID2 in the Transmission Passlist */
#define TJA115x_CMD10_TPL2_DEFAULT	TJA115x_CMD10_TPL2_DIS

/* Bus Blocklist for SFID2 */
#define TJA115x_CMD10_BBL2_DIS  (0U) /*! Do not include SFID2 in the Bus Blocklist */
#define TJA115x_CMD10_BBL2_EN  (1U) /*! Include SFID2 in the Bus Blocklist */
#define TJA115x_CMD10_BBL2_DEFAULT	TJA115x_CMD10_BBL2_DIS

/* The second Standard Identifier or Standard Mask */
#define TJA115x_CMD10_SFID2_DEFAULT  (0x000U)

/* Extended Filter Configuration */
#define TJA115x_CMD10_EFC_MASK  (0x0U) /*! Interpret EFID as a mask for EFID in the next Filter Element "/> *//* [0b00u] */
#define TJA115x_CMD10_EFC_BBL  (0x1U) /*! Bus Blocklist enabled "/> *//* [0b01u] */
#define TJA115x_CMD10_EFC_TPL  (0x2U) /*! Transmission Passlist enabled "/> *//* [0b10u] */
#define TJA115x_CMD10_EFC_BOTH  (0x3U) /*! Both, Transmission Passlist and Bus Blocklist enabled *//* [0b11u] */

/* The Extended Identifier or the Extended Mask */
#define TJA115x_CMD10_EFID_DEFAULT  (0x00000000U)

/* CMD40 Register Masks
* Configuration of flooding and tamper protection (command byte=0x40)
*/
		
/* Selects acceptance of single or multiple edges around end of bit and start */
#define TJA115x_CMD40_TXD_GL_SME_RMASK	(0x400U)
#define TJA115x_CMD40_TXD_GL_SME_SHIFT	(10U)
#define TJA115x_CMD40_TXD_GL_SME_WIDTH	(1U)
#define	TJA115x_CMD40_TXD_GL_SME(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD40_TXD_GL_SME_SHIFT))&TJA115x_CMD40_TXD_GL_SME_RMASK)
  
/* Glitch detection on TXD to prevent bit manipulation */
#define TJA115x_CMD40_TXD_GL_EN_RMASK	(0x200U)
#define TJA115x_CMD40_TXD_GL_EN_SHIFT	(9U)
#define TJA115x_CMD40_TXD_GL_EN_WIDTH	(1U)
#define	TJA115x_CMD40_TXD_GL_EN(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD40_TXD_GL_EN_SHIFT))&TJA115x_CMD40_TXD_GL_EN_RMASK)
  
/* Enables sending of an error frame (6bit long) in case the internal CAN engines run into an error or do not detect a valid CRC */
#define TJA115x_CMD40_ENG_ERR_EF_RMASK	(0x100U)
#define TJA115x_CMD40_ENG_ERR_EF_SHIFT	(8U)
#define TJA115x_CMD40_ENG_ERR_EF_WIDTH	(1U)
#define	TJA115x_CMD40_ENG_ERR_EF(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD40_ENG_ERR_EF_SHIFT))&TJA115x_CMD40_ENG_ERR_EF_RMASK)
  
/* Tamper protection enable */
#define TJA115x_CMD40_TMPE_RMASK	(0x40U)
#define TJA115x_CMD40_TMPE_SHIFT	(6U)
#define TJA115x_CMD40_TMPE_WIDTH	(1U)
#define	TJA115x_CMD40_TMPE(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD40_TMPE_SHIFT))&TJA115x_CMD40_TMPE_RMASK)
  
/* Flooding threshold limit */
#define TJA115x_CMD40_FLDTH_RMASK	(0x20U)
#define TJA115x_CMD40_FLDTH_SHIFT	(5U)
#define TJA115x_CMD40_FLDTH_WIDTH	(1U)
#define	TJA115x_CMD40_FLDTH(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD40_FLDTH_SHIFT))&TJA115x_CMD40_FLDTH_RMASK)
  
/* Max flooding bucket limit */
#define TJA115x_CMD40_FLDMAX_RMASK	(0x1CU)
#define TJA115x_CMD40_FLDMAX_SHIFT	(2U)
#define TJA115x_CMD40_FLDMAX_WIDTH	(3U)
#define	TJA115x_CMD40_FLDMAX(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD40_FLDMAX_SHIFT))&TJA115x_CMD40_FLDMAX_RMASK)
  
/* Flooding load */
#define TJA115x_CMD40_FLDL_RMASK	(0x3U)
#define TJA115x_CMD40_FLDL_SHIFT	(0U)
#define TJA115x_CMD40_FLDL_WIDTH	(2U)
#define	TJA115x_CMD40_FLDL(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD40_FLDL_SHIFT))&TJA115x_CMD40_FLDL_RMASK)
  
/* CMD40 Register Values
* Configuration of flooding and tamper protection (command byte=0x40)
*/

/* Selects acceptance of single or multiple edges around end of bit and start */
#define TJA115x_CMD40_TXD_GL_SME_SINGLE  (0U) /*! only single edge */
#define TJA115x_CMD40_TXD_GL_SME_MULTIPLE  (1U) /*! multiple edges */
#define TJA115x_CMD40_TXD_GL_SME_DEFAULT	TJA115x_CMD40_TXD_GL_SME_SINGLE

/* Glitch detection on TXD to prevent bit manipulation */
#define TJA115x_CMD40_TXD_GL_EN_DIS  (0U) /*! disabled */
#define TJA115x_CMD40_TXD_GL_EN_EN  (1U) /*! enabled */
#define TJA115x_CMD40_TXD_GL_EN_DEFAULT	TJA115x_CMD40_TXD_GL_EN_DIS

/* Enables sending of an error frame (6bit long) in case the internal CAN engines run into an error or do not detect a valid CRC */
#define TJA115x_CMD40_ENG_ERR_EF_DIS  (0U) /*! disabled */
#define TJA115x_CMD40_ENG_ERR_EF_EN  (1U) /*! enabled */
#define TJA115x_CMD40_ENG_ERR_EF_DEFAULT	TJA115x_CMD40_ENG_ERR_EF_DIS

/* Tamper protection enable */
#define TJA115x_CMD40_TMPE_DIS  (0U) /*! disabled */
#define TJA115x_CMD40_TMPE_EN  (1U) /*! enabled */
#define TJA115x_CMD40_TMPE_DEFAULT	TJA115x_CMD40_TMPE_DIS

/* Flooding threshold limit */
#define TJA115x_CMD40_FLDTH_600  (0U) /*! Standard ID 0x600, Extended ID 0x18000000 */
#define TJA115x_CMD40_FLDTH_500  (1U) /*! Standard ID 0x500, Extended ID 0x14000000 */
#define TJA115x_CMD40_FLDTH_DEFAULT	TJA115x_CMD40_FLDTH_600

/* Max flooding bucket limit */
#define TJA115x_CMD40_FLDMAX_OFF  (0x0U) /*! Flooding prevention turned off *//* [0b000u] */
#define TJA115x_CMD40_FLDMAX_004  (0x1U) /*! 0.04/(1-FLDL) *//* [0b001u] */
#define TJA115x_CMD40_FLDMAX_008  (0x2U) /*! 0.08/(1-FLDL) *//* [0b010u] */
#define TJA115x_CMD40_FLDMAX_016  (0x3U) /*! 0.16/(1-FLDL) *//* [0b011u] */
#define TJA115x_CMD40_FLDMAX_032  (0x4U) /*! 0.32/(1-FLDL) *//* [0b100u] */
#define TJA115x_CMD40_FLDMAX_065  (0x5U) /*! 0.65s/(1-FLDL) *//* [0b101u] */
#define TJA115x_CMD40_FLDMAX_098  (0x6U) /*! 0.98s/(1-FLDL) *//* [0b110u] */
#define TJA115x_CMD40_FLDMAX_131  (0x7U) /*! 1.31s/(1-FLDL) *//* [0b111u] */
#define TJA115x_CMD40_FLDMAX_DEFAULT	TJA115x_CMD40_FLDMAX_OFF

/* Flooding load */
#define TJA115x_CMD40_FLDL_3  (0x0U) /*! 3.125% *//* [0b00u] */
#define TJA115x_CMD40_FLDL_6  (0x1U) /*! 6.25% *//* [0b01u] */
#define TJA115x_CMD40_FLDL_12  (0x2U) /*! 12.5% *//* [0b10u] */
#define TJA115x_CMD40_FLDL_25  (0x3U) /*! 25% *//* [0b11u] */
#define TJA115x_CMD40_FLDL_DEFAULT	TJA115x_CMD40_FLDL_25

/* CMD50 Register Masks
* Configuration of CAN FD bit timing, Secure Mode timeout, length of error frame and blocklist behavior (command byte = 0x50)
*/

/* Config enable time out (for local re-configuration after power on) */
#define TJA115x_CMD50_CONFIG_EN_TO_RMASK	(0x3000U)
#define TJA115x_CMD50_CONFIG_EN_TO_SHIFT	(12U)
#define TJA115x_CMD50_CONFIG_EN_TO_WIDTH	(2U)
#define	TJA115x_CMD50_CONFIG_EN_TO(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD50_CONFIG_EN_TO_SHIFT))&TJA115x_CMD50_CONFIG_EN_TO_RMASK)
  
/* Defines if the local CAN node receives the original blocklisted ID */
#define TJA115x_CMD50_BBLM_RMASK	(0x800U)
#define TJA115x_CMD50_BBLM_SHIFT	(11U)
#define TJA115x_CMD50_BBLM_WIDTH	(1U)
#define	TJA115x_CMD50_BBLM(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD50_BBLM_SHIFT))&TJA115x_CMD50_BBLM_RMASK)
  
/* Defines the Error Frame length for Stinger functions TPL, BBL, Tampering and Flooding */
#define TJA115x_CMD50_LGEF_RMASK	(0x400U)
#define TJA115x_CMD50_LGEF_SHIFT	(10U)
#define TJA115x_CMD50_LGEF_WIDTH	(1U)
#define	TJA115x_CMD50_LGEF(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD50_LGEF_SHIFT))&TJA115x_CMD50_LGEF_RMASK)
  
/* Secure mode timeout */
#define TJA115x_CMD50_SEC_TO_RMASK	(0x300U)
#define TJA115x_CMD50_SEC_TO_SHIFT	(8U)
#define TJA115x_CMD50_SEC_TO_WIDTH	(2U)
#define	TJA115x_CMD50_SEC_TO(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD50_SEC_TO_SHIFT))&TJA115x_CMD50_SEC_TO_RMASK)
  
/* Defines sample point Classical CAN / CAN FD ARBITRATION PHASE */
#define TJA115x_CMD50_CAN_SP_RMASK	(0xC0U)
#define TJA115x_CMD50_CAN_SP_SHIFT	(6U)
#define TJA115x_CMD50_CAN_SP_WIDTH	(2U)
#define	TJA115x_CMD50_CAN_SP(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD50_CAN_SP_SHIFT))&TJA115x_CMD50_CAN_SP_RMASK)
  
/* Defines sample point DATA PHASE */
#define TJA115x_CMD50_CANFD_SP_RMASK	(0x30U)
#define TJA115x_CMD50_CANFD_SP_SHIFT	(4U)
#define TJA115x_CMD50_CANFD_SP_WIDTH	(2U)
#define	TJA115x_CMD50_CANFD_SP(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD50_CANFD_SP_SHIFT))&TJA115x_CMD50_CANFD_SP_RMASK)
  
/* Defines CAN/CANFD bitrate */
#define TJA115x_CMD50_CAN_CANFD_BR_RMASK	(0xFU)
#define TJA115x_CMD50_CAN_CANFD_BR_SHIFT	(0U)
#define TJA115x_CMD50_CAN_CANFD_BR_WIDTH	(4U)
#define	TJA115x_CMD50_CAN_CANFD_BR(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD50_CAN_CANFD_BR_SHIFT))&TJA115x_CMD50_CAN_CANFD_BR_RMASK)
  
/* CMD50 Register Values
* Configuration of CAN FD bit timing, Secure Mode timeout, length of error frame and blocklist behavior (command byte = 0x50)
*/

/* Config enable time out (for local re-configuration after power on) */
#define TJA115x_CMD50_CONFIG_EN_TO_1  (0x0U) /*! 1 second *//* [0b00u] */
#define TJA115x_CMD50_CONFIG_EN_TO_2  (0x1U) /*! 2 seconds *//* [0b01u] */
#define TJA115x_CMD50_CONFIG_EN_TO_4  (0x2U) /*! 4 seconds *//* [0b10u] */
#define TJA115x_CMD50_CONFIG_EN_TO_8  (0x3U) /*! 8 seconds *//* [0b11u] */
#define TJA115x_CMD50_CONFIG_EN_TO_DEFAULT	TJA115x_CMD50_CONFIG_EN_TO_8

/* Defines if the local CAN node receives the original blocklisted ID */
#define TJA115x_CMD50_BBLM_DIS  (0U) /*! local node does not receive the blocklisted frame */
#define TJA115x_CMD50_BBLM_EN  (1U) /*! local node does receive the blocklisted frame */
#define TJA115x_CMD50_BBLM_DEFAULT	TJA115x_CMD50_BBLM_DIS

/* Defines the Error Frame length for Stinger functions TPL, BBL, Tampering and Flooding */
#define TJA115x_CMD50_LGEF_NORM  (0U) /*! Normal length of error flag (6 bit) */
#define TJA115x_CMD50_LGEF_LONG  (1U) /*! Long error frame (11 bits) for logging purposes */
#define TJA115x_CMD50_LGEF_DEFAULT	TJA115x_CMD50_LGEF_NORM

/* Secure mode timeout */
#define TJA115x_CMD50_SEC_TO_500  (0x0U) /*! 500 ms *//* [0b00u] */
#define TJA115x_CMD50_SEC_TO_2  (0x1U) /*! 2 seconds *//* [0b01u] */
#define TJA115x_CMD50_SEC_TO_8  (0x2U) /*! 8 seconds *//* [0b10u] */
#define TJA115x_CMD50_SEC_TO_AUTO  (0x3U) /*! Auto (1st time: 0.5s, 2nd time: 2s, >=3rd time: 8s) *//* [0b11u] */
#define TJA115x_CMD50_SEC_TO_DEFAULT	TJA115x_CMD50_SEC_TO_2

/* Defines sample point Classical CAN / CAN FD ARBITRATION PHASE */
#define TJA115x_CMD50_CAN_SP_70  (0x0U) /*! 70% *//* [0b00u] */
#define TJA115x_CMD50_CAN_SP_75  (0x1U) /*! 75% *//* [0b01u] */
#define TJA115x_CMD50_CAN_SP_80  (0x2U) /*! 80% *//* [0b10u] */
#define TJA115x_CMD50_CAN_SP_RFU  (0x3U) /*! Reserved, do not use *//* [0b11u] */
#define TJA115x_CMD50_CAN_SP_DEFAULT	TJA115x_CMD50_CAN_SP_80

/* Defines sample point DATA PHASE */
#define TJA115x_CMD50_CANFD_SP_70  (0x0U) /*! 70% *//* [0b00u] */
#define TJA115x_CMD50_CANFD_SP_75  (0x1U) /*! 75% *//* [0b01u] */
#define TJA115x_CMD50_CANFD_SP_80  (0x2U) /*! 80% *//* [0b10u] */
#define TJA115x_CMD50_CANFD_SP_RFU  (0x3U) /*! Reserved, do not use *//* [0b11u] */
#define TJA115x_CMD50_CANFD_SP_DEFAULT	TJA115x_CMD50_CANFD_SP_80

/* Defines CAN/CANFD bitrate */
#define TJA115x_CMD50_CAN_CANFD_BR_125  (0x0U) /*! 125 kbps (Classical CAN Only) *//* [0b0000u] */
#define TJA115x_CMD50_CAN_CANFD_BR_250_500  (0x5U) /*! 250/500 kbps *//* [0b0101u] */
#define TJA115x_CMD50_CAN_CANFD_BR_250_1000  (0x6U) /*! 250/1000 kbps *//* [0b0110u] */
#define TJA115x_CMD50_CAN_CANFD_BR_500_1000  (0xAU) /*! 500/1000 kbps *//* [0b1010u] */
#define TJA115x_CMD50_CAN_CANFD_BR_500_2000  (0xBU) /*! 500/2000 kbps *//* [0b1011u] */
#define TJA115x_CMD50_CAN_CANFD_BR_DEFAULT	TJA115x_CMD50_CAN_CANFD_BR_500_2000

/* CMD60 Register Masks
* Configuration of command message ID (command byte = 0x60)
*/
		
/* Defines reconfiguration permissions */
#define TJA115x_CMD60_LCLREMn_RMASK	(0x80000000U)
#define TJA115x_CMD60_LCLREMn_SHIFT	(31U)
#define TJA115x_CMD60_LCLREMn_WIDTH	(1U)
#define	TJA115x_CMD60_LCLREMn(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD60_LCLREMn_SHIFT))&TJA115x_CMD60_LCLREMn_RMASK)
  
/* Defines new CONFIG Identifier to be used */
#define TJA115x_CMD60_CONFIG_ID_RMASK	(0x1FFFFFFFU)
#define TJA115x_CMD60_CONFIG_ID_SHIFT	(0U)
#define TJA115x_CMD60_CONFIG_ID_WIDTH	(29U)
#define	TJA115x_CMD60_CONFIG_ID(x_)	(((uint32)(((uint32)(x_))<<TJA115x_CMD60_CONFIG_ID_SHIFT))&TJA115x_CMD60_CONFIG_ID_RMASK)
  
/* CMD60 Register Values
* Configuration of command message ID (command byte = 0x60)
*/

/* Defines reconfiguration permissions */
#define TJA115x_CMD60_LCLREMn_CONFIG_REM  (0U) /*! Reconfiguration is only accepted remotely from the bus */
#define TJA115x_CMD60_LCLREMn_CONFIG_LOC  (1U) /*! Reconfiguration is only accepted locally */
#define TJA115x_CMD60_LCLREMn_DEFAULT	TJA115x_CMD60_LCLREMn_CONFIG_REM

/* Defines new CONFIG Identifier to be used */
#define TJA115x_CMD60_CONFIG_ID_CANID  (0x18DA00F1U) /*! Default value */
#define TJA115x_CMD60_CONFIG_ID_DEFAULT	TJA115x_CMD60_CONFIG_ID_CANID

#endif /* CANTRCV_TJA115X_IP_REGS_H */
