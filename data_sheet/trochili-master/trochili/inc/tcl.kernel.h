/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_KERNEL_H
#define _TCL_KERNEL_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.irq.h"
#include "tcl.timer.h"
#include "tcl.debug.h"

/* �û���ں������Ͷ���                         */
typedef void (*TUserEntry)(void);

/* �弶��ʼ���������Ͷ���                       */
typedef void (*TBoardSetupEntry)(void);

/* ��������ʼ���������Ͷ���                     */
typedef void (*TCpuSetupEntry)(void);

/* �弶�ַ�����ӡ�������Ͷ���                   */
typedef void (*TTraceEntry)(const char* pStr);

/* ϵͳIDLE�������Ͷ���                         */
typedef void (*TSysIdleEntry)(void);

/* ϵͳFault���������Ͷ���                    */
typedef void (*TSysFaultEntry)(void* pKernelVariable);

/* ϵͳWarning���������Ͷ���                  */
typedef void (*TSysWarningEntry)(void* pKernelVariable);

/* �������л������Ͷ��壬ϵͳ�������������л��� */
typedef enum
{
    OsOriginState  = 0,                               /* �����������ں˳�̬                    */
    OsExtremeState = 1,                               /* �����������ж�̬                      */
    OsThreadState  = 2,                               /* �����������߳�̬                      */
} TKernelState;

#define OS_KERNEL_DIAG_ERROR_NONE      (0U)           /* �߳�ջ���                            */
#define OS_KERNEL_DIAG_THREAD_ERROR    (0x1<<0U)      /* �̴߳���                              */
#define OS_KERNEL_DIAG_SCHED_ERROR     (0x1<<1U)      /* �ں˽�ֹ�̵߳���                      */
#define OS_KERNEL_DIAG_TIMER_ERROR     (0x1<<2U)      /* ��ʱ������                            */
#define OS_KERNEL_DIAG_IPC_ERROR       (0x1<<3U)      /* ���ж�������˻�����                  */

/* �ں˱����ṹ���壬��¼���ں�����ʱ�ĸ������� */
struct KernelVariableDef
{
    TBase32          SchedLockTimes;                  /* �����Ƿ������̵߳���                  */
    TThread*         NomineeThread;                   /* �ں˺�ѡ�߳�ָ��                      */
    TThread*         CurrentThread;                   /* �ں˵�ǰ�߳�ָ��                      */
    TKernelState     State;                           /* ��¼����ִ��ʱ����������״̬          */
    TBase32          IntrNestTimes;                   /* ��¼�ں˱��жϵ�Ƕ�״���              */
    TTimeTick        Jiffies;                         /* ϵͳ�����ܵĽ�����                    */
	TBase32          JiffyCycles;                     /* ϵͳjiffy�ֻش���                     */
    TBitMask         Diagnosis;                       /* �ں�����״����¼                      */
    TDBGLog          DBGLog;                          /* �ں�����״����¼                      */

    TBoardSetupEntry BoardSetupEntry;                 /* �弶��ʼ���������                    */
    TCpuSetupEntry   CpuSetupEntry;                   /* ��������ʼ���������                  */
    TUserEntry       UserEntry;                       /* �û��������                          */
    TTraceEntry      TraceEntry;                      /* �ں˴�ӡ����                          */
    TSysIdleEntry    SysIdleEntry;                    /* �ں�IDLE����                          */
    TSysFaultEntry   SysFaultEntry;                   /* �ں�FAULT����                         */
    TSysWarningEntry SysWarningEntry;                 /* �ں�WARNING����                       */
    TThread*         RootThread;                      /* �ں�ROOT�߳�ָ��                      */

#if (TCLC_TIMER_ENABLE)
    TThread*         TimerDaemon;                     /* �û���ʱ���߳�ָ��                    */
    TTimerList*      TimerList;                       /* �û���ʱ������ָ��                    */
#endif

#if (TCLC_IRQ_ENABLE)
    TAddr32*         IrqMapTable;                     /* �ں��ж�ӳ���                        */
    TIrqVector*      IrqVectorTable;                  /* �ں��ж�������                        */
#endif

#if ((TCLC_IRQ_ENABLE)&&(TCLC_IRQ_DAEMON_ENABLE))
    TThread*         IrqDaemon;                       /* IRQ�߳�ָ��                           */
#endif

    TThreadQueue*    ThreadAuxiliaryQueue;            /* �ں��̸߳�������ָ��                  */
    TThreadQueue*    ThreadReadyQueue;                /* �ں˽��������н�ָ��                  */
    TLinkNode*       ThreadTimerList;                 /* �̶߳�ʱ������ָ��                    */

    TLinkNode*       ObjectList;                      /* �ں˶���Ķ��нڵ�                    */
    TBase32          ObjectID;                        /* �ں˶��������ɼ���                  */
};
typedef struct KernelVariableDef TKernelVariable;


extern TKernelVariable OsKernelVariable;


extern void OsKernelAddObject(TObject* pObject, TChar* pName, TObjectType type, void* pOwner);
extern void OsKernelRemoveObject(TObject* pObject);
extern void OsKernelEnterIntrState(void);
extern void OsKernelLeaveIntrState(void);
extern void OsKernelTickISR(void);


extern void TclStartKernel(TUserEntry pUserEntry,
                           TCpuSetupEntry   pCpuEntry,
                           TBoardSetupEntry pBoardEntry,
                           TTraceEntry      pTraceEntry);
extern void TclSetSysIdleEntry(TSysIdleEntry pEntry);
extern void TclSetSysFaultEntry(TSysFaultEntry pEntry);
extern void TclSetSysWarningEntry(TSysWarningEntry pEntry);
extern void TclGetCurrentThread(TThread** pThread2);
extern void TclGetTimeStamp(TBase32* pCycles, TTimeTick* pJiffies);
extern TState TclUnlockScheduler(void);
extern TState TclLockScheduler(void);
extern void TclTrace(const char* pNote);


#endif /* _TCL_KERNEL_H */

