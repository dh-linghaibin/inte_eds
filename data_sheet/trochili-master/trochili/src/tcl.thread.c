/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.cpu.h"
#include "tcl.ipc.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"
#include "tcl.timer.h"
#include "tcl.thread.h"

/* �ں˽��������ж���,���ھ��������е��̶߳�������������� */
static TThreadQueue ThreadReadyQueue;

/* �ں��̸߳������ж��壬������ʱ���������ߵ��̶߳�������������� */
static TThreadQueue ThreadAuxiliaryQueue;


/*************************************************************************************************
 *  ���ܣ��߳����м��������̵߳����ж�����Ϊ����                                               *
 *  ������(1) pThread  �̵߳�ַ                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void SuperviseThread(TThread* pThread)
{
    /* ��ͨ�߳���Ҫע���û���С���˳����·Ƿ�ָ������������� */
    OS_ASSERT((pThread == OsKernelVariable.CurrentThread), "");
    pThread->Entry(pThread->Argument);

    OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
    pThread->Diagnosis |= OS_THREAD_DIAG_INVALID_EXIT;
    OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
}


/*************************************************************************************************
 *  ���ܣ����̴߳�ָ����״̬ת��������̬��ʹ���߳��ܹ������ں˵���                               *
 *  ������(1) pThread   �߳̽ṹ��ַ                                                             *
 *        (2) status    �̵߳�ǰ״̬�����ڼ��                                                   *
 *        (3) pError    ����������                                                             *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState SetThreadReady(TThread* pThread, TThreadStatus status, TBool* pHiRP, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_STATUS;

    /* �߳�״̬У��,ֻ��״̬���ϵ��̲߳��ܱ����� */
    if (pThread->Status == status)
    {
        /*
         * �����̣߳�����̶߳��к�״̬ת��
         * ��Ϊ�����̻߳����£����Դ�ʱpThreadһ�����ǵ�ǰ�߳�
         */
        OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
        OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkTail);
        pThread->Status = OsThreadReady;
        if (pThread->Priority < OsKernelVariable.CurrentThread->Priority)
        {
            *pHiRP = eTrue;
        }

        state = eSuccess;
        error = OS_THREAD_ERR_NONE;
    }

    /* �����ȡ����ʱ��������Ҫֹͣ�̶߳�ʱ�� */
    if ((state == eSuccess) && (status == OsThreadDelayed))
    {
        OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��̹߳�����                                                                           *
 *  ������(1) pThread   �߳̽ṹ��ַ                                                             *
 *        (2) status    �̵߳�ǰ״̬�����ڼ��                                                   *
 *        (3) ticks     �߳���ʱʱ��                                                             *
 *        (4) pError    ����������                                                             *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState SetThreadUnready(TThread* pThread, TThreadStatus status, TTimeTick ticks,
                               TBool* pHiRP, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_STATUS;

    /* ����������ǵ�ǰ�̣߳�����Ҫ���ȼ���ں��Ƿ�������� */
    if (pThread->Status == OsThreadRunning)
    {
        /* ����ں˴�ʱ��ֹ�̵߳��ȣ���ô��ǰ�̲߳��ܱ����� */
        if (OsKernelVariable.SchedLockTimes == 0U)
        {
            OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
            OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
            pThread->Status = status;
            *pHiRP = eTrue;

            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_THREAD_ERR_FAULT;
        }
    }
    else if (pThread->Status == OsThreadReady)
    {
        /* ������������̲߳��ǵ�ǰ�̣߳��򲻻������̵߳��ȣ�����ֱ�Ӵ����̺߳Ͷ��� */
        OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
        OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
        pThread->Status = status;

        error = OS_THREAD_ERR_NONE;
        state = eSuccess;
    }
    else
    {
        OsDebugWarning("");
    }

    /* ���ò������̶߳�ʱ�� */
    if ((state == eSuccess) && (status == OsThreadDelayed))
    {
        pThread->Timer.RemainTicks = ticks;
        OsObjListAddDiffNode(&(OsKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ���������̶߳����е�������ȼ�����                                                     *
 *  ��������                                                                                     *
 *  ���أ�HiRP (Highest Ready Priority)                                                          *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void CalcThreadHiRP(TPriority* priority)
{
    /* ����������ȼ���������˵���ں˷����������� */
    if (ThreadReadyQueue.PriorityMask == (TBitMask)0)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
    *priority = OsCpuCalcHiPRIO(ThreadReadyQueue.PriorityMask);
}


#if (TCLC_THREAD_STACK_CHECK_ENABLE)
/*************************************************************************************************
 *  ���ܣ��澯�ͼ���߳�ջ�������                                                               *
 *  ������(1) pThread  �̵߳�ַ                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void CheckThreadStack(TThread* pThread)
{
    if ((pThread->StackTop < pThread->StackBarrier) ||
            (*(TBase32*)(pThread->StackBarrier) != TCLC_THREAD_STACK_BARRIER_VALUE))
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= OS_THREAD_DIAG_STACK_OVERFLOW;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (pThread->StackTop < pThread->StackAlarm)
    {
        pThread->Diagnosis |= OS_THREAD_DIAG_STACK_ALARM;
    }
}

#endif


/* RULE
 * 1 ��ǰ�߳��뿪�������к��ٴμ����������ʱ��
 *   �����Ȼ�ǵ�ǰ�߳���һ��������Ӧ�Ķ���ͷ�������Ҳ����¼���ʱ��Ƭ��
 *   ����Ѿ����ǵ�ǰ�߳���һ��������Ӧ�Ķ���β�������Ҳ����¼���ʱ��Ƭ��
 * 2 ��ǰ�߳��ھ��������ڲ��������ȼ�ʱ�����µĶ�����Ҳһ��Ҫ�ڶ���ͷ��
 */

/*************************************************************************************************
 *  ���ܣ����̼߳��뵽ָ�����̶߳�����                                                           *
 *  ������(1) pQueue  �̶߳��е�ַ��ַ                                                           *
 *        (2) pThread �߳̽ṹ��ַ                                                               *
 *        (3) pos     �߳����̶߳����е�λ��                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsThreadEnterQueue(TThreadQueue* pQueue, TThread* pThread, TLinkPos pos)
{
    TPriority priority;
    TLinkNode** pHandle;

    /* ����̺߳��̶߳��� */
    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pThread->Queue == (TThreadQueue*)0), "");

    /* �����߳����ȼ��ó��߳�ʵ�������ֶ��� */
    priority = pThread->Priority;
    pHandle = &(pQueue->Handle[priority]);

    /* ���̼߳���ָ���ķֶ��� */
    OsObjQueueAddFifoNode(pHandle, &(pThread->LinkNode), pos);

    /* �����߳��������� */
    pThread->Queue = pQueue;

    /* �趨���߳����ȼ�Ϊ�������ȼ� */
    pQueue->PriorityMask |= (0x1 << priority);
}


/*************************************************************************************************
 *  ���ܣ����̴߳�ָ�����̶߳������Ƴ�                                                           *
 *  ������(1) pQueue  �̶߳��е�ַ��ַ                                                           *
 *        (2) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ���                                                                                     *
 *  ˵����FIFO PRIO���ַ�����Դ�ķ�ʽ                                                            *
 *************************************************************************************************/
void OsThreadLeaveQueue(TThreadQueue* pQueue, TThread* pThread)
{
    TPriority priority;
    TLinkNode** pHandle;

    /* ����߳��Ƿ����ڱ�����,������������ں˷����������� */
    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pQueue == pThread->Queue), "");

    /* �����߳����ȼ��ó��߳�ʵ�������ֶ��� */
    priority = pThread->Priority;
    pHandle = &(pQueue->Handle[priority]);

    /* ���̴߳�ָ���ķֶ�����ȡ�� */
    OsObjQueueRemoveNode(pHandle, &(pThread->LinkNode));

    /* �����߳��������� */
    pThread->Queue = (TThreadQueue*)0;

    /* �����߳��뿪���к�Զ������ȼ�������ǵ�Ӱ�� */
    if (pQueue->Handle[priority] == (TLinkNode*)0)
    {
        /* �趨���߳����ȼ�δ���� */
        pQueue->PriorityMask &= (~(0x1 << priority));
    }
}


/*************************************************************************************************
 *  ���ܣ��߳�ʱ��Ƭ����������ʱ��Ƭ�жϴ���ISR�л���ñ�����                                  *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵��������������˵�ǰ�̵߳�ʱ��Ƭ��������û��ѡ����Ҫ���ȵĺ���̺߳ͽ����߳��л�         *
 *************************************************************************************************/
/*
 * ��ǰ�߳̿��ܴ���3��λ��
 * 1 �������е�ͷλ��(�κ����ȼ�)
 * 2 �������е�����λ��(�κ����ȼ�)
 * 3 �������л�������������
 * ֻ�����1����Ҫ����ʱ��Ƭ��ת�Ĵ�������ʱ���漰�߳��л�,��Ϊ������ֻ��ISR�е��á�
 */
void OsThreadTickUpdate(void)
{
    TThread* pThread;
    TLinkNode* pHandle;

    /* ����ǰ�߳�ʱ��Ƭ��ȥ1��������,�߳������ܽ�������1 */
    pThread = OsKernelVariable.CurrentThread;
    pThread->Ticks--;
    pThread->Jiffies++;

    /* �������ʱ��Ƭ������� */
    if (pThread->Ticks == 0U)
    {
        /* �ָ��̵߳�ʱ�ӽ����� */
        pThread->Ticks = pThread->BaseTicks;

        /* �ж��߳��ǲ��Ǵ����ں˾����̶߳��е�ĳ�����ȼ��Ķ���ͷ */
        pHandle = ThreadReadyQueue.Handle[pThread->Priority];
        if ((TThread*)(pHandle->Owner) == pThread)
        {
            /* ����ں˴�ʱ�����̵߳��� */
            if (OsKernelVariable.SchedLockTimes == 0U)
            {
                /*
                 * ����ʱ��Ƭ���ȣ�֮��pThread�����̶߳���β��,
                 * ��ǰ�߳������̶߳���Ҳ����ֻ�е�ǰ�߳�Ψһ1���߳�
                 */
                ThreadReadyQueue.Handle[pThread->Priority] =
                    (ThreadReadyQueue.Handle[pThread->Priority])->Next;

                /* ���߳�״̬��Ϊ����,׼���߳��л� */
                pThread->Status = OsThreadReady;
            }
        }
    }
}


/*************************************************************************************************
 *  ���ܣ��̶߳�ʱ��������                                                                     *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵��:                                                                                        *
 *************************************************************************************************/
void OsThreadTimerUpdate(void)
{
    TThread* pThread;
    TTickTimer* pTimer;
    TBool HiRP = eFalse;

    /* �õ����ڶ���ͷ���̶߳�ʱ��������Ӧ�Ķ�ʱ������1 */
    if (OsKernelVariable.ThreadTimerList != (TLinkNode*)0)
    {
        pTimer = (TTickTimer*)(OsKernelVariable.ThreadTimerList->Owner);
        pTimer->RemainTicks--;

        /* �������Ϊ0���̶߳�ʱ�� */
        while (pTimer->RemainTicks == 0U)
        {
            /*
             * �����̣߳�����̶߳��к�״̬ת��,ע��ֻ���жϴ���ʱ��
             * ��ǰ�̲߳Żᴦ���ں��̸߳���������(��Ϊ��û���ü��߳��л�)
             * ��ǰ�̷߳��ؾ�������ʱ��һ��Ҫ�ص���Ӧ�Ķ���ͷ
             * ���߳̽�����������ʱ������Ҫ�����̵߳�ʱ�ӽ�����
             */
            pThread = (TThread*)(pTimer->Owner);
            if (pThread->Status == OsThreadDelayed)
            {
                OsThreadLeaveQueue(OsKernelVariable.ThreadAuxiliaryQueue, pThread);
                if (pThread == OsKernelVariable.CurrentThread)
                {
                    OsThreadEnterQueue(OsKernelVariable.ThreadReadyQueue, pThread, OsLinkHead);
                    pThread->Status = OsThreadRunning;
                }
                else
                {
                    OsThreadEnterQueue(OsKernelVariable.ThreadReadyQueue, pThread, OsLinkTail);
                    pThread->Status = OsThreadReady;
                }
                /* ���̶߳�ʱ���Ӳ�ֶ������Ƴ� */
                OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList), &(pTimer->LinkNode));
            }
#if (TCLC_IPC_ENABLE)
            /* ���̴߳����������н������ */
            else if (pThread->Status == OsThreadBlocked)
            {
                OsIpcUnblockThread(pThread->IpcContext, eFailure, OS_IPC_ERR_TIMEO, &HiRP);
            }
#endif
            else
            {
                OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
            }

            if (OsKernelVariable.ThreadTimerList == (TLinkNode*)0)
            {
                break;
            }

            /* �����һ���̶߳�ʱ�� */
            pTimer = (TTickTimer*)(OsKernelVariable.ThreadTimerList->Owner);
        }
    }
}


/*************************************************************************************************
 *  ���ܣ����������̵߳���                                                                       *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵�����̵߳ĵ���������ܱ�ISR����ȡ��                                                        *
 *************************************************************************************************/
/*
 * 1 ��ǰ�߳��뿪���м������������������У���ǰ�̷߳��ؾ�������ʱ��һ��Ҫ�ص���Ӧ�Ķ���ͷ
     ���߳̽�����������ʱ������Ҫ�����̵߳�ʱ�ӽ�����
 * 2 ���µ�ǰ�̲߳�����߾������ȼ���ԭ����
 *   1 ������ȼ����ߵ��߳̽����������
 *   2 ��ǰ�߳��Լ��뿪����
 *   3 ����̵߳����ȼ������
 *   4 ��ǰ�̵߳����ȼ�������
 *   5 ��ǰ�߳�Yiled
 *   6 ʱ��Ƭ�ж��У���ǰ�̱߳���ת
 * 3 ��cortex��������, ������һ�ֿ���:
 *   ��ǰ�߳��ͷ��˴�����������PendSV�жϵõ���Ӧ֮ǰ���������������ȼ��жϷ�����
 *   �ڸ߼�isr���ְѵ�ǰ�߳���Ϊ���У�
 *   1 ���ҵ�ǰ�߳���Ȼ����߾������ȼ���
 *   2 ���ҵ�ǰ�߳���Ȼ����߾����̶߳��еĶ���ͷ��
 *   ��ʱ��Ҫ����ȡ��PENDSV�Ĳ��������⵱ǰ�̺߳��Լ��л�
 */
void OsThreadSchedule(void)
{
    TPriority priority;

    /* ����������ȼ���������˵���ں˷����������� */
    if (ThreadReadyQueue.PriorityMask == (TBitMask)0)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* ������߾������ȼ�����ú���̣߳��������߳�ָ��Ϊ����˵���ں˷����������� */
    CalcThreadHiRP(&priority);
    OsKernelVariable.NomineeThread = (TThread*)((ThreadReadyQueue.Handle[priority])->Owner);
    if (OsKernelVariable.NomineeThread == (TThread*)0)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /*
     * �˴������߼����ӣ��漰���ܶ����̵߳����龰���ر���ʱ��Ƭ��Yiled��
     * �жϡ��ж���ռ����ĵ�ǰ�̵߳�״̬�仯��
     */
    if (OsKernelVariable.NomineeThread != OsKernelVariable.CurrentThread)
    {
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
        CheckThreadStack(OsKernelVariable.NomineeThread);
#endif
        /*
         * ��ʱ�����ֿ��ܣ�һ���߳�����ִ�У�Ȼ���и������ȼ����߳̾�����
         * ���ǵ�ǰ�̶߳��ݲ��������Ǻܿ��ַ�������״̬��(ͬʱ/Ȼ��)�и������ȼ����߳̾�����
         * �����������������Ҫ����ǰ�߳�����Ϊ����״̬��
         */
        if (OsKernelVariable.CurrentThread->Status == OsThreadRunning)
        {
            OsKernelVariable.CurrentThread->Status = OsThreadReady;
        }
        OsCpuConfirmThreadSwitch();
    }
    else
    {
        /*
         * �ڶ�ʱ����DAEMON����ز���ʱ���п����ڵ����߳���δ�л������ĵ�ʱ��
         * ���·Żؾ������У���ʱ����ش������Ѿ�����ǰ�߳��������ó�����״̬��
         * ����yeild��tick isr��п��ܽ���ǰ�߳����óɾ���̬������ʱ��ǰ�߳�����
         * ������ֻ��Ψһһ���߳̾�����������ʱ��Ҫ����ǰ�߳��������ó�����״̬��
         */
        if (OsKernelVariable.CurrentThread->Status == OsThreadReady)
        {
            OsKernelVariable.CurrentThread->Status = OsThreadRunning;
        }
        OsCpuCancelThreadSwitch();
    }
}


/*************************************************************************************************
 *  ���ܣ��߳̽ṹ��ʼ������                                                                     *
 *  ������(1)  pThread  �߳̽ṹ��ַ                                                             *
 *        (2)  status   �̵߳ĳ�ʼ״̬                                                           *
 *        (3)  property �߳�����                                                                 *
 *        (4)  acapi    ���̹߳���API����ɿ���                                                  *
 *        (5)  pEntry   �̺߳�����ַ                                                             *
 *        (6)  TArgument�̺߳�������                                                             *
 *        (7)  pStack   �߳�ջ��ַ                                                               *
 *        (8)  bytes    �߳�ջ��С������Ϊ��λ                                                   *
 *        (9)  priority �߳����ȼ�                                                               *
 *        (10) ticks    �߳�ʱ��Ƭ����                                                           *
 *  ���أ�(1)  eFailure                                                                          *
 *        (2)  eSuccess                                                                          *
 *  ˵����ע��ջ��ʼ��ַ��ջ��С��ջ�澯��ַ���ֽڶ�������                                       *
 *************************************************************************************************/
void OsThreadCreate(TThread* pThread, TChar* pName, TThreadStatus status, TProperty property,
                    TBitMask acapi, TThreadEntry pEntry, TArgument argument,
                    void* pStack, TBase32 bytes, TPriority priority, TTimeTick ticks)
{
    TThreadQueue* pQueue;

    /* ��ʼ���̻߳���������Ϣ */
    OsKernelAddObject(&(pThread->Object), pName, OsThreadObject, (void*)pThread);

    /* �����߳�ջ������ݺ͹����̳߳�ʼջջ֡ */
    OS_ASSERT((bytes >= TCLC_CPU_MINIMAL_STACK), "");

    /* ջ��С���¶��� */
    bytes &= (~((TBase32)(TCLC_CPU_STACK_ALIGNED - 1U)));
    pThread->StackBase = (TBase32)pStack + bytes;

    /* ����߳�ջ�ռ� */
    if (property & OS_THREAD_PROP_CLEAN_STACK)
    {
        memset(pStack, 0U, bytes);
    }

    /* ����(α��)�̳߳�ʼջ֡,���ｫ�߳̽ṹ��ַ��Ϊ��������SuperviseThread()���� */
    OsCpuBuildThreadStack(&(pThread->StackTop), pStack, bytes, (void*)(&SuperviseThread),
                          (TArgument)pThread);

    /* �����߳�ջ�澯��ַ */
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
    pThread->StackAlarm = (TBase32)pStack + bytes - (bytes* TCLC_THREAD_STACK_ALARM_RATIO) / 100;
    pThread->StackBarrier = (TBase32)pStack;
    (*(TAddr32*)pStack) = TCLC_THREAD_STACK_BARRIER_VALUE;
#endif

    /* �����߳�ʱ��Ƭ��ز��� */
    pThread->Ticks = ticks;
    pThread->BaseTicks = ticks;
    pThread->Jiffies = 0U;

    /* �����߳����ȼ� */
    pThread->Priority = priority;
    pThread->BasePriority = priority;

    /* �����߳���ں������̲߳��� */
    pThread->Entry = pEntry;
    pThread->Argument = argument;

    /* �����߳�����������Ϣ */
    pThread->Queue = (TThreadQueue*)0;

    /* �����̶߳�ʱ�� */
    pThread->Timer.LinkNode.Owner = (void*)(&(pThread->Timer));
    pThread->Timer.LinkNode.Data = (TBase32*)(&(pThread->Timer.RemainTicks));
    pThread->Timer.LinkNode.Prev = (TLinkNode*)0;
    pThread->Timer.LinkNode.Next = (TLinkNode*)0;
    pThread->Timer.LinkNode.Handle = (TLinkNode**)0;
    pThread->Timer.Owner = (void*)pThread;
    pThread->Timer.RemainTicks = (TTimeTick)0;

    /*
     * �߳�IPC���������Ľṹ��û��ֱ�Ӷ������߳̽ṹ���������Ҫ������ʱ��
     * ��ʱ���߳�ջ�ﰲ�ŵġ��ô��Ǽ������߳̽ṹռ�õ��ڴ档
     */
#if (TCLC_IPC_ENABLE)
    pThread->IpcContext = (TIpcContext*)0;
#endif

    /* �߳�ռ�е���(MUTEX)���� */
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
    pThread->LockList = (TLinkNode*)0;
#endif

    /* ��ʼ�߳����������Ϣ */
    pThread->Diagnosis = OS_THREAD_DIAG_NORMAL;

    /* �����߳��ܹ�֧�ֵ��̹߳���API */
    pThread->ACAPI = acapi;

    /* �����߳�����ڵ���Ϣ���̴߳�ʱ�������κ��̶߳��� */
    pThread->LinkNode.Owner = (void*)pThread;
    pThread->LinkNode.Data = (TBase32*)(&(pThread->Priority));
    pThread->LinkNode.Prev = (TLinkNode*)0;
    pThread->LinkNode.Next = (TLinkNode*)0;
    pThread->LinkNode.Handle = (TLinkNode**)0;

    /* ���̼߳����ں��̶߳��У������߳�״̬ */
    pQueue = (status == OsThreadReady) ? (&ThreadReadyQueue): (&ThreadAuxiliaryQueue);
    OsThreadEnterQueue(pQueue, pThread, OsLinkTail);
    pThread->Status = status;

    /* ����߳��Ѿ���ɳ�ʼ�� */
    pThread->Property = (property| OS_THREAD_PROP_READY);
}


/*************************************************************************************************
 *  ���ܣ��߳�ע��                                                                               *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵������ʼ���̡߳��ж��ػ��̺߳��û���ʱ���̲߳��ܱ�ɾ��                                     *
 *************************************************************************************************/
TState OsThreadDelete(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_STATUS;

    if (pThread->Status == OsThreadDormant)
    {
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
        if (pThread->LockList)
        {
            error = OS_THREAD_ERR_FAULT;
            state = eFailure;
        }
        else
#endif
        {
            OsKernelRemoveObject(&(pThread->Object));
            OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            memset(pThread, 0U, sizeof(TThread));
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
        }
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ������߳����ȼ�                                                                         *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) priority �߳����ȼ�                                                                *
 *        (3) flag     �Ƿ�SetPriority API����                                                 *
 *        (4) pError   ����������                                                              *
 *  ���أ�(1) eFailure �����߳����ȼ�ʧ��                                                        *
 *        (2) eSuccess �����߳����ȼ��ɹ�                                                        *
 *  ˵�����������ʱ�޸����ȼ������޸��߳̽ṹ�Ļ������ȼ�                                     *
 *************************************************************************************************/
TState OsThreadSetPriority(TThread* pThread, TPriority priority, TBool flag, TBool* pHiRP,
                           TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_PRIORITY;
    TPriority newPrio;

    if (pThread->Priority != priority)
    {
        if (pThread->Status == OsThreadBlocked)
        {
            /* ����״̬���̶߳��ڸ���������޸������ȼ� */
            OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            pThread->Priority = priority;
            OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);

            OsIpcSetPriority(pThread->IpcContext, priority);
            state = eSuccess;
            error = OS_THREAD_ERR_NONE;
        }
        /*
         * �����̵߳������ȼ�ʱ������ֱ�ӵ������ھ����̶߳����еķֶ���
         * ���ڴ��ھ����̶߳����еĵ�ǰ�̣߳�����޸��������ȼ���
         * ��Ϊ��������Ƴ��߳̾������У����Լ�ʹ�ں˲��������Ҳû����
         */
        else if (pThread->Status == OsThreadReady)
        {
            OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
            pThread->Priority = priority;
            OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkTail);

            /*
             * �õ���ǰ�������е���߾������ȼ�����Ϊ�����߳�(������ǰ�߳�)
             * ���߳̾��������ڵ����ڻᵼ�µ�ǰ�߳̿��ܲ���������ȼ���
             */
            if (priority < OsKernelVariable.CurrentThread->Priority)
            {
                *pHiRP = eTrue;
            }
            state = eSuccess;
            error = OS_THREAD_ERR_NONE;
        }
        else if (pThread->Status == OsThreadRunning)
        {
            /*
             * ���赱ǰ�߳����ȼ������Ψһ����������������ȼ�֮����Ȼ����ߣ�
             * �������µ����ȼ����ж�������̣߳���ô��ðѵ�ǰ�̷߳����µľ�������
             * ��ͷ������������������ʽ��ʱ��Ƭ��ת����ǰ�߳��Ⱥ󱻶�ε������ȼ�ʱ��
             * ֻ��ÿ�ζ��������ڶ���ͷ���ܱ�֤�����һ�ε������ȼ��󻹴��ڶ���ͷ��
             */
            OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
            pThread->Priority = priority;
            OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkHead);

            /*
             * ��Ϊ��ǰ�߳����߳̾��������ڵ����ڻᵼ�µ�ǰ�߳̿��ܲ���������ȼ���
             * ������Ҫ���¼��㵱ǰ�������е���߾������ȼ���
             */
            CalcThreadHiRP(&newPrio);
            if (newPrio < OsKernelVariable.CurrentThread->Priority)
            {
                *pHiRP = eTrue;
            }

            state = eSuccess;
            error = OS_THREAD_ERR_NONE;
        }
        else
        {
            /*����״̬���̶߳��ڸ���������޸������ȼ� */
            OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            pThread->Priority = priority;
            OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
            state = eSuccess;
            error = OS_THREAD_ERR_NONE;
        }

        /* �����Ҫ���޸��̶̹߳����ȼ� */
        if (flag == eTrue)
        {
            pThread->BasePriority = priority;
        }
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ����̴߳ӹ���״̬ת��������̬��ʹ���߳��ܹ������ں˵���                                 *
 *  ������(1) pThread   �߳̽ṹ��ַ                                                             *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsThreadResumeFromISR(TThread* pThread)
{
    /*
     * �����̣߳�����̶߳��к�״̬ת��,ע��ֻ���жϴ���ʱ��
     * ��ǰ�̲߳Żᴦ���ں��̸߳���������(��Ϊ��û���ü��߳��л�)
     * ��ǰ�̷߳��ؾ�������ʱ��һ��Ҫ�ص���Ӧ�Ķ���ͷ
     * ���߳̽�����������ʱ������Ҫ�����̵߳�ʱ�ӽ�����
     */
    if (pThread->Status == OsThreadSuspended)
    {
        OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
        if (pThread == OsKernelVariable.CurrentThread)
        {
            OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkHead);
            pThread->Status = OsThreadRunning;
        }
        else
        {
            OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkTail);
            pThread->Status = OsThreadReady;
        }
    }
}


/*************************************************************************************************
 *  ���ܣ����߳��Լ�����                                                                         *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsThreadSuspendSelf(void)
{
    /* ����Ŀ���ǵ�ǰ�߳� */
    TThread* pThread = OsKernelVariable.CurrentThread;

    /* ����ǰ�̹߳�������ں˴�ʱ��ֹ�̵߳��ȣ���ô��ǰ�̲߳��ܱ����� */
    if (OsKernelVariable.SchedLockTimes == 0U)
    {
        OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
        OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
        pThread->Status = OsThreadSuspended;
        OsThreadSchedule();
    }
    else
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_SCHED_ERROR;

        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
}


/* �ں�ROOT�̶߳����ջ���� */
static TThread RootThread;
static TBase32 RootThreadStack[TCLC_ROOT_THREAD_STACK_BYTES >> 2];

/* �ں�ROOT�̲߳������κ��̹߳���API���� */
#define OS_THREAD_ACAPI_ROOT (OS_THREAD_ACAPI_NONE)

/*************************************************************************************************
 *  ���ܣ��ں�ROOT�̺߳���                                                                       *
 *  ������(1) argument �̵߳Ĳ���                                                                *
 *  ���أ���                                                                                     *
 *  ˵�����ú������ȿ�����������ƣ�Ȼ����������߳�����                                         *
 *        ע���߳�ջ������С�����⣬����̺߳�����Ҫ��̫�๤��                                   *
 *************************************************************************************************/
static void RootThreadEntry(TBase32 argument)
{
    /* �رմ������ж� */
    OsCpuDisableInt();

    /* ����ں˽�����߳�ģʽ */
    OsKernelVariable.State = OsThreadState;

    /* ��ʱ�ر��̵߳��ȹ��� */
    OsKernelVariable.SchedLockTimes = 1U;

    /*
     * �����û���ں�������ʼ���û�����
     * �ú���������OsThreadState,���ǽ�ֹSchedulable��״̬��
     */
    if(OsKernelVariable.UserEntry == (TUserEntry)0)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
    OsKernelVariable.UserEntry();

    /* �����̵߳��ȹ��� */
    OsKernelVariable.SchedLockTimes = 0U;

    /* ��ϵͳʱ�ӽ��� */
    OsCpuStartTickClock();

    /* �򿪴������ж� */
    OsCpuEnableInt();

    /* ����IDLE Hook��������ʱ���̻߳����Ѿ��� */
    while (eTrue)
    {
        if (OsKernelVariable.SysIdleEntry != (TSysIdleEntry)0)
        {
            OsKernelVariable.SysIdleEntry();
        }
    }
}


/*************************************************************************************************
 *  ���ܣ���ʼ���ں��̹߳���ģ��                                                                 *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵�����ں��е��̶߳�����Ҫ��һ�¼��֣�                                                       *
 *        (1) �߳̾�������,���ڴ洢���еľ����ߺ������̡߳��ں���ֻ��һ���������С�              *
 *        (2) �̸߳�������, ���й���״̬����ʱ״̬������״̬���̶߳��洢����������С�           *
 *            ͬ���ں���ֻ��һ�����߶���                                                         *
 *        (3) IPC������߳��������У�������������������״̬���̶߳���������Ӧ���߳����������  *
 *************************************************************************************************/
void OsThreadModuleInit(void)
{
    /* ����ں��Ƿ��ڳ�ʼ״̬ */
    if (OsKernelVariable.State != OsOriginState)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    memset(&ThreadReadyQueue, 0U, sizeof(ThreadReadyQueue));
    memset(&ThreadAuxiliaryQueue, 0U, sizeof(ThreadAuxiliaryQueue));

    OsKernelVariable.ThreadReadyQueue = &ThreadReadyQueue;
    OsKernelVariable.ThreadAuxiliaryQueue = &ThreadAuxiliaryQueue;

    /* ��ʼ���ں�ROOT�߳� */
    OsThreadCreate(&RootThread,
                   "kernel root thread",
                   OsThreadReady,
                   OS_THREAD_PROP_PRIORITY_FIXED|\
                   OS_THREAD_PROP_CLEAN_STACK|\
                   OS_THREAD_PROP_KERNEL_ROOT,
                   OS_THREAD_ACAPI_ROOT,
                   RootThreadEntry,
                   (TArgument)0,
                   (void*)RootThreadStack,
                   (TBase32)TCLC_ROOT_THREAD_STACK_BYTES,
                   (TPriority)TCLC_ROOT_THREAD_PRIORITY,
                   (TTimeTick)TCLC_ROOT_THREAD_SLICE);

    /* ��ʼ����ص��ں˱��� */
    OsKernelVariable.RootThread    = &RootThread;
    OsKernelVariable.NomineeThread  = &RootThread;
    OsKernelVariable.CurrentThread = &RootThread;
}


/*************************************************************************************************
 *  ���ܣ��߳̽ṹ��ʼ������                                                                     *
 *  ������(1)  pThread  �߳̽ṹ��ַ                                                             *
 *        (2)  status   �̵߳ĳ�ʼ״̬                                                           *
 *        (3)  property �߳�����                                                                 *
 *        (4)  acapi    ���̹߳���API����ɿ���                                                  *
 *        (5)  pEntry   �̺߳�����ַ                                                             *
 *        (6)  pArg     �̺߳�������                                                             *
 *        (7)  pStack   �߳�ջ��ַ                                                               *
 *        (8)  bytes    �߳�ջ��С������Ϊ��λ                                                   *
 *        (9)  priority �߳����ȼ�                                                               *
 *        (10) ticks    �߳�ʱ��Ƭ����                                                           *
 *        (11) pError   ��ϸ���ý��                                                             *
 *  ���أ�(1)  eFailure                                                                          *
 *        (2)  eSuccess                                                                          *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateThread(TThread* pThread,
                       TChar*       pName,
                       TThreadEntry pEntry,
                       TArgument    argument,
                       void*        pStack,
                       TBase32      bytes,
                       TPriority    priority,
                       TTimeTick    ticks,
                       TError*      pError)

{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TReg32 imask;

    /* ��Ҫ�Ĳ������ */
    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pName != (TChar*)0), "");
    OS_ASSERT((pEntry != (void*)0), "");
    OS_ASSERT((pStack != (void*)0), "");
    OS_ASSERT((bytes > 0U), "");
    OS_ASSERT((priority <= TCLC_USER_PRIORITY_LOW), "");
    OS_ASSERT((priority >= TCLC_USER_PRIORITY_HIGH), "");
    OS_ASSERT((ticks > 0U), "");


    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (!(pThread->Property & OS_THREAD_PROP_READY))
        {
            OsThreadCreate(pThread,
                           pName,
                           OsThreadDormant,
                           OS_THREAD_PROP_PRIORITY_SAFE,
                           OS_THREAD_ACAPI_ALL,
                           pEntry,
                           argument,
                           pStack,
                           bytes,
                           priority,
                           ticks);
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳�ע��                                                                               *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����IDLE�̡߳��жϴ����̺߳Ͷ�ʱ���̲߳��ܱ�ע��                                           *
 *************************************************************************************************/
TState TclDeleteThread(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TReg32 imask;

    /* ��Ҫ�Ĳ������ */
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = OsKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_DELETE)
            {
                state = OsThreadDelete(pThread, &error);
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ������߳����ȼ�                                                                         *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) priority �߳����ȼ�                                                                *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ���أ�(1) eFailure �����߳����ȼ�ʧ��                                                        *
 *        (2) eSuccess �����߳����ȼ��ɹ�                                                        *
 *  ˵����(1) �������ʱ�޸����ȼ������޸��߳̽ṹ�Ļ������ȼ�����                             *
 *        (2) ������ʵʩ���ȼ��̳�Э���ʱ����AUTHORITY����                                    *
 *************************************************************************************************/
TState TclSetThreadPriority(TThread* pThread, TPriority priority, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((priority < TCLC_PRIORITY_NUM), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = OsKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_PRIORITY)
            {
                if ((!(pThread->Property & OS_THREAD_PROP_PRIORITY_FIXED)) &&
                        (pThread->Property & OS_THREAD_PROP_PRIORITY_SAFE))
                {
                    state = OsThreadSetPriority(pThread, priority, eTrue, &HiRP, &error);
                    if ((OsKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                    {
                        OsThreadSchedule();
                    }
                }
                else
                {
                    error = OS_THREAD_ERR_PRIORITY;
                }
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }

    OsCpuLeaveCritical(imask);
    *pError = error;
    return state;

}


/*************************************************************************************************
 *  ���ܣ��޸��߳�ʱ��Ƭ����                                                                     *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) slice   �߳�ʱ��Ƭ����                                                             *
 *        (3) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclSetThreadSlice(TThread* pThread, TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((ticks > 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = OsKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_SLICE)
            {
                /* �����߳�ʱ��Ƭ���� */
                if (pThread->BaseTicks > ticks)
                {
                    pThread->Ticks = (pThread->Ticks < ticks) ? (pThread->Ticks): ticks;
                }
                else
                {
                    pThread->Ticks += (ticks - pThread->BaseTicks);
                }
                pThread->BaseTicks = ticks;

                error = OS_THREAD_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}



/*************************************************************************************************
 *  ���ܣ��̼߳��̵߳��Ⱥ�������ǰ�߳������ó�������(���־���״̬)                               *
 *  ������(1) pError    ��ϸ���ý��                                                             *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵������Ϊ�����ƻ���߾������ȼ�ռ�ô�������ԭ��                                           *
 *        ����Yield����ֻ����ӵ����߾������ȼ����߳�֮�����                                    *
 *************************************************************************************************/
TState TclYieldThread(TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TReg32 imask;
    TThread* pThread;

    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�����̻߳����²��ܵ��ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ����Ŀ���ǵ�ǰ�߳� */
        pThread = OsKernelVariable.CurrentThread;

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_YIELD)
            {
                /* ֻ�����ں������̵߳��ȵ������²��ܵ��ñ����� */
                if (OsKernelVariable.SchedLockTimes == 0U)
                {
                    /*
                     * ������ǰ�߳����ڶ��е�ͷָ��
                     * ��ǰ�߳������̶߳���Ҳ����ֻ�е�ǰ�߳�Ψһ1���߳�
                     */
                    ThreadReadyQueue.Handle[pThread->Priority] =
                        (ThreadReadyQueue.Handle[pThread->Priority])->Next;
                    pThread->Status = OsThreadReady;

                    OsThreadSchedule();
                    error = OS_THREAD_ERR_NONE;
                    state = eSuccess;
                }
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳���ֹ��ʹ���̲߳��ٲ����ں˵���                                                     *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����(1) ��ʼ���̺߳Ͷ�ʱ���̲߳��ܱ�����                                                   *
 *************************************************************************************************/
TState TclDeactivateThread(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = OsKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_DEACTIVATE)
            {
                state = SetThreadUnready(pThread, OsThreadDormant, 0U, &HiRP, &error);
                if (HiRP == eTrue)
                {
                    OsThreadSchedule();
                }
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ������̣߳�ʹ���߳��ܹ������ں˵���                                                     *
 *  ������(1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclActivateThread(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;


    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_ACTIVATE)
            {
                state = SetThreadReady(pThread, OsThreadDormant, &HiRP, &error);
                if ((OsKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                {
                    OsThreadSchedule();
                }
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��̹߳�����                                                                           *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����(1) �ں˳�ʼ���̲߳��ܱ�����                                                           *
 *************************************************************************************************/
TState TclSuspendThread(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ���û�и������������̵߳�ַ����ǿ��ʹ�õ�ǰ�߳� */
        if (pThread == (TThread*)0)
        {
            pThread = OsKernelVariable.CurrentThread;
        }

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_SUSPEND)
            {
                state = SetThreadUnready(pThread, OsThreadSuspended, 0U, &HiRP, &error);
                if (HiRP == eTrue)
                {
                    OsThreadSchedule();
                }
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳̽�Һ���                                                                           *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclResumeThread(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_RESUME)
            {
                state = SetThreadReady(pThread, OsThreadSuspended, &HiRP, &error);
                if ((OsKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                {
                    OsThreadSchedule();
                }
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳���ʱģ��ӿں���                                                                   *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) ticks   ��Ҫ��ʱ�ĵδ���Ŀ                                                         *
 *        (3) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclDelayThread(TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;
    TThread* pThread;

    OS_ASSERT((ticks > 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ǿ��ʹ�õ�ǰ�߳� */
        pThread = OsKernelVariable.CurrentThread;

        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_DELAY)
            {
                state = SetThreadUnready(pThread, OsThreadDelayed, ticks, &HiRP, &error);
                if (HiRP == eTrue)
                {
                    OsThreadSchedule();
                }
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳���ʱȡ������                                                                       *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  ˵����(1) �����������ʱ�޵ȴ���ʽ������IPC�߳����������ϵ��߳���Ч                          *
 *************************************************************************************************/
TState TclUndelayThread(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_UNDELAY)
            {
                state = SetThreadReady(pThread, OsThreadDelayed, &HiRP, &error);
                if ((OsKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                {
                    OsThreadSchedule();
                }
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = OS_THREAD_ERR_UNREADY;
        }
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#if (TCLC_IPC_ENABLE)
/*************************************************************************************************
 *  ���ܣ�����߳���������                                                                       *
 *  ������(1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pError  ��ϸ���ý��                                                               *
 *  ���أ�(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclUnblockThread(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = OS_THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State == OsThreadState)
    {
        /* ����߳��Ƿ��Ѿ�����ʼ�� */
        if (pThread->Property & OS_THREAD_PROP_READY)
        {
            /* ����߳��Ƿ�������API���� */
            if (pThread->ACAPI & OS_THREAD_ACAPI_UNBLOCK)
            {
                if (pThread->Status == OsThreadBlocked)
                {
                    /*
                     * �����������ϵ�ָ�������߳��ͷ�
                     * ���̻߳����£������ǰ�̵߳����ȼ��Ѿ��������߳̾������е�������ȼ���
                     * �����ں˴�ʱ��û�йر��̵߳��ȣ���ô����Ҫ����һ���߳���ռ
                     */
                    OsIpcUnblockThread(pThread->IpcContext, eFailure, OS_IPC_ERR_ABORT, &HiRP);
                    if ((OsKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                    {
                        OsThreadSchedule();
                    }
                    error = OS_THREAD_ERR_NONE;
                    state = eSuccess;
                }
                else
                {
                    error = OS_THREAD_ERR_STATUS;
                }
            }
            else
            {
                error = OS_THREAD_ERR_ACAPI;
            }
        }
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}
#endif

