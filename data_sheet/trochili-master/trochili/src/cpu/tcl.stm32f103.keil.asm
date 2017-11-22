	    IMPORT  OsKernelVariable

        EXPORT  OsCpuDisableInt
        EXPORT  OsCpuEnableInt
        EXPORT  OsCpuEnterCritical
        EXPORT  OsCpuLeaveCritical
        EXPORT  OsCpuCalcHiPRIO
        EXPORT  PendSV_Handler

        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

OsCpuCalcHiPRIO
        RBIT    R0, R0
        CLZ     R0, R0
        BX      LR

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


;Cortex-M3�����쳣��������ʱ,�Զ�ѹջ��R0-R3,R12,LR(R14,���ӼĴ���),PSR(����״̬�Ĵ���)��PC(R15).
;PSP���Զ�ѹջ������Ҫ���浽ջ�У����Ǳ��浽�߳̽ṹ��

PendSV_Handler
    CPSID   I

; ȡ���߳�����
	LDR     R0,  =OsKernelVariable
	ADD     R1, R0, #4;Nominee
	ADD     R0, R0, #8;Current
	
; ���uThreadCurrent��uThreadNominee�������Ҫ����Ĵ�����ջ��
    LDR     R2,  [R0]
    LDR     R3,  [R1]
	CMP     R2,  R3
	BEQ     LOAD_NOMINEE_FILE
	
; ���uThreadCurrent�߳�û�б���ʼ������Ҫ����Ĵ�����ջ��
    LDR     R3,  [R2,#0]
	AND     R3,  R3, #0x1
    CBZ     R3,  SWAP_THREAD
	
STORE_CURRENT_FILE    
    MRS     R3,  PSP
    SUBS    R3,  R3, #0x20
    STM     R3,  {R4-R11} ;����r4-r11��uThreadCurrentջ��
    STR     R3,  [R2,#4]  ;����psp��uThreadCurrent�߳̽ṹ

SWAP_THREAD                ; ʹ��uThreadCurrent = uThreadNominee;
    LDR     R3,  [R1]
    STR     R3,  [R0]

LOAD_NOMINEE_FILE
    LDR     R3,  [R3,#4]   ; ����uThreadCurrent��ȡ��SP��ֵ��R0
    LDM     R3,  {R4-R11}  ; �����߳�ջ�е��� r4-11
    ADDS    R3,  R3, #0x20 ; pspָ���ж��Զ�ѹջ���ջ��
    MSR     PSP, R3

    ; �ϵ�󣬴����������߳�+��Ȩģʽ+msp��
    ; ���ڵ�һ��activate���񣬵�����pendsv�жϺ󣬴���������handlerģʽ��ʹ��msp,
    ; ����ʱ��������׼��ʹ��psp����psp�е���r0...��Щ�Ĵ�����������Ҫ�޸�LR��ǿ��ʹ��psp��
    ORR     LR, LR, #0x04
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