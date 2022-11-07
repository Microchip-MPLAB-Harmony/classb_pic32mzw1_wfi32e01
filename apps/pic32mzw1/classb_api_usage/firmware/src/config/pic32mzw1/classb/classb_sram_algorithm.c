/*******************************************************************************
  Class B Library v1.0.0 Release

  Company:
    Microchip Technology Inc.

  File Name:
    classb_sram_test.c

  Summary:
    Source file for SRAM self-tests internal routines.

  Description:
    This file provides internal functions for the SRAM test.

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

/*----------------------------------------------------------------------------
 *     include files
 *----------------------------------------------------------------------------*/
#include "classb/classb_sram_algorithm.h"

/*----------------------------------------------------------------------------
 *     Constants
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *     Functions
 *----------------------------------------------------------------------------*/

/*============================================================================
bool _CLASSB_ReadZeroWriteOne(uint32_t * ptr) 
------------------------------------------------------------------------------
Purpose: Read 0 and write 1 to each bits of the SRAM location
Input  : Address of the memory location
Output : Success or failure
Notes  : This function is used by SRAM tests.
         It checks bit by bit.
         Opmization level of this function must be -O0.
============================================================================*/
bool _CLASSB_ReadZeroWriteOne(uint32_t * ptr)
{
    uint32_t ram_data = 0U;
    int8_t bit_pos = 0;
    bool return_val = true;

    for (bit_pos = CLASSB_SRAM_MARCH_BIT_WIDTH - 1; bit_pos >= 0; bit_pos--)
    {
        ram_data =(((*ptr) >> bit_pos) & 1);
        if (ram_data != 0U) 
        {
            return_val = false;
            break;
        } 

        // Write one at the bit position
        *ptr = (*ptr | (1 << bit_pos));
    }

    return return_val;  
}

/*============================================================================
bool _CLASSB_ReadZeroWriteOneWriteZero(uint32_t * ptr) 
------------------------------------------------------------------------------
Purpose: Read 0, write 1 and write 0 to each bits of the SRAM location
Input  : Address of the memory location
Output : Success or failure
Notes  : This function is used by SRAM tests.
         It checks bit by bit.
         Opmization level of this function must be -O0.
============================================================================*/
bool _CLASSB_ReadZeroWriteOneWriteZero(uint32_t * ptr)
{
    uint32_t ram_data = 0U;
    int8_t bit_pos = 0;
    bool return_val = true;

    for (bit_pos = CLASSB_SRAM_MARCH_BIT_WIDTH - 1; bit_pos >= 0; bit_pos--)
    {
        ram_data =(((*ptr) >> bit_pos) & 1);
        if (ram_data != 0U) 
        {
            return_val = false;
            break;
        } 

        // Write one at the bit position
        *ptr = (*ptr | (1 << bit_pos));
        // Write zero at the bit position
        ram_data = *ptr  & (~(1 << bit_pos));
        *ptr = ram_data;
    }

    return return_val;  
}

/*============================================================================
bool _CLASSB_ReadOneWriteZero(uint32_t * ptr) 
------------------------------------------------------------------------------
Purpose: Read 1 and write 0 to each bits of the SRAM location
Input  : Address of the memory location
Output : Success or failure
Notes  : This function is used by SRAM tests.
         It checks bit by bit.
         Opmization level of this function must be -O0.
============================================================================*/
bool _CLASSB_ReadOneWriteZero(uint32_t * ptr) 
{
    uint32_t ram_data = 0U;
    int8_t bit_pos = 0;
    bool return_val = true;

    for (bit_pos = 0; bit_pos < CLASSB_SRAM_MARCH_BIT_WIDTH; bit_pos++)
    {
        ram_data = (((*ptr) >> bit_pos) & 1);

        if (ram_data != 1U) 
        {
            return_val = false;
            break;
        }
        
        // Write zero at the bit position
        ram_data = *ptr  & (~(1 << bit_pos));
        *ptr = ram_data;     
    }

    return return_val; 
}

/*============================================================================
bool _CLASSB_ReadOneWriteZeroWriteOne(uint32_t * ptr) 
------------------------------------------------------------------------------
Purpose: Read 1, write 1 and 0 to each bits of the SRAM location
Input  : Address of the memory location
Output : Success or failure
Notes  : This function is used by SRAM tests.
         It checks bit by bit.
         Opmization level of this function must be -O0.
============================================================================*/
bool _CLASSB_ReadOneWriteZeroWriteOne(uint32_t * ptr) 
{
    uint32_t ram_data = 0U;
    int8_t bit_pos = 0;
    bool return_val = true;

    for (bit_pos = 0; bit_pos < CLASSB_SRAM_MARCH_BIT_WIDTH; bit_pos++)
    {
        ram_data = (((*ptr) >> bit_pos) & 1);

        if (ram_data != 1U) 
        {
            return_val = false;
            break;
        }
        
        // Write zero at the bit position
        ram_data = *ptr  & (~(1 << bit_pos));
        *ptr = ram_data;
        // Write one at the bit position
        *ptr = (*ptr | (1 << bit_pos));
    }

    return return_val; 
}

/*============================================================================
bool _CLASSB_WriteOneWriteZero(uint32_t * ptr) 
------------------------------------------------------------------------------
Purpose: Write 1 and 0 to each bits of the SRAM location
Input  : Address of the memory location
Output : Success or failure
Notes  : This function is used by SRAM tests.
         It checks bit by bit.
         Opmization level of this function must be -O0.
============================================================================*/
bool _CLASSB_WriteOneWriteZero(uint32_t * ptr) 
{
    uint32_t ram_data = 0U;
    int8_t bit_pos = 0;
    bool return_val = true;

    for (bit_pos = 0; bit_pos < CLASSB_SRAM_MARCH_BIT_WIDTH; bit_pos++)
    {
        // Write one at the bit position
        *ptr = (*ptr | (1 << bit_pos));
        // Write zero at the bit position
        ram_data = *ptr  & (~(1 << bit_pos));
        *ptr = ram_data;
    }

    return return_val; 
}

/*============================================================================
bool _CLASSB_ReadZero(uint32_t * ptr) 
------------------------------------------------------------------------------
Purpose: Check whether all bits of a memory location in SRAM are 0s
Input  : Address of the memory location
Output : Success or failure
Notes  : This function is used by SRAM tests.
         It checks bit by bit.
         Opmization level of this function must be -O0.
============================================================================*/
bool _CLASSB_ReadZero(uint32_t * ptr) 
{
    uint32_t ram_data = 0U;
    int8_t bit_pos = 0;
    bool return_val = true;

    for (bit_pos = 0; bit_pos < CLASSB_SRAM_MARCH_BIT_WIDTH; bit_pos++)
    {
        ram_data = (((*ptr) >> bit_pos) & 1);

        if (ram_data != 0U) 
        {
            return_val = false;
            break;
        }
    }
    
    return return_val; 
}

/*============================================================================
bool CLASSB_RAMMarchC(uint32_t * start_addr, uint32_t test_size_bytes)
------------------------------------------------------------------------------
Purpose: Runs March C algorithm on the given SRAM area
Input  : Start address and size
Output : Success or failure
Notes  : This function is used by SRAM tests. It performs the following,
        // March C
        // Low to high, write zero
        // Low to high, read zero write one
        // Low to high, read one write zero
        // Low to high, read zero

        // High to low, read zero write one
        // High to low, read one write zero
        // High to low, read zero
============================================================================*/
bool CLASSB_RAMMarchC(uint32_t * start_addr, uint32_t test_size_bytes)
{
    bool sram_march_c_result = true;
    int32_t i = 0;
    uint32_t test_size_words = (uint32_t) (test_size_bytes / 4);

    /* Test size is limited to CLASSB_SRAM_TEST_BUFFER_SIZE,
     * start_addr need to be word aligned
     */
    if ((test_size_bytes > CLASSB_SRAM_TEST_BUFFER_SIZE)
            || (((uint32_t)start_addr % 4) != 0U))
    {
        sram_march_c_result = false;
    }

    // Perform the next check only if the previous stage is passed
    if (sram_march_c_result == true)
    {
        // Low to high, write zero
        for (i = 0; i < test_size_words; i++)
        {
            start_addr[i] = 0;
        }
        // Low to high, read zero write one
        for (i = 0; i < test_size_words; i++)
        {
            sram_march_c_result =  _CLASSB_ReadZeroWriteOne(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    if (sram_march_c_result == true)
    {
        // Low to high, read one write zero
        for (i = 0; i < test_size_words; i++)
        {
            sram_march_c_result =  _CLASSB_ReadOneWriteZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    if (sram_march_c_result == true)
    {
        // Low to high, read zero
        for (i = 0; i < test_size_words; i++)
        {
            sram_march_c_result =  _CLASSB_ReadZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    if (sram_march_c_result == true)
    {
        // High to low, read zero, write one
        for (i = (test_size_words - 1); i >= 0 ; i--)
        {
            sram_march_c_result =  _CLASSB_ReadZeroWriteOne(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    if (sram_march_c_result == true)
    {
        // High to low, read one, write zero
        for (i = (test_size_words - 1); i >= 0 ; i--)
        {
            sram_march_c_result =  _CLASSB_ReadOneWriteZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    if (sram_march_c_result == true)
    {
        // High to low, read zero
        for (i = (test_size_words - 1); i >= 0 ; i--)
        {
            sram_march_c_result =  _CLASSB_ReadZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    return sram_march_c_result;
}

/*============================================================================
bool CLASSB_RAMMarchCMinus(uint32_t * start_addr, uint32_t test_size_bytes)
------------------------------------------------------------------------------
Purpose: Runs March C algorithm on the given SRAM area
Input  : Start address and size
Output : Success or failure
Notes  : This function is used by SRAM tests. It performs the following,
        // March C minus
        // Low to high, write zero
        // Low to high, read zero write one
        // Low to high, read one write zero

        // High to low, read zero write one
        // High to low, read one write zero
        // High to low, read zero
============================================================================*/
bool CLASSB_RAMMarchCMinus(uint32_t * start_addr, uint32_t test_size_bytes)
{
    bool sram_march_c_result = true;
    int32_t i = 0;
    uint32_t test_size_words = (uint32_t) (test_size_bytes / 4);

    /* Test size is limited to CLASSB_SRAM_TEST_BUFFER_SIZE,
     * start_addr need to be word aligned
     */
    if ((test_size_bytes > CLASSB_SRAM_TEST_BUFFER_SIZE)
            || (((uint32_t)start_addr % 4) != 0U))
    {
        sram_march_c_result = false;
    }

    // Perform the next check only if the previous stage is passed
    if (sram_march_c_result == true)
    {
        // Low to high, write zero
        for (i = 0; i < test_size_words; i++)
        {
            start_addr[i] = 0;
        }
        // Low to high, read zero write one
        for (i = 0; i < test_size_words; i++)
        {
            sram_march_c_result =  _CLASSB_ReadZeroWriteOne(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    if (sram_march_c_result == true)
    {
        // Low to high, read one write zero
        for (i = 0; i < test_size_words; i++)
        {
            sram_march_c_result =  _CLASSB_ReadOneWriteZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }

    if (sram_march_c_result == true)
    {
        // High to low, read zero, write one
        for (i = (test_size_words - 1); i >= 0 ; i--)
        {
            sram_march_c_result =  _CLASSB_ReadZeroWriteOne(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    if (sram_march_c_result == true)
    {
        // High to low, read one, write zero
        for (i = (test_size_words - 1); i >= 0 ; i--)
        {
            sram_march_c_result =  _CLASSB_ReadOneWriteZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    if (sram_march_c_result == true)
    {
        // High to low, read zero
        for (i = (test_size_words - 1); i >= 0 ; i--)
        {
            sram_march_c_result =  _CLASSB_ReadZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }

    return sram_march_c_result;
}

/*============================================================================
bool CLASSB_RAMMarchB(uint32_t * start_addr, uint32_t test_size_bytes)
------------------------------------------------------------------------------
Purpose: Runs March C algorithm on the given SRAM area
Input  : Start address and size
Output : Success or failure
Notes  : This function is used by SRAM tests. It performs the following,
        // March B
        // Low to high, write zero
        // Low to high, read zero write one, read one write zero,
               read zero write one
        // Low to high, read one write zero, write one

        // High to low, read one write zero, write one write zero
        // High to low, read zero write one, write zero
============================================================================*/
bool CLASSB_RAMMarchB(uint32_t * start_addr, uint32_t test_size_bytes)
{
    bool sram_march_c_result = true;
    int32_t i = 0;
    uint32_t test_size_words = (uint32_t) (test_size_bytes / 4);

    /* Test size is limited to CLASSB_SRAM_TEST_BUFFER_SIZE,
     * start_addr need to be word aligned
     */
    if ((test_size_bytes > CLASSB_SRAM_TEST_BUFFER_SIZE)
            || (((uint32_t)start_addr % 4) != 0U))
    {
        sram_march_c_result = false;
    }

    // Perform the next check only if the previous stage is passed
    if (sram_march_c_result == true)
    {
        // Low to high, write zero
        for (i = 0; i < test_size_words; i++)
        {
            start_addr[i] = 0;
        }
        // Low to high
        for (i = 0; i < test_size_words; i++)
        {
            // Read zero write one
            sram_march_c_result =  _CLASSB_ReadZeroWriteOne(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
            // Read one write zero
            sram_march_c_result =  _CLASSB_ReadOneWriteZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
            // Read zero write one
            sram_march_c_result =  _CLASSB_ReadZeroWriteOne(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }

    if (sram_march_c_result == true)
    {
        // Low to high
        for (i = 0; i < test_size_words; i++)
        {
            // Read one, write zero, write one
            sram_march_c_result =  _CLASSB_ReadOneWriteZeroWriteOne(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }

    // High to low tests
    if (sram_march_c_result == true)
    {
        for (i = (test_size_words - 1); i >= 0 ; i--)
        {
            //High to low, read one, write zero
            sram_march_c_result =  _CLASSB_ReadOneWriteZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
            //High to low, write one, write zero
            sram_march_c_result =  _CLASSB_WriteOneWriteZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }
    
    if (sram_march_c_result == true)
    {
        // High to low, read zero, write one, write zero
        for (i = (test_size_words - 1); i >= 0 ; i--)
        {
            sram_march_c_result =  _CLASSB_ReadZeroWriteOneWriteZero(start_addr + i);
  
            if (sram_march_c_result == false)
            {
                break;
            }
        }
    }

    return sram_march_c_result;
}
