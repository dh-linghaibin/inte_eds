/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"
#include "tcl.ipc.h"
#include "tcl.mutex.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MUTEX_ENABLE))

/*************************************************************************************************
 *  ����: ����ʹ���̻߳�û��⻥����                                                             *
 *  ����: (1) pThread  �߳̽ṹ��ַ                                                              *
 *        (2) pMutex   �������ṹ��ַ                                                            *
 *        (3) pHiRP    �Ƿ��и������ȼ�����                                                      *
 *  ����: (1) ��                                                                                 *
 *  ˵����������һ���ǵ�ǰ�̵߳��ã����ߵ�ǰ�̻߳�û����������߰ѻ�������������߳�             *
 *        ������ĳ�������ȼ�������ߣ����Ը���ǰ�߳�ֱ�ӱȽ����ȼ�                               *
 *************************************************************************************************/
/* 1 ���̻߳����£��������ض�����ǰ�̵߳���
     1.1 ��ǰ�߳̿��ܻ���ñ�����(lock)��ռ�õĻ�������
     1.2 ��ǰ�߳̿��ܻ���ñ�����(free)�����������������߳�(���������ľ���״̬)
   2 ��isr�����²����ܵ��ñ����� */
static TState AddLock(TThread* pThread, TMutex* pMutex, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = OS_IPC_ERR_NONE;

    /* �������������߳������У������ȼ����� */
    OsObjListAddPriorityNode(&(pThread->LockList), &(pMutex->LockNode));
    pMutex->Nest = 1U;
    pMutex->Owner = pThread;

    /* ����߳����ȼ�û�б��̶� */
    if (!(pThread->Property & OS_THREAD_PROP_PRIORITY_FIXED))
    {
        /* �߳����ȼ���mutex��������API�����޸� */
        pThread->Property &= ~(OS_THREAD_PROP_PRIORITY_SAFE);

        /* PCP �õ�������֮�󣬵�ǰ�߳�ʵʩ�컨���㷨,��Ϊ���߳̿��ܻ�ö����������
        ���̵߳ĵ�ǰ���ȼ����ܱ��»�õĻ��������컨�廹�ߡ� �����������Ƚ�һ�����ȼ���
        ����ֱ�����ó��»��������컨�����ȼ� */
        if (pThread->Priority > pMutex->Priority)
        {
            state = OsThreadSetPriority(pThread, pMutex->Priority, eFalse, pHiRP, &error);
        }
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ���߳���������ɾ��������                                                               *
 *  ����: (1) pThread �߳̽ṹ��ַ                                                               *
 *        (2) pMutex  �������ṹ��ַ                                                             *
 *        (3) pHiRP   �Ƿ��и������ȼ�����                                                       *
 *  ����: ��                                                                                     *
 *  ˵������ǰ�߳����ȼ����ͣ�ֻ�ܸ������̱߳Ƚ����ȼ�                                           *
 *************************************************************************************************/
static TState RemoveLock(TThread* pThread, TMutex* pMutex, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = OS_IPC_ERR_NONE;
    TPriority priority = TCLC_LOWEST_PRIORITY;
    TLinkNode* pHead = (TLinkNode*)0;
    TBool     nflag = eFalse;

    /* �����������߳����������Ƴ� */
    pHead = pThread->LockList;
    OsObjListRemoveNode(&(pThread->LockList), &(pMutex->LockNode));
    pMutex->Owner = (TThread*)0;
    pMutex->Nest = 0U;

    /* ����߳����ȼ�û�б��̶� */
    if (!(pThread->Property & OS_THREAD_PROP_PRIORITY_FIXED))
    {
        /* ����߳�������Ϊ�գ����߳����ȼ��ָ����������ȼ�,
           ��mutex��߳����ȼ�һ���������̻߳������ȼ� */
        if (pThread->LockList == (TLinkNode*)0)
        {
            /* ����߳�û��ռ�б�Ļ�������,�������߳����ȼ����Ա�API�޸� */
            pThread->Property |= (OS_THREAD_PROP_PRIORITY_SAFE);

            /* ׼���ָ��߳����ȼ� */
            priority = pThread->BasePriority;
            nflag = eTrue;
        }
        else
        {
            /* ��Ϊ�������ǰ������ȼ��½����������̵߳���һ�����ȼ�һ������Ȼ��ߵ͵�,
               ע��ɾ�����������ڶ�������κ�λ�ã���������ڶ���ͷ������Ҫ�����߳����ȼ� */
            if (pHead == &(pMutex->LockNode))
            {
                /* ׼���ָ��߳����ȼ� */
                priority = *((TPriority*)(pThread->LockList->Data));
                nflag = eTrue;
            }
        }

        /* ����߳����ȼ��б仯(nflag = eTrue)������Ҫ����(priority > pThread->Priority) */
        if (nflag && (priority > pThread->Priority))
        {
            /* �޸��߳����ȼ� */
            state = OsThreadSetPriority(pThread, priority, eFalse, pHiRP, &error);

        }
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: �̻߳�û��⻥����                                                                     *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pHiRP    �Ƿ��и������ȼ�����                                                      *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *        (3) eError   ��������                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState LockMutex(TMutex* pMutex, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = OS_IPC_ERR_NONE;

    /* �̻߳�û���������
     * Priority Ceilings Protocol
     * ������ɹ�, PCP�����µ�ǰ�߳����ȼ����ή��,ֱ�ӷ���
     * �����ʧ�ܲ����Ƿ�������ʽ���ʻ�������ֱ�ӷ���
     * �����ʧ�ܲ�����������ʽ���ʻ����������߳������ڻ����������������У�Ȼ����ȡ�
    */
    if (pMutex->Owner == (TThread*)0)
    {
        /*
         * ��ǰ�̻߳�û����������ȼ���ʹ�б䶯Ҳ���ɱ������, ����Ҫ�߳����ȼ���ռ��
         * HiRP��ֵ��ʱ���ô�
         */
        state = AddLock(OsKernelVariable.CurrentThread, pMutex, pHiRP, &error);
    }
    else if (pMutex->Owner == OsKernelVariable.CurrentThread)
    {
        pMutex->Nest++;
    }
    else
    {
        error = OS_IPC_ERR_NORMAL;
        state = eFailure;
    }

    *pError  = error;
    return state;
}

/*************************************************************************************************
 *  ����: �ͷŻ��⻥����                                                                         *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pHiRP    �Ƿ��и������ȼ�����                                                      *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����ֻ�е�ǰ�̲߳��ܹ��ͷ�ĳ��������������ǰ�߳�һ����������״̬��                         *
 *        Ҳ�Ͳ�������ʽ���ȼ�����������                                                         *
 *************************************************************************************************/
static TState FreeMutex(TMutex* pMutex, TBool* pHiRP, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_NORMAL;
    TIpcContext* pContext;
    TThread* pThread;

    /* ֻ��ռ�л��������̲߳����ͷŸû����� */
    if (pMutex->Owner == OsKernelVariable.CurrentThread)
    {
        /* ���߳�Ƕ��ռ�л�����������£���Ҫ����������Ƕ�״��� */
        pMutex->Nest--;

        /*
         * ���������Ƕ����ֵΪ0��˵��Ӧ�ó����ͷŻ�����,
         * �����ǰ�߳������������ȼ��컨��Э�飬���ǵ����߳����ȼ�
         */
        if (pMutex->Nest == 0U)
        {
            /* �����������߳����������Ƴ�,���û�����������Ϊ��. */
            state = RemoveLock(OsKernelVariable.CurrentThread, pMutex, pHiRP, &error);

            /* ���Դӻ���������������ѡ����ʵ��̣߳�ʹ�ø��̵߳õ������� */
            if (pMutex->Property & OS_IPC_PROP_PRIMQ_AVAIL)
            {
                pContext = (TIpcContext*)(pMutex->Queue.PrimaryHandle->Owner);
                OsIpcUnblockThread(pContext, eSuccess, OS_IPC_ERR_NONE, pHiRP);

                pThread = (TThread*)(pContext->Owner);
                state = AddLock(pThread, pMutex, pHiRP, &error);
            }
        }

        error = OS_IPC_ERR_NONE;
        state = eSuccess;
    }

    *pError = error;
    return state;
}

/*
 * ���������ݲ�������ISR�б�����
 */

/*************************************************************************************************
 *  ����: �ͷŻ��⻥����                                                                         *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����mutex֧������Ȩ�ĸ�������߳��ͷ�mutex�Ĳ����������̷��ص�,���ͷ�mutex�������ᵼ��   *
 *        �߳�������mutex���߳�����������                                                        *
 *************************************************************************************************/
TState TclFreeMutex(TMutex* pMutex, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pMutex != (TMutex*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_IPC_ERROR;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (pMutex->Property & OS_IPC_PROP_READY)
    {
        state = FreeMutex(pMutex, &HiRP, &error);
        if (OsKernelVariable.SchedLockTimes == 0U)
        {
            if (state == eSuccess)
            {
                if (HiRP == eTrue)
                {
                    OsThreadSchedule();
                }
            }
        }

    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: �̻߳�û��⻥����                                                                     *
 *  ����: (1) pMutex �������ṹ��ַ                                                              *
 *        (2) option   ���������ģʽ                                                            *
 *        (3) timeo    ʱ������ģʽ�·��ʻ�������ʱ�޳���                                        *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *        (3) eError   ��������                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
/*
 * �̲߳��÷�������ʽ��������ʽ����ʱ�޵ȴ���ʽ��û�����
 * Priority Ceilings Protocol
 * ������ɹ�, PCP�����µ�ǰ�߳����ȼ����ή��,ֱ�ӷ���
 * �����ʧ�ܲ����Ƿ�������ʽ���ʻ�������ֱ�ӷ���
 * �����ʧ�ܲ�����������ʽ���ʻ����������߳������ڻ����������������У�Ȼ����ȡ�
 */
TState TclLockMutex(TMutex* pMutex, TOption option, TTimeTick timeo, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TIpcContext context;
    TReg32 imask;

    OS_ASSERT((pMutex != (TMutex*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= OS_USER_MUTEX_OPTION;

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_IPC_ERROR;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (pMutex->Property & OS_IPC_PROP_READY)
    {
        state = LockMutex(pMutex, &HiRP, &error);
        if (OsKernelVariable.SchedLockTimes == 0U)
        {
            if (state == eFailure)
            {
                if (option & OS_IPC_OPT_WAIT)
                {
                    /* �����ǰ�̲߳��ܱ���������ֱ�ӷ��� */
                    if (OsKernelVariable.CurrentThread->ACAPI & OS_THREAD_ACAPI_BLOCK)
                    {
                        /* �趨�߳����ڵȴ�����Դ����Ϣ */
                        OsIpcInitContext(&context, (void*)pMutex, 0U, 0U,
                                         (option | OS_IPC_OPT_MUTEX), timeo, &state, &error);

                        /*
                         * ��ǰ�߳������ڸû��������������У�ʱ�޻������޵ȴ���
                         * ��OS_IPC_OPT_TIMEO��������
                         */
                        OsIpcBlockThread(&context, &(pMutex->Queue));

                        /* ��ǰ�̱߳������������̵߳���ִ�� */
                        OsThreadSchedule();

                        OsCpuLeaveCritical(imask);
                        /*
                        * ��Ϊ��ǰ�߳��Ѿ�������IPC������߳��������У����Դ�������Ҫִ�б���̡߳�
                        * ���������ٴδ����߳�ʱ���ӱ����������С�
                        */
                        OsCpuEnterCritical(&imask);

                        /* ����̹߳�����Ϣ */
                        OsIpcCleanContext(&context);
                    }
                    else
                    {
                        error = OS_IPC_ERR_ACAPI;
                    }
                }
            }
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ��ʼ��������                                                                           *
 *  ����: (1) pMute    �������ṹ��ַ                                                            *
 *        (2) pName    ������������                                                              *
 *        (3) property �������ĳ�ʼ����                                                          *
 *        (4) priority �����������ȼ��컨��                                                      *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateMutex(TMutex* pMutex, TChar* pName, TProperty property, TPriority priority,
                      TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((pMutex != (TMutex*)0), "");
    OS_ASSERT((pName != (TChar*)0), "");
    OS_ASSERT((priority <= TCLC_USER_PRIORITY_LOW), "");
    OS_ASSERT((priority >= TCLC_USER_PRIORITY_HIGH), "");
    OS_ASSERT((pError != (TError*)0), "");

    property &= OS_USER_MUTEX_PROP;

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_IPC_ERROR;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (!(pMutex->Property & OS_IPC_PROP_READY))
    {
        /* ��ʼ��������������Ϣ */
        OsKernelAddObject(&(pMutex->Object), pName, OsMutexObject, (void*)pMutex);

        /* ��ʼ��������������Ϣ */
        property |= OS_IPC_PROP_READY;
        pMutex->Property = property;
        pMutex->Nest = 0U;
        pMutex->Owner = (TThread*)0;
        pMutex->Priority = priority;

        pMutex->Queue.PrimaryHandle   = (TLinkNode*)0;
        pMutex->Queue.AuxiliaryHandle = (TLinkNode*)0;
        pMutex->Queue.Property        = &(pMutex->Property);

        pMutex->LockNode.Owner = (void*)pMutex;
        pMutex->LockNode.Data = (TBase32*)(&(pMutex->Priority));
        pMutex->LockNode.Next = 0;
        pMutex->LockNode.Prev = 0;
        pMutex->LockNode.Handle = (TLinkNode**)0;

        error = OS_IPC_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: �����������ʼ��                                                                       *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclDeleteMutex(TMutex* pMutex, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_FAULT;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pMutex != (TMutex*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_IPC_ERROR;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (pMutex->Property & OS_IPC_PROP_READY)
    {
        /* ֻ�е����������߳�ռ�е�����£����п��ܴ��ڱ��������������߳� */
        if (pMutex->Owner != (TThread*)0)
        {
            /* �����������߳����������Ƴ� */
            state = RemoveLock(pMutex->Owner, pMutex, &HiRP, &error);

            /* �����������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������OS_IPC_ERR_DELETE��
             * ������Щ�̵߳����ȼ�һ�������ڻ����������ߵ����ȼ�
             */
            OsIpcUnblockAll(&(pMutex->Queue), eFailure, OS_IPC_ERR_DELETE,
                            (void**)0, &HiRP);
        }

        /* ���ں����Ƴ������� */
        OsKernelRemoveObject(&(pMutex->Object));

        /* ��������������ȫ������ */
        memset(pMutex, 0U, sizeof(TMutex));

        /*
         * ���̻߳����£������ǰ�̵߳����ȼ��Ѿ��������߳̾������е�������ȼ���
         * �����ں˴�ʱ��û�йر��̵߳��ȣ���ô����Ҫ����һ���߳���ռ
         */
        if (/* (OsKernelVariable.State == OsThreadState) && */
            (OsKernelVariable.SchedLockTimes == 0U) &&
            (HiRP == eTrue))
        {
            OsThreadSchedule();
        }

        state = eSuccess;
        error = OS_IPC_ERR_NONE;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ���û�����                                                                             *
 *  ����: (1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclResetMutex(TMutex* pMutex, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pMutex != (TMutex*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_IPC_ERROR;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (pMutex->Property & OS_IPC_PROP_READY)
    {

        /* ֻ�е����������߳�ռ�е�����£����п��ܴ��ڱ��������������߳� */
        if (pMutex->Owner != (TThread*)0)
        {
            /* �����������߳����������Ƴ� */
            state = RemoveLock(pMutex->Owner, pMutex, &HiRP, &error);

            /* �����������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������OS_IPC_ERR_RESET��
               ������Щ�̵߳����ȼ�һ�������ڻ����������ߵ����ȼ� */
            OsIpcUnblockAll(&(pMutex->Queue), eFailure, OS_IPC_ERR_RESET,
                            (void**)0, &HiRP);

            /* �ָ����������� */
            pMutex->Property &= OS_RESET_MUTEX_PROP;
        }

        /* ռ�и���Դ�Ľ���Ϊ�� */
        pMutex->Property &= OS_RESET_MUTEX_PROP;
        pMutex->Owner = (TThread*)0;
        pMutex->Nest = 0U;
        /* pMutex->Priority = keep recent value; */
        pMutex->LockNode.Owner = (void*)0;
        pMutex->LockNode.Data = (TBase32*)0;

        /*
         * ���̻߳����£������ǰ�̵߳����ȼ��Ѿ��������߳̾������е�������ȼ���
         * �����ں˴�ʱ��û�йر��̵߳��ȣ���ô����Ҫ����һ���߳���ռ
         */
        if (/* (OsKernelVariable.State == OsThreadState) && */
            (OsKernelVariable.SchedLockTimes == 0U) &&
            (HiRP == eTrue))
        {
            OsThreadSchedule();
        }

        state = eSuccess;
        error = OS_IPC_ERR_NONE;

    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ�������������ֹ����,��ָ�����̴߳ӻ��������߳�������������ֹ����������                  *
 *  ������(1) pMutex   �������ṹ��ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclFlushMutex(TMutex* pMutex, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pMutex != (TMutex*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* ֻ�������̴߳�������ñ����� */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_IPC_ERROR;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (pMutex->Property & OS_IPC_PROP_READY)
    {
        /* �����������������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������OS_IPC_ERR_FLUSH */
        OsIpcUnblockAll(&(pMutex->Queue), eFailure, OS_IPC_ERR_FLUSH, (void**)0, &HiRP);

        state = eSuccess;
        error = OS_IPC_ERR_NONE;

        /*
         * ���̻߳����£������ǰ�̵߳����ȼ��Ѿ��������߳̾������е�������ȼ���
         * �����ں˴�ʱ��û�йر��̵߳��ȣ���ô����Ҫ����һ���߳���ռ
         */
        if (/* (OsKernelVariable.State == OsThreadState) && */
            (OsKernelVariable.SchedLockTimes == 0U) &&
            (HiRP == eTrue))
        {
            OsThreadSchedule();
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}
#endif

