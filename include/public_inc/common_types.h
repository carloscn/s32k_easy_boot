/**
  @file             common_types.h
  @version          0.0.0.0

  @brief            Common types
  @details          

  Project           M4_ETH_PROJECT_NAME
  Platform          M4_ETH_PLATFORM

  SWVersion         M4_ETH_RELEASE_VERSION


  Copyright 2016,2018 NXP
  
 This software is owned or controlled by NXP and may only be used strictly 
 in accordance with the applicable license terms.  By expressly accepting 
 such terms or by downloading, installing, activating and/or otherwise using 
 the software, you are agreeing that you have read, and that you agree to 
 comply with and are bound by, such license terms.  If you do not agree to 
 be bound by the applicable license terms, then you may not retain, 
 install, activate or otherwise use the software.
*/
/*==================================================================================================
==================================================================================================*/

/*==================================================================================================
                                         MISRA VIOLATIONS
==================================================================================================*/

/**
* @page misra_violations MISRA-C:2004 violations
*
* @section common_types_h_REF_1
* Violates MISRA 2004 Advisory Rule 19.7, A function should be used in preference to a function-like macro.
* Function-like macros does not support type control of parameters but inline functions usage causes
* problems with some kind of compilers. Was also checked that macros are used correctly!
*/

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H
/*==================================================================================================
                                         INCLUDE FILES
 1) system and project includes
 2) needed interfaces from external units
 3) internal and external interfaces from this unit
==================================================================================================*/  

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*==================================================================================================
                               SOURCE FILE VERSION INFORMATION
==================================================================================================*/

/*==================================================================================================
                                      FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
                                           CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                       DEFINES AND MACROS
==================================================================================================*/
#ifndef NULL
    #define NULL ((void *)0)
#endif  /* NULL */

#ifndef NULL_PTR
#define NULL_PTR ((void *)0)
#endif

#ifndef FALSE
    #define FALSE 0U
#endif /* FALSE */

#ifndef TRUE 
    #define TRUE 1U
#endif /* TRUE */

#ifndef LITTLE
    #define LITTLE 1
#endif

#ifndef BIG
    #define BIG 2
#endif

#ifndef MACHINE_ENDIAN
    #define MACHINE_ENDIAN LITTLE
#endif    


#if(LITTLE == MACHINE_ENDIAN)
#ifndef ntohs
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohs(a) (((((uint16_t)(a)) >> 8) & 0x00ffU) | \
                  (((uint16_t)(((uint16_t)(a)) << 8)) & 0xff00U))
#endif

#ifndef ntohl
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohl(a) (((((uint32_t)(a)) >> 24) & 0x000000ffU) | \
                  ((((uint32_t)(a)) >> 8 ) & 0x0000ff00U) | \
                  ((uint32_t)(((uint32_t)(a)) << 8 ) & 0x00ff0000U) | \
                  ((uint32_t)(((uint32_t)(a)) << 24) & 0xff000000U))
#endif
                  
#ifndef ntohll                  
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohll(a) (((((uint64_t)(a)) & 0xFF00000000000000ULL) >> 56U) | \
                  ((((uint64_t)(a)) & 0x00FF000000000000ULL) >> 40U) | \
                  ((((uint64_t)(a)) & 0x0000FF0000000000ULL) >> 24U) | \
                  ((((uint64_t)(a)) & 0x000000FF00000000ULL) >> 8U)  | \
                  ((((uint64_t)(a)) & 0x00000000FF000000ULL) << 8U)  | \
                  ((((uint64_t)(a)) & 0x0000000000FF0000ULL) << 24U) | \
                  ((((uint64_t)(a)) & 0x000000000000FF00ULL) << 40U) | \
                  ((((uint64_t)(a)) & 0x00000000000000FFULL) << 56U))
#endif
                  
#ifndef htons                  
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htons(a) ntohs(a)
#endif

#ifndef htonl
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htonl(a) ntohl(a)
#endif

#ifndef htonll
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htonll(a) ntohll(a)
#endif

#elif (BIG == MACHINE_ENDIAN)
#ifndef ntohs    
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohs(a) (a)
#endif 

#ifndef ntohl
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohl(a) (a)
#endif

#ifndef htons
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htons(a) (a)
#endif

#ifndef htonl
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htonl(a) (a)
#endif
#else
    #error "Setup MACHINE_ENDIAN! Options: BIG or LITTLE."
#endif
/*==================================================================================================
                                             ENUMS
==================================================================================================*/
/* Definition of the platform specific types */
#if (!defined PLATFORMTYPES_H)
typedef signed char sint8_t;
typedef signed short sint16_t;
typedef signed int sint32_t;
typedef unsigned char boolean;
#endif
/* Needed by the PTPDRV for timestamps storage and calculations */
typedef signed long long sint64_t;
typedef int int_t;
typedef char char_t;
typedef unsigned int uint_t;
typedef double float64_t;
typedef unsigned int uaddr_t;
/*==================================================================================================
                                 GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/
    
/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

#endif /* COMMON_TYPES_H */
