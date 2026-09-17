/**
 * @file iso14229_port.h
 * @brief Boot port wrapper for amalgamated iso14229 (same macros as FreeRTOS).
 */
#ifndef ISO14229_PORT_H
#define ISO14229_PORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <inttypes.h>

#ifndef UDS_ISOTP_MTU
#define UDS_ISOTP_MTU (512)
#endif

#ifndef UDS_CUSTOM_MILLIS
#define UDS_CUSTOM_MILLIS 1
#endif

#ifndef UDS_TP_ISOTP_C
#define UDS_TP_ISOTP_C 1
#endif

#include "iso14229.h"

#endif /* ISO14229_PORT_H */
