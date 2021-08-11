/*******************************************************************************
  Class B Library v2.0.0 Release

  Company:
    Microchip Technology Inc.

  File Name:
    classb.c

  Summary:
    Class B Library main source file

  Description:
    This file provides general functions for the Class B library.

 *******************************************************************************/

/*******************************************************************************
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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
#include "classb.h"
#include "device.h"
/*----------------------------------------------------------------------------
 *     Constants
 *----------------------------------------------------------------------------*/
 


#define FLASH_START_ADDR 0x90000000
#define FLASH_SIZE       0x100000

#define SRAM_BASE_ADDRESS                   (0xA0000000)
#define CLASSB_RESULT_ADDR                  (0x${CLASSB_SRAM_START_ADDRESS}U)
#define CLASSB_COMPL_RESULT_ADDR            (0x${CLASSB_SRAM_START_MSB + "04"}U)
#define CLASSB_ONGOING_TEST_VAR_ADDR        (0x${CLASSB_SRAM_START_MSB + "08"}U)
#define CLASSB_TEST_IN_PROG_VAR_ADDR        (0x${CLASSB_SRAM_START_MSB + "0c"}U)
#define CLASSB_WDT_TEST_IN_PROG_VAR_ADDR    (0x${CLASSB_SRAM_START_MSB + "10"}U)
#define CLASSB_FLASH_TEST_VAR_ADDR          (0x${CLASSB_SRAM_START_MSB + "14"}U)
#define CLASSB_INTERRUPT_TEST_VAR_ADDR      (0x${CLASSB_SRAM_START_MSB + "18"}U)
#define CLASSB_INTERRUPT_COUNT_VAR_ADDR     (0x${CLASSB_SRAM_START_MSB + "1c"}U)
#define CLASSB_SRAM_STARTUP_TEST_SIZE       (${CLASSB_SRAM_MARCH_SIZE}U)
#define CLASSB_CLOCK_ERROR_PERCENT          (${CLASSB_CLOCK_TEST_PERCENT}U)
<#if CLASSB_FLASH_CRC_CONF?has_content>
    <#if CLASSB_FLASH_CRC_CONF == true>
#define CLASSB_FLASH_CRC32_ADDR             (0x${CLASSB_FLASHCRC_ADDR}U)
    </#if>
</#if>
#define CLASSB_CLOCK_TEST_RTC_CYCLES        (200U)
// RTC is clocked from 32768 Hz Crystal. One RTC cycle is 30517 nano sec
#define CLASSB_CLOCK_TEST_TMR1_RATIO_NS      (30517U)
#define CLASSB_CLOCK_TEST_RATIO_NS_MS       (1000000U)
#define CLASSB_CLOCK_DEFAULT_CLOCK_FREQ     (200000000U)
#define CLASSB_INVALID_TEST_ID              (0xFFU)

typedef enum {
    RESET_REASON_NONE = 0x00000000,
    RESET_REASON_POWERON = 0x00000003,
    RESET_REASON_BROWNOUT = 0x00000002,
    RESET_REASON_WDT_TIMEOUT = 0x00000010,
    RESET_REASON_DMT_TIMEOUT = 0x00000020,
    RESET_REASON_SOFTWARE = 0x00000040,
    RESET_REASON_MCLR = 0x00000080,
    RESET_REASON_CONFIG_MISMATCH = 0x00000200,
    RESET_REASON_VBAT = 0x00010000,
    RESET_REASON_VBPOR = 0x00020000,
    RESET_REASON_PORIO = 0x80000000,
    RESET_REASON_PORCORE = 0x40000000,
    RESET_REASON_ALL = 0xC00302F5

} RESET_REASON;
RESET_REASON resetReason;


/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/
volatile uint8_t * ongoing_sst_id;
volatile uint8_t * classb_test_in_progress;
volatile uint8_t * wdt_test_in_progress;
volatile uint8_t * interrupt_tests_status;
volatile uint32_t * interrupt_count;

/*----------------------------------------------------------------------------
 *     Functions
 *----------------------------------------------------------------------------*/

/*============================================================================
void Classb_SystemReset(void)
------------------------------------------------------------------------------
Purpose: For software reset.
Input  : None
Output : None
Notes  : None
============================================================================*/

static void Classb_SystemReset(void) {
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    RSWRSTSET = _RSWRST_SWRST_MASK;

    RSWRST;

    Nop();
    Nop();
    Nop();
    Nop();
}

/*============================================================================
void CLASSB_WDT_Clear(void)
------------------------------------------------------------------------------
Purpose: For WDT clear.
Input  : None
Output : None
Notes  : None
============================================================================*/
void CLASSB_WDT_Clear( void )
{
    /* Writing specific value to only upper 16 bits of WDTCON register clears WDT counter */
    /* Only write to the upper 16 bits of the register when clearing. */
    /* WDTCLRKEY = 0x5743 */
    volatile uint16_t * wdtclrkey;
    wdtclrkey = ( (volatile uint16_t *)&WDTCON ) + 1;
    *wdtclrkey = 0x5743;
}

/*============================================================================
void CLASSB_WDT_Config(void)
------------------------------------------------------------------------------
Purpose: For configuring WDT.
Input  : None
Output : None
Notes  : None
============================================================================*/
void CLASSB_WDT_Config( void )
{
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    
    CFGCON2bits.WDTPS = 11; //2 sec
    
    SYSKEY = 0x33333333;
     Nop();
    Nop();
    Nop();
    Nop();
}
/*============================================================================
void CLASSB_SelfTest_FailSafe(CLASSB_TEST_ID cb_test_id)
------------------------------------------------------------------------------
Purpose: Called if a non-critical self-test is failed.
Input  : The ID of the failed test.
Output : None
Notes  : The application decides the contents of this function. This function
         should perform failsafe operation after checking the 'cb_test_id'.
         This function must not return.
============================================================================*/

void CLASSB_SelfTest_FailSafe(CLASSB_TEST_ID cb_test_id) {
#if (defined(__DEBUG) || defined(__DEBUG_D)) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    // Infinite loop
    while (1) {
        ;
    }
}

/*============================================================================
static void CLASSB_GlobalsInit(void)
------------------------------------------------------------------------------
Purpose: Initialization of global variables for the classb library.
Input  : None
Output : None
Notes  : This function is called before C startup code
============================================================================*/

static void CLASSB_GlobalsInit(void) {
    /* Initialize persistent pointers
     * These variables point to address' in the reserved SRAM for the
     * Class B library.
     */
    ongoing_sst_id = (volatile uint8_t *)CLASSB_ONGOING_TEST_VAR_ADDR;
    classb_test_in_progress = (volatile uint8_t *)CLASSB_TEST_IN_PROG_VAR_ADDR;
    wdt_test_in_progress = (volatile uint8_t *)CLASSB_WDT_TEST_IN_PROG_VAR_ADDR;
    interrupt_tests_status = (volatile uint8_t *)CLASSB_INTERRUPT_TEST_VAR_ADDR;
    interrupt_count = (volatile uint32_t *)CLASSB_INTERRUPT_COUNT_VAR_ADDR;

    // Initialize variables
    *ongoing_sst_id = CLASSB_INVALID_TEST_ID;
    *classb_test_in_progress = CLASSB_TEST_NOT_STARTED;
    *wdt_test_in_progress = CLASSB_TEST_NOT_STARTED;
    *interrupt_tests_status = CLASSB_TEST_NOT_STARTED;
    
}

/*============================================================================
static void CLASSB_App_WDT_Recovery(void)
------------------------------------------------------------------------------
Purpose: Called if a WDT reset is caused by the application
Input  : None
Output : None
Notes  : The application decides the contents of this function.
============================================================================*/


static void CLASSB_App_WDT_Recovery(void) {
#if (defined(__DEBUG) || defined(__DEBUG_D)) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    // Infinite loop
    while (1) {
        ;
    }
}


/*============================================================================
static void CLASSB_SST_WDT_Recovery(void)
------------------------------------------------------------------------------
Purpose: Called after WDT reset, to indicate that a Class B function is stuck.
Input  : None
Output : None
Notes  : The application decides the contents of this function.
============================================================================*/
void CLASSB_SST_WDT_Recovery(void) {
#if (defined(__DEBUG) || defined(__DEBUG_D)) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    // Infinite loop
    while (1) {
        ;
    }
}

/*============================================================================
static void CLASSB_TestWDT(void)
------------------------------------------------------------------------------
Purpose: Function to check WDT after a device reset.
Input  : None
Output : None
Notes  : None
============================================================================*/

static void CLASSB_TestWDT(void) {
    /* This persistent flag is checked after reset */
    *wdt_test_in_progress = CLASSB_TEST_STARTED;

    /* If WDT ALWAYSON is set, wait till WDT resets the device */
    if (DEVCFG2bits.WDTEN == 1) {
        // Infinite loop
        while (1) {
            ;
        }
    } else {
        // If WDT is not enabled, enable WDT and wait
        if (WDTCONbits.ON == 0) {
            // Configure timeout
            WDTCONbits.ON = 1;
            // Infinite loop
            while (1) {
                ;
            }
        } else {
            // Infinite loop
            while (1) {
                ;
            }
        }
    }
}

inline RESET_REASON SYS_RESET_ReasonGet(void) {
    return (RESET_REASON) (RCON & (_RCON_CMR_MASK | _RCON_EXTR_MASK |
            _RCON_SWR_MASK | _RCON_DMTO_MASK | _RCON_WDTO_MASK |
            _RCON_BOR_MASK | _RCON_POR_MASK | _RCON_VBPOR_MASK |
            _RCON_VBAT_MASK | _RCON_PORIO_MASK | _RCON_PORCORE_MASK));
}

inline void SYS_RESET_ReasonClear(RESET_REASON reason) {
    RCONCLR = reason;
}
/*============================================================================
static CLASSB_INIT_STATUS CLASSB_Init(void)
------------------------------------------------------------------------------
Purpose: To check reset cause and decide the startup flow.
Input  : None
Output : None
Notes  : This function is executed on every device reset. This shall be
         called right after the reset, before any other initialization is
         performed.
============================================================================*/

static CLASSB_INIT_STATUS CLASSB_Init(void) {
    /* Initialize persistent pointers
     * These variables point to address' in the reserved SRAM for the
     * Class B library.
     */
    ongoing_sst_id = (volatile uint8_t *)CLASSB_ONGOING_TEST_VAR_ADDR;
    classb_test_in_progress = (volatile uint8_t *)CLASSB_TEST_IN_PROG_VAR_ADDR;
    wdt_test_in_progress = (volatile uint8_t *)CLASSB_WDT_TEST_IN_PROG_VAR_ADDR;
    interrupt_tests_status = (volatile uint8_t *)CLASSB_INTERRUPT_TEST_VAR_ADDR;
    interrupt_count = (volatile uint32_t *)CLASSB_INTERRUPT_COUNT_VAR_ADDR;

    CLASSB_INIT_STATUS ret_val = CLASSB_SST_NOT_DONE;

    resetReason = SYS_RESET_ReasonGet();
    SYS_RESET_ReasonClear(RESET_REASON_ALL);
    
    /*Check if reset was triggered by WDT */
    if ((resetReason & RESET_REASON_WDT_TIMEOUT) == RESET_REASON_WDT_TIMEOUT) {

        if (*wdt_test_in_progress == CLASSB_TEST_STARTED) {
            *wdt_test_in_progress = CLASSB_TEST_NOT_STARTED;
        } else if (*classb_test_in_progress == CLASSB_TEST_STARTED) {
            CLASSB_SST_WDT_Recovery();
        } else {
            CLASSB_App_WDT_Recovery();
        }
    } else {
        /* If it is a software reset and the Class B library has issued it */
        if ((*classb_test_in_progress == CLASSB_TEST_STARTED) &&
                ((resetReason & RESET_REASON_SOFTWARE) == RESET_REASON_SOFTWARE)) {
            *classb_test_in_progress = CLASSB_TEST_NOT_STARTED;
            ret_val = CLASSB_SST_DONE;
        } else {

            /* For all other reset causes,
             * test the reserved SRAM,
             * initialize Class B variables
             * clear the test results and test WDT
             */
            bool result_area_test_ok = false;
            bool ram_buffer_test_ok = false;
            // Test the reserved SRAM
            result_area_test_ok = CLASSB_RAMMarchC((uint32_t *) SRAM_BASE_ADDRESS, CLASSB_SRAM_TEST_BUFFER_SIZE);
            ram_buffer_test_ok = CLASSB_RAMMarchC((uint32_t *) SRAM_BASE_ADDRESS + CLASSB_SRAM_TEST_BUFFER_SIZE, CLASSB_SRAM_TEST_BUFFER_SIZE);
            if ((result_area_test_ok == true) && (ram_buffer_test_ok == true)) {

                // Initialize all Class B variables after the March test
                CLASSB_GlobalsInit();
                CLASSB_ClearTestResults(CLASSB_TEST_TYPE_SST);
                CLASSB_ClearTestResults(CLASSB_TEST_TYPE_RST);
                // Perform WDT test
                CLASSB_TestWDT();
            } else {
                while (1) {
                    ;
                }
            }
        }
    }

    return ret_val;
}

/*============================================================================
static CLASSB_STARTUP_STATUS CLASSB_Startup_Tests(void)
------------------------------------------------------------------------------
Purpose: Call all startup self-tests.
Input  : None
Output : None
Notes  : This function calls all the configured self-tests during startup.
         The MPLAB Harmony Configurator (MHC) has options to configure
         the startup self-tests. If startup tests are not enabled via MHC,
         this function enables the WDT and returns CLASSB_STARTUP_TEST_NOT_EXECUTED.
============================================================================*/


static CLASSB_STARTUP_STATUS CLASSB_Startup_Tests(void) {
    CLASSB_STARTUP_STATUS cb_startup_status = CLASSB_STARTUP_TEST_NOT_EXECUTED;
    <#if (CLASSB_CPU_TEST_OPT?? && CLASSB_CPU_TEST_OPT == true) ||
         (CLASSB_FPU_OPT?? && CLASSB_FPU_OPT == true) ||
         (CLASSB_SRAM_TEST_OPT?? && CLASSB_SRAM_TEST_OPT == true) ||
         (CLASSB_FLASH_CRC_CONF?? && CLASSB_FLASH_CRC_CONF == true) ||
         (CLASSB_CLOCK_TEST_OPT?? && CLASSB_CLOCK_TEST_OPT == true) ||
         (CLASSB_INTERRUPT_TEST_OPT?? && CLASSB_INTERRUPT_TEST_OPT == true)>
    CLASSB_STARTUP_STATUS cb_temp_startup_status = CLASSB_STARTUP_TEST_NOT_EXECUTED;
    CLASSB_TEST_STATUS cb_test_status = CLASSB_TEST_NOT_EXECUTED;
    </#if>
    <#if CLASSB_CLOCK_TEST_OPT??>
        <#if CLASSB_CLOCK_TEST_OPT == true>
            <#if CLASSB_CLOCK_TEST_DURATION?has_content>
            <#lt>    uint16_t clock_test_tmr1_cycles = ((${CLASSB_CLOCK_TEST_DURATION} * CLASSB_CLOCK_TEST_RATIO_NS_MS) / CLASSB_CLOCK_TEST_TMR1_RATIO_NS);
            </#if>
        </#if>
    </#if>

    //Enable watchdog if it is not enabled
        if (WDTCONbits.ON == 0) {
            // Configure timeout
            WDTCONbits.ON = 1;
        }
    <#if CLASSB_CPU_TEST_OPT?? && CLASSB_CPU_TEST_OPT == true>
        // CPU reg test
        *ongoing_sst_id = CLASSB_TEST_CPU;
        cb_test_status = CLASSB_CPU_RegistersTest(false);

        if (cb_test_status == CLASSB_TEST_PASSED) {
            cb_temp_startup_status = CLASSB_STARTUP_TEST_PASSED;
        } else if (cb_test_status == CLASSB_TEST_FAILED) {
            cb_temp_startup_status = CLASSB_STARTUP_TEST_FAILED;
        }
        CLASSB_WDT_Clear();
        
        // Program Counter test
        *ongoing_sst_id = CLASSB_TEST_PC;
        cb_test_status = CLASSB_CPU_PCTest(false);

        if (cb_test_status == CLASSB_TEST_PASSED) {
            cb_temp_startup_status = CLASSB_STARTUP_TEST_PASSED;
        } else if (cb_test_status == CLASSB_TEST_FAILED) {
            cb_temp_startup_status = CLASSB_STARTUP_TEST_FAILED;
        }
        CLASSB_WDT_Clear();    
    </#if>
    <#if CLASSB_SRAM_TEST_OPT??>
        <#if CLASSB_SRAM_TEST_OPT == true>
            <#lt>        //SRAM test
            <#lt>        *ongoing_sst_id = CLASSB_TEST_RAM;
            <#if CLASSB_SRAM_MARCH_ALGORITHM?has_content>
                <#lt>    cb_test_status = CLASSB_SRAM_MarchTestInit((uint32_t *)CLASSB_SRAM_APP_AREA_START,
                <#lt>        CLASSB_SRAM_STARTUP_TEST_SIZE, ${CLASSB_SRAM_MARCH_ALGORITHM}, false);
            <#else>
                <#lt>    cb_test_status = CLASSB_SRAM_MarchTestInit((uint32_t *)CLASSB_SRAM_APP_AREA_START,
                <#lt>    CLASSB_SRAM_STARTUP_TEST_SIZE, CLASSB_SRAM_MARCH_C, false);
            </#if>
            <#lt>        if (cb_test_status == CLASSB_TEST_PASSED) {

            <#lt>            cb_temp_startup_status = CLASSB_STARTUP_TEST_PASSED;
            <#lt>        } else if (cb_test_status == CLASSB_TEST_FAILED) {


            <#lt>            cb_temp_startup_status = CLASSB_STARTUP_TEST_FAILED;
            <#lt>        }
            <#lt>        CLASSB_WDT_Clear();
        </#if>
    </#if>

    <#if CLASSB_CLOCK_TEST_OPT??>
        <#if CLASSB_CLOCK_TEST_OPT == true>
            
            <#lt>        // Clock Test
            <#lt>        *ongoing_sst_id = CLASSB_TEST_CLOCK;
            <#lt>        cb_test_status = CLASSB_ClockTest(CLASSB_CLOCK_DEFAULT_CLOCK_FREQ, CLASSB_CLOCK_ERROR_PERCENT, clock_test_tmr1_cycles, false);
            <#lt>        if (cb_test_status == CLASSB_TEST_PASSED)
            <#lt>        {
            <#lt>            cb_temp_startup_status = CLASSB_STARTUP_TEST_PASSED;
            <#lt>        }
            <#lt>        else if (cb_test_status == CLASSB_TEST_FAILED)
            <#lt>        {

            <#lt>            cb_temp_startup_status = CLASSB_STARTUP_TEST_FAILED;
            <#lt>        }
            <#lt>        CLASSB_WDT_Clear();
        </#if>
    </#if>
    
    <#if CLASSB_FLASH_CRC_CONF?has_content>
        <#if CLASSB_FLASH_CRC_CONF == true>
            
            <#lt>    // Flash Test
            <#lt>    *ongoing_sst_id = CLASSB_TEST_FLASH;
            <#lt>    cb_test_status = CLASSB_FlashCRCTest(FLASH_START_ADDR, FLASH_SIZE,*(uint32_t *)CLASSB_FLASH_CRC32_ADDR, false);
            <#lt>    if (cb_test_status == CLASSB_TEST_PASSED)
            <#lt>    {

            <#lt>        cb_temp_startup_status = CLASSB_STARTUP_TEST_PASSED;
            <#lt>    }
            <#lt>    else if (cb_test_status == CLASSB_TEST_FAILED)
            <#lt>    {

            <#lt>        cb_temp_startup_status = CLASSB_STARTUP_TEST_FAILED;
            <#lt>    }
            <#lt>    CLASSB_WDT_Clear();
        </#if>
    </#if>    
    
    <#if CLASSB_INTERRUPT_TEST_OPT??>
        <#if CLASSB_INTERRUPT_TEST_OPT == true>

            <#lt>    // Interrupt Test
            <#lt>    *ongoing_sst_id = CLASSB_TEST_INTERRUPT;
            <#lt>    cb_test_status = CLASSB_SST_InterruptTest();
            <#lt>    if (cb_test_status == CLASSB_TEST_PASSED)
            <#lt>    {
            <#lt>        cb_temp_startup_status = CLASSB_STARTUP_TEST_PASSED;
            <#lt>    }
            <#lt>    else if (cb_test_status == CLASSB_TEST_FAILED)
            <#lt>    {
            <#lt>        cb_temp_startup_status = CLASSB_STARTUP_TEST_FAILED;
            <#lt>    }
            <#lt>    CLASSB_WDT_Clear();
        </#if>
    </#if>


    <#if (CLASSB_CPU_TEST_OPT?? && CLASSB_CPU_TEST_OPT == true) ||
         (CLASSB_FPU_OPT?? && CLASSB_FPU_OPT == true) ||
         (CLASSB_SRAM_TEST_OPT?? && CLASSB_SRAM_TEST_OPT == true) ||
         (CLASSB_FLASH_CRC_CONF?? && CLASSB_FLASH_CRC_CONF == true) ||
         (CLASSB_CLOCK_TEST_OPT?? && CLASSB_CLOCK_TEST_OPT == true) ||
         (CLASSB_INTERRUPT_TEST_OPT?? && CLASSB_INTERRUPT_TEST_OPT == true)>
    if (cb_temp_startup_status == CLASSB_STARTUP_TEST_PASSED) {
        cb_startup_status = CLASSB_STARTUP_TEST_PASSED;
    } else {
        cb_startup_status = CLASSB_STARTUP_TEST_FAILED;
    }
    </#if>
    return cb_startup_status;
}

/*============================================================================
void _on_reset(void)
------------------------------------------------------------------------------
Purpose: Handle reset causes and perform start-up self-tests.
Input  : None
Output : None
Notes  : This function is called from Reset_Handler.
============================================================================*/


void _on_bootstrap(void) {
    
    CLASSB_WDT_Config();
    
    CLASSB_STARTUP_STATUS startup_tests_status = CLASSB_STARTUP_TEST_FAILED;
    
    CLASSB_INIT_STATUS init_status = CLASSB_Init();

    if (init_status == CLASSB_SST_NOT_DONE) {

        *classb_test_in_progress = CLASSB_TEST_STARTED;
        // Run all startup self-tests
        startup_tests_status = CLASSB_Startup_Tests();

        if (startup_tests_status == CLASSB_STARTUP_TEST_PASSED) {
            // Reset the device if all tests are passed.
            Classb_SystemReset();
        } else if (startup_tests_status == CLASSB_STARTUP_TEST_FAILED) {
#if (defined(__DEBUG) || defined(__DEBUG_D)) && defined(__XC32)
            __builtin_software_breakpoint();
#endif
            
            // Infinite loop
            
            while (1) {
                ;
            }
            
        } else {
            // If startup tests are not enabled via MHC, do nothing.
            ;
        }
    } else if (init_status == CLASSB_SST_DONE) {
        // Clear flags
        *classb_test_in_progress = CLASSB_TEST_NOT_STARTED;
    }
}
