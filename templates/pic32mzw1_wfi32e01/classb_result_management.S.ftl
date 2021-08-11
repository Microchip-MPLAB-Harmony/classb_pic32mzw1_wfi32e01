/*******************************************************************************
  Class B Library v2.0.0 Release

  Company:
    Microchip Technology Inc.

  File Name:
    classb_result_management.S

  Summary:
    Assembly functions to clear, modify and read test results.

  Description:
    This file provides assembly functions to clear, modify and read self-test
    results.

*******************************************************************************/

/*******************************************************************************
Copyright (c) 2020 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS  WITHOUT  WARRANTY  OF  ANY  KIND,
EITHER EXPRESS  OR  IMPLIED,  INCLUDING  WITHOUT  LIMITATION,  ANY  WARRANTY  OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A  PARTICULAR  PURPOSE.
IN NO EVENT SHALL MICROCHIP OR  ITS  LICENSORS  BE  LIABLE  OR  OBLIGATED  UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,  BREACH  OF  WARRANTY,  OR
OTHER LEGAL  EQUITABLE  THEORY  ANY  DIRECT  OR  INDIRECT  DAMAGES  OR  EXPENSES
INCLUDING BUT NOT LIMITED TO ANY  INCIDENTAL,  SPECIAL,  INDIRECT,  PUNITIVE  OR
CONSEQUENTIAL DAMAGES, LOST  PROFITS  OR  LOST  DATA,  COST  OF  PROCUREMENT  OF
SUBSTITUTE  GOODS,  TECHNOLOGY,  SERVICES,  OR  ANY  CLAIMS  BY  THIRD   PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE  THEREOF),  OR  OTHER  SIMILAR  COSTS.
*******************************************************************************/

; /* CLASSB_TEST_STATUS CLASSB_GetTestResult(CLASSB_TEST_TYPE test_type,
;       CLASSB_TEST_ID test_id) */
.global CLASSB_GetTestResult

; /* void CLASSB_ClearTestResults(CLASSB_TEST_TYPE test_type) */
.global CLASSB_ClearTestResults

; /* void _CLASSB_UpdateTestResult(CLASSB_TEST_TYPE test_type,
;       CLASSB_TEST_ID test_id, CLASSB_TEST_STATUS value) */
.global _CLASSB_UpdateTestResult

; /* SRAM address for SST results */
.equ SST_RES_ADDRESS, 0xA0000000
; /* SRAM address for RST results */
.equ RST_RES_ADDRESS, 0xA0000002
; /* Result mask is 0x03 (2-bit results) */
.equ RESULT_BIT_MASK, 0x03
; /* CLASSB_TEST_STATUS */
.equ CLASSB_TEST_NOT_EXECUTED, 0x0

; /* Load to clear*/
.equ REG_CLEAR_VAL, 0x00000000


#if 1
CLASSB_GetTestResult:
; /* Input arguments are test_type and test_id */
; /* test_type in a0, test_id in a1 */
    ; /* push used registers */
    
    /*************PUSH******************/
    sub     $sp, $sp, 16
    sw      $t4, 12($sp)
    sw      $t5, 8($sp)
    sw      $t6, 4($sp)    
    sw      $t7, 0($sp) 
    /***********************************/
 
    la      $t4, SST_RES_ADDRESS

    beq      $a0, $0, GetResTestTypeSST
    la      $t4, RST_RES_ADDRESS
GetResTestTypeSST:
    li      $t6, RESULT_BIT_MASK

    ; /* Load test result into t5 */
    lhu      $t5,($t4)
    
    ; /* Extract the test result and return via v0 */

    move    $v0,$t5
    srl     $v0,$v0,$a1
    and     $v0,$v0,$t6
            
    ; /* Load 1s complement result into t7 */
    lhu      $t7,4($t4)
    ; /* Check whether the read results are 1s complements */
    xor     $t5,$t7,$t5
    srl     $t5,$t5,$a1
    and     $t5,$t5,$t6
    beq     $t5,3,TestResultValid        
    ; /* Result mismatch. Return CLASSB_TEST_NOT_EXECUTED */
    li      $v0,CLASSB_TEST_NOT_EXECUTED
TestResultValid:
    /*************POP******************/
    
    lw      $t4, 12($sp)
    lw      $t5, 8($sp)
    lw      $t6, 4($sp)    
    lw      $t7, 0($sp) 
    addiu   $sp,$sp,16
    /***********************************/
    jr      $ra
    
_CLASSB_UpdateTestResult:
; /* Input arguments are test_type, test_id and value */
; /* test_type in a0, test_id in a1, value in a2 */
    ; /* push used registers */
    
    /*************PUSH******************/
    sub     $sp, $sp, 16
    sw      $t4, 12($sp)
    sw      $t5, 8($sp)
    sw      $t6, 4($sp)    
    sw      $t7, 0($sp) 
    /***********************************/

    la      $t4, SST_RES_ADDRESS  
    beq      $a0, $0, UpdateResTestTypeSST        
    la      $t4, RST_RES_ADDRESS          
            
UpdateResTestTypeSST:

    ;/* read the existing result */

    lhu     $t5,($t4)

    li      $t6, RESULT_BIT_MASK
    sll     $t6,$t6,$a1
    not     $t6,$t6

    ; /* Now t6 has ((~RESULT_BIT_MASK) << test_id) */
    and	    $t5, $t5, $t6
    move     $t7, $a2
    sll     $t7, $t7, $a1   
    or      $t5, $t5, $t7      
    sh      $t5,($t4)  
    ; /* Load the 1s complement of the result */
    ; /* For SSTs, this address is 0xA0000004 */
    ; /* For RSTs, this address is 0xA0000006 */
    ; /* Take 1s complement of the value */
    not     $t2, $t5
    
    /*store half word to memory*/
    sh      $t2, 4($t4)
    /*************POP******************/
    
    lw      $t4, 12($sp)
    lw      $t5, 8($sp)
    lw      $t6, 4($sp)    
    lw      $t7, 0($sp) 
    addiu   $sp,$sp,16
    /***********************************/
    jr      $ra

CLASSB_ClearTestResults:
; /* Input arguments are test_type and test_id */
; /* test_type in a0 */
    ; /* push used registers */
    /*************PUSH******************/
    sub     $sp, $sp, 16
    sw      $t4, 12($sp)
    sw      $t5, 8($sp)
    sw      $t6, 4($sp)    
    sw      $t7, 0($sp) 
    /***********************************/
    la      $t4, SST_RES_ADDRESS 
    beq      $a0, 0, ClearResTestTypeSST 
    la      $t4, RST_RES_ADDRESS
    
ClearResTestTypeSST:
    
    li      $t5, REG_CLEAR_VAL
    /*store half word to memory*/
    sh      $t5,($t4)
    ; /* Store 1s complement of the result */
    not     $t5,$t5
    ; /* Add 4 to get the address of 1s complement result */
    sh      $t5,4($t4)
    /*************POP******************/
    
    lw      $t4, 12($sp)
    lw      $t5, 8($sp)
    lw      $t6, 4($sp)    
    lw      $t7, 0($sp) 
    addiu   $sp,$sp,16
    /***********************************/
    jr      $ra
#endif