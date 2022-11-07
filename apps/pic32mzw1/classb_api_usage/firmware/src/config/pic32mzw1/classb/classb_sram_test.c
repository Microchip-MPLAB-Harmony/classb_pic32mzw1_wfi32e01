/*******************************************************************************
  Class B Library v1.0.0 Release

  Company:
    Microchip Technology Inc.

  File Name:
    classb_sram_test.c

  Summary:
    Class B Library SRAM self-test source file

  Description:
    This file provides SRAM self-test function.

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
#include "classb/classb_sram_test.h"
#include "classb/classb_sram_algorithm.h"

/*----------------------------------------------------------------------------
 *     Constants
 *----------------------------------------------------------------------------*/
#define CLASSB_SRAM_FINAL_WORD_ADDRESS      (0xa003fffcU)
#define CLASSB_SRAM_BUFF_START_ADDRESS      (0x80000200U)
#define CLASSB_SRAM_TEMP_STACK_ADDRESS      (0x80000100U)
#define CLASSB_SRAM_ALL_32BITS_HIGH         (0xFFFFFFFFU)

/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/
extern uint32_t _heap;
extern uint32_t _min_heap_size;
extern uint32_t _stack;

/*----------------------------------------------------------------------------
 *     Functions
 *----------------------------------------------------------------------------*/
extern void _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE test_type,
    CLASSB_TEST_ID test_id, CLASSB_TEST_STATUS value);

volatile register unsigned char* regstackptr asm("sp");  // Stack register variable.
/*============================================================================
static uint32_t _CLASSB_GetStackPointer(void)
------------------------------------------------------------------------------
Purpose: Get the address stored in the Stack Pointer.
Input  : None
Output : Address stored in the Stack Pointer register.
Notes  : This function is used by SRAM tests.
============================================================================*/
static uint32_t _CLASSB_GetStackPointer(void)
{
    unsigned char* result = (unsigned char*)regstackptr;
    return (uint32_t)result;
}

/*============================================================================
static void _CLASSB_SetStackPointer(uint32_t new_stack_address)
------------------------------------------------------------------------------
Purpose: Store a new address into the Stack Pointer.
Input  : New address for the Stack.
Output : None
Notes  : This function is used by SRAM tests.
============================================================================*/
static void _CLASSB_SetStackPointer(uint32_t new_stack_address)
{
    //__ASM volatile ("MSR msp, %0\n" : : "r" (new_stack_address));
    regstackptr = (unsigned char *)(new_stack_address) ;
}

/*============================================================================
static void _CLASSB_MemCopy(uint32_t* dest, uint32_t* src, uint32_t size_in_bytes)
------------------------------------------------------------------------------
Purpose: Copies given number of bytes, from one SRAM area to the other.
Input  : Destination address, source address and size
Output : None
Notes  : This function is used by SRAM tests.
============================================================================*/
static void OPTIMIZE_O0 _CLASSB_MemCopy(uint32_t* dest, uint32_t* src, uint32_t size_in_bytes)
{
    uint32_t i = 0u;
    uint32_t size_in_words = (uint32_t) (size_in_bytes / 4);

    for (i = 0; i < size_in_words; i++)
    {
        dest[i] = src[i];
    }
}

volatile unsigned char* sram_stack_addr __attribute__((persistent));



/*============================================================================
CLASSB_TEST_STATUS _CLASSB_SRAM_MarchTest(uint32_t * start_addr,
    uint32_t test_size_bytes, CLASSB_SRAM_MARCH_ALGO march_algo,
    bool running_context)
------------------------------------------------------------------------------
Purpose: Perform March-tests on SRAM.
Input  : Start address, size of SRAM area to be tested,
         selected algorithm and the context (startup or run-time)
Output : Test status.
Notes  : None.
============================================================================*/
CLASSB_TEST_STATUS _CLASSB_SRAM_MarchTest(uint32_t * start_addr,
    uint32_t test_size_bytes, CLASSB_SRAM_MARCH_ALGO march_algo,
    bool running_context)
{
    CLASSB_TEST_STATUS classb_sram_status = CLASSB_TEST_NOT_EXECUTED;
    bool march_test_retval = false;
    // Use a local variable for calculations
    uint32_t mem_start_address = (uint32_t)start_addr;
    // Test will be done on blocks on 512 bytes
    volatile uint32_t march_c_iterations = (uint32_t) (test_size_bytes / CLASSB_SRAM_TEST_BUFFER_SIZE);
    // If the size is not a multiple of 512, then check the remaining area
    volatile uint32_t march_c_short_itr_size = (uint32_t) (test_size_bytes % CLASSB_SRAM_TEST_BUFFER_SIZE);
    // Variable for loops
    int32_t i = 0;
    uint32_t * iteration_start_addr = 0U;
    uint32_t itr_start_addr = 0U;

    for (i = 0; i < march_c_iterations; i++)
    {
        itr_start_addr = (uint32_t) mem_start_address + (i * CLASSB_SRAM_TEST_BUFFER_SIZE);
        iteration_start_addr = (uint32_t *) itr_start_addr;
        // Copy the tested area
        _CLASSB_MemCopy((uint32_t *)CLASSB_SRAM_BUFF_START_ADDRESS,
            iteration_start_addr, CLASSB_SRAM_TEST_BUFFER_SIZE);
        // Run the selected RAM March algorithm
        if (march_algo == CLASSB_SRAM_MARCH_C)
        {
            march_test_retval = CLASSB_RAMMarchC(iteration_start_addr,
                CLASSB_SRAM_TEST_BUFFER_SIZE);
        }
        else if (march_algo == CLASSB_SRAM_MARCH_C_MINUS)
        {
            march_test_retval = CLASSB_RAMMarchCMinus(iteration_start_addr,
                CLASSB_SRAM_TEST_BUFFER_SIZE);
        }
        else if (march_algo == CLASSB_SRAM_MARCH_B)
        {
            march_test_retval = CLASSB_RAMMarchB(iteration_start_addr,
                CLASSB_SRAM_TEST_BUFFER_SIZE);
        }
        if (march_test_retval == false)
        {
            // If March test fails, exit the loop
            classb_sram_status = CLASSB_TEST_FAILED;
            break;
        }
        else
        {
        // Restore the tested area
        _CLASSB_MemCopy(iteration_start_addr, (uint32_t *)CLASSB_SRAM_BUFF_START_ADDRESS,
            CLASSB_SRAM_TEST_BUFFER_SIZE);
        }
        classb_sram_status = CLASSB_TEST_PASSED;
    }

    // If the tested area is not a multiple of 512 bytes
    if ((march_c_short_itr_size > 0) && (march_test_retval == true))
    {
        iteration_start_addr = (uint32_t *)(mem_start_address + (march_c_iterations * CLASSB_SRAM_TEST_BUFFER_SIZE));
        _CLASSB_MemCopy((uint32_t *)CLASSB_SRAM_BUFF_START_ADDRESS,
            iteration_start_addr, march_c_short_itr_size);
        // Run the selected RAM March algorithm
        if (march_algo == CLASSB_SRAM_MARCH_C)
        {
            march_test_retval = CLASSB_RAMMarchC(iteration_start_addr,
                CLASSB_SRAM_TEST_BUFFER_SIZE);
        }
        else if (march_algo == CLASSB_SRAM_MARCH_C_MINUS)
        {
            march_test_retval = CLASSB_RAMMarchCMinus(iteration_start_addr,
                CLASSB_SRAM_TEST_BUFFER_SIZE);
        }
        else if (march_algo == CLASSB_SRAM_MARCH_B)
        {
            march_test_retval = CLASSB_RAMMarchB(iteration_start_addr,
                CLASSB_SRAM_TEST_BUFFER_SIZE);
        }
        if (march_test_retval == false)
        {
            classb_sram_status = CLASSB_TEST_FAILED;
        }
        else
        {
            classb_sram_status = CLASSB_TEST_PASSED;
            // Restore the tested area
            _CLASSB_MemCopy(iteration_start_addr,
                (uint32_t *)CLASSB_SRAM_BUFF_START_ADDRESS, march_c_short_itr_size);
        }
    }

    return classb_sram_status;
}

/*============================================================================
CLASSB_TEST_STATUS CLASSB_SRAM_MarchTestInit(uint32_t * start_addr,
    uint32_t test_size_bytes, CLASSB_SRAM_MARCH_ALGO march_algo,
    bool running_context)
------------------------------------------------------------------------------
Purpose: Initialize to perform March-tests on SRAM.
Input  : Start address, size of SRAM area to be tested,
         selected algorithm and the context (startup or run-time)
Output : Test status.
Notes  : This function uses register variables since the stack
         in SRAM also need to be tested.
============================================================================*/

CLASSB_TEST_STATUS CLASSB_SRAM_MarchTestInit(uint32_t * start_addr,
    uint32_t test_size_bytes, CLASSB_SRAM_MARCH_ALGO march_algo,
    bool running_context)
{
    /* This function uses register variables since the Stack also
     * need to be tested
     */
    register uint32_t march_test_end_address asm("t0");
    register uint32_t mem_start_address  asm("t1");
    register uint32_t stack_address asm("t3");
    register CLASSB_TEST_STATUS sram_init_retval asm("t4");
    // Find the last word address in the tested area
    march_test_end_address = (uint32_t)start_addr +
        test_size_bytes - 4U;
    mem_start_address = (uint32_t)start_addr;
    stack_address = 0U;
    sram_init_retval = CLASSB_TEST_NOT_EXECUTED;

    if (running_context == true)
    {
        _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_RAM,
            CLASSB_TEST_NOT_EXECUTED);
    }
    else
    {
        _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_RAM,
            CLASSB_TEST_NOT_EXECUTED);
    }
    /* The address and test size must be a multiple of 4
     * The tested area should be above the reserved SRAM for Class B library
     * Address should be within the last SRAM word address.
     */

    if ((((uint32_t)start_addr % 4) != 0U)
            || ((test_size_bytes % 4) != 0U)
            || (march_test_end_address > CLASSB_SRAM_FINAL_WORD_ADDRESS)
            || (mem_start_address < CLASSB_SRAM_APP_AREA_START))
    {
        ;
    }
    else
    {
        // Move stack pointer to the reserved area before any SRAM test
        stack_address = _CLASSB_GetStackPointer();
        _CLASSB_SetStackPointer(CLASSB_SRAM_TEMP_STACK_ADDRESS);
        sram_stack_addr = regstackptr;

        if (running_context == true)
        {
            _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_RAM,
                CLASSB_TEST_INPROGRESS);
        }
        else
        {
            _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_RAM,
                CLASSB_TEST_INPROGRESS);
        }
       
        sram_init_retval = _CLASSB_SRAM_MarchTest(start_addr, test_size_bytes,march_algo, running_context);

        if (sram_init_retval == CLASSB_TEST_PASSED)
        {
            if (running_context == true)
            {
                _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_RAM,
                    CLASSB_TEST_PASSED);
            }
            else
            {
                _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_RAM,
                    CLASSB_TEST_PASSED);
            }
        }
        else if (sram_init_retval == CLASSB_TEST_FAILED)
        {
            if (running_context == true)
            {
                _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_RAM,
                    CLASSB_TEST_FAILED);
            }
            else
            {
                _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_RAM,
                    CLASSB_TEST_FAILED);
            }
            
            CLASSB_SelfTest_FailSafe(CLASSB_TEST_RAM);
        }
        
        _CLASSB_SetStackPointer(stack_address);
        

    }

    return sram_init_retval;
}