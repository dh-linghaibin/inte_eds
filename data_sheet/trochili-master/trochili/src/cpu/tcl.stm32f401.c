/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.kernel.h"
#include "tcl.stm32f401.h"

/* SysTick Ctrl & Status Reg.          */
#define CM3_SYSTICK_CTRL     (0xE000E010)
#define CM3_SYSTICK_CLKSRC   (0x00000004)   /* Clock Source.                    */
#define CM3_SYSTICK_INTEN    (0x00000002)   /* Interrupt enable.                */
#define CM3_SYSTICK_ENABLE   (0x00000001)   /* Counter mode.                    */

/* SysTick Reload  Value Reg.          */
#define CM3_SYSTICK_RELOAD   (0xE000E014)

/* SysTick Current Value Reg.          */
#define CM3_SYSTICK_CURRENT  (0xE000E018)

/* SysTick Cal     Value Reg.          */
#define CM3_SYSTICK_TENMS    (0xE000E01C)

/* Interrupt control & state register. */
#define CM3_ICSR             (0xE000ED04)
#define CM3_ICSR_PENDSVSET   (0x1<<28)       /* Value to trigger PendSV exception.  */
#define CM3_ICSR_PENDSVCLR   (0x1<<27)       /* Value to clear PendSV exception.    */
#define CM3_ICSR_PENDSTSET   (0x1<<26)       /* Value to trigger PendST exception.  */
#define CM3_ICSR_PENDSTCLR   (0x1<<25)       /* Value to clear PendST exception.    */

/* PendSV priority register            */
#define CM3_PRIO_PENDSV      (0xE000ED22)
#define CM3_PENDSV_PRIORITY  (0xFF)


/*************************************************************************************************
 *  ���ܣ������ں˽��Ķ�ʱ��                                                                     *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsCpuStartTickClock(void)
{
    /* ��ʼ��systick��ʱ�� */
    TBase32 value = TCLC_CPU_CLOCK_FREQ / TCLC_TIME_TICK_RATE;
    TCLM_SET_REG32(CM3_SYSTICK_RELOAD, value - 1U);
    TCLM_SET_REG32(CM3_SYSTICK_CTRL, CM3_SYSTICK_CLKSRC|CM3_SYSTICK_INTEN|CM3_SYSTICK_ENABLE);
}

/*************************************************************************************************
 *  ���ܣ��ں˼��ص�һ���߳�                                                                     *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsCpuLoadRootThread()
{
    TCLM_SET_REG32(CM3_ICSR, CM3_ICSR_PENDSVSET);
}


/*************************************************************************************************
 *  ���ܣ������̵߳���                                                                           *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsCpuConfirmThreadSwitch(void)
{
    TCLM_SET_REG32(CM3_ICSR, CM3_ICSR_PENDSVSET);
}


/*************************************************************************************************
 *  ���ܣ�ȡ���̵߳���                                                                           *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsCpuCancelThreadSwitch(void)
{
    TCLM_SET_REG32(CM3_ICSR, CM3_ICSR_PENDSVCLR);
}


/*************************************************************************************************
 *  ���ܣ��߳�ջ��ջ֡��ʼ������                                                                 *
 *  ������(1) pTop      �߳�ջ����ַ                                                             *
 *        (2) pStack    �߳�ջ�׵�ַ                                                             *
 *        (3) bytes     �߳�ջ��С���Խ�Ϊ��λ                                                   *
 *        (4) pEntry    �̺߳�����ַ                                                             *
 *        (5) pData     �̺߳�������                                                             *
 *  ���أ���                                                                                     *
 *  ˵�����߳�ջ��ʼ��ַ����4�ֽڶ���                                                            *
 *************************************************************************************************/
void OsCpuBuildThreadStack(TAddr32* pTop, void* pStack, TBase32 bytes,
                         void* pEntry, TArgument argument)
{
    TReg32* pTemp;
    pTemp = (TReg32*)((TBase32)pStack + bytes);

    /* α�촦�����ж�ջ�ֳ������̵߳�һ�α���������ʱʹ�á�
       ע��LR��ֵ�Ǹ��Ƿ�ֵ����������߳�û��ͨ��LR�˳� */
    *(--pTemp) = (TReg32)0x01000000;    /* PSR                     */
    *(--pTemp) = (TReg32)pEntry;         /* �̺߳���                */
    *(--pTemp) = (TReg32)0xFFFFFFFE;    /* R14 (LR)                */
    *(--pTemp) = (TReg32)0x12121212;    /* R12                     */
    *(--pTemp) = (TReg32)0x03030303;    /* R3                      */
    *(--pTemp) = (TReg32)0x02020202;    /* R2                      */
    *(--pTemp) = (TReg32)0x01010101;    /* R1                      */
    *(--pTemp) = (TReg32)argument;       /* R0, �̲߳���            */

    /* ��ʼ���ڴ�����Ӳ���ж�ʱ�����Զ�������߳������ģ�
       �⼸���Ĵ�����ֵû��ʲô����,�����ں˵�ָ�ư� */
    *(--pTemp) = (TReg32)0x00000054;    /* R11 ,T                  */
    *(--pTemp) = (TReg32)0x00000052;    /* R10 ,R                  */
    *(--pTemp) = (TReg32)0x0000004F;    /* R9  ,O                  */
    *(--pTemp) = (TReg32)0x00000043;    /* R8  ,C                  */
    *(--pTemp) = (TReg32)0x00000048;    /* R7  ,H                  */
    *(--pTemp) = (TReg32)0x00000049;    /* R6  ,I                  */
    *(--pTemp) = (TReg32)0x0000004C;    /* R5  ,L                  */
    *(--pTemp) = (TReg32)0x00000049;    /* R4  ,I                  */

    *pTop = (TReg32)pTemp;
}


/*************************************************************************************************
 *  ���ܣ���ʼ��������                                                                           *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsCpuSetupEntry(void)
{
    /* ����PENDSV�ж����ȼ� */
    TCLM_SET_REG32(CM3_PRIO_PENDSV, CM3_PENDSV_PRIORITY);
}


/* ��д�⺯�� */
void SysTick_Handler(void)
{
    OsKernelEnterIntrState();
    OsKernelTickISR();
    OsKernelLeaveIntrState();
}

/* ��д�⺯�� */
void EXTI15_10_IRQHandler(void)
{
#if (TCLC_IRQ_ENABLE)
    OsKernelEnterIntrState();
    OsIrqEnterISR(EXTI15_10_IRQ_ID);
    OsKernelLeaveIntrState();
#else
    return;
#endif
}


/* ��д�⺯�� */
void TIM2_IRQHandler(void)
{
#if (TCLC_IRQ_ENABLE)
    OsKernelEnterIntrState();
    OsIrqEnterISR(TIM2_IRQ_ID);
    OsKernelLeaveIntrState();
#else
    return;
#endif
}

/* ��д�⺯�� */
void USART1_IRQHandler(void)	
{
#if (TCLC_IRQ_ENABLE)
    OsKernelEnterIntrState();
    OsIrqEnterISR(USART1_IRQ_ID);
    OsKernelLeaveIntrState();
#else
    return;
#endif    
    
}
