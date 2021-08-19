/*******************************************************************************
  Class B Library v1.0.0 implementation file

  Summary:
    This file contains the implementation for 
    the Class B Safety Software Library CPU Clock test
    for PIC32M MCUs.
        
*******************************************************************************/
//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2021 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to  you  the  right  to  use,  modify,  copy  and  distribute
Software only when embedded on a Microchip  microcontroller  or  digital  signal
controller  that  is  integrated  into  your  product  or  third  party  product
(pursuant to the  sublicense  terms  in  the  accompanying  license  agreement).

You should refer  to  the  license  agreement  accompanying  this  Software  for
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
// DOM-IGNORE-END
#include "xc.h"

/* global defines */
.equ   CPU_REGISTER_TEST_PASS,  1
.equ  CPU_REGISTER_TEST_FAIL,  2
.global CLASSB_CPURegistersTest

#; test used defines and storage 
.equ  MASK_AA_LOW,  0x0000AAAA
.equ  MASK_55_LOW,  0x00005555

    .text
ROM_AA_WORD:
	.word	0xaaaaaaaa;
ROM_55_WORD:
	.word	0x55555555;


/* implementation */

# This macro sets the Previous Shadow Set number.
# This allows the RDGPR and WRGPR instructions to 
# set the current register set to the number in the
# Previous Shadow Set
.macro set_shadow_file file_no temp1_reg temp2_reg
    mfc0    \temp1_reg, $12, 2      # Read the SRSCtl register into t0.
    li      \temp2_reg, \file_no    # Put Register set number into bits 4:0 of t1.
    ins     \temp1_reg, \temp2_reg, 26, 4   # Move the Set number into bits 26:26 
    mtc0    \temp1_reg, $12, 2      # Write the changes back to SRSCtl.
                            # At this point, all of the wrpgpr or rdpgpr
                            # instructions will access Shadow Register set <file_no>.
.endm

    
# define a macro to perform register 0 check
# in case of error a jump to err_label is performed
# make sure reg 0 is 0x0
# xori is used and a flash value
# temporary register also used

.macro check_r0 temp_reg err_label
    lui     \temp_reg,  0xffff;             # set hi half, lower low half, don't use $0    
    rotr    \temp_reg, \temp_reg, 16;       # swap halves
    xori    \temp_reg, \temp_reg, 0xffff;   # make the register 0

    bne     \temp_reg, $0, \err_label
    nop;
    #; finally
    bltz    $0, \err_label
    nop;
    bgtz    $0, \err_label;
    nop;
#; everything seems to be ok.
    
.endm


#; define a macro to perform the basic register check
#; in case of error a jump to err_label is performed
#; since comparing 2 regs loaded with the same value doesn't say
#; anything when both have same stuck bits a different approach is taken:
#; we make sure the value is all aa's or 55's
#; use 16 bit xori operations to not rely on another register
#; we use just register 0 which should be 0!

.macro check_register check_reg err_label
    la      \check_reg, ROM_AA_WORD;
    lw      \check_reg, 0(\check_reg);              # all 0xaa    
    xori    \check_reg, \check_reg, MASK_AA_LOW;    # clear low half
    rotr    \check_reg, \check_reg, 16;
    xori    \check_reg, \check_reg, MASK_AA_LOW;    # clear hi half
    bne     \check_reg, $0, \err_label
    nop;
    #; same for the other 0x55
    la      \check_reg, ROM_55_WORD;
    lw      \check_reg, 0(\check_reg);              # all 0x55    
    xori    \check_reg, \check_reg, MASK_55_LOW;    # clear low half
    rotr    \check_reg, \check_reg, 16;
    xori    \check_reg, \check_reg, MASK_55_LOW;    # clear hi half
    bne     \check_reg, $0, \err_label
    nop;
.endm

#; define a macro to perform the basic register check
#; on one of the shadow registers.
#; In case of error a jump to err_label is performed
.macro check_SHAD_register SHAD_reg temp1_reg temp2_reg err_label
    la      \temp1_reg, ROM_AA_WORD;
    lw      \temp1_reg, 0(\temp1_reg);              # Fill Temp registers with test pattern.
    wrpgpr  \temp1_reg, \SHAD_reg
    rdpgpr  \temp2_reg, \SHAD_reg
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;
    
    la      \temp1_reg, ROM_55_WORD;
    lw      \temp1_reg, 0(\temp1_reg);              # Fill Temp registers with test pattern.
    wrpgpr  \temp1_reg, \SHAD_reg
    rdpgpr  \temp2_reg, \SHAD_reg
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;
    
.endm
    
#; define a macro to perform the basic register check
#; on one of the accumulator registers.
#; In case of error a jump to err_label is performed
#; Essentially a similar test is done as that done to 
#; the other registers.  However, it is assumed that 
#; the general purpose registers can be trusted.
#; essentially, this means they should be tested first.
.macro check_accumulator temp1_reg temp2_reg err_label
    la      \temp1_reg, ROM_AA_WORD;
    lw      \temp1_reg, 0(\temp1_reg);              # Fill Temp registers with test pattern.
    la      \temp2_reg, ROM_AA_WORD;
    lw      \temp2_reg, 0(\temp2_reg);              # Fill Temp registers with test pattern.
    mthi    \temp1_reg
    mtlo    \temp2_reg
    mfhi    \temp1_reg
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;
    mflo    \temp1_reg
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;

    #; same for the other 0x55
    la      \temp1_reg, ROM_55_WORD;
    lw      \temp1_reg, 0(\temp1_reg);              # Fill Temp registers with test pattern.
    la      \temp2_reg, ROM_55_WORD;
    lw      \temp2_reg, 0(\temp2_reg);              # Fill Temp registers with test pattern.
    mthi    \temp1_reg
    mtlo    \temp2_reg
    mfhi    \temp1_reg
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;
    mflo    \temp1_reg
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;
.endm

#; define a macro to perform the basic register check
#; on one of the additional accumulator registers
#; provided by the DSP ASE module.
#; In case of error a jump to err_label is performed
.macro check_DSP_accumulator accumulator temp1_reg temp2_reg err_label
    la      \temp1_reg, ROM_AA_WORD;
    lw      \temp1_reg, 0(\temp1_reg);              # Fill Temp registers with test pattern.
    la      \temp2_reg, ROM_AA_WORD;
    lw      \temp2_reg, 0(\temp2_reg);              # Fill Temp registers with test pattern.
    mthi    \temp1_reg, \accumulator
    mtlo    \temp2_reg, \accumulator
    mfhi    \temp1_reg, \accumulator
    bne     \temp1_reg, \temp2_reg, \err_label
    mflo    \temp1_reg, \accumulator
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;

    #; same for the other 0x55
    la      \temp1_reg, ROM_55_WORD;
    lw      \temp1_reg, 0(\temp1_reg);              # Fill Temp registers with test pattern.
    la      \temp2_reg, ROM_55_WORD;
    lw      \temp2_reg, 0(\temp2_reg);              # Fill Temp registers with test pattern.
    mthi    \temp1_reg, \accumulator
    mtlo    \temp2_reg, \accumulator
    mfhi    \temp1_reg, \accumulator
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;
    mflo    \temp1_reg, \accumulator
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;
.endm

#; define a macro to perform the basic register check
#; on one of the additional registers provided by the 
#; FPU module.
#; In case of error a jump to err_label is performed
.macro check_FPU_register FPU_reg temp1_reg temp2_reg err_label
    la      \temp1_reg, ROM_AA_WORD;
    lw      \temp1_reg, 0(\temp1_reg);              # Fill Temp registers with test pattern.
    mtc1    \temp1_reg, \FPU_reg
    mfc1    \temp2_reg, \FPU_reg
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;
    
    la      \temp1_reg, ROM_55_WORD;
    lw      \temp1_reg, 0(\temp1_reg);              # Fill Temp registers with test pattern.
    mtc1    \temp1_reg, \FPU_reg
    mfc1    \temp2_reg, \FPU_reg
    bne     \temp1_reg, \temp2_reg, \err_label
    nop;
    
.endm
    
.set    noreorder
.set    noat

.text

/*******************************************************************************
  Function:
    int CLASSB_CPURegistersTest ( void )

  Summary:
    The CPU Register test implements the functional test
    H.2.16.5 as defined by the IEC 60730 standard.
    

  Description:
    This routine detects stuck-at Faults in the CPU registers.
    This ensures that the bits in the registers are not stuck at
    a value ?0? or ?1?.

  Precondition:
    None.

  Parameters:
    None.
    
  Returns:
    Result identifying the pass/fail status of the test:
    * CPU_REGISTER_TEST_PASS    - The test passed. CPU registers have not been detected to have stuck bits. 
    * CPU_REGISTER_TEST_FAIL    - The test failed. Some CPU register(s) has been detected to have stuck bits. 

  Example:
    <code>
    int testRes=CLASSB_CPURegistersTest();
    if(testRes==CPU_REGISTER_TEST_PASS)
    {
        // process test success
    }
    else
    {
        // process tests failure
    }
    </code>

  Remarks:
    This is a non-destructive test.
    Interrupts should be disabled when calling this test function.
    Refer to the AN1229 for details regarding the CLASSB_CPURegistersTest()
    and the Class B Software Library.
  *****************************************************************************/
.ent    CLASSB_CPURegistersTest
CLASSB_CPURegistersTest:

# preserve all registers required by the compiler for a safe return
# interrupts must be disabled as sp, k0 and k1 cannot be tested otherwise
    addiu   sp, sp, -44
    sw      fp, 0(sp)
    sw      gp, 4(sp)
    sw      ra, 8(sp)
    sw      s0, 12(sp)
    sw      s1, 16(sp)
    sw      s2, 20(sp)
    sw      s3, 24(sp)
    sw      s4, 28(sp)
    sw      s5, 32(sp)
    sw      s6, 36(sp)
    sw      s7, 40(sp)


#;*********************************
#; Test $0 register
    check_r0    $at CpuTestError

#;*********************************
#; Test $at register (1)
   check_register $at CpuTestError

#;*********************************
#; Test $v0-$v1 registers (2-3)
   check_register v0 CpuTestError
   check_register v1 CpuTestError 

#;*********************************
#; Test $a0-$a3 registers (4-7)
   check_register a0 CpuTestError 
   check_register a1 CpuTestError 
   check_register a2 CpuTestError 
   check_register a3 CpuTestError 

#;*********************************
#; Test $t0-$t7 registers (8-15)
   check_register t0 CpuTestError 
   check_register t1 CpuTestError 
   check_register t2 CpuTestError 
   check_register t3 CpuTestError 
   check_register t4 CpuTestError 
   check_register t5 CpuTestError 
   check_register t6 CpuTestError 
   check_register t7 CpuTestError 

#;*********************************
#; Test $s0-$s7 registers (16-23)
   check_register s0 CpuTestError 
   check_register s1 CpuTestError 
   check_register s2 CpuTestError 
   check_register s3 CpuTestError 
   check_register s4 CpuTestError 
   check_register s5 CpuTestError 
   check_register s6 CpuTestError 
   check_register s7 CpuTestError 

#;*********************************
#; Test $t8-$t9 registers (24-25)
   check_register t8 CpuTestError 
   check_register t9 CpuTestError 

#;*********************************
#; Test $k0-$k1 registers (26-27) 
   check_register k0 CpuTestError
   check_register k1 CpuTestError 

#;*********************************
#; Test $gp, $fp, $ra registers (28, 30, 31) 
   check_register gp CpuTestError
   check_register fp CpuTestError 
   check_register ra CpuTestError

   
#;*********************************
#; Test $hi, $lo
   check_accumulator t0 t1 CpuTestError

#if defined (__PIC32_HAS_FPU64)
#;*********************************
#; Test $f0-$f31 registers
   check_FPU_register $f0 t0 t1 CpuTestError
   check_FPU_register $f1 t0 t1 CpuTestError
   check_FPU_register $f2 t0 t1 CpuTestError
   check_FPU_register $f3 t0 t1 CpuTestError
   check_FPU_register $f4 t0 t1 CpuTestError
   check_FPU_register $f5 t0 t1 CpuTestError
   check_FPU_register $f6 t0 t1 CpuTestError
   check_FPU_register $f7 t0 t1 CpuTestError
   check_FPU_register $f8 t0 t1 CpuTestError
   check_FPU_register $f9 t0 t1 CpuTestError
   check_FPU_register $f10 t0 t1 CpuTestError
   check_FPU_register $f11 t0 t1 CpuTestError
   check_FPU_register $f12 t0 t1 CpuTestError
   check_FPU_register $f13 t0 t1 CpuTestError
   check_FPU_register $f14 t0 t1 CpuTestError
   check_FPU_register $f15 t0 t1 CpuTestError
   check_FPU_register $f16 t0 t1 CpuTestError
   check_FPU_register $f17 t0 t1 CpuTestError
   check_FPU_register $f18 t0 t1 CpuTestError
   check_FPU_register $f19 t0 t1 CpuTestError
   check_FPU_register $f20 t0 t1 CpuTestError
   check_FPU_register $f21 t0 t1 CpuTestError
   check_FPU_register $f22 t0 t1 CpuTestError
   check_FPU_register $f23 t0 t1 CpuTestError
   check_FPU_register $f24 t0 t1 CpuTestError
   check_FPU_register $f25 t0 t1 CpuTestError
   check_FPU_register $f26 t0 t1 CpuTestError
   check_FPU_register $f27 t0 t1 CpuTestError
   check_FPU_register $f28 t0 t1 CpuTestError
   check_FPU_register $f29 t0 t1 CpuTestError
   check_FPU_register $f30 t0 t1 CpuTestError
   check_FPU_register $f31 t0 t1 CpuTestError
#endif


#if defined (__PIC32_SRS_SET_COUNT)
#if (__PIC32_SRS_SET_COUNT >= 0)
#;*********************************
#; Test shadow register set 0 
   set_shadow_file 0 t0 t1
   check_SHAD_register $0 t0 t1 CpuTestError
   check_SHAD_register $1 t0 t1 CpuTestError
   check_SHAD_register $2 t0 t1 CpuTestError
   check_SHAD_register $3 t0 t1 CpuTestError
   check_SHAD_register $4 t0 t1 CpuTestError
   check_SHAD_register $5 t0 t1 CpuTestError
   check_SHAD_register $6 t0 t1 CpuTestError
   check_SHAD_register $7 t0 t1 CpuTestError
   check_SHAD_register $8 t0 t1 CpuTestError
   check_SHAD_register $9 t0 t1 CpuTestError
   check_SHAD_register $10 t0 t1 CpuTestError
   check_SHAD_register $11 t0 t1 CpuTestError
   check_SHAD_register $12 t0 t1 CpuTestError
   check_SHAD_register $13 t0 t1 CpuTestError
   check_SHAD_register $14 t0 t1 CpuTestError
   check_SHAD_register $15 t0 t1 CpuTestError
   check_SHAD_register $16 t0 t1 CpuTestError
   check_SHAD_register $17 t0 t1 CpuTestError
   check_SHAD_register $18 t0 t1 CpuTestError
   check_SHAD_register $19 t0 t1 CpuTestError
   check_SHAD_register $20 t0 t1 CpuTestError
   check_SHAD_register $21 t0 t1 CpuTestError
   check_SHAD_register $22 t0 t1 CpuTestError
   check_SHAD_register $23 t0 t1 CpuTestError
   check_SHAD_register $24 t0 t1 CpuTestError
   check_SHAD_register $25 t0 t1 CpuTestError
   check_SHAD_register $26 t0 t1 CpuTestError
   check_SHAD_register $27 t0 t1 CpuTestError
   check_SHAD_register $28 t0 t1 CpuTestError
   check_SHAD_register $29 t0 t1 CpuTestError
   check_SHAD_register $30 t0 t1 CpuTestError
   check_SHAD_register $31 t0 t1 CpuTestError
#endif   
#if (__PIC32_SRS_SET_COUNT >= 1)
#;*********************************
#; Test shadow register set 1
   set_shadow_file 1 t0 t1
   check_SHAD_register $0 t0 t1 CpuTestError
   check_SHAD_register $1 t0 t1 CpuTestError
   check_SHAD_register $2 t0 t1 CpuTestError
   check_SHAD_register $3 t0 t1 CpuTestError
   check_SHAD_register $4 t0 t1 CpuTestError
   check_SHAD_register $5 t0 t1 CpuTestError
   check_SHAD_register $6 t0 t1 CpuTestError
   check_SHAD_register $7 t0 t1 CpuTestError
   check_SHAD_register $8 t0 t1 CpuTestError
   check_SHAD_register $9 t0 t1 CpuTestError
   check_SHAD_register $10 t0 t1 CpuTestError
   check_SHAD_register $11 t0 t1 CpuTestError
   check_SHAD_register $12 t0 t1 CpuTestError
   check_SHAD_register $13 t0 t1 CpuTestError
   check_SHAD_register $14 t0 t1 CpuTestError
   check_SHAD_register $15 t0 t1 CpuTestError
   check_SHAD_register $16 t0 t1 CpuTestError
   check_SHAD_register $17 t0 t1 CpuTestError
   check_SHAD_register $18 t0 t1 CpuTestError
   check_SHAD_register $19 t0 t1 CpuTestError
   check_SHAD_register $20 t0 t1 CpuTestError
   check_SHAD_register $21 t0 t1 CpuTestError
   check_SHAD_register $22 t0 t1 CpuTestError
   check_SHAD_register $23 t0 t1 CpuTestError
   check_SHAD_register $24 t0 t1 CpuTestError
   check_SHAD_register $25 t0 t1 CpuTestError
   check_SHAD_register $26 t0 t1 CpuTestError
   check_SHAD_register $27 t0 t1 CpuTestError
   check_SHAD_register $28 t0 t1 CpuTestError
   check_SHAD_register $29 t0 t1 CpuTestError
   check_SHAD_register $30 t0 t1 CpuTestError
   check_SHAD_register $31 t0 t1 CpuTestError
#endif   
   
#if (__PIC32_SRS_SET_COUNT >= 2)
#;*********************************
#; Test shadow register set 2
   set_shadow_file 2 t0 t1
   check_SHAD_register $0 t0 t1 CpuTestError
   check_SHAD_register $1 t0 t1 CpuTestError
   check_SHAD_register $2 t0 t1 CpuTestError
   check_SHAD_register $3 t0 t1 CpuTestError
   check_SHAD_register $4 t0 t1 CpuTestError
   check_SHAD_register $5 t0 t1 CpuTestError
   check_SHAD_register $6 t0 t1 CpuTestError
   check_SHAD_register $7 t0 t1 CpuTestError
   check_SHAD_register $8 t0 t1 CpuTestError
   check_SHAD_register $9 t0 t1 CpuTestError
   check_SHAD_register $10 t0 t1 CpuTestError
   check_SHAD_register $11 t0 t1 CpuTestError
   check_SHAD_register $12 t0 t1 CpuTestError
   check_SHAD_register $13 t0 t1 CpuTestError
   check_SHAD_register $14 t0 t1 CpuTestError
   check_SHAD_register $15 t0 t1 CpuTestError
   check_SHAD_register $16 t0 t1 CpuTestError
   check_SHAD_register $17 t0 t1 CpuTestError
   check_SHAD_register $18 t0 t1 CpuTestError
   check_SHAD_register $19 t0 t1 CpuTestError
   check_SHAD_register $20 t0 t1 CpuTestError
   check_SHAD_register $21 t0 t1 CpuTestError
   check_SHAD_register $22 t0 t1 CpuTestError
   check_SHAD_register $23 t0 t1 CpuTestError
   check_SHAD_register $24 t0 t1 CpuTestError
   check_SHAD_register $25 t0 t1 CpuTestError
   check_SHAD_register $26 t0 t1 CpuTestError
   check_SHAD_register $27 t0 t1 CpuTestError
   check_SHAD_register $28 t0 t1 CpuTestError
   check_SHAD_register $29 t0 t1 CpuTestError
   check_SHAD_register $30 t0 t1 CpuTestError
   check_SHAD_register $31 t0 t1 CpuTestError
#endif   
#if (__PIC32_SRS_SET_COUNT >= 3)
#;*********************************
#; Test shadow register set 3
   set_shadow_file 3 t0 t1
   check_SHAD_register $0 t0 t1 CpuTestError
   check_SHAD_register $1 t0 t1 CpuTestError
   check_SHAD_register $2 t0 t1 CpuTestError
   check_SHAD_register $3 t0 t1 CpuTestError
   check_SHAD_register $4 t0 t1 CpuTestError
   check_SHAD_register $5 t0 t1 CpuTestError
   check_SHAD_register $6 t0 t1 CpuTestError
   check_SHAD_register $7 t0 t1 CpuTestError
   check_SHAD_register $8 t0 t1 CpuTestError
   check_SHAD_register $9 t0 t1 CpuTestError
   check_SHAD_register $10 t0 t1 CpuTestError
   check_SHAD_register $11 t0 t1 CpuTestError
   check_SHAD_register $12 t0 t1 CpuTestError
   check_SHAD_register $13 t0 t1 CpuTestError
   check_SHAD_register $14 t0 t1 CpuTestError
   check_SHAD_register $15 t0 t1 CpuTestError
   check_SHAD_register $16 t0 t1 CpuTestError
   check_SHAD_register $17 t0 t1 CpuTestError
   check_SHAD_register $18 t0 t1 CpuTestError
   check_SHAD_register $19 t0 t1 CpuTestError
   check_SHAD_register $20 t0 t1 CpuTestError
   check_SHAD_register $21 t0 t1 CpuTestError
   check_SHAD_register $22 t0 t1 CpuTestError
   check_SHAD_register $23 t0 t1 CpuTestError
   check_SHAD_register $24 t0 t1 CpuTestError
   check_SHAD_register $25 t0 t1 CpuTestError
   check_SHAD_register $26 t0 t1 CpuTestError
   check_SHAD_register $27 t0 t1 CpuTestError
   check_SHAD_register $28 t0 t1 CpuTestError
   check_SHAD_register $29 t0 t1 CpuTestError
   check_SHAD_register $30 t0 t1 CpuTestError
   check_SHAD_register $31 t0 t1 CpuTestError
#endif   
#if (__PIC32_SRS_SET_COUNT >= 4)
#;*********************************
#; Test shadow register set 4
   set_shadow_file 4 t0 t1
   check_SHAD_register $0 t0 t1 CpuTestError
   check_SHAD_register $1 t0 t1 CpuTestError
   check_SHAD_register $2 t0 t1 CpuTestError
   check_SHAD_register $3 t0 t1 CpuTestError
   check_SHAD_register $4 t0 t1 CpuTestError
   check_SHAD_register $5 t0 t1 CpuTestError
   check_SHAD_register $6 t0 t1 CpuTestError
   check_SHAD_register $7 t0 t1 CpuTestError
   check_SHAD_register $8 t0 t1 CpuTestError
   check_SHAD_register $9 t0 t1 CpuTestError
   check_SHAD_register $10 t0 t1 CpuTestError
   check_SHAD_register $11 t0 t1 CpuTestError
   check_SHAD_register $12 t0 t1 CpuTestError
   check_SHAD_register $13 t0 t1 CpuTestError
   check_SHAD_register $14 t0 t1 CpuTestError
   check_SHAD_register $15 t0 t1 CpuTestError
   check_SHAD_register $16 t0 t1 CpuTestError
   check_SHAD_register $17 t0 t1 CpuTestError
   check_SHAD_register $18 t0 t1 CpuTestError
   check_SHAD_register $19 t0 t1 CpuTestError
   check_SHAD_register $20 t0 t1 CpuTestError
   check_SHAD_register $21 t0 t1 CpuTestError
   check_SHAD_register $22 t0 t1 CpuTestError
   check_SHAD_register $23 t0 t1 CpuTestError
   check_SHAD_register $24 t0 t1 CpuTestError
   check_SHAD_register $25 t0 t1 CpuTestError
   check_SHAD_register $26 t0 t1 CpuTestError
   check_SHAD_register $27 t0 t1 CpuTestError
   check_SHAD_register $28 t0 t1 CpuTestError
   check_SHAD_register $29 t0 t1 CpuTestError
   check_SHAD_register $30 t0 t1 CpuTestError
   check_SHAD_register $31 t0 t1 CpuTestError
#endif   
   
#if (__PIC32_SRS_SET_COUNT >= 5)
#;*********************************
#; Test shadow register set 5
   set_shadow_file 5 t0 t1
   check_SHAD_register $0 t0 t1 CpuTestError
   check_SHAD_register $1 t0 t1 CpuTestError
   check_SHAD_register $2 t0 t1 CpuTestError
   check_SHAD_register $3 t0 t1 CpuTestError
   check_SHAD_register $4 t0 t1 CpuTestError
   check_SHAD_register $5 t0 t1 CpuTestError
   check_SHAD_register $6 t0 t1 CpuTestError
   check_SHAD_register $7 t0 t1 CpuTestError
   check_SHAD_register $8 t0 t1 CpuTestError
   check_SHAD_register $9 t0 t1 CpuTestError
   check_SHAD_register $10 t0 t1 CpuTestError
   check_SHAD_register $11 t0 t1 CpuTestError
   check_SHAD_register $12 t0 t1 CpuTestError
   check_SHAD_register $13 t0 t1 CpuTestError
   check_SHAD_register $14 t0 t1 CpuTestError
   check_SHAD_register $15 t0 t1 CpuTestError
   check_SHAD_register $16 t0 t1 CpuTestError
   check_SHAD_register $17 t0 t1 CpuTestError
   check_SHAD_register $18 t0 t1 CpuTestError
   check_SHAD_register $19 t0 t1 CpuTestError
   check_SHAD_register $20 t0 t1 CpuTestError
   check_SHAD_register $21 t0 t1 CpuTestError
   check_SHAD_register $22 t0 t1 CpuTestError
   check_SHAD_register $23 t0 t1 CpuTestError
   check_SHAD_register $24 t0 t1 CpuTestError
   check_SHAD_register $25 t0 t1 CpuTestError
   check_SHAD_register $26 t0 t1 CpuTestError
   check_SHAD_register $27 t0 t1 CpuTestError
   check_SHAD_register $28 t0 t1 CpuTestError
   check_SHAD_register $29 t0 t1 CpuTestError
   check_SHAD_register $30 t0 t1 CpuTestError
   check_SHAD_register $31 t0 t1 CpuTestError
#endif   
   
#if (__PIC32_SRS_SET_COUNT >= 6)
#;*********************************
#; Test shadow register set 6
   set_shadow_file 6 t0 t1
   check_SHAD_register $0 t0 t1 CpuTestError
   check_SHAD_register $1 t0 t1 CpuTestError
   check_SHAD_register $2 t0 t1 CpuTestError
   check_SHAD_register $3 t0 t1 CpuTestError
   check_SHAD_register $4 t0 t1 CpuTestError
   check_SHAD_register $5 t0 t1 CpuTestError
   check_SHAD_register $6 t0 t1 CpuTestError
   check_SHAD_register $7 t0 t1 CpuTestError
   check_SHAD_register $8 t0 t1 CpuTestError
   check_SHAD_register $9 t0 t1 CpuTestError
   check_SHAD_register $10 t0 t1 CpuTestError
   check_SHAD_register $11 t0 t1 CpuTestError
   check_SHAD_register $12 t0 t1 CpuTestError
   check_SHAD_register $13 t0 t1 CpuTestError
   check_SHAD_register $14 t0 t1 CpuTestError
   check_SHAD_register $15 t0 t1 CpuTestError
   check_SHAD_register $16 t0 t1 CpuTestError
   check_SHAD_register $17 t0 t1 CpuTestError
   check_SHAD_register $18 t0 t1 CpuTestError
   check_SHAD_register $19 t0 t1 CpuTestError
   check_SHAD_register $20 t0 t1 CpuTestError
   check_SHAD_register $21 t0 t1 CpuTestError
   check_SHAD_register $22 t0 t1 CpuTestError
   check_SHAD_register $23 t0 t1 CpuTestError
   check_SHAD_register $24 t0 t1 CpuTestError
   check_SHAD_register $25 t0 t1 CpuTestError
   check_SHAD_register $26 t0 t1 CpuTestError
   check_SHAD_register $27 t0 t1 CpuTestError
   check_SHAD_register $28 t0 t1 CpuTestError
   check_SHAD_register $29 t0 t1 CpuTestError
   check_SHAD_register $30 t0 t1 CpuTestError
   check_SHAD_register $31 t0 t1 CpuTestError
#endif   
   
#if (__PIC32_SRS_SET_COUNT >= 7)
#;*********************************
#; Test shadow register set 7
   set_shadow_file 7 t0 t1
   check_SHAD_register $0 t0 t1 CpuTestError
   check_SHAD_register $1 t0 t1 CpuTestError
   check_SHAD_register $2 t0 t1 CpuTestError
   check_SHAD_register $3 t0 t1 CpuTestError
   check_SHAD_register $4 t0 t1 CpuTestError
   check_SHAD_register $5 t0 t1 CpuTestError
   check_SHAD_register $6 t0 t1 CpuTestError
   check_SHAD_register $7 t0 t1 CpuTestError
   check_SHAD_register $8 t0 t1 CpuTestError
   check_SHAD_register $9 t0 t1 CpuTestError
   check_SHAD_register $10 t0 t1 CpuTestError
   check_SHAD_register $11 t0 t1 CpuTestError
   check_SHAD_register $12 t0 t1 CpuTestError
   check_SHAD_register $13 t0 t1 CpuTestError
   check_SHAD_register $14 t0 t1 CpuTestError
   check_SHAD_register $15 t0 t1 CpuTestError
   check_SHAD_register $16 t0 t1 CpuTestError
   check_SHAD_register $17 t0 t1 CpuTestError
   check_SHAD_register $18 t0 t1 CpuTestError
   check_SHAD_register $19 t0 t1 CpuTestError
   check_SHAD_register $20 t0 t1 CpuTestError
   check_SHAD_register $21 t0 t1 CpuTestError
   check_SHAD_register $22 t0 t1 CpuTestError
   check_SHAD_register $23 t0 t1 CpuTestError
   check_SHAD_register $24 t0 t1 CpuTestError
   check_SHAD_register $25 t0 t1 CpuTestError
   check_SHAD_register $26 t0 t1 CpuTestError
   check_SHAD_register $27 t0 t1 CpuTestError
   check_SHAD_register $28 t0 t1 CpuTestError
   check_SHAD_register $29 t0 t1 CpuTestError
   check_SHAD_register $30 t0 t1 CpuTestError
   check_SHAD_register $31 t0 t1 CpuTestError
#endif   
   
#if (__PIC32_SRS_SET_COUNT >= 8)
#;*********************************
#; Test shadow register set 8
   set_shadow_file 8 t0 t1
   check_SHAD_register $0 t0 t1 CpuTestError
   check_SHAD_register $1 t0 t1 CpuTestError
   check_SHAD_register $2 t0 t1 CpuTestError
   check_SHAD_register $3 t0 t1 CpuTestError
   check_SHAD_register $4 t0 t1 CpuTestError
   check_SHAD_register $5 t0 t1 CpuTestError
   check_SHAD_register $6 t0 t1 CpuTestError
   check_SHAD_register $7 t0 t1 CpuTestError
   check_SHAD_register $8 t0 t1 CpuTestError
   check_SHAD_register $9 t0 t1 CpuTestError
   check_SHAD_register $10 t0 t1 CpuTestError
   check_SHAD_register $11 t0 t1 CpuTestError
   check_SHAD_register $12 t0 t1 CpuTestError
   check_SHAD_register $13 t0 t1 CpuTestError
   check_SHAD_register $14 t0 t1 CpuTestError
   check_SHAD_register $15 t0 t1 CpuTestError
   check_SHAD_register $16 t0 t1 CpuTestError
   check_SHAD_register $17 t0 t1 CpuTestError
   check_SHAD_register $18 t0 t1 CpuTestError
   check_SHAD_register $19 t0 t1 CpuTestError
   check_SHAD_register $20 t0 t1 CpuTestError
   check_SHAD_register $21 t0 t1 CpuTestError
   check_SHAD_register $22 t0 t1 CpuTestError
   check_SHAD_register $23 t0 t1 CpuTestError
   check_SHAD_register $24 t0 t1 CpuTestError
   check_SHAD_register $25 t0 t1 CpuTestError
   check_SHAD_register $26 t0 t1 CpuTestError
   check_SHAD_register $27 t0 t1 CpuTestError
   check_SHAD_register $28 t0 t1 CpuTestError
   check_SHAD_register $29 t0 t1 CpuTestError
   check_SHAD_register $30 t0 t1 CpuTestError
   check_SHAD_register $31 t0 t1 CpuTestError
#endif   
#endif
   
#if defined (__PIC32_HAS_DSPR2)
#;*********************************
#; Test $hi, $lo of the DSP accumulator 
#; registers AC0 - AC3
   check_DSP_accumulator $ac0 t0 t1 CpuTestError
   check_DSP_accumulator $ac1 t0 t1 CpuTestError
   check_DSP_accumulator $ac2 t0 t1 CpuTestError
   check_DSP_accumulator $ac3 t0 t1 CpuTestError
#endif
   
   
#;*********************************
# lastly registers 29 $sp needs to be preserved (saved in $t0)
    move t0, sp;          # save $sp       
    check_register sp SpTestError
    move sp, t0;          # restore $sp 

CpuTestSuccess:
    b       CpuTestDone;     # done
    addiu   v0, zero, CPU_REGISTER_TEST_PASS   #return Success code (Branch slot)
    
SpTestError:
    move sp, t0           # restore $sp
    #;  fall through
CpuTestError:
    addiu   v0, zero, CPU_REGISTER_TEST_FAIL   #return Error code

CpuTestDone:
#;  restore saved regs
    lw      fp, 0(sp)
    lw      gp, 4(sp)
    lw      ra, 8(sp)
    lw      s0, 12(sp)
    lw      s1, 16(sp)
    lw      s2, 20(sp)
    lw      s3, 24(sp)
    lw      s4, 28(sp)
    lw      s5, 32(sp)
    lw      s6, 36(sp)
    lw      s7, 40(sp)
    addiu   sp, sp, 44

#   return
    jr ra  
    nop


.end CLASSB_CPURegistersTest

.set	at
.set	reorder



