/*******************************************************************************
  Class B Library v2.0.1 Release

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
#include <time.h>
/*----------------------------------------------------------------------------
 *     Constants
 *----------------------------------------------------------------------------*/

#define decimaltobcd(x)                 (((x/10)<<4)+((x - ((x/10)*10))))
#define bcdtodecimal(x)                 ((x & 0xF0) >> 4) * 10 + (x & 0x0F)

#define CLASSB_INTR_DEVICE_VECT_OFFSET      (16U)
//#define CLASSB_INTR_VECTOR_TABLE_SIZE       (CLASSB_INTR_DEVICE_VECT_OFFSET + PERIPH_MAX_IRQn)
#define CLASSB_INTR_MAX_INT_COUNT           (60U)
#define CLASSB_INTR_TEST_RTC_COUNT          (50U)
#define CLASSB_INTR_TEST_TC_COUNT           (100U)

/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/
extern uint32_t __svectors;
extern volatile uint8_t * interrupt_tests_status;
extern volatile uint32_t * interrupt_count;
// Align the vector table at 1024 byte boundary
__attribute__ ((aligned (1024)))
//uint32_t classb_ram_vector_table[CLASSB_INTR_VECTOR_TABLE_SIZE];
uint32_t vtor_default_value = 0;

/*----------------------------------------------------------------------------
 *     Functions
 *----------------------------------------------------------------------------*/
extern void _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE test_type,
    CLASSB_TEST_ID test_id, CLASSB_TEST_STATUS value);

/*============================================================================
static void _CLASSB_RTC_handler(void)
------------------------------------------------------------------------------
Purpose: Custom handler used for RTC Interrupt test
Input  : None.
Output : None.
Notes  : The RTC is reset after successfully performing the test.
============================================================================*/
#if 0
static void _CLASSB_RTC_Handler(void)
{
    // Clear the checked interrupt flag
    RTC_REGS->MODE0.RTC_INTFLAG = RTC_MODE0_INTFLAG_CMP0_Msk;
    *interrupt_tests_status = CLASSB_TEST_STARTED;
    // Reset the tested peripheral
    RTC_REGS->MODE0.RTC_CTRLA = RTC_MODE0_CTRLA_SWRST_Msk;
    while((RTC_REGS->MODE0.RTC_SYNCBUSY & RTC_MODE0_SYNCBUSY_SWRST_Msk) == RTC_MODE0_SYNCBUSY_SWRST_Msk)
    {
        // Wait for Synchronization after Software Reset
        ;
    }
}
#endif
/*============================================================================
static void _CLASSB_TC0_Handler(void)
------------------------------------------------------------------------------
Purpose: Custom handler used for TC Interrupt test. It clears the interrupt
         flag and updates the interrupt count variable.
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
#if 0
static void _CLASSB_TC0_Handler(void)
{
    // Clear the checked interrupt flag
    TC0_REGS->COUNT16.TC_INTFLAG = TC_INTFLAG_OVF_Msk;
    (*interrupt_count)++;
}
#endif

/* All the handlers are defined here.  Each will call its PLIB-specific function. */
void __ISR(_TIMER_1_VECTOR, ipl1SRS) TIMER_1_Handler (void)
{
    IFS0CLR = _IFS0_T1IF_MASK;
    (*interrupt_count)++;
    
}

void __ISR(_RTCC_VECTOR, ipl1SRS) RTCC_Handler (void)
{
    /* Clear the status flag */
    IFS1CLR = 0x2;
    //CLASSB_INTR_TEST_RTC_COUNT++;
    *interrupt_tests_status = CLASSB_TEST_STARTED;
}

/*============================================================================
static void _CLASSB_BuildVectorTable(void)
------------------------------------------------------------------------------
Purpose: Build the vector table for Interrupt self-test
Input  : None.
Output : None.
Notes  : The vector table used by this test is placed in SRAM.
============================================================================*/
#if 0
static void _CLASSB_BuildVectorTable(void)
{
    uint32_t i = 0;
    uint32_t vector_start = (uint32_t)&__svectors;

    for(i = 0; i < CLASSB_INTR_VECTOR_TABLE_SIZE; i++)
    {
        // Get the interrupt handler address from the original vector table.
        classb_ram_vector_table[i] = *(uint32_t *)(vector_start + (i * 4));
    }
    // Modify the tested interrupt handler address
    classb_ram_vector_table[CLASSB_INTR_DEVICE_VECT_OFFSET + RTC_IRQn] = (uint32_t )&_CLASSB_RTC_Handler;
    classb_ram_vector_table[CLASSB_INTR_DEVICE_VECT_OFFSET + TC0_IRQn] = (uint32_t )&_CLASSB_TC0_Handler;
    vtor_default_value = SCB->VTOR;
    // Update VTOR to point to the new vector table in SRAM
    SCB->VTOR = ((uint32_t)&classb_ram_vector_table[0] & SCB_VTOR_TBLOFF_Msk);
}
#endif
/*============================================================================
static void _CLASSB_RTC_Init(void)
------------------------------------------------------------------------------
Purpose: Configure RTC peripheral for Interrupt self-test
Input  : None.
Output : None.
Notes  : The clocks required for RTC are enabled after reset. This function
         does not modify the default clocks.
============================================================================*/
#if 0
static void _CLASSB_RTC_Init(void) {
    
   /* Unlock System */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    /* Initialize RTCC */
    RTCCONSET = _RTCCON_RTCWREN_MASK;  /* Enable writes to RTCC */

    /* Lock System */
    SYSKEY = 0x00000000;

    RTCCONCLR = _RTCCON_ON_MASK;   /* Disable clock to RTCC */

    /* wait for clock to stop. Block too long? */
    while(RTCCONbits.RTCCLKON);  /* clock disabled? */

    /* initialize the time, date and alarm */
    RTCTIME = 0x23595000;   /* Set RTCC time */

    RTCDATE = 0x18123100;  /* Set RTCC date */

    RTCALRMCLR = _RTCALRM_ALRMEN_MASK;  /* Disable alarm */

    while(RTCALRMbits.ALRMSYNC);  /* Wait for disable */

    ALRMTIME = 0x23595100;   /* Set alarm time */

    ALRMDATE = 0x00123100;   /* Set alarm date */

    /* repeat forever or 0-255 times */
    RTCALRMSET = _RTCALRM_CHIME_MASK;  /* Set alarm to repeat forever */

    RTCALRMbits.AMASK = 6;

    RTCCONCLR = _RTCCON_RTCOE_MASK;  /* Enable RTCC output */

    /* Set RTCC clock source (LPRC/SOSC) */
    RTCCONbits.RTCCLKSEL = 0;

    /* start the RTC */
    RTCCONSET = _RTCCON_ON_MASK;
}
#endif
static void _CLASSB_INT_CLK_Initialize(void)
{
    return ;
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
    CFGCON0bits.PMDLOCK = 0;

    PMD1 = 0x25808981;
    PMD2 = 0x7e0f0f;
    PMD3 = 0x19031317;

    CFGCON0bits.PMDLOCK = 1;

    /* Lock system since done with clock configuration */
    SYSKEY = 0x33333333;
}




void rtc_start(void)
{
    IEC1SET = 0x2;
    RTCALRMSET = _RTCALRM_ALRMEN_MASK;
}
/*============================================================================
static void _CLASSB_TC0_CompareInit(void)
------------------------------------------------------------------------------
Purpose: Configure TC peripheral for Interrupt self-test
Input  : None.
Output : None.
Notes  : The TC0 is reset after successfully performing the test.
============================================================================*/
void _CLASSB_TMR1_INIT(void)
{
    /* Disable Timer */
    T1CONCLR = _T1CON_ON_MASK;

    /*
    SIDL = 0
    TWDIS = 0
    TECS = 1
    TGATE = 0
    TCKPS = 3
    TSYNC = 0
    TCS = 0
    */
    T1CONSET = 0x130;

    /* Clear counter */
    TMR1 = 0x0;

    /*Set period */
    PR1 = 7811; // 7811 = 20ms
    
    IEC0SET = _IEC0_T1IE_MASK;
    
    T1CONSET = _T1CON_ON_MASK;
}

/*============================================================================
static void _CLASSB_NVIC_Init(void)
------------------------------------------------------------------------------
Purpose: Initializes the NVIC
Input  : None.
Output : None.
Notes  : None.
============================================================================*/
void Classb_EVIC_Initialize( void )
{
    INTCONSET = _INTCON_MVEC_MASK;

    /* Set up priority and subpriority of enabled interrupts */
    IPC1SET = 0x4 | 0x0;  /* TIMER_1:  Priority 1 / Subpriority 0 */
    IPC8SET = 0x400 | 0x0;  /* RTCC:  Priority 1 / Subpriority 0 */
    
    /* Configure Shadow Register Set */
    PRISS = 0x76543210;
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
    
    //CLASSB_TEST_STATUS intr_test_status = CLASSB_TEST_NOT_EXECUTED;

    // Reset the counter
    *interrupt_count = 0;
    _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_INTERRUPT,
        CLASSB_TEST_INPROGRESS);
    
#if 0
    _CLASSB_BuildVectorTable();
#endif
    __builtin_disable_interrupts();
    _CLASSB_INT_CLK_Initialize();
    
    //_CLASSB_RTC_Init();
    _CLASSB_TMR1_INIT();
    Classb_EVIC_Initialize();
    
    /* Enable global interrupts */
    __builtin_enable_interrupts();
    //rtc_start();
    
    // Wait until the flags are updated from the interrupt handlers
    /*while((*interrupt_tests_status == CLASSB_TEST_NOT_STARTED))
    {
        ;
    }*/
    
    if ((*interrupt_count < CLASSB_INTR_MAX_INT_COUNT)
        &&  (*interrupt_count > 0))
    {
        T1CONCLR = _T1CON_ON_MASK;// stop timer
        RTCALRMCLR = _RTCALRM_ALRMEN_MASK; // stop rtc
        //intr_test_status = CLASSB_TEST_PASSED;
        _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_INTERRUPT,
            CLASSB_TEST_PASSED);
        return 1;

    }
    else
    {
        //intr_test_status = CLASSB_TEST_FAILED;
        _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_INTERRUPT,
            CLASSB_TEST_FAILED);
        return 1;
        // The failsafe function must not return.
        //CLASSB_SelfTest_FailSafe(CLASSB_TEST_INTERRUPT);
    }
   // __builtin_disable_interrupts();
    
}
