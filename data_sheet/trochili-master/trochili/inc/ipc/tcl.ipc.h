/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_IPC_H
#define _TCLC_IPC_H

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"

#if (TCLC_IPC_ENABLE)

/* IPC�߳��������нṹ���� */
struct IpcBlockedQueueDef
{
    TProperty*  Property;                             /* �߳�������������                       */
    TLinkNode*  PrimaryHandle;                        /* �����л����̷ֶ߳���                   */
    TLinkNode*  AuxiliaryHandle;                      /* �����и����̷ֶ߳���                   */
};
typedef struct IpcBlockedQueueDef TIpcQueue;


/* IPC����������ں˴���ʹ�� */
#define OS_IPC_ERR_NONE           (TError)(0x0)       /* �����ɹ�                               */
#define OS_IPC_ERR_FAULT          (TError)(0x1<<0)    /* ����/�����÷�����                      */
#define OS_IPC_ERR_UNREADY        (TError)(0x1<<1)    /* IPC����û�б���ʼ��                    */
#define OS_IPC_ERR_NORMAL         (TError)(0x1<<2)    /* �ź���:�ź�������ֵ�����������
                                                       * �������Ϣ����:״̬���ܱ��������
                                                       * �¼����:�����͵��¼��Ѿ����� or
                                                       * �����¼�ʱ�ڴ����¼����ܱ�����
                                                       * �������� �������ѱ������߳�ռ�� or
                                                       * �����������ڵ�ǰ�߳�
                                                       */
#define OS_IPC_ERR_TIMEO          (TError)(0x1<<3)    /* ������ʱ�޵����̱߳�����               */
#define OS_IPC_ERR_DELETE         (TError)(0x1<<4)    /* IPC�������٣��̱߳�����              */
#define OS_IPC_ERR_RESET          (TError)(0x1<<5)    /* IPC�������ã��̱߳�����              */
#define OS_IPC_ERR_FLUSH          (TError)(0x1<<6)    /* IPC���������ϵ��̱߳�����ֹ            */
#define OS_IPC_ERR_ABORT          (TError)(0x1<<7)    /* IPC���������ϵ��̱߳�����ֹ            */
#define OS_IPC_ERR_ACAPI          (TError)(0x1<<9)    /* �̲߳�����������ʽ����IPC����          */

/* IPC�������ԣ��ں˴���ʹ�� */
#define OS_IPC_PROP_DEFAULT       (TProperty)(0x0)
#define OS_IPC_PROP_READY         (TProperty)(0x1<<0) /* IPC�����Ѿ�����ʼ��                    */
#define OS_IPC_PROP_PREEMP_AUXIQ  (TProperty)(0x1<<1) /* �����߳��������в������ȼ����ȷ���     */
#define OS_IPC_PROP_PREEMP_PRIMIQ (TProperty)(0x1<<2) /* �����߳��������в������ȼ����ȷ���     */
#define OS_IPC_PROP_AUXIQ_AVAIL   (TProperty)(0x1<<8) /* �����߳�������������ڱ��������߳�     */
#define OS_IPC_PROP_PRIMQ_AVAIL   (TProperty)(0x1<<9) /* �����߳�������������ڱ��������߳�     */

#define OS_RESET_SEMAPHORE_PROP   (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP_PRIMIQ)
#define OS_RESET_MUTEX_PROP       (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP_PRIMIQ)
#define OS_RESET_MBOX_PROP        (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP_PRIMIQ |\
                                  OS_IPC_PROP_PREEMP_AUXIQ)
#define OS_RESET_MQUE_PROP        (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP_PRIMIQ |\
                                  OS_IPC_PROP_PREEMP_AUXIQ)
#define OS_RESET_FLAG_PROP        (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP_PRIMIQ)

#define OS_USER_SEMAPHORE_PROP    (OS_IPC_PROP_PREEMP_PRIMIQ)
#define OS_USER_MUTEX_PROP        (OS_IPC_PROP_PREEMP_PRIMIQ)
#define OS_USER_MBOX_PROP         (OS_IPC_PROP_PREEMP_PRIMIQ | OS_IPC_PROP_PREEMP_AUXIQ)
#define OS_USER_MQUE_PROP         (OS_IPC_PROP_PREEMP_PRIMIQ | OS_IPC_PROP_PREEMP_AUXIQ)
#define OS_USER_FLAG_PROP         (OS_IPC_PROP_PREEMP_PRIMIQ)

/* �߳�IPCѡ��ں˴���ʹ�� */
#define OS_IPC_OPT_DEFAULT        (TOption)(0x0)
#define OS_IPC_OPT_WAIT           (TOption)(0x1<<1)   /* ���÷�ʽ�ȴ�IPC                        */
#define OS_IPC_OPT_TIMEO          (TOption)(0x1<<2)   /* ʱ�޷�ʽ�ȴ����                       */
#define OS_IPC_OPT_UARGENT        (TOption)(0x1<<3)   /* ��Ϣ���С��ʼ�ʹ��                     */
#define OS_IPC_OPT_AND            (TOption)(0x1<<4)   /* ����¼���ǲ�����AND����              */
#define OS_IPC_OPT_OR             (TOption)(0x1<<5)   /* ����¼���ǲ�����OR����               */
#define OS_IPC_OPT_CONSUME        (TOption)(0x1<<6)   /* �¼����ʹ��                           */

#define OS_IPC_OPT_SEMAPHORE      (TOption)(0x1<<16)  /* ����߳��������ź������߳�����������   */
#define OS_IPC_OPT_MUTEX          (TOption)(0x1<<17)  /* ����߳������ڻ��������߳�����������   */
#define OS_IPC_OPT_MAILBOX        (TOption)(0x1<<18)  /* ����߳�������������߳�����������     */
#define OS_IPC_OPT_MSGQUEUE       (TOption)(0x1<<19)  /* ����߳���������Ϣ���е��߳����������� */
#define OS_IPC_OPT_FLAGS          (TOption)(0x1<<20)  /* ����߳��������¼���ǵ��߳����������� */

#define OS_IPC_OPT_READ_DATA      (TOption)(0x1<<24)  /* �����ʼ�������Ϣ                       */
#define OS_IPC_OPT_WRITE_DATA     (TOption)(0x1<<25)  /* �����ʼ�������Ϣ                       */

#define OS_USER_SEMAPHORE_OPTION  (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO)
#define OS_USER_MUTEX_OPTION      (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO)
#define OS_USER_MBOX_OPTION       (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO | OS_IPC_OPT_UARGENT)
#define OS_USER_MSGQ_OPTION       (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO | OS_IPC_OPT_UARGENT)
#define OS_USER_FLAG_OPTION       (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO | OS_IPC_OPT_AND |\
                                   OS_IPC_OPT_OR   | OS_IPC_OPT_CONSUME)

#define OS_ISR_SEMAPHORE_OPTION   (TOption)0
#define OS_ISR_MBOX_OPTION        (OS_IPC_OPT_UARGENT)
#define OS_ISR_MSGQ_OPTION        (OS_IPC_OPT_UARGENT)

/* NOTE: not compliant MISRA2004 18.4: Unions shall not be used. */
union IpcDataDef
{
    TBase32 Value;                                    /* ���汻�������ݱ����ĵ�ֵַַ           */
    void*   Addr1;                                    /* ָ���¼���ǵ�һ��ָ��                 */
    void**  Addr2;                                    /* ָ����Ϣ�����ʼ��Ķ���ָ��             */
};
typedef union IpcDataDef TIpcData;

/* �߳����ڼ�¼IPC�������ϸ��Ϣ�ļ�¼�ṹ */
struct IpcContextDef
{
    void*        Object;                              /* ָ��IPC�����ַ��ָ��                  */
    TIpcQueue*   Queue;                               /* �߳�����IPC�̶߳���ָ��                */
    TIpcData     Data;                                /* ��IPC���������ص�����ָ��            */
    TBase32      Length;                              /* ��IPC���������ص����ݳ���            */
    TOption      Option;                              /* ����IPC����Ĳ�������                  */
    TTimeTick    Ticks;	                              /* ����IPC�����ʱ��                      */
    TState*      State;                               /* IPC��������ķ���ֵ                    */
    TError*      Error;                               /* IPC��������Ĵ������                  */
    void*        Owner;                               /* IPC���������߳�                        */
    TLinkNode    LinkNode;                            /* �߳�����IPC���е�����ڵ�              */
};
typedef struct IpcContextDef TIpcContext;


extern void OsIpcInitContext(TIpcContext* pContext, void* pIpc, TBase32 data, TBase32 len,
                             TOption option, TTimeTick ticks, TState* pState, TError* pError);
extern void OsIpcCleanContext(TIpcContext* pContext);
extern void OsIpcBlockThread(TIpcContext* pContext, TIpcQueue* pQueue);
extern void OsIpcUnblockThread(TIpcContext* pContext,  TState state, TError error, TBool* pHiRP);
extern void OsIpcUnblockAll(TIpcQueue* pQueue, TState state, TError error,
                            void** pData2, TBool* pHiRP);
extern void OsIpcSetPriority(TIpcContext* pContext, TPriority priority);

#endif

#endif /* _TCLC_IPC_H */

