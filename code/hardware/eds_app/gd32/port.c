/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

//__asm void PendSV_Handler() {
//	CPSID   I                                                   ; Prevent interruption during context switch
//    MRS     R0, PSP                                             ; PSP is process stack pointer
//    CBZ     R0, PendSV_Handler_nosave                     	; Skip register save the first time

//    SUBS    R0, R0, #0x20                                       ; Save remaining regs r4-11 on process stack
//    STM     R0, {R4-R11}

//    LDR     R1, =curTaskPtr                                     ; curTask->sp = SP;
//    LDR     R1, [R1]
//    STR     R0, [R1]                                            ; R0 is SP of process being switched out
//}


//__asm void PendSV_Handler_nosave() {
//    LDR     R0, =curPid                                     	; curPid = nextPid
//    LDR     R1, =nextPid
//    LDRB    R2, [R1]
//    STRB    R2, [R0]

//    LDR     R0, =curTaskPtr                                     ; curTaskPtr = nextTaskPtr
//    LDR     R1, =nextTaskPtr
//    LDR     R2, [R1]
//    STR     R2, [R0]

//    LDR     R0, [R2]                                            ; R0 is new process SP; SP = OSTCBHighRdy->OSTCBStkPtr;
//    LDM     R0, {R4-R11}                                        ; Restore r4-11 from new process stack
//    ADDS    R0, R0, #0x20
//    MSR     PSP, R0                                             ; Load PSP with new process SP
//    ORR     LR, LR, #0x04                                       ; Ensure exception return uses process stack
//    CPSIE   I
//    BX      LR                                                  ; Exception return will restore remaining context
//}