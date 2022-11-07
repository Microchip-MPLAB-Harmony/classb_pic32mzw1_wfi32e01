/*******************************************************************************
  Class B Library v1.0.0 Release

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
#include "classb/classb_clock_test.h"
#include "definitions.h"
/*----------------------------------------------------------------------------
 *     Constants
 *----------------------------------------------------------------------------*/
#define CLASSB_CLOCK_MAX_CLOCK_FREQ         (200000000U)
#define CLASSB_CLOCK_MAX_SYSTICK_VAL        (0xffffffffU)
#define CLASSB_CLOCK_MAX_TMR1_PERIOD_VAL        (654U)
#define CLASSB_CLOCK_TMR1_CLK_FREQ           (32768U)
#define CLASSB_CLOCK_MAX_TEST_ACCURACY      (5U)
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
    TSYNC = 1
    TCS = 1
    */
    //T1CONSET = 0x206; //Prescaler - 1:1, External clock LPRC, Synchronization mode enabled
    
    /*
    SIDL = 0
    TWDIS = 0
    TECS = 0
    TGATE = 0
    TCKPS = 0
    TSYNC = 1
    TCS = 1
    */
    T1CONSET = 0x6; //Prescaler - 1:1, External clock SOSC, Synchronization mode enabled

    /* Clear counter */
    TMR1 = 0x0;

    /*Set period */
    PR1 = 162; // 5 Ms

    /* Setup TMR1 Interrupt */
    //TMR1_InterruptEnable();  /* Enable interrupt on the way out */
    
    /* Wait for SOSC ready */
    while(!(CLKSTAT & 0x00000014)) ;
}

/*============================================================================
void _CLASSB_CLock_EVIC_SourceEnable( INT_SOURCE source )
------------------------------------------------------------------------------
Purpose: Enable EVIC source for CPU clock self-test
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
void _CLASSB_CLock_EVIC_SourceEnable( INT_SOURCE source )
{
    volatile uint32_t *IECx = (volatile uint32_t *) (&IEC0 + ((0x10 * (source / 32)) / 4));
    volatile uint32_t *IECxSET = (volatile uint32_t *)(IECx + 2);

    *IECxSET = 1 << (source & 0x1f);
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
    _CLASSB_CLock_EVIC_SourceEnable(INT_SOURCE_TIMER_1);
}

/*============================================================================
static void DelayMs ( uint32_t delay_ms)
------------------------------------------------------------------------------
Purpose: This function is used for introducing delay during clock initialization
Input  : delay in ms.
Output : None.
============================================================================*/
static void DelayMs ( uint32_t delay_ms)
{
    uint32_t startCount, endCount;
    /* Calculate the end count for the given delay */
    endCount=(CORE_TIMER_FREQ/1000)*delay_ms;
    startCount=_CP0_GET_COUNT();
    while((_CP0_GET_COUNT()-startCount)<endCount);
}

/*============================================================================
static void _CLASSB_Wifi_Spi_Write(unsigned int spi_addr, unsigned int data)
------------------------------------------------------------------------------
Purpose: This function is used for clock initialization
Input  : None.
Output : None.
============================================================================*/
static void _CLASSB_Wifi_Spi_Write(unsigned int spi_addr, unsigned int data)
{
    unsigned int  addr_bit, data_bit, bit_idx;
    unsigned int cs_high, clk_high, en_bit_bang;
    unsigned int *wifi_spi_ctrl_reg = (unsigned int *)0xBF8C8028;
    clk_high = 0x1 ;
    cs_high  = 0x2;
    en_bit_bang  = 0x1 << 31;
    addr_bit = 0; data_bit = 0;

    *wifi_spi_ctrl_reg = en_bit_bang | cs_high ;
    *wifi_spi_ctrl_reg = (en_bit_bang | cs_high | clk_high );
     *wifi_spi_ctrl_reg = (en_bit_bang);
     *wifi_spi_ctrl_reg = (en_bit_bang | clk_high);

    for (bit_idx=0;bit_idx<=7;bit_idx++) {
        addr_bit = (spi_addr>>(7-bit_idx)) & 0x1;
        *wifi_spi_ctrl_reg = (en_bit_bang | (addr_bit << 2 ));               // Falling edge of clk
        *wifi_spi_ctrl_reg = (en_bit_bang | (addr_bit << 2 ) | clk_high);    // Rising edge of clk
    }

    for (bit_idx=0;bit_idx<=15;bit_idx++) {
        data_bit = (data>>(15-bit_idx)) & 0x1;
        *wifi_spi_ctrl_reg = (en_bit_bang | (data_bit << 2 ));                // Falling edge of clk with data bit
        *wifi_spi_ctrl_reg = (en_bit_bang | (data_bit << 2 ) | clk_high);     // Rising edge of clk
    }

    *wifi_spi_ctrl_reg = (en_bit_bang | cs_high | clk_high); // Rising edge of clk
    *wifi_spi_ctrl_reg = 0;                                // Set the RF override bit and CS_n high
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
    volatile unsigned int *PMDRCLR = (unsigned int *) 0xBF8000B4;
	//volatile unsigned int *RFSPICTL = (unsigned int *) 0xBF8C8028;
    /* unlock system for clock configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    
    switch(running_context)
    {
        
        case false :
        {
            if(((DEVID & 0x0FF00000) >> 20) == PIC32MZW1_B0)
            {
                CFGCON2  |= 0x300; // Start with POSC Turned OFF
                /* if POSC was on give some time for POSC to shut off */
                DelayMs(2);
                // Read counter part is there only for debug and testing, or else not needed, so use ifdef as needed
                _CLASSB_Wifi_Spi_Write(0x85, 0x00F0); /* MBIAS filter and A31 analog_test */ //if (wifi_spi_read (0x85) != 0xF0) {Error, Stop};
                _CLASSB_Wifi_Spi_Write(0x84, 0x0001); /* A31 Analog test */// if (wifi_spi_read (0x84) != 0x1) {Error, Stop};
                _CLASSB_Wifi_Spi_Write(0x1e, 0x510); /* MBIAS reference adjustment */ //if (wifi_spi_read (0x1e) != 0x510) {Error, Stop};
                _CLASSB_Wifi_Spi_Write(0x82, 0x6400); /* XTAL LDO feedback divider (1.3+v) */ //if (wifi_spi_read (0x82) != 0x6000) {Error, Stop};

                /* Enable POSC */
                CFGCON2  &= 0xFFFFFCFF; // enable POSC

                /* Wait for POSC ready */
                while(!(CLKSTAT & 0x00000004)) ;

                /*Configure SPLL*/
                CFGCON3 = 10;
                CFGCON0bits.SPLLHWMD = 1;

                /* SPLLCON = 0x01496869 */
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

                while( OSCCONbits.OSWEN );
                DelayMs(5);

                /* Power down the EWPLL */
                EWPLLCONbits.EWPLLPWDN = 1;

                /* Power down the UPLL */
                UPLLCONbits.UPLLPWDN = 1;

                /* Power down the BTPLL */
                BTPLLCONbits.BTPLLPWDN = 1;

                *(PMDRCLR)  = 0x1000;
            }
            
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
Purpose: Initialize EVIC register set
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
static void _CLASSB_EVIC_Initialize(void)
{
    INTCONSET = _INTCON_MVEC_MASK;
    
    /* Configure Shadow Register Set */
    PRISS = 0x76543210;
}

/*============================================================================
bool _CLASSB_Clock_EVIC_SourceStatusGet( INT_SOURCE source )
------------------------------------------------------------------------------
Purpose: Get EVIC source status
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
bool _CLASSB_Clock_EVIC_SourceStatusGet( INT_SOURCE source )
{
    volatile uint32_t *IFSx = (volatile uint32_t *)(&IFS0 + ((0x10 * (source / 32)) / 4));

    return (bool)((*IFSx >> (source & 0x1f)) & 0x1);
}

/*============================================================================
void _CLASSB_Clock_EVIC_SourceStatusClear( INT_SOURCE source )
------------------------------------------------------------------------------
Purpose: Clear EVIC source status
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
void _CLASSB_Clock_EVIC_SourceStatusClear( INT_SOURCE source )
{
    volatile uint32_t *IFSx = (volatile uint32_t *) (&IFS0 + ((0x10 * (source / 32)) / 4));
    volatile uint32_t *IFSxCLR = (volatile uint32_t *)(IFSx + 1);

    *IFSxCLR = 1 << (source & 0x1f);
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
    uint32_t clock_test_tmr1_cycles,
    bool running_context)
{
    
    CLASSB_TEST_STATUS clock_test_status = CLASSB_TEST_NOT_EXECUTED;
    uint64_t expected_ticks = (uint64_t)(((uint64_t)cpu_clock_freq / CLASSB_CLOCK_TMR1_CLK_FREQ) * clock_test_tmr1_cycles);
    volatile uint32_t systick_count_a = 0U;
    volatile uint32_t systick_count_b = 0U;
    int32_t ticks_passed = 0;
    uint8_t calculated_error_limit = 0U;
    
    if (running_context == true)
    {
        _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_CLOCK,
            CLASSB_TEST_NOT_EXECUTED);
    }
    else
    {
        _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_CLOCK,
            CLASSB_TEST_NOT_EXECUTED);
    }
    
    if ((clock_test_tmr1_cycles > CLASSB_CLOCK_MAX_TMR1_PERIOD_VAL)
        ||(expected_ticks > CLASSB_CLOCK_MAX_SYSTICK_VAL)
        || (cpu_clock_freq > CLASSB_CLOCK_MAX_CLOCK_FREQ)
        || (error_limit < CLASSB_CLOCK_MAX_TEST_ACCURACY))
    {
        ;
    }
    else
    {
        /*Below initialization required for TMR1 module and system clock to operate properly */
        __builtin_disable_interrupts();
        _CLASSB_Clock_CLK_Initialize(running_context);
        _CLASSB_EVIC_Initialize();
        __builtin_enable_interrupts();
        
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
		
        while(!_CLASSB_Clock_EVIC_SourceStatusGet(INT_SOURCE_TIMER_1));
        _CLASSB_Clock_EVIC_SourceStatusClear(INT_SOURCE_TIMER_1);
		
        systick_count_a = _CLASSB_Clock_SysTickGetVal();
        while(!_CLASSB_Clock_EVIC_SourceStatusGet(INT_SOURCE_TIMER_1))
        {
            ;
        }
        
        systick_count_b = _CLASSB_Clock_SysTickGetVal();
        
        /*Core timer increments at half the system clock frequency (SYSCLK).*/
        expected_ticks = expected_ticks/2 ;
        ticks_passed = (systick_count_b - systick_count_a);


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
