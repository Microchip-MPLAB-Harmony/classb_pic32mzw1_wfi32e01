/*******************************************************************************
  Class B Library ${REL_VER} Release

  Company:
    Microchip Technology Inc.

  File Name:
    classb_clock_test.c

  Summary:
    Class B Library CPU clock frequency self-test source file

  Description:
    This file provides CPU clock frequency self-test.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) ${REL_YEAR} Microchip Technology Inc. and its subsidiaries.
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
#include "classb/classb_clock_test.h"
#include "definitions.h"
/*----------------------------------------------------------------------------
 *     Constants
 *----------------------------------------------------------------------------*/
#define CLASSB_CLOCK_MAX_CLOCK_FREQ         (${CLASSB_CPU_MAX_CLOCK}U)
#define CLASSB_CLOCK_MAX_SYSTICK_VAL        (0x${CLASSB_SYSTICK_MAXCOUNT}U)
#define CLASSB_CLOCK_TMR1_CLK_FREQ           (${CLASSB_TMR1_EXPECTED_CLOCK}U)
#define CLASSB_CLOCK_MAX_TEST_ACCURACY      (${CLASSB_CPU_CLOCK_TEST_ACCUR}U)
/* Since no floating point is used for clock test, multiply intermediate
 * values with 128.
 */
#define CLASSB_CLOCK_MUL_FACTOR             (128U)

/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *     Functions
 *----------------------------------------------------------------------------*/
extern void _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE test_type,
    CLASSB_TEST_ID test_id, CLASSB_TEST_STATUS value);

/*============================================================================
static uint32_t _CLASSB_Clock_SysTickGetVal(void)
------------------------------------------------------------------------------
Purpose: Reads the VAL register of the SysTick
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
static uint32_t _CLASSB_Clock_SysTickGetVal ( void )
{
	uint32_t count;
    count = _CP0_GET_COUNT();
    return count;
}

/*============================================================================
static void _CLASSB_Clock_SysTickStart(void)
------------------------------------------------------------------------------
Purpose: Configure the SysTick for clock self-test
Input  : None.
Output : None.
Notes  : If SysTick is used by the application, ensure that it
         is reconfigured after the clock self test.
============================================================================*/
static void _CLASSB_Clock_SysTickStart ( void )
{
	// Disable Timer by setting Disable Count (DC) bit
    _CP0_SET_CAUSE(_CP0_GET_CAUSE() | _CP0_CAUSE_DC_MASK);
    // Disable Interrupt
    IEC0CLR=0x1;
    // Clear Core Timer
    _CP0_SET_COUNT(0);
    _CP0_SET_COMPARE(0xFFFFFFFF);
    // Enable Timer by clearing Disable Count (DC) bit
    _CP0_SET_CAUSE(_CP0_GET_CAUSE() & (~_CP0_CAUSE_DC_MASK));
    
}

/*============================================================================
static void _CLASSB_Clock_TMR1_Enable(void)
------------------------------------------------------------------------------
Purpose: Enables the TMR1
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
static void _CLASSB_Clock_TMR1_Enable ( void )
{
    T1CONSET = _T1CON_ON_MASK;
}

/*============================================================================
static void _CLASSB_Clock_TMR1_Init(void)
------------------------------------------------------------------------------
Purpose: Configure TMR1 peripheral
Input  : None.
Output : None.
Notes  : This self-test configures TMR1 to use an external
         32.768kHz Crystal as reference clock. Do not use this self-test
         if the external crystal is not available.
============================================================================*/
static void _CLASSB_Clock_TMR1_Init(void)
{
    /* Disable Timer */
    T1CONCLR = _T1CON_ON_MASK;

    /*
    SIDL = 0
    TWDIS = 0
    TECS = 2
    TGATE = 0
    TCKPS = 0
    TSYNC = 0
    TCS = 1
    */
    T1CONSET = 0x202;

    /* Clear counter */
    TMR1 = 0x0;

    /*Set period */
    PR1 = 162;

    /* Setup TMR1 Interrupt */
    //TMR1_InterruptEnable();  /* Enable interrupt on the way out */
}

/*============================================================================
static void _CLASSB_Clock_TMR1_Period_Set(void)
------------------------------------------------------------------------------
Purpose: Configure TMR1 peripheral for CPU clock self-test
Input  : None.
Output : None.
Notes  : The clocks required for TMR1 are configured in a separate function.
============================================================================*/
static void _CLASSB_Clock_TMR1_Period_Set(uint32_t period)
{
    PR1 = period;
    EVIC_SourceEnable(INT_SOURCE_TIMER_1);
}

/*============================================================================
static void _CLASSB_Clock_CLK_Initialize(void)
------------------------------------------------------------------------------
Purpose: Configure clock for CPU clock self-test
Input  : None.
Output : None.
============================================================================*/
static void _CLASSB_Clock_CLK_Initialize(bool running_context)
{
    /* unlock system for clock configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    switch(running_context)
    {
        
        case false :
        {
            OSCCONbits.FRCDIV = 0;

            /* SPLLBSWSEL   = 5   */
            /* SPLLPWDN     = PLL_ON     */
            /* SPLLPOSTDIV1 = 4 */
            /* SPLLFLOCK    = NO_ASSERT    */
            /* SPLLRST      = NO_ASSERT      */
            /* SPLLFBDIV    = 20  */
            /* SPLLREFDIV   = 1   */
            /* SPLLICLK     = POSC     */
            /* SPLL_BYP     = NO_BYPASS     */
            SPLLCON = 0x414045;

            /* Power down the UPLL */
            UPLLCONbits.UPLLPWDN = 1;

            /* Power down the EWPLL */
            EWPLLCONbits.EWPLLPWDN = 1;

            /* Power down the BTPLL */
            BTPLLCONbits.BTPLLPWDN = 1;

            /* ETHPLLPOSTDIV2 = a */
            /* SPLLPOSTDIV2   = 0 */
            /* BTPLLPOSTDIV2  = 0 */
            CFGCON3 = 10;

            /* OSWEN    = SWITCH_COMPLETE    */
            /* SOSCEN   = OFF   */
            /* UFRCEN   = USBCLK   */
            /* CF       = NO_FAILDET       */
            /* SLPEN    = IDLE    */
            /* CLKLOCK  = UNLOCKED  */
            /* NOSC     = SPLL     */
            /* WAKE2SPD = SELECTED_CLK */
            /* DRMEN    = NO_EFFECT    */
            /* FRCDIV   = OSC_FRC_DIV_1   */
            OSCCON = 0x100;

            OSCCONSET = _OSCCON_OSWEN_MASK;  /* request oscillator switch to occur */

            Nop();
            Nop();

            while( OSCCONbits.OSWEN );        /* wait for indication of successful clock change before proceeding */

            /* Peripheral Module Disable Configuration */

            PMD1 = 0x25818981;
            PMD2 = 0x7e0f0f;
            PMD3 = 0x19031316;
            
            break;
        }
        case true:
        {
            PMD2bits.T1MD = 0;
        }
    }
    /* Lock system since done with clock configuration */
    SYSKEY = 0x33333333;
}

/*============================================================================
static void _CLASSB_EVIC_Initialize(void)
------------------------------------------------------------------------------
Purpose: Configure EVIC settings for TMR1
Input  : None.
Output : None.
============================================================================*/
static void _CLASSB_EVIC_Initialize(void)
{
    INTCONSET = _INTCON_MVEC_MASK;
    
    /* Configure Shadow Register Set */
    PRISS = 0x76543210;
}

/*============================================================================
CLASSB_TEST_STATUS CLASSB_ClockTest(uint32_t cpu_clock_freq,
    uint8_t error_limit,
    uint8_t clock_test_tmr1_cycles,
    bool running_context);
------------------------------------------------------------------------------
Purpose: Check whether CPU clock frequency is within acceptable limits.
Input  : Expected CPU clock frequency value, acceptable error percentage,
         test duration (in TMR1 cycles) and running context.
Output : Test status.
Notes  : None.
============================================================================*/


CLASSB_TEST_STATUS CLASSB_ClockTest(uint32_t cpu_clock_freq,
    uint8_t error_limit,
    uint16_t clock_test_tmr1_cycles,
    bool running_context)
{
    
    CLASSB_TEST_STATUS clock_test_status = CLASSB_TEST_NOT_EXECUTED;
    int64_t expected_ticks = ((cpu_clock_freq / CLASSB_CLOCK_TMR1_CLK_FREQ) * clock_test_tmr1_cycles);
    volatile uint32_t systick_count_a = 0;
    volatile uint32_t systick_count_b = 0;
    int64_t ticks_passed = 0;
    uint8_t calculated_error_limit = 0;
    /*Below initialization required for TMR1 module and system clock to operate properly */
    __builtin_disable_interrupts();
    _CLASSB_Clock_CLK_Initialize(running_context);
    _CLASSB_EVIC_Initialize();
    __builtin_enable_interrupts();

    if ((expected_ticks > CLASSB_CLOCK_MAX_SYSTICK_VAL)
        || (cpu_clock_freq > CLASSB_CLOCK_MAX_CLOCK_FREQ)
        || (error_limit < CLASSB_CLOCK_MAX_TEST_ACCURACY))
    {
        ;
    }
    else
    {
        if (running_context == true)
        {
            _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_CLOCK,
                CLASSB_TEST_INPROGRESS);
        }
        else
        {
            _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_CLOCK,
                CLASSB_TEST_INPROGRESS);
        }

        _CLASSB_Clock_TMR1_Init();
        _CLASSB_Clock_TMR1_Period_Set(clock_test_tmr1_cycles);
        _CLASSB_Clock_SysTickStart();
        _CLASSB_Clock_TMR1_Enable();
		
        while(!EVIC_SourceStatusGet(INT_SOURCE_TIMER_1));
        EVIC_SourceStatusClear(INT_SOURCE_TIMER_1);
		
        systick_count_a = _CLASSB_Clock_SysTickGetVal();
        while(!EVIC_SourceStatusGet(INT_SOURCE_TIMER_1))
        {
            ;
        }
        
        systick_count_b = _CLASSB_Clock_SysTickGetVal();

        expected_ticks = expected_ticks * CLASSB_CLOCK_MUL_FACTOR;
        
        /*Core timer increments at half the system clock frequency (SYSCLK).*/
        expected_ticks = expected_ticks/2 ;
        ticks_passed = (systick_count_b - systick_count_a) * CLASSB_CLOCK_MUL_FACTOR;


        if (ticks_passed < expected_ticks)
        {
            // The CPU clock is slower than expected
            calculated_error_limit = ((((expected_ticks - ticks_passed) * CLASSB_CLOCK_MUL_FACTOR)/ (expected_ticks)) * 100) / CLASSB_CLOCK_MUL_FACTOR;
        }
        else
        {
            // The CPU clock is faster than expected
            calculated_error_limit = ((((ticks_passed - expected_ticks) * CLASSB_CLOCK_MUL_FACTOR)/ (expected_ticks)) * 100) / CLASSB_CLOCK_MUL_FACTOR;
        }

        if (error_limit > calculated_error_limit)
        {
            clock_test_status = CLASSB_TEST_PASSED;
            if (running_context == true)
            {
                _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_CLOCK,
                    CLASSB_TEST_PASSED);
            }
            else
            {
                _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_CLOCK,
                    CLASSB_TEST_PASSED);
            }
        }
        else
        {
            clock_test_status = CLASSB_TEST_FAILED;
            if (running_context == true)
            {
                _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_CLOCK,
                    CLASSB_TEST_FAILED);
            }
            else
            {
                _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_CLOCK,
                    CLASSB_TEST_FAILED);
            }
            CLASSB_SelfTest_FailSafe(CLASSB_TEST_CLOCK);
        }
    }

    return clock_test_status;
}
