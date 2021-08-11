/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include "definitions.h"                // SYS function prototypes


#define APP_FLASH_ADDRESS       (NVM_FLASH_START_ADDRESS + (NVM_FLASH_SIZE / 2))


#define FLASH_CRC32_ADDR    (0x900FE000)
#define SRAM_RST_SIZE       32768


uint32_t crc_val[1] CACHE_ALIGN;
uint32_t crc_val_flash[1] ;
uint32_t * app_crc_addr = (uint32_t *)FLASH_CRC32_ADDR;



char test_status_str[4][25] = {"CLASSB_TEST_NOT_EXECUTED",
                                "CLASSB_TEST_PASSED",
                                "CLASSB_TEST_FAILED",
                                "CLASSB_TEST_INPROGRESS"};

static volatile bool xferDone = false;



static void eventHandler(uintptr_t context)
{
    xferDone = true;
}
#define CLASSB_CLOCK_TEST_TMR1_RATIO_NS      (30517U)
#define CLASSB_CLOCK_TEST_RATIO_NS_MS       (1000000U)
#define CLASSB_CLOCK_DEFAULT_CLOCK_FREQ     (200000000U)
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    

    /* Initialize all modules */
    SYS_Initialize ( NULL );
        
     SYS_CONSOLE_PRINT("\r\n\r\n        Class B API Usage Demo      \r\n");
    CLASSB_TEST_STATUS classb_test_status = CLASSB_TEST_NOT_EXECUTED;
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_CPU);
    SYS_CONSOLE_PRINT("\r\n Result of CPU SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_PC);
    SYS_CONSOLE_PRINT("\r\n Result of PC SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_RAM);
    SYS_CONSOLE_PRINT("\r\n Result of SRAM SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_FLASH);
    SYS_CONSOLE_PRINT("\r\n Result of Flash SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_CLOCK);
    SYS_CONSOLE_PRINT("\r\n Result of Clock SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_INTERRUPT);
    SYS_CONSOLE_PRINT("\r\n Result of Interrupt SST is %s\r\n", test_status_str[classb_test_status]);
    
    
    SYS_CONSOLE_PRINT("\r\n\r\n Class B run-time self-tests \r\n");
    classb_test_status = CLASSB_TEST_FAILED;
    classb_test_status = CLASSB_CPU_RegistersTest(true);
    SYS_CONSOLE_PRINT("\r\n Result of CPU RST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_TEST_FAILED;
    classb_test_status = CLASSB_CPU_PCTest(true);
    SYS_CONSOLE_PRINT("\r\n Result of PC RST is %s\r\n", test_status_str[classb_test_status]);
    
    __builtin_disable_interrupts();
    classb_test_status = CLASSB_SRAM_MarchTestInit((uint32_t *)CLASSB_SRAM_APP_AREA_START,
        SRAM_RST_SIZE, CLASSB_SRAM_MARCH_C, false);
    /* Enable global interrupts */
    __builtin_enable_interrupts();
    
    SYS_CONSOLE_PRINT("\r\n Result of SRAM RST is %s\r\n", test_status_str[classb_test_status]);
    
    
    
    NVM_CallbackRegister(eventHandler, (uintptr_t)NULL);
    
    while(NVM_IsBusy() == true);
    
    
     xferDone = false;
    
    while(NVM_IsBusy() == true);
    //xferDone = false;
    /* Erase the Page */
    NVM_PageErase(0x900FE000);
    
    while(xferDone == false);
    
    xferDone = false;
    // Generate CRC-32 over internal flash address 0 (Virtual address : 0x90000000) to 0xFE000 (Virtual address : 0x900FE000)
    crc_val[0] = CLASSB_FlashCRCGenerate(0x90000000, 0xFE000);
    while(NVM_IsBusy() == true);
    // Use NVMCTRL to write the calculated CRC into a Flash location
    NVM_RowWrite( (uint32_t *)crc_val, FLASH_CRC32_ADDR);
    while(xferDone == false);

    xferDone = false;
        
    NVM_Read((uint32_t *)&crc_val_flash, sizeof (crc_val_flash), FLASH_CRC32_ADDR);
    
    classb_test_status = CLASSB_TEST_FAILED;
    classb_test_status = CLASSB_FlashCRCTest(0x90000000, 0xFE000,
            *(uint32_t *)crc_val_flash, true);
    
    SYS_CONSOLE_PRINT("\r\n Result of Flash RST is %s\r\n", test_status_str[classb_test_status]);

    
    __builtin_disable_interrupts();
    classb_test_status = CLASSB_ClockTest(200000000, 5, 1638, true);
    //SYS_Initialize(NULL);
    __builtin_enable_interrupts();
    SYS_CONSOLE_PRINT("\r\n Result of CPU Clock RST is %s\r\n", test_status_str[classb_test_status]);
    
    LED_RED_Off();
    CLASSB_RST_IOTest(GPIO_PORT_K, PIN1, PORT_PIN_LOW);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_IO);
    SYS_CONSOLE_PRINT("\r\n Result of PK1 LOW test is %s\r\n", test_status_str[classb_test_status]);
    
    //Drive HIGH on the pin to be tested.
    LED_RED_On();
    
    CLASSB_RST_IOTest(GPIO_PORT_K, PIN1, PORT_PIN_HIGH);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_IO);
    SYS_CONSOLE_PRINT("\r\n Result of PK1 HIGH is %s\r\n", test_status_str[classb_test_status]);
    while ( true )
    {
    }

    /* Execution should not come here during normal operation */
    return ( EXIT_FAILURE );
}
