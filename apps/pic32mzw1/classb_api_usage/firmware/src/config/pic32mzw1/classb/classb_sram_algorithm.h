/*******************************************************************************
  Class B Library v1.0.0 Release

  Company:
    Microchip Technology Inc.

  File Name:
    classb_sram_algorithm.h

  Summary:
    Header file for internal routines used in SRAM self-tests.

  Description:
    This file provides function prototypes, macros and data types for the
    internal routines used in SRAM self-tests SRAM tests.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef CLASSB_SRAM_ALGO_H
#define CLASSB_SRAM_ALGO_H

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

/*----------------------------------------------------------------------------
 *     Include files
 *----------------------------------------------------------------------------*/
#include "classb/classb_common.h"

/*----------------------------------------------------------------------------
 *     Constants
 *----------------------------------------------------------------------------*/
#define CLASSB_SRAM_ALL_32BITS_HIGH         (0xFFFFFFFFU)
#define CLASSB_SRAM_MARCH_BIT_WIDTH         (32U)
#define CLASSB_SRAM_TEST_BUFFER_SIZE        (512U) // Do not modify

/*----------------------------------------------------------------------------
 *     Data types
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *     Functions
 *----------------------------------------------------------------------------*/

/* RAM march algorithms
 * Optimization is set to zero, else the compiler optimizes these function away.
 */
bool OPTIMIZE_O0 CLASSB_RAMMarchC(uint32_t * start_addr, uint32_t test_size);
bool OPTIMIZE_O0 CLASSB_RAMMarchCMinus(uint32_t * start_addr, uint32_t test_size);
bool OPTIMIZE_O0 CLASSB_RAMMarchB(uint32_t * start_addr, uint32_t test_size);

bool OPTIMIZE_O0 _CLASSB_ReadZeroWriteOne(uint32_t * ptr);
bool OPTIMIZE_O0 _CLASSB_ReadZeroWriteOneWriteZero(uint32_t * ptr);
bool OPTIMIZE_O0 _CLASSB_ReadOneWriteZero(uint32_t * ptr);
bool OPTIMIZE_O0 _CLASSB_ReadOneWriteZeroWriteOne(uint32_t * ptr);
bool OPTIMIZE_O0 _CLASSB_WriteOneWriteZero(uint32_t * ptr);
bool OPTIMIZE_O0 _CLASSB_ReadZero(uint32_t * ptr );

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // CLASSB_SRAM_ALGO_H
