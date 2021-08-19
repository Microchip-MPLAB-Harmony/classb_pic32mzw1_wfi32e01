/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "system/console/sys_console.h"
#include "bsp/bsp.h"
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define APP_FLASH_ADDRESS                   (NVM_FLASH_START_ADDRESS + (NVM_FLASH_SIZE / 2))
#define FLASH_CRC32_ADDR                    (0x900FE000)
#define SRAM_RST_SIZE                       (32768U)
#define CLASSB_CLOCK_TEST_TMR1_RATIO_NS     (30517U)
#define CLASSB_CLOCK_TEST_RATIO_NS_MS       (1000000U)
#define CLASSB_CLOCK_DEFAULT_CLOCK_FREQ     (200000000U)

uint32_t crc_val[1] CACHE_ALIGN;
uint32_t crc_val_flash[1];
uint32_t * app_crc_addr = (uint32_t *) FLASH_CRC32_ADDR;
static volatile bool xferDone = false;
char test_status_str[4][25] = {"CLASSB_TEST_NOT_EXECUTED",
    "CLASSB_TEST_PASSED",
    "CLASSB_TEST_FAILED",
    "CLASSB_TEST_INPROGRESS"};

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
 */

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void eventHandler(uintptr_t context) {
    xferDone = true;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

void app_classb_tests(void) {
    SYS_CONSOLE_PRINT("\r\n\r\n        Class B API Usage Demo      \r\n");
    SYS_CONSOLE_PRINT("\r\n\r\n Class B startup self-test results \r\n");
    CLASSB_TEST_STATUS classb_test_status = CLASSB_TEST_NOT_EXECUTED;
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_CPU);
    SYS_CONSOLE_PRINT("\r\n\tResult of CPU SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_PC);
    SYS_CONSOLE_PRINT("\r\n\tResult of PC SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_RAM);
    SYS_CONSOLE_PRINT("\r\n\tResult of SRAM SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_FLASH);
    SYS_CONSOLE_PRINT("\r\n\tResult of Flash SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_CLOCK);
    SYS_CONSOLE_PRINT("\r\n\tResult of Clock SST is %s\r\n", test_status_str[classb_test_status]);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_SST, CLASSB_TEST_INTERRUPT);
    SYS_CONSOLE_PRINT("\r\n\tResult of Interrupt SST is %s\r\n", test_status_str[classb_test_status]);


    SYS_CONSOLE_PRINT("\r\n\r\n Class B run-time self-tests (RST) \r\n");

    /*CPU self test*/
    classb_test_status = CLASSB_TEST_FAILED;
    classb_test_status = CLASSB_CPU_RegistersTest(true);
    SYS_CONSOLE_PRINT("\r\n\tResult of CPU RST is %s\r\n", test_status_str[classb_test_status]);

    /*Program Counter self test*/
    classb_test_status = CLASSB_TEST_FAILED;
    classb_test_status = CLASSB_CPU_PCTest(true);
    SYS_CONSOLE_PRINT("\r\n\tResult of PC RST is %s\r\n", test_status_str[classb_test_status]);

    /*SRAM self test*/
    __builtin_disable_interrupts();
    classb_test_status = CLASSB_SRAM_MarchTestInit((uint32_t *) CLASSB_SRAM_APP_AREA_START,
            SRAM_RST_SIZE, CLASSB_SRAM_MARCH_C, false);
    /* Enable global interrupts */
    __builtin_enable_interrupts();
    SYS_CONSOLE_PRINT("\r\n\tResult of SRAM RST is %s\r\n", test_status_str[classb_test_status]);

    /*NVM self test*/
    NVM_CallbackRegister(eventHandler, (uintptr_t) NULL);
    while (NVM_IsBusy() == true);
    xferDone = false;
    while (NVM_IsBusy() == true);
    //xferDone = false;
    /* Erase the Page */
    NVM_PageErase(0x900FE000);
    while (xferDone == false);
    xferDone = false;
    // Generate CRC-32 over internal flash address 0 (Virtual address : 0x90000000) to 0xFE000 (Virtual address : 0x900FE000)
    crc_val[0] = CLASSB_FlashCRCGenerate(0x90000000, 0xFE000);
    while (NVM_IsBusy() == true);
    // Use NVMCTRL to write the calculated CRC into a Flash location
    NVM_RowWrite((uint32_t *) crc_val, FLASH_CRC32_ADDR);
    while (xferDone == false);
    xferDone = false;
    NVM_Read((uint32_t *) & crc_val_flash, sizeof (crc_val_flash), FLASH_CRC32_ADDR);
    classb_test_status = CLASSB_TEST_FAILED;
    classb_test_status = CLASSB_FlashCRCTest(0x90000000, 0xFE000,
            *(uint32_t *) crc_val_flash, true);
    SYS_CONSOLE_PRINT("\r\n\tResult of Flash RST is %s\r\n", test_status_str[classb_test_status]);
    SYS_CONSOLE_PRINT("\r\n\t\tFlash test is disabled in the demo since user needs to add CRC. Please refer to the class B documentation.\r\n");

    /*CPU Clock self test*/
    __builtin_disable_interrupts();
    classb_test_status = CLASSB_ClockTest(200000000, 5, 1638, true);
    //SYS_Initialize(NULL);
    __builtin_enable_interrupts();
    SYS_CONSOLE_PRINT("\r\n\tResult of CPU Clock RST is %s\r\n", test_status_str[classb_test_status]);

    /*GPIO self test*/
    LED_RED_Off();
    CLASSB_RST_IOTest(GPIO_PORT_K, PIN1, PORT_PIN_LOW);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_IO);
    SYS_CONSOLE_PRINT("\r\n\tResult of PK1 LOW test is %s\r\n", test_status_str[classb_test_status]);
    //Drive HIGH on the pin to be tested.
    LED_RED_On();
    CLASSB_RST_IOTest(GPIO_PORT_K, PIN1, PORT_PIN_HIGH);
    classb_test_status = CLASSB_GetTestResult(CLASSB_TEST_TYPE_RST, CLASSB_TEST_IO);
    SYS_CONSOLE_PRINT("\r\n\tResult of PK1 HIGH is %s\r\n", test_status_str[classb_test_status]);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize(void) {
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks(void) {
    /* Check the application's current state. */
    switch (appData.state) {
            /* Application's initial state. */
        case APP_STATE_INIT:
        {
            app_classb_tests();
            appData.state = APP_STATE_SERVICE_TASKS;
            break;
        }
        case APP_STATE_SERVICE_TASKS:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
