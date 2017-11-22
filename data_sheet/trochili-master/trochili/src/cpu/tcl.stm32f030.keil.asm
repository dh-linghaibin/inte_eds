	    IMPORT  OsKernelVariable

        EXPORT  OsCpuDisableInt
        EXPORT  OsCpuEnableInt
        EXPORT  OsCpuEnterCritical
        EXPORT  OsCpuLeaveCritical
        EXPORT  OsCpuLoadRootThread
        EXPORT  PendSV_Handler

        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8


OsCpuDisableInt
        CPSID   I
        BX      LR

OsCpuEnableInt
        CPSIE   I
        BX      LR

OsCpuEnterCritical
    MRS     R1, PRIMASK
    STR     R1, [R0]
    CPSID   I
    BX      LR

OsCpuLeaveCritical
    MSR     PRIMASK, R0
    BX      LR


NVIC_INT_CTRL2   EQU     0xE000ED04                              ; Interrupt control state register.
NVIC_SYSPRI142   EQU     0xE000ED20                              ; System priority register (priority 14).
NVIC_PENDSV_PRI2 EQU     0x00FF0000                              ; PendSV priority value (lowest).
NVIC_PENDSVSET2  EQU     0x10000000                              ; Value to trigger PendSV exception.


OsCpuLoadRootThread
    MOVS    R0, #0
    MSR     PSP, R0

    LDR     R0, =NVIC_SYSPRI142                                ; Set the PendSV exception priority
    LDR     R1, =NVIC_PENDSV_PRI2
    STR     R1, [R0]                                            ; Not storing entire byte with STRB to avoid error


    LDR     R0, =NVIC_INT_CTRL2
    LDR     R1, =NVIC_PENDSVSET2
    STR     R1, [R0]

    CPSIE   I

NeverHere
    B       NeverHere


;Cortex-M3�����쳣��������ʱ,�Զ�ѹջ��R0-R3,R12,LR(R14,���ӼĴ���),PSR(����״̬�Ĵ���)��PC(R15).
;PSP���Զ�ѹջ������Ҫ���浽ջ�У����Ǳ��浽�߳̽ṹ��

;CpuSwitchThread
PendSV_Handler
    CPSID   I

; ȡ���߳�����
	LDR     R0, =OsKernelVariable
	ADDS    R1, R0, #4;Nominee
	ADDS    R0, R0, #8;Current
	
; ���uThreadCurrent��uThreadNominee�������Ҫ����Ĵ�����ջ��
    LDR     R2,  [R0]
    LDR     R3,  [R1]
	CMP     R2,  R3
	BEQ     LOAD_NOMINEE_FILE
	
; ���uThreadCurrent�߳�û�б���ʼ������Ҫ����Ĵ�����ջ��
    LDR     R3,  [R2,#0]
	MOVS    R2,  #1
	ANDS    R3,  R3, R2
	CMP     R3,  #0
	BEQ     SWAP_THREAD
    ;CBZ     R3,  SWAP_THREAD
	
STORE_CURRENT_FILE   
    MRS     R3, PSP
    SUBS    R3, R3, #0x20
	;����r4-r11��uThreadCurrentջ��
	STR     R4, [R3,#0x0]
	STR     R5, [R3,#0x4]
	STR     R6, [R3,#0x8]
	STR     R7, [R3,#0xc]	
	MOV     R4, R8
	MOV     R4, R9
	MOV     R4, R10
	MOV     R4, R11
	STR     R4, [R3,#0x10]
	STR     R5, [R3,#0x14]
	STR     R6, [R3,#0x18]
	STR     R7, [R3,#0x1c]
   
	;����psp��uThreadCurrent�߳̽ṹ
    LDR     R2,  [R0]
    STR     R3,  [R2,#4]  
	
;STORE_CURRENT_FILE2    
    ;MRS     R3,  PSP
    ;SUBS    R3,  R3, #0x10
    ;STM     R3!,  {R4-R7} ;����r4-r11��uThreadCurrentջ��
    ;SUBS    R0, R0, #0x10
    ;LDR     R2,  [R0]
    ;STR     R3,  [R2,#4]  ;����psp��uThreadCurrent�߳̽ṹ
SWAP_THREAD                ; ʹ��uThreadCurrent = uThreadNominee;
    LDR     R3,  [R1]
    STR     R3,  [R0]

;LOAD_NOMINEE_FILE
    ;LDR     R3,  [R3,#4]   ; ����uThreadNormine��ȡ��SP��ֵ��R3
    ;LDM     R3!,  {R4-R7}  ; �����߳�ջ�е��� r4-11
    ;ADDS    R3,  R3, #0x20 ; pspָ���ж��Զ�ѹջ���ջ��
LOAD_NOMINEE_FILE
   ; ����uThreadNormine��ȡ��SP��ֵ��R3
    LDR     R3, [R3,#4]   
	; �����߳�ջ�е��� r4-11
    LDR     R4, [R3,#0x10]
	LDR     R5, [R3,#0x14]
	LDR     R6, [R3,#0x18]
    LDR     R7, [R3,#0x1c]
    MOV     R8, R4
	MOV     R9, R5
	MOV     R10,R6
	MOV     R11,R7
	LDR     R4, [R3,#0x0]
	LDR     R5, [R3,#0x4]
	LDR     R6, [R3,#0x8]
	LDR     R7, [R3,#0xc]
    ; pspָ���ж��Զ�ѹջ���ջ��	
    ADDS    R3,  R3, #0x20 
    MSR     PSP, R3

    ; �ϵ�󣬴����������߳�+��Ȩģʽ+msp��
    ; ���ڵ�һ��activate���񣬵�����pendsv�жϺ󣬴���������handlerģʽ��ʹ��msp,
    ; ����ʱ��������׼��ʹ��psp����psp�е���r0...��Щ�Ĵ�����������Ҫ�޸�LR��ǿ��ʹ��psp��
    MOVS    R2, #0x4
    MOV     R3, R14
    ORRS    R3, R3, R2
	MOV     R14, R3
    CPSIE   I
    
    ;�������п��ܷ����жϣ�����ʱ�µĵ�ǰ�̵߳������Ĳ�û����ȫ�ָ������̱߳��жϵ��龰���ƣ�
    ;Ӳ���Զ����沿�ּĴ������߳�ջ�У������Ĵ����������ڴ������������С�
    ;�����ڴ�ʱ�������ж�ISR�е�����Щ
    ; (1)�Ὣ��ǰ�̴߳Ӿ����������Ƴ���API��
    ; (2)���߻����˸������ȼ����̣߳�
    ; (3)������ǰ�̻߳������������̵߳����ȼ�
    ; (4)ϵͳ��ʱ���жϣ�����ʱ��Ƭ��ת
    ;��ô�п��ܵ���һ���µ�PensSv���󱻹���
    ;���������������쳣��������ʱ���ᷢ��ǰ������PendSVҧβ�жϡ�
    ;����uCM3PendSVHandler�����̣���ǰ�̵߳�����������Щ����ļĴ������ٴα����浽�߳�ջ�У���������
    ;��ջ��Ҳ����˵��һ���߳��������л���ǿ��ȡ���ˣ�ת��ִ�еڶ��ε��߳��������л���

    ; �����쳣�������̣�����r0��r1��r2��r3�Ĵ������л�������
    BX      LR
    ; ���غ󣬴�����ʹ���߳�+��Ȩģʽ+psp���߳̾������ֻ��������С�

    END