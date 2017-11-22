/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.debug.h"
#include "tcl.cpu.h"
#include "tcl.ipc.h"
#include "tcl.kernel.h"
#include "tcl.thread.h"
#include "tcl.timer.h"

#if (TCLC_TIMER_ENABLE)

/*
 * �ں˶�ʱ����Ϊ2�֣��ֱ������߳���ʱ��ʱ�޷�ʽ������Դ���̶߳�ʱ�����û���ʱ��
 * ��ģ��ֻ�����û���ʱ����
 */
static TTimerList TimerActiveListA;
static TTimerList TimerActiveListB;
static TLinkNode* TimerDormantList;
static TLinkNode* TimerExpiredList;

/*************************************************************************************************
 *  ���ܣ���ʱ��ִ�д�����                                                                     *
 *  ������(1) pTimer ��ʱ��                                                                      *
 *  ���أ���                                                                                     *
 *  ˵��: (1)�߳���ʱ�޷�ʽ������Դ��ʱ������ò�����Դ�����̻߳ᱻͬʱ������Դ���߳���������  *
 *           ���ں��̸߳��������С�                                                              *
 *        (2)�û������Զ�ʱ�������󣬻����̽�����һ�ּ�ʱ��ͬʱҲ������������С������û���ʱ��  *
 *           ������ϵͳ��ʱ���ػ��̴߳���Ҳ����˵���û���ʱ���Ļص����������߳�ִ̬�еġ�      *
 *************************************************************************************************/
static void DispatchExpiredTimer(TTimer* pTimer)
{
    TIndex spoke;
    TTimerList *pList;

    /*
     * ����ʱ�������ں˶�ʱ�������б�
     * ����ɶ�ʱ���ػ��̴߳��������Ķ�ʱ�����ȴ���
     */
    if (!(pTimer->Property & OS_TIMER_PROP_EXPIRED))
    {
        OsObjListAddPriorityNode(&TimerExpiredList, &(pTimer->ExpiredNode));
        pTimer->ExpiredTicks = pTimer->MatchTicks;
        pTimer->ExpiredTicksCycles = OsKernelVariable.JiffyCycles;
        pTimer->Property |= OS_TIMER_PROP_EXPIRED;
    }

    /* ����ʱ���ӻ�������Ƴ� */
    OsObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));

    /* ���������͵��û���ʱ�����·Żػ��ʱ�������� */
    if (pTimer->Property & OS_TIMER_PROP_PERIODIC)
    {
        /* ��ʱ������������1 */
        pTimer->ExpiredTimes++;

        /* �����´�����ʱ�䣬�����ʱ��ʱ��������򽫶�ʱ��������һ����ʱ�������� */
        pTimer->MatchTicks += pTimer->PeriodTicks;
        if (pTimer->MatchTicks <= OsKernelVariable.Jiffies)
        {
            pList = (OsKernelVariable.TimerList == &TimerActiveListA)?\
                    (&TimerActiveListB): (&TimerActiveListA);
        }
        else
        {
            pList = OsKernelVariable.TimerList;
        }

        spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
        OsObjListAddPriorityNode(&(pList->Handle[spoke]), &(pTimer->LinkNode));
        pTimer->Status = OsTimerActive;
    }
    else
    {
        /* �����λص���ʱ���ŵ����߶����� */
        OsObjListAddNode(&TimerDormantList, &(pTimer->LinkNode), OsLinkTail);
        pTimer->Status = OsTimerDormant;
    }
}


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ��ISR������                                                                  *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵��:                                                                                        *
 *************************************************************************************************/
void OsTimerTickUpdate(void)
{
    TTimer* pTimer;
    TIndex  spoke;
    TLinkNode* pNode;
    TLinkNode* pNext;
    TTimerList* pList;

    /* ���ϵͳJiffes�ﵽ0˵��Jiffes�����������ʱ��Ҫ�л��û���ʱ������ */
    if (OsKernelVariable.Jiffies == 0U)
    {
        OsKernelVariable.TimerList =
            (OsKernelVariable.TimerList == &TimerActiveListA)?\
            (&TimerActiveListB): (&TimerActiveListA);
    }
    pList = OsKernelVariable.TimerList;

    /* ���ݵ�ǰϵͳʱ�ӽ��ļ����������ǰ���ʱ������ */
    spoke = (TIndex)(OsKernelVariable.Jiffies % TCLC_TIMER_WHEEL_SIZE);
    pNode = pList->Handle[spoke];

    /*
     * ��鵱ǰ���ʱ��������Ķ�ʱ����������Ķ�ʱ��������������ֵ��С�������С�
     * ���ж��׶�ʱ������һ������С�ڵ�ǰϵͳʱ�ӽ��ļ�����
     * �ڱ�ϵͳ�У�ϵͳʱ�ӽ��ļ���Ĭ��Ϊ64Bits��
     * ��Ϊ˫��ʱ�����л�Ϊ���壬���Լ�ʹ��ʱ���������������Ҳ���ᶪʧ������
     */
    while (pNode != (TLinkNode*)0)
    {
        pNext = pNode->Next;
        pTimer = (TTimer*)(pNode->Owner);

        /*
         * �Ƚ϶�ʱ������ʱ�������ʹ�ʱϵͳʱ�ӽ�������
         * �����������ö�ʱ��;
         * ����������˳���������;
         */
        if (pTimer->MatchTicks == OsKernelVariable.Jiffies)
        {
            DispatchExpiredTimer(pTimer);
            pNode = pNext;
        }
        else
        {
            break;
        }
    }

    /* �����Ҫ�����ں����õ��û���ʱ���ػ��߳� */
    if (TimerExpiredList != (TLinkNode*)0)
    {
        OsThreadResumeFromISR(OsKernelVariable.TimerDaemon);
    }
}


/*************************************************************************************************
 *  ���ܣ��û���ʱ����ʼ������                                                                   *
 *  ������(1) pTimer   ��ʱ����ַ                                                                *
 *        (2) pName    ��ʱ������                                                                *
 *        (3) property ��ʱ������                                                                *
 *        (4) ticks    ��ʱ���δ���Ŀ                                                            *
 *        (5) pRoutine �û���ʱ���ص�����                                                        *
 *        (6) pData    �û���ʱ���ص���������                                                    *
 *        (7) priority ��ʱ�����ȼ�                                                              *
 *        (8) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵��                                                                                         *
 *************************************************************************************************/
TState TclCreateTimer(TTimer* pTimer, TChar* pName, TProperty property, TTimeTick ticks,
                      TTimerRoutine pRoutine, TArgument data, TPriority priority, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((pName != (TChar*)0), "");
    OS_ASSERT((pRoutine != (TTimerRoutine)0), "");
    OS_ASSERT((ticks > 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    property &= OS_TIMER_USER_PROPERTY;

    OsCpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (!(pTimer->Property & OS_TIMER_PROP_READY))
    {
        /* ����ʱ�����뵽�ں˶�������� */
        OsKernelAddObject(&(pTimer->Object), pName, OsTimerObject, (void*)pTimer);

        /* ��ʼ����ʱ�������ö�ʱ����Ϣ */
        pTimer->Status       = OsTimerDormant;
        pTimer->Property     = (property | OS_TIMER_PROP_READY);
        pTimer->PeriodTicks  = ticks;
        pTimer->MatchTicks   = (TTimeTick)0;
        pTimer->Routine      = pRoutine;
        pTimer->Argument     = data;
        pTimer->Priority     = priority;
        pTimer->ExpiredTicks = (TTimeTick)0;
        pTimer->ExpiredTicksCycles  = 0U;
        pTimer->ExpiredTimes = 0U;

        /* ���ö�ʱ����������ڵ���Ϣ */
        pTimer->ExpiredNode.Next   = (TLinkNode*)0;
        pTimer->ExpiredNode.Prev   = (TLinkNode*)0;
        pTimer->ExpiredNode.Handle = (TLinkNode**)0;
        pTimer->ExpiredNode.Data   = (TBase32*)(&(pTimer->Priority));
        pTimer->ExpiredNode.Owner  = (void*)pTimer;

        /* ���ö�ʱ������ڵ���Ϣ, ������ʱ���������߶����� */
        pTimer->LinkNode.Next   = (TLinkNode*)0;
        pTimer->LinkNode.Prev   = (TLinkNode*)0;
        pTimer->LinkNode.Handle = (TLinkNode**)0;
        pTimer->LinkNode.Data   = (TBase32*)(&(pTimer->MatchTicks));
        pTimer->LinkNode.Owner  = (void*)pTimer;
        OsObjListAddNode(&TimerDormantList, &(pTimer->LinkNode), OsLinkTail);

        error = OS_TIMER_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��ں˶�ʱ��ȡ����ʼ��                                                                   *
 *  ������(1) pTimer   ��ʱ���ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵��                                                                                         *
 *************************************************************************************************/
TState TclDeleteTimer(TTimer* pTimer, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_UNREADY;
    TReg32 imask;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (pTimer->Property & OS_TIMER_PROP_READY)
    {
        if (pTimer->Status == OsTimerDormant)
        {
            /* ����ʱ�����ں˶����б����Ƴ� */
            OsKernelRemoveObject(&(pTimer->Object));

            /* ����ʱ����������ʱ���������Ƴ� */
            OsObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));

            /* ��ն�ʱ������ */
            memset(pTimer, 0U, sizeof(TTimer));
            error = OS_TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_TIMER_ERR_STATUS;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ���ʱ����������                                                                         *
 *  ������(1) pTimer     ��ʱ���ṹ��ַ                                                          *
 *        (2) lagticks   ��ʱ���ӻ���ʼ����ʱ��                                                  *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵������                                                                                     *
 *************************************************************************************************/
TState TclStartTimer(TTimer* pTimer,TTimeTick lagticks, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_UNREADY;
    TReg32 imask;
    TIndex spoke;
    TTimerList* pList;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (pTimer->Property & OS_TIMER_PROP_READY)
    {
        if (pTimer->Status == OsTimerDormant)
        {
            /* ����ʱ�������߶������Ƴ� */
            OsObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));

            /* ����ʱ������������ */
            pTimer->MatchTicks  = OsKernelVariable.Jiffies + pTimer->PeriodTicks + lagticks;
            if (pTimer->MatchTicks <= OsKernelVariable.Jiffies)
            {
                pList = (OsKernelVariable.TimerList == &TimerActiveListA)?\
                        (&TimerActiveListB): (&TimerActiveListA);
            }
            else
            {
                pList = OsKernelVariable.TimerList;
            }

            spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
            OsObjListAddPriorityNode(&(pList->Handle[spoke]), &(pTimer->LinkNode));
            pTimer->Status = OsTimerActive;
            error = OS_TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_TIMER_ERR_STATUS;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ�ֹͣ�û���ʱ������                                                                     *
 *  ������(1) pTimer   ��ʱ����ַ                                                                *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclStopTimer(TTimer* pTimer, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_UNREADY;
    TReg32 imask;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (pTimer->Property & OS_TIMER_PROP_READY)
    {
        /* ����ʱ���ӻ����/�����������Ƴ����ŵ����߶����� */
        if (pTimer->Status == OsTimerActive)
        {
            if (pTimer->Property & OS_TIMER_PROP_EXPIRED)
            {
                OsObjListRemoveNode(pTimer->ExpiredNode.Handle, &(pTimer->ExpiredNode));
                pTimer->Property &= ~OS_TIMER_PROP_EXPIRED;
            }

            OsObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));
            OsObjListAddNode(&TimerDormantList, &(pTimer->LinkNode), OsLinkTail);
            pTimer->Status = OsTimerDormant;

            error = OS_TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_TIMER_ERR_STATUS;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ����ö�ʱ�����͡���ʱʱ������ȼ�                                                       *
 *  ������(1) pTimer   ��ʱ���ṹ��ַ                                                            *
 *        (2) ticks    ��ʱ��ʱ�ӽ�����Ŀ                                                        *
 *        (3) priority ��ʱ�����ȼ�                                                              *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵��                                                                                         *
 *************************************************************************************************/
TState TclConfigTimer(TTimer* pTimer, TTimeTick ticks, TPriority priority, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_UNREADY;
    TReg32 imask;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((ticks > 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ��鶨ʱ���������� */
    if (pTimer->Property & OS_TIMER_PROP_READY)
    {
        if (pTimer->Status == OsTimerDormant)
        {
            pTimer->PeriodTicks = ticks;
            pTimer->Priority    = priority;
            error = OS_TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_TIMER_ERR_STATUS;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/* �ں˶�ʱ���ػ��̶߳����ջ���� */
static TBase32 TimerDaemonStack[TCLC_TIMER_DAEMON_STACK_BYTES >> 2];
static TThread TimerDaemonThread;

/* �ں˶�ʱ���ػ��̲߳������κ��̹߳���API���� */
#define TIMER_DAEMON_ACAPI (OS_THREAD_ACAPI_NONE)

/*************************************************************************************************
 *  ���ܣ��ں��еĶ�ʱ���ػ��̺߳���                                                             *
 *  ������(1) argument ��ʱ���̵߳��û�����                                                      *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void TimerDaemonEntry(TArgument argument)
{
    TBase32       imask;
    TTimer*       pTimer;
    TTimerRoutine pRoutine;
    TArgument     data;
    TTimeTick     ticks;
    TBase32       cycles;

    /*
     * ��������û���ʱ�������̻߳����´���ʱ���ص�����
     * ���������ʱ������Ϊ���򽫶�ʱ���ػ��̹߳���
     */
    while(eTrue)
    {
        OsCpuEnterCritical(&imask);

        if (TimerExpiredList == (TLinkNode*)0)
        {
            OsThreadSuspendSelf();
            OsCpuLeaveCritical(imask);
        }
        else
        {
            /* ������������ȡ��һ����ʱ�� */
            pTimer = (TTimer*)(TimerExpiredList->Owner);

            /* ����ʱ���������������Ƴ� */
            OsObjListRemoveNode(pTimer->ExpiredNode.Handle, &(pTimer->ExpiredNode));
            pTimer->Property &= ~OS_TIMER_PROP_EXPIRED;

            /* ���ƶ�ʱ�������ͺ������� */
            pRoutine = pTimer->Routine;
            data     = pTimer->Argument;
            ticks    = 0U;
            cycles   = 0U;
					
            /* ����Ǿ�׼��ʱ������㶨ʱ����Ư��ʱ�� */
            if (pTimer->Property & OS_TIMER_PROP_ACCURATE)
            {
                if (OsKernelVariable.Jiffies >= pTimer->ExpiredTicks)
                {
                    ticks = OsKernelVariable.Jiffies - pTimer->ExpiredTicks;
                    cycles = OsKernelVariable.JiffyCycles - pTimer->ExpiredTicksCycles;
                }
                else
                {
                    ticks = (~(TTimeTick)(0U)) - pTimer->ExpiredTicks + OsKernelVariable.Jiffies;
                    cycles = OsKernelVariable.JiffyCycles - pTimer->ExpiredTicksCycles - 1U;
                }
            }

            OsCpuLeaveCritical(imask);

            /* ���̻߳�����ִ�ж�ʱ������ */
            pRoutine(data, cycles, ticks);
        }
    }
}


/*************************************************************************************************
 *  ���ܣ���ʱ��ģ���ʼ��                                                                       *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsTimerModuleInit(void)
{
    /* ����ں��Ƿ��ڳ�ʼ״̬ */
    if(OsKernelVariable.State != OsOriginState)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

	/* ��ʼ���û���ʱ������ */
    TimerDormantList = (TLinkNode*)0;
    TimerExpiredList = (TLinkNode*)0;
    memset(&TimerActiveListA, 0U, sizeof(TimerActiveListA));
    memset(&TimerActiveListB, 0U, sizeof(TimerActiveListB));

    /* ��ʼ����ص��ں˱��� */
    OsKernelVariable.TimerList = &TimerActiveListA;

    /* ��ʼ���ں˶�ʱ�������߳� */
    OsThreadCreate(&TimerDaemonThread,
                   "kernel timer daemon",
                   OsThreadSuspended,
                   OS_THREAD_PROP_PRIORITY_FIXED|\
                   OS_THREAD_PROP_CLEAN_STACK|\
                   OS_THREAD_PROP_KERNEL_DAEMON,
                   TIMER_DAEMON_ACAPI,
                   TimerDaemonEntry,
                   (TArgument)(0U),
                   (void*)TimerDaemonStack,
                   (TBase32)TCLC_TIMER_DAEMON_STACK_BYTES,
                   (TPriority)TCLC_TIMER_DAEMON_PRIORITY,
                   (TTimeTick)TCLC_TIMER_DAEMON_SLICE);

    /* ��ʼ����ص��ں˱��� */
    OsKernelVariable.TimerDaemon = &TimerDaemonThread;
}
#endif

