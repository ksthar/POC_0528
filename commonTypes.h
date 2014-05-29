/*------------------------------------------------------------------------------
 * Copyright(c)2014 Allegion PLC as an unpublished work.
 *
 * The information contained herein is confidential property of Allegion PLC.
 * All rights reserved.  Reproduction, adaptation, or translation without the
 * express written consent of Allegion PLC is prohibited, except as allowed
 * under the copyright laws.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * File Name: commonTypes.h
 * Purpose of File: This file contains common naming conventions used for data
 *                  types and macros.These should be used by octopus source
 *                  code.
 *----------------------------------------------------------------------------*/

#ifndef COMMONTYPES_H_
#define COMMONTYPES_H_

/*------- HEADER INCLUDE FILES -----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*------- MACRO DEFINITIONS --------------------------------------------------*/

// General numeric constants
#define ZERO    (0)
#define ONE     (1)

// General return status values
#define STATUS_SUCCESS    (0)
#define STATUS_FAILURE    (-1)

/*------- TYPEDEFS -----------------------------------------------------------*/
typedef char sInt8_t;
typedef unsigned char uInt8_t;
typedef int16_t sInt16_t;
typedef uint16_t uInt16_t;
typedef int32_t sInt32_t;
typedef uint32_t uInt32_t;
typedef int64_t sInt64_t;
typedef uint64_t uInt64_t;

/*------- FUNCTION PROTOTYPES ------------------------------------------------*/
// Common functions used by octopus should be declared here

#endif /* COMMONTYPES_H_ */

