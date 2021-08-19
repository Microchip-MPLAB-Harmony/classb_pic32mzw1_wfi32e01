---
grand_parent: Harmony 3 Class B Library for PIC32MZW1
parent: PIC32MZW1 Class B Library
title: Class B Peripheral Library Usage
nav_order: 3
---

# Class B Peripheral Library Usage

This document describes the basic architecture of the Class B library and provides information and examples on how to use it.
APIs defined by the Class B library can be used either at start-up or by the application code.
The application may use PLIBs, drivers or middleware from the Harmony 3 software framework along with the Class B library code.

**Interface Header File** : classb.h

The interface to the Class B Library is defined in the classb.h header file. Any C language source (.c) file
that uses the Class B should include classb.h.

## Library Header Files

1. classb_clock_test.h
2. classb_common.h
3. classb_cpu_reg_test.h
4. classb_flash_test.h
5. classb_interrupt_test.h
6. classb_io_pin_test.h
7. classb_sram_test.h

## Library Source Files

1. classb.c
2. classb_clock_test.c
3. classb_cpu_reg_test.c
3. classb_cpu_reg_test_asm.S
4. classb_cpu_pc_test.c
5. classb_flash_test.c
6. classb_interrupt_test.c
7. classb_io_pin_test.c
8. classb_result_management.S
9. classb_sram_test.c

## Abstraction Model

The following picture shows positioning of Class B library in a Harmony 3 based application.

<figure align="center">
<img src="./images/H3_ClassB_Architecture.png"/>
<figcaption>Class B library in MPLAB Harmony 3</figcaption>
</figure>

## Start-up vs. Run-time

The Class B library contains many self-test routines those can be executed at startup and run-time.
If a self-test is executed at startup, it is called as a Start-up Self-test (SST) and if it is executed
at run-time, then it is called a Run-time Self-test (RST). There are a few self-tests which can be used
only as SST or as RST, such self-tests have &#39;SST&#39; or &#39;RST&#39; in the API name
eg: `CLASSB_RST_IOTest()`, `CLASSB_SST_InterruptTest()`. If a self-test API does not have &#39;SST&#39;
or &#39;RST&#39; in its name, then it can be used at startup as well as runtime.

### Start-up Self-test (SST)

SSTs are used to test a component inside the microcontroller before it is initialized and used.
When the Class B library is added via MHC, the selected SSTs are inserted into the `_on_bootstrap()`
function which is called during startup , before main. This means that none of the data initialization
could have happened before running SSTs. So, the Class B library initializes necessary variables
before using them. It is not mandatory to test all the components during startup. The SRAM can be
tested partially if a faster startup is needed by the application. In this case, modify the
corresponding configuration macro (`CLASSB_SRAM_STARTUP_TEST_SIZE`) present in `classb.h` file
to change the size of the tested area.

### Run-time Self-test (RST)

RSTs can be used by the application during run-time to check safe operation of different components
in the microcontroller. These tests are non-destructive. In the case of run-time tests, the application
shall decide which test to execute when.

**Note**
> Disable IRQs before running self-test for SRAM or CPU clock. Interrupting SRAM test can result in unexpected variable values if the test has been running on the SRAM area used by the application. Interrupting clock test can result in incorrect test results since the test is timing dependent.


## Components in the Library

The Class B library contains self-test routines for different components inside the CPU.

<figure align="center">
<img src="./images/ClassB_SYS_Elements.png"/>
<figcaption>Components in the Class B library</figcaption>
</figure>

### Critical and Non-critical Components

Based on the impact of failure, different components inside this Class B library are categorized
as `critical` or `non-critical`.

If the self-test for CPU registers, PC or Flash detects a failure, the code execution is stopped,
and it remains in an infinite loop. This is to prevent unsafe code execution. In the case of non-critical
components, a failsafe function (`CLASSB_SelfTest_FailSafe`) is called when a failure is detected.
This function contains a software break point and an infinite loop. Further code shall be added into
this function as per the application need. The failsafe function must not return to the Class B library,
since it is called due to a self-test failure.
Avoid use of features which depend on the failed component. For example, if self-test for clock is failed,
it is not advisable to use UART for error reporting as BAUD rate may not be accurate. In the case of SRAM
failure, avoid the use of function calls or use of variables in SRAM. A simple error reporting mechanism
in this case of SRAM failure can be toggling of an IO pin.

#### Critical Components

1. CPU registers including the Program Counter
2. Internal Flash program memory

<figure align="center">
<img src="./images/Generic_flow_Critical_test.png"/>
<figcaption>Generic Flow of Critical Tests</figcaption>
</figure>

#### Non-critical Components

1. Internal SRAM
2. CPU clock
3. Interrupts
4. IO pins


<figure align="center">
<img src="./images/Generic_flow_Non_critial_tests.png"/>
<figcaption>Generic Flow of Non-Critical Tests</figcaption>
</figure>

## Self-tests for Components in the Library

## CPU Registers

The MIPS32® M-Class Microprocessor Core is the CPU on the PIC32MZW1 devices. The Class B library checks the processor
core registers for stuck-at faults. The stuck-at condition causes
register bit to remain at logic 0 or logic 1. Code execution should be stopped if this error condition
is detected in any of the CPU registers.

It can be used at startup as well as run-time. The Program Counter (PC) self-test is designed
as a separate test since this register cannot be checked with usual test data patterns.

<figure align="center">
<img src="./images/DD_Test_CPU.png"/>
<figcaption>Flow chart of the self-test for CPU registers</figcaption>
</figure>



## Program Counter (PC)

The self-test for PC checks whether a stuck-at condition is present in the PC register.
The stuck-at condition causes register bit to remain at logic 0 or logic 1.
Code execution should be stopped if this error condition is detected.

The self-test for PC calls multiple functions in predefined order and verifies that each function
is executed and returns the expected value. If the return values of all test functions are correct,
the Program Counter is assumed to be working fine. This self-test can be used at startup as well as run-time.

<figure align="center">
<img src="./images/DD_PC_TEST.png"/>
<figcaption>Flow chart of the self-test for Program Counter (PC)</figcaption>
</figure>

The optimization level for the source file classb_cpu_pc_test.c must be kept as -O1.
If compiler optimization level for the MPLAB Harmony 3 project is set to something other than -O0 or -O1,
the file level optimization need to be configured. To do this, open file properties by
right clicking on the source file. Modify the settings as shown in the below screen shots
after generating the project.

Select the check box 'override build options'

<figure align="center">
<img src="./images/FileOptimization_1.png"/>
</figure>


In xc32-gcc options, set optimization to -O1 using the highlighted drop-down menus,
<figure align="center">
<img src="./images/FileOptimization_2.png"/>
</figure>



## Flash

The internal flash memory of the device needs to be checked for proper functionality.
The self-test for internal flash performs CRC check on the internal flash memory of the device.
The address range is configurable for this self-test. It runs CRC-32 algorithm with
reversed representation of the polynomial 0x04C11DB7 and compares the generated checksum
with the expected checksum. It uses table-based approach where the table is generated during the execution.

This self-test uses a CRC-32 generation function. This function is used inside the Class B library to generate
CRC-32 of the internal Flash memory but it can be used on any contiguous memory area.
The flash self-test can be used at startup as well as run-time. If this self-test is used during start up,
it must be ensured that the CRC of the application area is precalculated and stored at a specific memory
address which is passed as an argument for the Flash self-test. If this self-test detects a failure,
it remains in an infinite loop.

Flow chart of the self-test for internal flash program memory

<figure align="center">
<img src="./images/DD_FLASH.png"/>
</figure>


## SRAM

Self-test for the SRAM element looks for stuck-at faults, DC faults and addressing faults with the help
of RAM March algorithms. One of the input arguments to this self-test selects the algorithm. This self-test
copies the data from the tested area of the SRAM into the reserved area in the SRAM and restore the data
after the test. Refer to section &#39;Configuring the Library&#39; for the details on reserving the SRAM.
The stack pointer is moved to the reserved area in the SRAM before running this self-test.
The SRAM self-test can be used at startup as well as run-time.

It provides three standard tests to detect error conditions,

1. March C
2. March C minus
3. March B

<figure align="center">
<img src="./images/DD_Test_Init_SRAM.png"/>
<figcaption>Flow chart of the self-test for SRAM</figcaption>
</figure>

<figure align="center">
<img src="./images/DD_Test_SRAM.png"/>
<figcaption>Flow chart of the internal routine for SRAM self-test</figcaption>
</figure>


## Clock

The self-test for CPU clock checks whether the CPU clock frequency is within the permissible range.
It uses TMR1 and SysTick to measure the CPU clock frequency. The TMR1 is clocked at 32768 Hz from the
32 kHz External Crystal Oscillator and CPU clock can be from any other high frequency oscillator.
If the CPU clock frequency is within specified error limit, it returns PASS. The test duration is
defined by one of the input arguments.
The clock self-test can be used at startup as well as run-time.

**Note**
1. This self-test uses the TMR1 peripheral. Thus, if it is used during run-time, the TMR1 shall not
be used by the application for continuous modes . If the TMR1 is used
for some other purpose, it must be reconfigured after running the clock self-test.
2. This self-test configures XOSC32K to clock the TMR1. If the application cannot afford to reconfigure
the XOSC32K, this self-test cannot not be used during run-time.
3. Keep the clock test duration lesser than the WDT timeout period, to avoid the WDT resetting the device.


<figure align="center">
<img src="./images/DD_Clock.png"/>
<figcaption>Flow chart of the self-test for CPU clock frequency</figcaption>
</figure>


## Interrupt

The self-test for this element checks the interrupts functionality of the microcontroller. It configures the
EVIC, the Timer 1 (TMR1) and the Timer 2 (TMR2)
peripherals to test the interrupt handling mechanism.
It verifies that at least one interrupt is generated and handled properly.
This self-test also checks whether the number of interrupts generated are too many within a given time period.
TMR1 is configured for polling (Without ISR).
It reports a PASS if the total number of interrupts generated
by the TMR2 is greater than one and less than the specified upper limit. The clock used for TMR1 and TMR2 is from the
Internal peripheral clock .
The interrupt self-test can be used only at startup.

> **Note** This startup self-test utilizes the interrupts generated by TMR2 and TMR1 with polling method. For run-time testing of interrupts,
a separate self-test need to be developed.

<figure align="center">
<img src="./images/DD_SST_Interrupt.png"/>
<figcaption>Flow chart of the self-test for interrupts</figcaption>
</figure>


## IO pin

The self-test for IO pins verifies that any output pin is able to keep the configured logic state on the pin
and any input pin is able to read the logic state present on the pin.

As the exact use of an IO pin is decide by the application, it is the responsibility of the application to
configure the IO pin direction and drive the pin to the expected state before calling this self-test.
When testing an input pin, ensure that the IO pin is externally kept at a defined logic state.
The IO pin self-test can be used only at run-time.


<figure align="center">
<img src="./images/DD_RST_IO.png"/>
<figcaption>Flow chart of the self-test for IO pins</figcaption>
</figure>
