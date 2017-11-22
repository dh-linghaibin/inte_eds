/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_THREAD_H
#define _TCL_THREAD_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.ipc.h"
#include "tcl.timer.h"

/* �߳����д����붨��                 */
#define OS_THREAD_DIAG_NORMAL            (TBitMask)(0x0)     /* �߳�����                                */
#define OS_THREAD_DIAG_STACK_OVERFLOW    (TBitMask)(0x1<<0)  /* �߳�ջ���                              */
#define OS_THREAD_DIAG_STACK_ALARM       (TBitMask)(0x1<<1)  /* �߳�ջ�澯                              */
#define OS_THREAD_DIAG_INVALID_EXIT      (TBitMask)(0x1<<2)  /* �̷߳Ƿ��˳�                            */
#define OS_THREAD_DIAG_INVALID_STATE     (TBitMask)(0x1<<3)  /* �̲߳���ʧ��                            */
#define OS_THREAD_DIAG_INVALID_TIMEO     (TBitMask)(0x1<<4)  /* �߳�ʱ��������ֹ                        */

/* �̵߳��ô����붨��                 */
#define OS_THREAD_ERR_NONE               (TError)(0x0)
#define OS_THREAD_ERR_UNREADY            (TError)(0x1<<0)    /* �߳̽ṹδ��ʼ��                        */
#define OS_THREAD_ERR_ACAPI              (TError)(0x1<<1)    /* �̲߳����ܲ���                          */
#define OS_THREAD_ERR_FAULT              (TError)(0x1<<2)    /* һ���Դ��󣬲�������������              */
#define OS_THREAD_ERR_STATUS             (TError)(0x1<<3)    /* �߳�״̬����                            */
#define OS_THREAD_ERR_PRIORITY           (TError)(0x1<<4)    /* �߳����ȼ�����                          */

/* �߳����Զ���                       */
#define OS_THREAD_PROP_NONE              (TProperty)(0x0)
#define OS_THREAD_PROP_READY             (TProperty)(0x1<<0) /* �̳߳�ʼ����ϱ��λ,
                                                           * ����Ա�ڽṹ���е�λ�ø����������    */
#define OS_THREAD_PROP_PRIORITY_FIXED    (TProperty)(0x1<<1) /* �߳����ȼ��������                      */
#define OS_THREAD_PROP_PRIORITY_SAFE     (TProperty)(0x1<<2) /* �߳����ȼ���ȫ���                      */
#define OS_THREAD_PROP_CLEAN_STACK       (TProperty)(0x1<<3) /* ��������߳�ջ�ռ�                      */
#define OS_THREAD_PROP_KERNEL_ROOT       (TProperty)(0x1<<6) /* ROOT�̱߳��λ                          */
#define OS_THREAD_PROP_KERNEL_DAEMON     (TProperty)(0x1<<4) /* �ں��ػ��̱߳��λ                      */

/* �߳�Ȩ�޿��ƣ������߳�API����ʱ�����λ */
#define OS_THREAD_ACAPI_NONE             (TBitMask)(0x0)
#define OS_THREAD_ACAPI_DELETE           (TBitMask)(0x1<<0)
#define OS_THREAD_ACAPI_ACTIVATE         (TBitMask)(0x1<<1)
#define OS_THREAD_ACAPI_DEACTIVATE       (TBitMask)(0x1<<2)
#define OS_THREAD_ACAPI_SUSPEND          (TBitMask)(0x1<<3)
#define OS_THREAD_ACAPI_RESUME           (TBitMask)(0x1<<4)
#define OS_THREAD_ACAPI_DELAY            (TBitMask)(0x1<<5)
#define OS_THREAD_ACAPI_UNDELAY          (TBitMask)(0x1<<6)
#define OS_THREAD_ACAPI_YIELD            (TBitMask)(0x1<<7)
#define OS_THREAD_ACAPI_PRIORITY         (TBitMask)(0x1<<8)
#define OS_THREAD_ACAPI_SLICE            (TBitMask)(0x1<<9)
#define OS_THREAD_ACAPI_UNBLOCK          (TBitMask)(0x1<<10)
#define OS_THREAD_ACAPI_BLOCK            (TBitMask)(0x1<<11) /* ��IPC�����й�,�����ں��̲߳�����������ʽ����IPC���� */

#define OS_THREAD_ACAPI_ALL \
    (OS_THREAD_ACAPI_DELETE|\
    OS_THREAD_ACAPI_ACTIVATE|\
    OS_THREAD_ACAPI_DEACTIVATE|\
    OS_THREAD_ACAPI_SUSPEND|\
    OS_THREAD_ACAPI_RESUME|\
    OS_THREAD_ACAPI_DELAY|\
    OS_THREAD_ACAPI_UNDELAY|\
    OS_THREAD_ACAPI_PRIORITY|\
    OS_THREAD_ACAPI_SLICE|\
    OS_THREAD_ACAPI_UNBLOCK|\
    OS_THREAD_ACAPI_BLOCK|\
    OS_THREAD_ACAPI_YIELD)

/* �߳�״̬����  */
enum ThreadStausdef
{
    OsThreadRunning   = (TBitMask)(0x1<<0),     /* ����                                           */
    OsThreadReady     = (TBitMask)(0x1<<1),     /* ����                                           */
    OsThreadDormant   = (TBitMask)(0x1<<2),     /* ����                                           */
    OsThreadBlocked   = (TBitMask)(0x1<<3),     /* ����                                           */
    OsThreadDelayed   = (TBitMask)(0x1<<4),     /* ��ʱ����                                       */
    OsThreadSuspended = (TBitMask)(0x1<<5),     /* ��������                                       */
};
typedef enum ThreadStausdef TThreadStatus;

/*
 * �̶߳��нṹ���壬�ýṹ��С���ں�֧�ֵ����ȼ���Χ���仯��
 * ����ʵ�̶ֹ�ʱ����߳����ȼ������㷨
 */
struct ThreadQueueDef
{
    TBitMask   PriorityMask;                 /* �����о������ȼ�����                             */
    TLinkNode* Handle[TCLC_PRIORITY_NUM];    /* �������̷ֶ߳���                                 */
};
typedef struct ThreadQueueDef TThreadQueue;


/* �߳���ʱ��ʱ���ṹ���� */
struct TickTimerDef
{
    TTimeTick     RemainTicks;               /* �̶߳�ʱ����ʱ��                                 */
    void*         Owner;                     /* �̶߳�ʱ�������߳�                               */
    TLinkNode     LinkNode;                  /* �̶߳�ʱ�����е�����ڵ�                         */
};
typedef struct TickTimerDef TTickTimer;

/* �߳����������Ͷ���                                                                            */
typedef void (*TThreadEntry)(TArgument data);

/* �ں��߳̽ṹ���壬���ڱ����̵߳Ļ�����Ϣ                                                      */
struct ThreadDef
{
    TProperty     Property;                  /* �̵߳�����,����Ա�ڽṹ���е�λ�ø����������  */
    TThreadStatus Status;                    /* �߳�״̬,����Ա�ڽṹ���е�λ�ø����������    */
    TAddr32       StackTop;                  /* �߳�ջ��ָ��,����Ա�ڽṹ���е�λ�ø����������*/
    TAddr32       StackBase;                 /* �߳�ջ��ָ��                                     */
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
    TBase32       StackAlarm;                /* �߳�ջ��������                                   */
    TBase32       StackBarrier;              /* �߳�ջ��Χ��                                     */
#endif
    TBitMask      ACAPI;                     /* �߳̿ɽ��ܵ�API                                  */
    TPriority     Priority;                  /* �̵߳�ǰ���ȼ�                                   */
    TPriority     BasePriority;              /* �̻߳������ȼ�                                   */
    TTimeTick     Ticks;                     /* ʱ��Ƭ�л�ʣ�µ�ticks��Ŀ                        */
    TTimeTick     BaseTicks;                 /* ʱ��Ƭ���ȣ�ticks��Ŀ��                          */
    TTimeTick     Jiffies;                   /* �߳��ܵ�����ʱ�ӽ�����                           */
    TThreadEntry  Entry;                     /* �̵߳�������                                     */
    TArgument     Argument;                  /* �߳����������û�����,�û�����ֵ                  */
    TBitMask      Diagnosis;                 /* �߳����д�����                                   */
    TTickTimer    Timer;                     /* �����߳���ʱ�����߳�ʱ��������ʱ�����ṹ       */
#if (TCLC_IPC_ENABLE)
    TIpcContext*  IpcContext;                /* �̻߳��⡢ͬ������ͨ�ŵ�������                   */
#endif
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
    TLinkNode*    LockList;                  /* �߳�ռ�е����Ķ���                               */
#endif
    TThreadQueue* Queue;                     /* ָ���߳������̶߳��е�ָ��                       */
    TLinkNode     LinkNode;                  /* �߳����ڶ��еĽڵ�                               */
    TObject       Object;                    /* �̵߳��ں˶���ڵ�                               */
};
typedef struct ThreadDef TThread;

#define TCLM_NODE2THREAD(NODE) ((TThread*)((TByte*)(NODE) - OFF_SET_OF(TThread, LinkNode)))

extern void OsThreadModuleInit(void);
extern void OsThreadEnterQueue(TThreadQueue* pQueue, TThread* pThread, TLinkPos pos);
extern void OsThreadLeaveQueue(TThreadQueue* pQueue, TThread* pThread);
extern void OsThreadTickUpdate(void);
extern void OsThreadTimerUpdate(void);
extern void OsThreadSchedule(void);
extern void OsThreadCreate(TThread* pThread, TChar* pName, TThreadStatus status, TProperty property,
                           TBitMask acapi, TThreadEntry pEntry, TArgument argument,
                           void* pStack, TBase32 bytes, TPriority priority, TTimeTick ticks);
extern TState OsThreadDelete(TThread* pThread, TError* pError);
extern TState OsThreadSetPriority(TThread* pThread, TPriority priority, TBool flag, TBool* pHiRP, TError* pError);
extern void OsThreadResumeFromISR(TThread* pThread);
extern void OsThreadSuspendSelf(void);

extern TState TclCreateThread(TThread* pThread,
                              TChar* pName,
                              TThreadEntry pEntry,
                              TBase32 argument,
                              void* pStack,
                              TBase32 bytes,
                              TPriority priority,
                              TTimeTick ticks,
                              TError* pError);
extern TState TclDeleteThread(TThread* pThread, TError* pError);
extern TState TclActivateThread(TThread* pThread, TError* pError);
extern TState TclDeactivateThread(TThread* pThread, TError* pError);
extern TState TclSuspendThread(TThread* pThread, TError* pError);
extern TState TclResumeThread(TThread* pThread, TError* pError);
extern TState TclSetThreadPriority(TThread* pThread, TPriority priority, TError* pError);
extern TState TclSetThreadSlice(TThread* pThread, TTimeTick ticks, TError* pError);
extern TState TclYieldThread(TError* pError);
extern TState TclDelayThread(TTimeTick ticks, TError* pError);
extern TState TclUndelayThread(TThread* pThread, TError* pError);
#if (TCLC_IPC_ENABLE)
extern TState TclUnblockThread(TThread* pThread, TError* pError);
#endif

#endif /*_TCL_THREAD_H */

