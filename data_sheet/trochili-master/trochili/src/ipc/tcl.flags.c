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
#include "tcl.thread.h"
#include "tcl.kernel.h"
#include "tcl.ipc.h"
#include "tcl.flags.h"

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))

/*************************************************************************************************
 *  ���ܣ����Խ����¼����                                                                       *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���յı�ǵ����                                                      *
 *        (3) option   �����¼���ǵĲ���                                                        *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState ReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_NORMAL;
    TBitMask match;
    TBitMask pattern;

    pattern = *pPattern;
    match = (pFlags->Value) & pattern;
    if (((option & OS_IPC_OPT_AND) && (match == pattern)) ||
            ((option & OS_IPC_OPT_OR) && (match != 0U)))
    {
        if (option & OS_IPC_OPT_CONSUME)
        {
            pFlags->Value &= (~match);
        }

        *pPattern = match;

        error = OS_IPC_ERR_NONE;
        state = eSuccess;
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ����Է����¼����                                                                       *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���͵ı�ǵ����                                                      *
 *        (3) pHiRP    �Ƿ��ں����л��ѹ������߳�                                                *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static TState SendFlags(TFlags* pFlags, TBitMask pattern, TBool* pHiRP, TError* pError)
{
    TState state = eError;
    TError error = OS_IPC_ERR_NORMAL;
    TLinkNode* pHead;
    TLinkNode* pTail;
    TLinkNode* pCurrent;
    TOption   option;
    TBitMask  mask;
    TBitMask* pTemp;
    TIpcContext* pContext;

    /* ����¼��Ƿ���Ҫ���� */
    mask = pFlags->Value | pattern;
    if (mask != pFlags->Value)
    {
        error = OS_IPC_ERR_NONE;
        state = eSuccess;

        /* ���¼����͵��¼������ */
        pFlags->Value |= pattern;

        /* �¼�����Ƿ����߳��ڵȴ��¼��ķ��� */
        if (pFlags->Property & OS_IPC_PROP_PRIMQ_AVAIL)
        {
            /* ��ʼ�����¼����������� */
            pHead = pFlags->Queue.PrimaryHandle;
            pTail = pFlags->Queue.PrimaryHandle->Prev;
            do
            {
                pCurrent = pHead;
                pHead = pHead->Next;

                /* ��õȴ��¼���ǵ��̺߳���ص��¼��ڵ� */
                pContext =  (TIpcContext*)(pCurrent->Owner);
                option = pContext->Option;
                pTemp = (TBitMask*)(pContext->Data.Addr1);

                /* �õ�����Ҫ����¼���� */
                mask = pFlags->Value & (*pTemp);
                if (((option & OS_IPC_OPT_AND) && (mask == *pTemp)) ||
                        ((option & OS_IPC_OPT_OR) && (mask != 0U)))
                {
                    *pTemp = mask;
                    OsIpcUnblockThread(pContext, eSuccess, OS_IPC_ERR_NONE, pHiRP);

                    /* ����ĳЩ�¼�������¼�ȫ�������Ĵ��������˳� */
                    if (option & OS_IPC_OPT_CONSUME)
                    {
                        pFlags->Value &= (~mask);
                        if (pFlags->Value == 0U)
                        {
                            break;
                        }
                    }
                }
            }
            while(pCurrent != pTail);
        }
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��߳�/ISR�����¼����                                                                   *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���յı�ǵ����                                                      *
 *        (3) timeo    ʱ������ģʽ�·����¼���ǵ�ʱ�޳���                                      *
 *        (4) option   �����¼���ǵĲ���                                                        *
 *        (5) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option, TTimeTick timeo,
                       TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TIpcContext context;
    TReg32 imask;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((option & (OS_IPC_OPT_AND | OS_IPC_OPT_OR)) != 0U, "");
    OS_ASSERT((pError != (TError*)0), "");

    /* ��������ѡ����β���Ҫ֧�ֵ�ѡ�� */
    option &= OS_USER_FLAG_OPTION;

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /*
         * ������жϳ�����ñ�������ֻ���Է�������ʽ����¼����,
         * ������ʱ�������̵߳������⡣
         * ���ж���,��ǰ�߳�δ������߾������ȼ��߳�,Ҳδ�ش����ں˾����̶߳��У�
         * �����ڴ˴��õ���HiRP������κ����塣
         */
        state = ReceiveFlags(pFlags, pPattern, option, &error);

        /*
         * ��Ϊ�¼�����̶߳����в�������¼����Ͷ��У����Բ���Ҫ�ж��Ƿ������߳�Ҫ���ȣ�
         * ����Ҫ�����Ƿ���Ҫ���¼����ĵ�����
         */
        if ((OsKernelVariable.State == OsThreadState) &&
                (OsKernelVariable.SchedLockTimes == 0U))
        {
            /*
             * �����ǰ�̲߳��ܵõ��¼������Ҳ��õ��ǵȴ���ʽ��
             * ��ô��ǰ�̱߳����������¼���ǵĵȴ������У�����ǿ���̵߳���
             */
            if (state == eFailure)
            {
                if (option & OS_IPC_OPT_WAIT)
                {
                    /* �����ǰ�̲߳��ܱ���������ֱ�ӷ��� */
                    if (OsKernelVariable.CurrentThread->ACAPI & OS_THREAD_ACAPI_BLOCK)
                    {
                        /* �����̹߳�����Ϣ */
                        OsIpcInitContext(&context, (void*)pFlags, (TBase32)pPattern, sizeof(TBase32),
                                         option | OS_IPC_OPT_FLAGS, timeo, &state, &error);

                        /* ��ǰ�߳������ڸ��¼���ǵ��������У�ʱ�޻������޵ȴ�����OS_IPC_OPT_TIMEO�������� */
                        OsIpcBlockThread(&context, &(pFlags->Queue));

                        /* ��ǰ�̱߳������������̵߳���ִ�� */
                        OsThreadSchedule();

                        OsCpuLeaveCritical(imask);
                        /*
                         * ��Ϊ��ǰ�߳��Ѿ�������IPC������߳��������У����Դ�������Ҫִ�б���̡߳�
                         * ���������ٴδ����߳�ʱ���ӱ����������С�
                         */
                        OsCpuEnterCritical(&imask);

                        /* ����߳�IPC������Ϣ */
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
 *  ���ܣ��߳�/ISR���¼���Ƿ����¼�                                                             *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pPattern ��Ҫ���յı�ǵ����                                                      *
 *        (3) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵������������������ǰ�߳�����,���Բ��������̻߳���ISR������                               *
 *************************************************************************************************/
TState TclSendFlags(TFlags* pFlags, TBitMask pattern, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /*
         * ������жϳ�����ñ�������ֻ���Է�������ʽ�����¼�,
         * ������ʱ�������̵߳������⡣
         * ���ж���,��ǰ�߳�δ������߾������ȼ��߳�,Ҳδ�ش����ں˾����̶߳��У�
         * �����ڴ˴��õ���HiRP������κ����塣
        */
        state = SendFlags(pFlags, pattern, &HiRP, &error);
        /*
         * �����ISR��������ֱ�ӷ��ء�
         * ֻ�����̻߳����²��������̵߳��Ȳſɼ�������
         */
        if ((OsKernelVariable.State == OsThreadState) &&
                (OsKernelVariable.SchedLockTimes == 0U))
        {
            /* �����ǰ�߳̽���˸������ȼ��̵߳���������е��ȡ�*/
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
 *  ���ܣ���ʼ���¼����                                                                         *
 *  ������(1) pFlags     �¼���ǵĵ�ַ                                                          *
 *        (2) pName      �¼���ǵ�����                                                          *
 *        (3) property   �¼���ǵĳ�ʼ����                                                      *
 *        (4) pError     ����������ϸ����ֵ                                                      *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateFlags(TFlags* pFlags, TChar* pName, TProperty property, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pName != (TChar*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    property &= OS_USER_FLAG_PROP;

    OsCpuEnterCritical(&imask);

    if (!(pFlags->Property & OS_IPC_PROP_READY))
    {
        /* ��ʼ���¼���Ƕ�����Ϣ */
        OsKernelAddObject(&(pFlags->Object), pName, OsFlagObject, (void*)pFlags);

        /* ��ʼ���¼���ǻ�����Ϣ */
        property |= OS_IPC_PROP_READY;
        pFlags->Property = property;
        pFlags->Value = 0U;

        pFlags->Queue.PrimaryHandle   = (TLinkNode*)0;
        pFlags->Queue.AuxiliaryHandle = (TLinkNode*)0;
        pFlags->Queue.Property        = &(pFlags->Property);

        state = eSuccess;
        error = OS_IPC_ERR_NONE;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ�ȡ���¼���ǳ�ʼ��                                                                     *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pError   ����������ϸ����ֵ                                                        *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclDeleteFlags(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /* �����������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������OS_IPC_ERR_DELETE  */
        OsIpcUnblockAll(&(pFlags->Queue), eFailure, OS_IPC_ERR_DELETE, (void**)0, &HiRP);

        /* ���ں����Ƴ��¼���Ƕ��� */
        OsKernelRemoveObject(&(pFlags->Object));

        /* ����¼���Ƕ����ȫ������ */
        memset(pFlags, 0U, sizeof(TFlags));

        /*
         * ���̻߳����£������ǰ�̵߳����ȼ��Ѿ��������߳̾������е�������ȼ���
         * �����ں˴�ʱ��û�йر��̵߳��ȣ���ô����Ҫ����һ���߳���ռ
         */
        if ((OsKernelVariable.State == OsThreadState) &&
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
 *  ����: ����¼������������                                                                   *
 *  ������(1) pFlags   �¼���ǵĵ�ַ                                                            *
 *        (2) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure   ����ʧ��                                                                *
 *        (2) eSuccess   �����ɹ�                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclResetFlags(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /* �����������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������OS_IPC_ERR_RESET */
        OsIpcUnblockAll(&(pFlags->Queue), eFailure, OS_IPC_ERR_RESET, (void**)0, &HiRP);

        pFlags->Property &= OS_RESET_FLAG_PROP;
        pFlags->Value = 0U;

        /*
         * ���̻߳����£������ǰ�̵߳����ȼ��Ѿ��������߳̾������е�������ȼ���
         * �����ں˴�ʱ��û�йر��̵߳��ȣ���ô����Ҫ����һ���߳���ռ
         */
        if ((OsKernelVariable.State == OsThreadState) &&
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
 *  ���ܣ��¼������ֹ����,��ָ�����̴߳��¼���ǵ��߳�������������ֹ����������                  *
 *  ������(1) pFlags   �¼���ǽṹ��ַ                                                          *
 *        (2) option   ����ѡ��                                                                  *
 *        (3) pThread  �̵߳�ַ                                                                  *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ���أ�(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclFlushFlags(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /* ���¼�������������ϵ����еȴ��̶߳��ͷţ������̵߳ĵȴ��������TCLE_IPC_FLUSH */
        OsIpcUnblockAll(&(pFlags->Queue), eFailure, OS_IPC_ERR_FLUSH, (void**)0, &HiRP);

        /*
         * ���̻߳����£������ǰ�̵߳����ȼ��Ѿ��������߳̾������е�������ȼ���
         * �����ں˴�ʱ��û�йر��̵߳��ȣ���ô����Ҫ����һ���߳���ռ
         */
        if ((OsKernelVariable.State == OsThreadState) &&
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

#endif

