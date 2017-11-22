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
#include "tcl.kernel.h"
#include "tcl.thread.h"
#include "tcl.debug.h"
#include "tcl.irq.h"

#if (TCLC_IRQ_ENABLE)

/* �ж��������������� */
#define IRQ_VECTOR_PROP_NONE   (TProperty)(0x0)
#define IRQ_VECTOR_PROP_READY  (TProperty)(0x1<<0)
#define IRQ_VECTOR_PROP_LOCKED (TProperty)(0x1<<1)

#if (TCLC_IRQ_DAEMON_ENABLE)
/* IRQ����������Ͷ��� */
typedef struct IrqListDef
{
    TLinkNode* Handle;
} TIrqList;
#endif

/* �ں��ж������� */
static TIrqVector IrqVectorTable[TCLC_IRQ_VECTOR_NUM];

/* MCU�жϺŵ��ں��ж�������ת���� */
static TAddr32 IrqMapTable[TCLC_CPU_IRQ_NUM];


/*************************************************************************************************
 *  ���ܣ��жϴ����������                                                                     *
 *  ������(1) irqn �жϺ�                                                                        *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsIrqEnterISR(TIndex irqn)
{
    TReg32      imask;
    TIrqVector* pVector;
    TISR        pISR;
    TArgument   data;
    TBitMask    retv;

    OS_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");
    OsCpuEnterCritical(&imask);

    /* ��ú��жϺŶ�Ӧ���ж����� */
    pVector = (TIrqVector*)(IrqMapTable[irqn]);
    if ((pVector != (TIrqVector*)0) &&
            (pVector->Property & IRQ_VECTOR_PROP_READY))
    {
        /* �ڴ����ж϶�Ӧ������ʱ����ֹ���������޸����� */
        pVector->Property |= IRQ_VECTOR_PROP_LOCKED;

        /* ���жϻ����µ��õͼ��жϴ����� */
        if (pVector->ISR != (TISR)0)
        {
            pISR = pVector->ISR;
            data = pVector->Argument;
            OsCpuLeaveCritical(imask);
            retv = pISR(data);
            OsCpuEnterCritical(&imask);

            /*
             * �����Ҫ������жϴ����߳�DAEMON(�ں��ж��ػ��߳�),
             * ע���ʱDAEMON���ܴ���OsThreadReady״̬
             */
#if (TCLC_IRQ_DAEMON_ENABLE)
            if (retv & OS_IRQ_DAEMON)
            {
                OsThreadResumeFromISR(OsKernelVariable.IrqDaemon);

            }
#endif
        }
        pVector->Property &= (~IRQ_VECTOR_PROP_LOCKED);
    }

    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ������ж���������                                                                       *
 *  ������(1) irqn     �жϺ�                                                                    *
 *        (2) pISR     ISR������                                                               *
 *        (3) data     Ӧ���ṩ�Ļص�����                                                        *
 *        (4) pError   ��ϸ���ý��                                                              *
 *  ����: (1) eFailure ����ʧ��                                                                  *
 *        (2) eSuccess �����ɹ�                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclSetIrqVector(TIndex irqn, TISR pISR, TArgument data, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IRQ_ERR_FAULT;
    TReg32 imask;
    TIndex index;
    TIrqVector* pVector;

    OS_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");
    OS_ASSERT((pISR != (TISR)0), "");

    OsCpuEnterCritical(&imask);

    /* ���ָ�����жϺ��Ѿ�ע����ж���������ôֱ�Ӹ��� */
    if (IrqMapTable[irqn] != (TAddr32)0)
    {
        pVector = (TIrqVector*)(IrqMapTable[irqn]);

        /* ����֮ǰȷ��û�б����� */
        if ((pVector->Property & IRQ_VECTOR_PROP_LOCKED))
        {
            error = OS_IRQ_ERR_LOCKED;
        }
        else
        {
            /* �����ж�������Ӧ���жϷ������ */
            pVector->ISR      = pISR;
            pVector->Argument = data;

            error = OS_IRQ_ERR_NONE;
            state = eSuccess;
        }
    }
    else
    {
        /* Ϊ���жϺ������ж������� */
        for (index = 0; index < TCLC_IRQ_VECTOR_NUM; index++)
        {
            pVector = (TIrqVector*)IrqVectorTable + index;
            if (!(pVector->Property & IRQ_VECTOR_PROP_READY))
            {
                /*
                 * �����жϺźͶ�Ӧ���ж���������ϵ,
                 * �����ж�������Ӧ���жϷ������
                 */
                IrqMapTable[irqn] = (TAddr32)pVector;
                pVector->IRQn     = irqn;
                pVector->ISR      = pISR;
                pVector->Argument = data;
                pVector->Property = IRQ_VECTOR_PROP_READY;

                error = OS_IRQ_ERR_NONE;
                state = eSuccess;
                break;
            }
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ�����ж���������                                                                       *
 *  ������(1) irqn   �жϱ��                                                                    *
 *        (2) pError ��ϸ���ý��                                                                *
 *  ����: (1) eSuccess �����ɹ�                                                                  *
 *        (2) eFailure ����ʧ��                                                                  *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCleanIrqVector(TIndex irqn, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IRQ_ERR_FAULT;
    TReg32 imask;
    TIrqVector* pVector;

    OS_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");

    OsCpuEnterCritical(&imask);

    /* �ҵ����ж�����������������Ϣ */
    if (IrqMapTable[irqn] != (TAddr32)0)
    {
        pVector = (TIrqVector*)(IrqMapTable[irqn]);
        if ((pVector->Property & IRQ_VECTOR_PROP_READY) &&
                (pVector->IRQn == irqn))
        {
            if (!(pVector->Property & IRQ_VECTOR_PROP_LOCKED))
            {
                IrqMapTable[irqn] = (TAddr32)0;
                memset(pVector, 0U, sizeof(TIrqVector));
                error = OS_IRQ_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                error = OS_IRQ_ERR_LOCKED;
            }
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}



#if (TCLC_IRQ_DAEMON_ENABLE)

/* IRQ�ػ��̶߳����ջ���� */
static TThread IrqDaemonThread;
static TBase32 IrqDaemonStack[TCLC_IRQ_DAEMON_STACK_BYTES >> 2];

/* IRQ�ػ��̲߳������κ��̹߳���API���� */
#define IRQ_DAEMON_ACAPI (OS_THREAD_ACAPI_NONE)

/* IRQ������� */
static TIrqList IrqReqList;


/*************************************************************************************************
 *  ���ܣ��ύ�ж�����                                                                           *
 *  ������(1) pIRQ      �ж�����ṹ��ַ                                                         *
 *        (2) pEntry    �жϴ���ص�����                                                         *
 *        (3) data      �жϴ���ص�����                                                         *
 *        (4) priority  �ж��������ȼ�                                                           *
 *        (5) pError    ��ϸ���ý��                                                             *
 *  ����: (1) eFailure  ����ʧ��                                                                 *
 *        (2) eSuccess  �����ɹ�                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclPostIRQ(TIrq* pIRQ, TIrqEntry pEntry, TArgument data, TPriority priority,
                         TError* pError)
{
    TState state = eFailure;
    TError error = OS_IRQ_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((pIRQ != (TIrq*)0), "");

    OsCpuEnterCritical(&imask);

    if (!(pIRQ->Property & OS_IRQ_PROP_READY))
    {
        pIRQ->Property        = OS_IRQ_PROP_READY;
        pIRQ->Entry           = pEntry;
        pIRQ->Argument        = data;
        pIRQ->Priority        = priority;
        pIRQ->LinkNode.Next   = (TLinkNode*)0;
        pIRQ->LinkNode.Prev   = (TLinkNode*)0;
        pIRQ->LinkNode.Handle = (TLinkNode**)0;
        pIRQ->LinkNode.Data   = (TBase32*)(&(pIRQ->Priority));
        pIRQ->LinkNode.Owner  = (void*)pIRQ;
        OsObjListAddPriorityNode(&(IrqReqList.Handle), &(pIRQ->LinkNode));

        error = OS_IRQ_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ������ж�����                                                                           *
 *  ������(1) pIRQ      �ж�����ṹ��ַ                                                         *
 *        (2) pError    ��ϸ���ý��                                                             *
 *  ����: (1) eFailure  ����ʧ��                                                                 *
 *        (2) eSuccess  �����ɹ�                                                                 *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCancelIRQ(TIrq* pIRQ, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IRQ_ERR_UNREADY;
    TReg32 imask;

    OS_ASSERT((pIRQ != (TIrq*)0), "");

    OsCpuEnterCritical(&imask);
    if (pIRQ->Property & OS_IRQ_PROP_READY)
    {
        OsObjListRemoveNode( pIRQ->LinkNode.Handle, &(pIRQ->LinkNode));
        memset(pIRQ, 0U, sizeof(TIrq));

        error = OS_IRQ_ERR_NONE;
        state = eSuccess;
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ���ܣ��ں��е�IRQ�ػ��̺߳���                                                                *
 *  ������(1) argument IRQ�ػ��̵߳Ĳ���                                                         *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
static void IrqDaemonEntry(TArgument argument)
{
    TReg32    imask;
    TIrq*     pIRQ;
    TIrqEntry pEntry;
    TArgument data;

    /*
     * �Ӷ�����������IRQ��������̻߳����´����IRQ�ص�����
     * ���IRQ�������Ϊ����IRQ�ػ��̹߳���
     */
    while(eTrue)
    {
        OsCpuEnterCritical(&imask);
        if (IrqReqList.Handle == (TLinkNode*)0)
        {
            OsThreadSuspendSelf();
            OsCpuLeaveCritical(imask);
        }
        else
        {
            pIRQ   = (TIrq*)(IrqReqList.Handle->Owner);
            pEntry = pIRQ->Entry;
            data   = pIRQ->Argument;
            OsObjListRemoveNode(pIRQ->LinkNode.Handle, &(pIRQ->LinkNode));
            memset(pIRQ, 0U, sizeof(TIrq));
            OsCpuLeaveCritical(imask);

            pEntry(data);
        }
    }
}


#endif


/*************************************************************************************************
 *  ���ܣ���ʱ��ģ���ʼ��                                                                       *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsIrqModuleInit(void)
{
    /* ����ں��Ƿ��ڳ�ʼ״̬ */
    if(OsKernelVariable.State != OsOriginState)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* ��ʼ����ص��ں˱��� */
    OsKernelVariable.IrqMapTable = IrqMapTable;
    OsKernelVariable.IrqVectorTable = IrqVectorTable;

    memset(IrqMapTable, 0U, sizeof(IrqMapTable));
    memset(IrqVectorTable, 0U, sizeof(IrqVectorTable));

#if (TCLC_IRQ_DAEMON_ENABLE)
    memset(&IrqReqList, 0U, sizeof(TIrqList));

    /* ��ʼ���ں��жϷ����߳� */
    OsThreadCreate(&IrqDaemonThread,
                   "kernel irq daemon",
                   OsThreadSuspended,
                   OS_THREAD_PROP_PRIORITY_FIXED | \
                   OS_THREAD_PROP_CLEAN_STACK | \
                   OS_THREAD_PROP_KERNEL_DAEMON,
                   IRQ_DAEMON_ACAPI,
                   IrqDaemonEntry,
                   (TArgument)0,
                   (void*)IrqDaemonStack,
                   (TBase32)TCLC_IRQ_DAEMON_STACK_BYTES,
                   (TPriority)TCLC_IRQ_DAEMON_PRIORITY,
                   (TTimeTick)TCLC_IRQ_DAEMON_SLICE);

    /* ��ʼ����ص��ں˱��� */
    OsKernelVariable.IrqDaemon = &IrqDaemonThread;
#endif
}

#undef IRQ_VECTOR_PROP_NONE
#undef IRQ_VECTOR_PROP_READY
#undef IRQ_VECTOR_PROP_LOCKED
#endif

