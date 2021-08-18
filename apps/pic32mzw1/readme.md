---
grand_parent: Harmony 3 Class B Library for PIC32MZW1
parent: PIC32MZW1 Class B Library Examples
title: Class B Library API Usage Example
nav_order: 7
---

# PIC32MZW1 Class B API Usage Example

This example shows how to use the self-test APIs from the Class B library during startup as well as run-time.
When a self-test is used at startup, it is referred as Startup Self-test (SST) and when it is used at
run-time, it is referred as run-time self-test (RST).
When this demo application starts, it reads the status of startup self-tests and sends them to the UART1 COM port.
It then executes a few run-time self-tests for different components inside microcontroller and reports
whether the tests are passed or not.
For the Flash memory self-test, CRC-32 for the internal flash memory from address 0 to 0xFE000 is calculated
and the CRC value is stored at address 0xFE000 using NVM write command.
Refer the User Guide for the Class B Library for details on the self-tests. 

## Building The Application
The parent folder for all the MPLAB X IDE projects for this application is given below:

**Application Path** : \classb\apps\classb_pic32mzw1_wfi32e01\classb_api_usage\firmware

To build the application, open the project file classb_api_usage.X in MPLABX IDE.

## MPLAB Harmony Configurations

Refer to the MHC project graph for the components used and the respective configuration options.

## Hardware Setup

1. Project classb_api_usage.X
    * Hardware Used
        * PIC32MZ W1 Curiosity Board
    * Hardware Setup
        * Ensure the tested IO pins are kept at specified logic levels.

## Setting up PIC32MZ W1 Curiosity Board

- Connect the Debug USB port on the board to the computer using a micro USB cable
- On the GPIO Header (J207), connect U1RX (PIN 13) and U1TX (PIN 23) to TX and RX pin of any USB to UART converter

## Running The Application
1.  Open the project "classb_api_usage".
2. Connect to a USB to UART converter to UART1 and Open a Terminal application (Ex.:Tera term) on the computer. Configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None
3. Build and Program the application using the MPLAB X IDE.
4. Observe the messages shown on the console.
5. Code for Runtime self-test are included in `main.c` file. 

**NOTE:** 

**1) In the application, for runtime flash test, at first `CLASSB_FlashCRCGenerate()` function is being used to generate CRC and then store it in a memory location using Harmony provided NVM component. `CLASSB_FlashCRCGenerate()` function is part of classB library.**

**2) Currently MHC generated code, locks PMD registers during initialzation part after configurig it with user defined configuration via MHC.    Due to this PMD registers can not be modified by the rest of the application.**

**Clock self test, requires PMD register to enable TMR1 module . Due this reason, Currently , to support clock test during runtime user need to comment/remove below lines of code manually in "CLK_Initialize()" function :**
-   CFGCON0bits.PMDLOCK = 0;
-   CFGCON0bits.PMDLOCK = 1;

    **Without above modification, Clock test during runtime will fail, and remain in infinite loop.** 


