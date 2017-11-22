/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.kernel.h"
#include "tcl.stm32f030.h"

//NVIC_INT_CTRL   EQU     0xE000ED04                              ; Interrupt control state register.
//NVIC_SYSPRI14   EQU     0xE000ED20                              ; System priority register (priority 14).
//NVIC_PENDSV_PRI EQU     0x00FF0000                              ; PendSV priority value (lowest).
//NVIC_PENDSVSET  EQU     0x10000000                              ; Value to trigger PendSV exception.

#define CM0_SYSTICK_CTRL    (*((volatile TReg32 *)0xE000E010))  /* SysTick Ctrl & Status Reg.       */
#define CM0_SYSTICK_RELOAD  (*((volatile TReg32 *)0xE000E014))  /* SysTick Reload  Value Reg.       */
#define CM0_SYSTICK_CURRENT (*((volatile TReg32 *)0xE000E018))  /* SysTick CurrentThread Value Reg. */
#define CM0_SYSTICK_CAL     (*((volatile TReg32 *)0xE000E01C))  /* SysTick Cal     Value Reg.       */

#define CM0_SYSTICK_CTRL_COUNT         (0x00010000U)            /* Count flag.                      */
#define CM0_SYSTICK_CTRL_CLK_SRC       (0x00000004u)            /* Clock Source.                    */
#define CM0_SYSTICK_CTRL_INTEN         (0x00000002U)            /* Interrupt enable.                */
#define CM0_SYSTICK_CTRL_ENABLE        (0x00000001U)            /* Counter mode.                    */

#define CM0_NVIC_SHPR_PENDSV        (0xE000ED20)
#define PENSV_ACTIVE_MASK           (0x1<<10)
#define CM0_SYS_INT_STATUS          (0xE000ED24)
#define CM0_NVIC_PENDSV_PRIORITY    (0x00FF0000)

#define CM0_NVIC_INT_CTRL           (0xE000ED04)          /* Interrupt control state register.   */
#define CM0_NVIC_INT_CTRL_PENDSVSET (0x1U<<28)            /* Value to trigger PendSV exception.  */
#define CM0_NVIC_INT_CTRL_PENDSVCLR (0x1U<<27)            /* Value to clear PendSV exception.    */
#define CM0_NVIC_INT_CTRL_PENDSTSET (0x1U<<26)            /* Value to trigger PendST exception.  */
#define CM0_NVIC_INT_CTRL_PENDSTCLR (0x1U<<25)            /* Value to clear PendST exception.    */

/*************************************************************************************************
 *  ���ܣ������ں˽��Ķ�ʱ��                                                                     *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsCpuStartTickClock(void)
{
    CM0_SYSTICK_CTRL |= (CM0_SYSTICK_CTRL_CLK_SRC | CM0_SYSTICK_CTRL_ENABLE);
    CM0_SYSTICK_CTRL |= CM0_SYSTICK_CTRL_INTEN;
}


/*************************************************************************************************
 *  ���ܣ������̵߳���                                                                           *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsCpuConfirmThreadSwitch(void)
{
    TCLM_SET_REG32(CM0_NVIC_INT_CTRL, CM0_NVIC_INT_CTRL_PENDSVSET);
}


/*************************************************************************************************
 *  ���ܣ�ȡ���̵߳���                                                                           *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsCpuCancelThreadSwitch(void)
{
    TCLM_SET_REG32(CM0_NVIC_INT_CTRL, CM0_NVIC_INT_CTRL_PENDSVCLR);
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
    /* ��ʼ��systick��ʱ�� */
    TBase32 value = TCLC_CPU_CLOCK_FREQ / TCLC_TIME_TICK_RATE;
    CM0_SYSTICK_RELOAD = (value - 1U);

    /* ����PENDSV�ж����ȼ� */
    // TCLM_SET_REG32(CM0_NVIC_SHPR_PENDSV, CM0_NVIC_PENDSV_PRIORITY);
}

TPriority OsCpuCalcHiPRIO(TBase32 data)
{
    int i = 0;
    while(!(data & 0x1))
    {
        i++;
        data = data>>1;
    }
    return i;
}


/* ��д�⺯�� */
void SysTick_Handler(void)
{
    OsKernelEnterIntrState();
    OsKernelTickISR();
    OsKernelLeaveIntrState();
}

/* ��д�⺯�� */
void EXTI4_15_IRQHandler(void)
{
#if (TCLC_IRQ_ENABLE)
    OsKernelEnterIntrState();
    OsIrqEnterISR(EXTI4_15_IRQ_ID);
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
