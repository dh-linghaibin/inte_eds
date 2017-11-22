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
    ADD     R1, R0, #4    ;pNominee
    ADD     R0, R0, #8    ;pCurrent

; ����Nominee״̬Ϊ����
    LDR     R3,  [R1]     ;Nominee
    MOV     R2,  #0x1     ;enum��  OsThreadObjectRunning
    STR     R2,  [R3,#4]  ;offset��Thread status 
	
; ���uThreadCurrent��uThreadNominee�������Ҫ����Ĵ�����ջ��
    LDR     R2,  [R0] ;Current
    CMP     R2,  R3
    BEQ     LOAD_NOMINEE_FILE
	
STORE_CURRENT_FILE    
    MRS     R3,  PSP      ;Current psp
    SUBS    R3,  R3, #0x20
    STM     R3,  {R4-R11} ;����r4-r11��CurrentThreadջ��
    STR     R3,  [R2,#8]  ;����psp��CurrentThread�߳̽ṹ

    ; ʹ��CurrentThread = NomineeThread;
    LDR     R3,  [R1]     ;Nominee
    STR     R3,  [R0]     ;Current

LOAD_NOMINEE_FILE
    LDR     R3,  [R3,#8]   ; ����CurrentThread��ȡ��SP��ֵ��R3
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

    ;�ں˱�֤����API�������ж��б�����
    ; (1)����ǰ�̴߳Ӿ����������Ƴ���API��
    ; (2)������ǰ�̻߳������������߳����ȼ���API
    
    ;�����ڴ�ʱ�������ж�ISR�е�����Щ 
    ; (1)�����˸������ȼ����̣߳�
    ; (2)ϵͳ��ʱ���жϣ�����ʱ��Ƭ��ת
    ;��ô�п��ܵ���һ���µ�PensSv�������롣
    ;���������������쳣��������ʱ���ᷢ��ǰ������PendSVҧβ�жϡ�
    
    ;����PendSV_Handler�����̣���ǰ�̵߳�����������Щ����ļĴ������ٴα����浽�߳�ջ�У���������
    ;��ջ��Ҳ����˵��һ���߳��������л���Ч��������ȡ����ת��ִ�еڶ��ε��߳��������л���

    ; �����쳣�������̣�����r0��r1��r2��r3�Ĵ������л�������
    BX      LR
    ; ���غ󣬴�����ʹ���߳�+��Ȩģʽ+psp���߳̾������ֻ��������С�

    END