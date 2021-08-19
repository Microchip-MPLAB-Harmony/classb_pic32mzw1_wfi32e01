/*******************************************************************************
  Class B Library v1.0.0 Release

  Company:
    Microchip Technology Inc.

  File Name:
    classb_interrupt_test.c

  Summary:
    Class B Library source file for the Interrupt test

  Description:
    This file provides self-test functions for the Interrupt.

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
#include "classb/classb_interrupt_test.h"
#include "definitions.h"

/*----------------------------------------------------------------------------
 *     Constants
 *----------------------------------------------------------------------------*/


#define CLASSB_INTR_MAX_INT_COUNT           (15U)


/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/

extern volatile uint8_t * interrupt_tests_status;
extern volatile uint32_t * interrupt_count;
static uint32_t ebase_org;
static uint32_t off_org;

/*----------------------------------------------------------------------------
 *     Functions
 *----------------------------------------------------------------------------*/
extern void _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE test_type,
    CLASSB_TEST_ID test_id, CLASSB_TEST_STATUS value);



/*============================================================================
static void _CLASSB_TMR2_Handler(void)
------------------------------------------------------------------------------
Purpose: Custom handler used for Timer Interrupt test. It clears the interrupt
         flag and updates the interrupt count variable.
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
void  __attribute__((interrupt(IPL1SRS))) __attribute__((address(0x90002200),nomips16,nomicromips) ) _CLASSB_TMR2_Handler(void)
{
    /* Clear the status flag */
    IFS0CLR = _IFS0_T2IF_MASK;
    (*interrupt_count)++;
}

/*============================================================================
static void set_ebase(void)
------------------------------------------------------------------------------
Purpose: Setting value of Ebase and TMR2 OFF register   
Input  : None.
Output : None.
============================================================================*/
static void set_ebase(unsigned int value)
{
    /* unlock system */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    
    
    /*Set the CP0 registers for multi-vector interrupt */
    INTCONCLR = _INTCON_MVEC_MASK;// Set the MVEC bit
    
    unsigned int temp_CP0;// Temporary register for CP0 register storing 
    asm volatile("di");// Disable all interrupts 
    asm volatile("ehb");// Disable all interrupts 
    
    temp_CP0 = _CP0_GET_STATUS();// Get Status 
    temp_CP0 |= 0x00400000;// Set Bev 
    _CP0_SET_STATUS(temp_CP0);// Update Status
    asm volatile("ehb");
    
    _CP0_SET_INTCTL(0x10 << 5);
    asm volatile("ehb");
    
    /****************Setting new ebase and offset *****************/
    _CP0_SET_EBASE(value);// Set an EBase value
    asm volatile("ehb");
    OFF009 = 0x200;// setting new offset value
    /**********************************/
    
    temp_CP0 = _CP0_GET_CAUSE();// Get Cause 
    temp_CP0 |= 0x00800000;// Set IV 
    _CP0_SET_CAUSE(temp_CP0);// Update Cause
    asm volatile("ehb");
    
    temp_CP0 = _CP0_GET_STATUS();// Get Status 
    //temp_CP0 &= 0xFFBFFFFD;//  
    temp_CP0 &= 0xFFBFFFFF;//Clear Bev
    _CP0_SET_STATUS(temp_CP0);// Update Status
    asm volatile("ehb");
    
    INTCONSET = _INTCON_MVEC_MASK;// Set the MVEC bit
    /* Lock system  */
    SYSKEY = 0x33333333;
}

/*============================================================================
static void _CLASSB_BuildVectorTable(void)
------------------------------------------------------------------------------
Purpose: Build the vector table for Interrupt self-test . Internally it will call set_ebase function to set new ebase value and timer2 OFF register value
Input  : None.
Output : None.
============================================================================*/
static void _CLASSB_BuildVectorTable(void)
{
    ebase_org = _CP0_GET_EBASE();
    off_org = OFF009;
    __builtin_disable_interrupts();
    set_ebase( ((uint32_t)&_CLASSB_TMR2_Handler) - 0x200 ); 
}

/*============================================================================
static void set_ebase_org(void)
------------------------------------------------------------------------------
Purpose: Setting original value of Ebase and TMR2 OFF register   
Input  : None.
Output : None.
============================================================================*/
static void set_ebase_org(unsigned int value)
{
    /* unlock system */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    
    
    /*Set the CP0 registers for multi-vector interrupt */
    INTCONCLR = _INTCON_MVEC_MASK;// Set the MVEC bit
    
    unsigned int temp_CP0;// Temporary register for CP0 register storing 
    asm volatile("di");// Disable all interrupts 
    asm volatile("ehb");// Disable all interrupts 
    
    temp_CP0 = _CP0_GET_STATUS();// Get Status 
    temp_CP0 |= 0x00400000;// Set Bev 
    _CP0_SET_STATUS(temp_CP0);// Update Status
    asm volatile("ehb");
    
    _CP0_SET_INTCTL(0x10 << 5);
    asm volatile("ehb");
    
    /****************Setting new ebase and offset *****************/
    _CP0_SET_EBASE(value);// Set an EBase value
    asm volatile("ehb");
    OFF009 = off_org;// setting new offset value
    /**********************************/
    
    temp_CP0 = _CP0_GET_CAUSE();// Get Cause 
    temp_CP0 |= 0x00800000;// Set IV 
    _CP0_SET_CAUSE(temp_CP0);// Update Cause
    asm volatile("ehb");
    
    temp_CP0 = _CP0_GET_STATUS();// Get Status 
    //temp_CP0 &= 0xFFBFFFFD;//  
    temp_CP0 &= 0xFFBFFFFF;//Clear Bev
    _CP0_SET_STATUS(temp_CP0);// Update Status
    asm volatile("ehb");
    
    INTCONSET = _INTCON_MVEC_MASK;// Set the MVEC bit
    /* Lock system  */
    SYSKEY = 0x33333333;
}

/*============================================================================
static void _CLASSB_INT_CLK_Initialize(void)
------------------------------------------------------------------------------
Purpose: Configure clock for Interrupt self-test
Input  : None.
Output : None.
============================================================================*/
static void _CLASSB_INT_CLK_Initialize(void)
{
    /* unlock system for clock configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

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
    PMD2 = 0x7c0f0f;
    PMD3 = 0x19031316;

   
    /* Lock system since done with clock configuration */
    SYSKEY = 0x33333333;
}


/*============================================================================
static void _CLASSB_TMR1_Initialize(void)
------------------------------------------------------------------------------
Purpose: Configure TMR1 peripheral for Interrupt self-test
Input  : None.
Output : None.
Notes  : The TMR1 is reset after successfully performing the test.
============================================================================*/
void _CLASSB_TMR1_Initialize(void)
{
    /* Disable Timer */
    T1CONCLR = _T1CON_ON_MASK;

    /*
    SIDL = 0
    TWDIS = 0
    TECS = 2
    TGATE = 0
    TCKPS = 3
    TSYNC = 0
    TCS = 0
    */
    T1CONbits.TCS = 0;
    T1CONSET = 0x00;
    T1CONSET = 0x230;

    /* Clear counter */
    TMR1 = 0x0;

    /*Set period */
    PR1 = 39061;
}

/*============================================================================
static void Classb_EVIC_Initialize(void)
------------------------------------------------------------------------------
Purpose: Initializes the EVIC
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
void Classb_EVIC_Initialize( void )
{
    INTCONSET = _INTCON_MVEC_MASK;

    /* Set up priority and subpriority of enabled interrupts */
    IPC2SET = 0x400 | 0x0;  /* TIMER_2:  Priority 1 / Subpriority 0 */
    
    /* Configure Shadow Register Set */
    PRISS = 0x76543210;
}

/*============================================================================
static void _CLASSB_TMR2_Initialize(void)
------------------------------------------------------------------------------
Purpose: Configure TMR2 peripheral for Interrupt self-test
Input  : None.
Output : None.
Notes  : The TMR2 is reset after successfully performing the test.
============================================================================*/

void _CLASSB_TMR2_Initialize(void)
{
    /* Disable Timer */
    T2CONCLR = _T2CON_ON_MASK;

    /*
    SIDL = 0
    TCKPS =7
    T32   = 0
    TCS = 0
    */
    T2CONSET = 0x70;

    /* Clear counter */
    TMR2 = 0x0;

    /*Set period */
    PR2 = 3905U;

    /* Enable TMR Interrupt */
    IEC0SET = _IEC0_T2IE_MASK;

}


/*============================================================================
static void _CLASSB_INT_TMR1_Period_Set(void)
------------------------------------------------------------------------------
Purpose: Configure TMR1 peripheral for Interrupt self-test
Input  : None.
Output : None.
Notes  : The clocks required for TMR1 are configured in a separate function.
============================================================================*/
static void _CLASSB_INT_TMR1_Period_Set(uint32_t period)
{
    PR1 = period;
    EVIC_SourceEnable(INT_SOURCE_TIMER_1);
}


/*============================================================================
static void _CLASSB_INT_TMR1_Enable(void)
------------------------------------------------------------------------------
Purpose: Enables the TMR1
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
static void _CLASSB_INT_TMR1_Start ( void )
{
    T1CONSET = _T1CON_ON_MASK;
}

/*============================================================================
static void _CLASSB_INT_TMR2_Start(void)
------------------------------------------------------------------------------
Purpose: Enables the TMR2
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
static void _CLASSB_INT_TMR2_Start(void)
{
    T2CONSET = _T2CON_ON_MASK;
}

/*============================================================================
static void _CLASSB_INT_TMR2_Stop(void)
------------------------------------------------------------------------------
Purpose: Stops the TMR2
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
static void _CLASSB_INT_TMR2_Stop (void)
{
    T2CONCLR = _T2CON_ON_MASK;
}

/*============================================================================
CLASSB_TEST_STATUS CLASSB_SST_InterruptTest(void)
------------------------------------------------------------------------------
Purpose: Test interrupt generation and handling.
Input  : None.
Output : Test status.
Notes  : None.
============================================================================*/
CLASSB_TEST_STATUS CLASSB_SST_InterruptTest(void)
{
    
    CLASSB_TEST_STATUS intr_test_status = CLASSB_TEST_NOT_EXECUTED;
    EVIC_SourceStatusClear(INT_SOURCE_TIMER_1); 
    // Reset the counter
    *interrupt_count = 0;
    _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_INTERRUPT,
        CLASSB_TEST_INPROGRESS);
    

    _CLASSB_BuildVectorTable();
    __builtin_disable_interrupts();
    _CLASSB_INT_CLK_Initialize();
    _CLASSB_TMR2_Initialize();
    _CLASSB_TMR1_Initialize();
    Classb_EVIC_Initialize();
    
    /* Enable global interrupts */
    __builtin_enable_interrupts();
    
    _CLASSB_INT_TMR1_Period_Set(39061);
       
    _CLASSB_INT_TMR1_Start();
    _CLASSB_INT_TMR2_Start();
    
    while(!EVIC_SourceStatusGet(INT_SOURCE_TIMER_1));
    EVIC_SourceStatusClear(INT_SOURCE_TIMER_1);    
        
    _CLASSB_INT_TMR2_Stop();
    
    
    if ((*interrupt_count < CLASSB_INTR_MAX_INT_COUNT)
        &&  (*interrupt_count > 0))
    {
        T1CONCLR = _T1CON_ON_MASK;// stop timer1
        intr_test_status = CLASSB_TEST_PASSED;
        _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_INTERRUPT,
            CLASSB_TEST_PASSED);
        set_ebase_org(((uint32_t)ebase_org));
    }
    else
    {
        intr_test_status = CLASSB_TEST_FAILED;
        _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_INTERRUPT,
            CLASSB_TEST_FAILED);
        set_ebase_org(((uint32_t)ebase_org));
        // The failsafe function must not return.
        CLASSB_SelfTest_FailSafe(CLASSB_TEST_INTERRUPT);
    }
   return intr_test_status;
    
}
