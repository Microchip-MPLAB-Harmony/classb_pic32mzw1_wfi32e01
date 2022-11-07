/*******************************************************************************
  Class B Library ${REL_VER} Release

  Company:
    Microchip Technology Inc.

  File Name:
    classb_io_pin_test.h

  Summary:
    Header file for I/O pin self-tests

  Description:
    This file provides function prototypes, macros and datatypes for the
    I/O pin test.

*******************************************************************************/

/*******************************************************************************
Copyright (c) ${REL_YEAR} released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS  WITHOUT  WARRANTY  OF  ANY  KIND,
EITHER EXPRESS  OR  IMPLIED,  INCLUDING  WITHOUT  LIMITATION,  ANY  WARRANTY  OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A  PARTICULAR  PURPOSE.
IN NO EVENT SHALL MICROCHIP OR  ITS  LICENSORS  BE  LIABLE  OR  OBLIGATED  UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,  BREACH  OF  WARRANTY,  OR
OTHER LEGAL  EQUITABLE  THEORY  ANY  DIRECT  OR  INDIRECT  DAMAGES  OR  EXPENSES
INCLUDING BUT NOT LIMITED TO ANY  INCIDENTAL,  SPECIAL,  INDIRECT,  PUNITIVE  OR
CONSEQUENTIAL DAMAGES, LOST  PROFITS  OR  LOST  DATA,  COST  OF  PROCUREMENT  OF
SUBSTITUTE  GOODS,  TECHNOLOGY,  SERVICES,  OR  ANY  CLAIMS  BY  THIRD   PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE  THEREOF),  OR  OTHER  SIMILAR  COSTS.
*******************************************************************************/

#ifndef CLASSB_IO_PIN_TEST_H
#define CLASSB_IO_PIN_TEST_H

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

/*----------------------------------------------------------------------------
 *     Data types
 *----------------------------------------------------------------------------*/
// *****************************************************************************
/* PORT index definitions

  Summary:
    PORT index definitions for Class B library I/O pin test

  Description:
    This can be used in the I/O pin test.

  Remarks:
    None.
*/
typedef enum classb_port_index
{
    CLASSB_GPIO_PORT_A = 0U,
    CLASSB_GPIO_PORT_B = 1U,
    CLASSB_GPIO_PORT_C = 2U,
    CLASSB_GPIO_PORT_K = 3U,
} CLASSB_PORT_INDEX;

// *****************************************************************************
/* PIN definitions

  Summary:
    PIN definitions for Class B library I/O pin test

  Description:
    This can be used in the I/O pin test.

  Remarks:
    None.
*/
typedef enum classb_port_pin
{
    PIN0 = 0U,
    PIN1 = 1U,
    PIN2 = 2U,
    PIN3 = 3U,
    PIN4 = 4U,
    PIN5 = 5U,
    PIN6 = 6U,
    PIN7 = 7U,
    PIN8 = 8U,
    PIN9 = 9U,
    PIN10 = 10U,
    PIN11 = 11U,
    PIN12 = 12U,
    PIN13 = 13U,
    PIN14 = 14U,
    PIN15 = 15U,
    PIN16 = 16U,
    PIN17 = 17U,
    PIN18 = 18U,
    PIN19 = 19U,
    PIN20 = 20U,
    PIN21 = 21U,
    PIN22 = 22U,
    PIN23 = 23U,
    PIN24 = 24U,
    PIN25 = 25U,
    PIN26 = 26U,
    PIN27 = 27U,
    PIN28 = 28U,
    PIN29 = 29U,
    PIN30 = 30U,
    PIN31 = 31U
} CLASSB_PORT_PIN;

// *****************************************************************************
/* PORT pin state

  Summary:
    PORT pin state

  Description:
    This can be used in the I/O pin test.

  Remarks:
    None.
*/
typedef enum classb_port_pin_state
{
    PORT_PIN_LOW  = 0U,
    PORT_PIN_HIGH = 1U,
    PORT_PIN_INVALID = 2U
} CLASSB_PORT_PIN_STATE;

/*----------------------------------------------------------------------------
 *     Functions
 *----------------------------------------------------------------------------*/
CLASSB_TEST_STATUS CLASSB_RST_IOTest(CLASSB_PORT_INDEX port, CLASSB_PORT_PIN pin,
    CLASSB_PORT_PIN_STATE state);
void CLASSB_IO_InputSamplingEnable(CLASSB_PORT_INDEX port, CLASSB_PORT_PIN pin);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // CLASSB_IO_PIN_TEST_H
