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
#include "tcl.kernel.h"
#include "tcl.timer.h"
#include "tcl.thread.h"
#include "tcl.ipc.h"

#if (TCLC_IPC_ENABLE)

/*************************************************************************************************
 *  ���ܣ����̼߳��뵽ָ����IPC�߳�����������                                                    *
 *  ������(1) pQueue   IPC���е�ַ                                                               *
 *        (2) pThread  �߳̽ṹ��ַ                                                              *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void EnterBlockedQueue(TIpcQueue* pQueue, TIpcContext* pContext)
{
    TProperty property;

    property = *(pQueue->Property);
    if ((pContext->Option) & OS_IPC_OPT_UARGENT)
    {
        if (property & OS_IPC_PROP_PREEMP_AUXIQ)
        {
            OsObjQueueAddPriorityNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode));
        }
        else
        {
            OsObjQueueAddFifoNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode), OsLinkTail);
        }
        property |= OS_IPC_PROP_AUXIQ_AVAIL;
    }
    else
    {
        if (property & OS_IPC_PROP_PREEMP_PRIMIQ)
        {
            OsObjQueueAddPriorityNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode));
        }
        else
        {
            OsObjQueueAddFifoNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode), OsLinkTail);
        }
        property |= OS_IPC_PROP_PRIMQ_AVAIL;
    }

    *(pQueue->Property) = property;

    /* �����߳��������� */
    pContext->Queue = pQueue;
}


/*************************************************************************************************
 *  ���ܣ����̴߳�ָ�����̶߳������Ƴ�                                                           *
 *  ������(1) pQueue   IPC���е�ַ                                                               *
 *        (2) pThread  �߳̽ṹ��ַ                                                              *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void LeaveBlockedQueue(TIpcQueue* pQueue, TIpcContext* pContext)
{
    TProperty property;

    property = *(pQueue->Property);

    /* ���̴߳�ָ���ķֶ�����ȡ�� */
    if ((pContext->Option) & OS_IPC_OPT_UARGENT)
    {
        OsObjQueueRemoveNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode));
        if (pQueue->AuxiliaryHandle == (TLinkNode*)0)
        {
            property &= ~OS_IPC_PROP_AUXIQ_AVAIL;
        }
    }
    else
    {
        OsObjQueueRemoveNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode));
        if (pQueue->PrimaryHandle == (TLinkNode*)0)
        {
            property &= ~OS_IPC_PROP_PRIMQ_AVAIL;
        }
    }

    *(pQueue->Property) = property;

    /* �����߳��������� */
    pContext->Queue = (TIpcQueue*)0;
}


/*************************************************************************************************
 *  ���ܣ����̷߳�����Դ��������                                                                 *
 *  ������(1) pContext���������ַ                                                               *
 *        (2) pQueue  �̶߳��нṹ��ַ                                                           *
 *  ���أ���                                                                                     *
 *  ˵���������߳̽�����ض��еĲ��Ը��ݶ��в�������������                                       *
 *************************************************************************************************/
void OsIpcBlockThread(TIpcContext* pContext, TIpcQueue* pQueue)
{
    TThread* pThread;

    OS_ASSERT((OsKernelVariable.State != OsExtremeState), "");

    /* ����̵߳�ַ */
    pThread = (TThread*)(pContext->Owner);

    /* ֻ�д��ھ���״̬���̲߳ſ��Ա����� */
    if (pThread->Status != OsThreadRunning)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= OS_THREAD_DIAG_INVALID_STATE;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* ���̷߳����ں��̸߳������� */
    OsThreadLeaveQueue(OsKernelVariable.ThreadReadyQueue, pThread);
    OsThreadEnterQueue(OsKernelVariable.ThreadAuxiliaryQueue, pThread, OsLinkTail);
    pThread->Status = OsThreadBlocked;

    /* ���̷߳����������� */
    EnterBlockedQueue(pQueue, pContext);

    /* �����Ҫ�������߳����ڷ�����Դ��ʱ�޶�ʱ�� */
    if ((pContext->Option & OS_IPC_OPT_TIMEO) && (pContext->Ticks > 0U))
    {
        pThread->Timer.RemainTicks = pContext->Ticks;
        OsObjListAddDiffNode(&(OsKernelVariable.ThreadTimerList),
                             &(pThread->Timer.LinkNode));
    }
}


/*************************************************************************************************
 *  ���ܣ�����IPC����������ָ�����߳�                                                            *
 *  ������(1) pContext���������ַ                                                               *
 *        (2) state   �߳���Դ���ʷ��ؽ��                                                       *
 *        (3) error   ��ϸ���ý��                                                               *
 *        (4) pHiRP   �Ƿ����Ѹ������ȼ���������Ҫ�����̵߳��ȵı��                           *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsIpcUnblockThread(TIpcContext* pContext, TState state, TError error, TBool* pHiRP)
{
    TThread* pThread;
    pThread = (TThread*)(pContext->Owner);

    /* ֻ�д�������״̬���̲߳ſ��Ա�������� */
    if (pThread->Status != OsThreadBlocked)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= OS_THREAD_DIAG_INVALID_STATE;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /*
     * �����̣߳�����̶߳��к�״̬ת��,ע��ֻ���жϴ���ʱ��
     * ��ǰ�̲߳Żᴦ���ں��̸߳���������(��Ϊ��û���ü��߳��л�)
     * ��ǰ�̷߳��ؾ�������ʱ��һ��Ҫ�ص���Ӧ�Ķ���ͷ
     * ���߳̽�����������ʱ������Ҫ�����̵߳�ʱ�ӽ�����
     */
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

    /* ���̴߳����������Ƴ� */
    LeaveBlockedQueue(pContext->Queue, pContext);

    /* �����̷߳�����Դ�Ľ���ʹ������ */
    *(pContext->State) = state;
    *(pContext->Error) = error;

    /* ����߳�����ʱ�޷�ʽ������Դ��رո��̵߳�ʱ�޶�ʱ�� */
    if (pContext->Option & OS_IPC_OPT_TIMEO)
    {
        OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList),
                                &(pThread->Timer.LinkNode));
    }

    /* �����̵߳���������,�˱��ֻ���̻߳�������Ч��
     * ��ISR���ǰ�߳̿������κζ��������ǰ�߳���Ƚ����ȼ�Ҳ��������ġ�
     * ���̻߳����£������ǰ�̵߳����ȼ��Ѿ��������߳̾������е�������ȼ���
     * �����ں˴�ʱ��û�йر��̵߳��ȣ���ô����Ҫ����һ���߳���ռ
     */
    if (pThread->Priority < OsKernelVariable.CurrentThread->Priority)
    {
        *pHiRP = eTrue;
    }
}


/*************************************************************************************************
 *  ���ܣ�ѡ�������������е�ȫ���߳�                                                           *
 *  ������(1) pQueue  �̶߳��нṹ��ַ                                                           *
 *        (2) state   �߳���Դ���ʷ��ؽ��                                                       *
 *        (3) error   ��ϸ���ý��                                                               *
 *        (4) pData   �̷߳���IPC�õ�������                                                      *
 *        (5) pHiRP  �߳��Ƿ���Ҫ���ȵı��                                                      *
 *  ���أ�                                                                                       *
 *  ˵����ֻ���������Ϣ���й㲥ʱ�Żᴫ��pData2����                                             *
 *************************************************************************************************/
void OsIpcUnblockAll(TIpcQueue* pQueue, TState state, TError error, void** pData2, TBool* pHiRP)
{
    TIpcContext* pContext;

    /* ���������е��߳����ȱ�������� */
    while (pQueue->AuxiliaryHandle != (TLinkNode*)0)
    {
        pContext = (TIpcContext*)(pQueue->AuxiliaryHandle->Owner);
        OsIpcUnblockThread(pContext, state, error, pHiRP);

        /* ���������������߳����ڵȴ���ȡ���� */
        if ((pContext->Option & OS_IPC_OPT_READ_DATA) &&
                (pContext->Data.Addr2 != (void**)0) &&
                (pData2 != (void**)0) )
        {
            *(pContext->Data.Addr2) = *pData2;
        }
    }

    /* ���������е��߳���󱻽������ */
    while (pQueue->PrimaryHandle != (TLinkNode*)0)
    {
        pContext = (TIpcContext*)(pQueue->PrimaryHandle->Owner);
        OsIpcUnblockThread(pContext, state, error, pHiRP);

        /* ���������������߳����ڵȴ���ȡ���� */
        if ((pContext->Option & OS_IPC_OPT_READ_DATA) &&
                (pContext->Data.Addr2 != (void**)0) &&
                (pData2 != (void**)0) )
        {
            *(pContext->Data.Addr2) = *pData2;
        }
    }
}


/*************************************************************************************************
 *  ���ܣ��ı䴦��IPC���������е��̵߳����ȼ�                                                    *
 *  ������(1) pContext ���������ַ                                                              *
 *        (2) priority ��Դ�ȴ�ʱ��                                                              *
 *  ���أ���                                                                                     *
 *  ˵��������߳������������в������ȼ����ԣ����̴߳������������������Ƴ���Ȼ���޸��������ȼ�,*
 *        ����ٷŻ�ԭ���С�����������ȳ������򲻱ش���                                       *
 *************************************************************************************************/
void OsIpcSetPriority(TIpcContext* pContext, TPriority priority)
{
    TProperty property;
    TIpcQueue* pQueue;

    pQueue = pContext->Queue;

    /* ����ʵ����������°����߳���IPC�����������λ�� */
    property = *(pContext->Queue->Property);
    if (pContext->Option & OS_IPC_OPT_UARGENT)
    {
        if (property & OS_IPC_PROP_PREEMP_AUXIQ)
        {
            OsObjQueueRemoveNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode));
            OsObjQueueAddPriorityNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode));
        }
    }
    else
    {
        if (property & OS_IPC_PROP_PREEMP_PRIMIQ)
        {
            OsObjQueueRemoveNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode));
            OsObjQueueAddPriorityNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode));
        }
    }
}


/*************************************************************************************************
 *  ���ܣ��趨�����̵߳�IPC�������Ϣ                                                            *
 *  ������(1) pContext���������ַ                                                               *
 *        (2) pIpc    ���ڲ�����IPC����ĵ�ַ                                                    *
 *        (3) data    ָ������Ŀ�����ָ���ָ��                                                 *
 *        (4) len     ���ݵĳ���                                                                 *
 *        (5) option  ����IPC����ʱ�ĸ��ֲ���                                                    *
 *        (6) state   IPC������ʽ��                                                            *
 *        (7) pError  ��ϸ���ý��                                                               *
 *  ���أ���                                                                                     *
 *  ˵����dataָ���ָ�룬������Ҫͨ��IPC���������ݵ��������߳̿ռ��ָ��                        *
 *************************************************************************************************/
void OsIpcInitContext(TIpcContext* pContext, void* pIpc, TBase32 data, TBase32 len,
                      TOption option, TTimeTick ticks, TState* pState, TError* pError)
{
    TThread* pThread;

    pThread = OsKernelVariable.CurrentThread;
    pThread->IpcContext = pContext;

    pContext->Object     = pIpc;
    pContext->Queue      = (TIpcQueue*)0;
    pContext->Data.Value = data;
    pContext->Length     = len;
    pContext->Option     = option;
    pContext->Ticks      = ticks;
    pContext->State      = pState;
    pContext->Error      = pError;
    pContext->Owner      = (void*)pThread;
    pContext->LinkNode.Next   = (TLinkNode*)0;
    pContext->LinkNode.Prev   = (TLinkNode*)0;
    pContext->LinkNode.Handle = (TLinkNode**)0;
    pContext->LinkNode.Data   = (TBase32*)(&(pThread->Priority));
    pContext->LinkNode.Owner  = (void*)pContext;

    *pState = eError;
    *pError = OS_IPC_ERR_FAULT;
}


/*************************************************************************************************
 *  ���ܣ���������̵߳�IPC�������Ϣ                                                            *
 *  ������(1) pContext ���������ַ                                                              *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsIpcCleanContext(TIpcContext* pContext)
{
    TThread* pThread;

    pThread = (TThread*)(pContext->Owner);
    pThread->IpcContext = (TIpcContext*)0;

    memset(pContext, 0U, sizeof(TIpcContext));
}

#endif

