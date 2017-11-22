/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "string.h"
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.timer.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"

/* �ں˹ؼ��������� */
TKernelVariable OsKernelVariable;


/*************************************************************************************************
 *  ���ܣ����ں˶������ϵͳ��                                                                   *
 *  ������(1) pObject �ں˶����ַ                                                               *
 *        (2) pName   �ں˶�������                                                               *
 *        (3) type    �ں˶�������                                                               *
 *        (4) pOwner  �ں˶���������ַ                                                           *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsKernelAddObject(TObject* pObject, TChar* pName, TObjectType type, void* pOwner)
{
    TBase32 len;

    len = strlen(pName);
    len = (len > TCL_OBJ_NAME_LEN)?TCL_OBJ_NAME_LEN:len;
    strncpy(pObject->Name, pName, len);
    pObject->Type  = type;
    pObject->Owner = pOwner;
    pObject->ID = OsKernelVariable.ObjectID;
    OsKernelVariable.ObjectID++;
    OsObjListAddNode(&(OsKernelVariable.ObjectList), &(pObject->LinkNode), OsLinkTail);
}


/*************************************************************************************************
 *  ���ܣ����ں˶����ϵͳ���Ƴ�                                                                 *
 *  ������(1) pObject �ں˶����ַ                                                               *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsKernelRemoveObject(TObject* pObject)
{
    OsObjListRemoveNode(&(OsKernelVariable.ObjectList), &(pObject->LinkNode));
    memset(pObject, 0U, sizeof(TObject));
}


/*************************************************************************************************
 *  ���ܣ��ں˽����жϴ������                                                                   *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsKernelEnterIntrState(void)
{
    TReg32 imask;
    OsCpuEnterCritical(&imask);

    OsKernelVariable.IntrNestTimes++;
    OsKernelVariable.State = OsExtremeState;

    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ��ں��˳��жϴ������                                                                   *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsKernelLeaveIntrState(void)
{
    TReg32 imask;

    OsCpuEnterCritical(&imask);

    OS_ASSERT((OsKernelVariable.IntrNestTimes > 0U), "");
    OsKernelVariable.IntrNestTimes--;
    if (OsKernelVariable.IntrNestTimes == 0U)
    {
        /*
         * ������������жϱ���ڹ���򼤻˵����ǰ�ж���������ȼ��жϣ���Ȼû�з���Ƕ�ף�
         * ���Ƿ��غ󽫽���ͼ�����жϣ����������������Ҫ���������л�����Ӧ�������һ��������
         * ��ͼ�����Ǹ��ж����˳��ж�ʱ����ɡ�
         * �˴����̵߳������ֵ���"��ռ"
         */
        if (OsKernelVariable.SchedLockTimes == 0U)
        {
            OsThreadSchedule();
        }
        OsKernelVariable.State = OsThreadState;
    }

    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ�ʱ�ӽ����жϴ�����                                                                   *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵������ʱ���������������У��������̵߳��ȴ���                                           *
 *************************************************************************************************/
void OsKernelTickISR(void)
{
    TReg32 imask;

    OsCpuEnterCritical(&imask);

    /* �ں�������ʱ�����������1�� */
    OsKernelVariable.Jiffies++;
    if (OsKernelVariable.Jiffies == 0U)
    {
        OsKernelVariable.JiffyCycles++;
    }

    /* �����߳�ʱ�ӽ��� */
    OsThreadTickUpdate();

    /* �����̶߳�ʱ��ʱ�ӽ��� */
    OsThreadTimerUpdate();

    /* �����û���ʱ��ʱ�ӽ��� */
#if (TCLC_TIMER_ENABLE)
    OsTimerTickUpdate();
#endif

    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ��弶�ַ�����ӡ����                                                                     *
 *  ������(1) pStr ����ӡ���ַ���                                                                *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclTrace(const char* pNote)
{
    TReg32 imask;
    OS_ASSERT((pNote != (char*)0), "");
    OsCpuEnterCritical(&imask);
    if (OsKernelVariable.TraceEntry != (TTraceEntry)0)
    {
        OsKernelVariable.TraceEntry(pNote);
    }
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ��ر�������ȹ���                                                                       *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����������ֻ�ܱ��̵߳���                                                                   *
 *************************************************************************************************/
TState TclLockScheduler(void)
{
    TState state = eFailure;
    TReg32 imask;

    OsCpuEnterCritical(&imask);
    if (OsKernelVariable.State == OsThreadState)
    {
        OsKernelVariable.SchedLockTimes++;
        state = eSuccess;
    }
    OsCpuLeaveCritical(imask);
    return state;
}


/*************************************************************************************************
 *  ���ܣ�����������ȹ���                                                                       *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����������ֻ�ܱ��̵߳���                                                                   *
 *************************************************************************************************/
TState TclUnlockScheduler(void)
{
    TState state = eFailure;
    TReg32 imask;

    OsCpuEnterCritical(&imask);
    if (OsKernelVariable.State == OsThreadState)
    {
        if (OsKernelVariable.SchedLockTimes > 0U)
        {
            OsKernelVariable.SchedLockTimes--;
            /*
             * �ڹرյ������Ľ׶Σ���ǰ�߳��п���ʹ�������������ȼ����߳̾�����ISRҲ���ܽ�
             * һЩ�����ȼ����߳̽�������������ڴ򿪵�������ʱ����Ҫ��һ���̵߳��ȼ�飬
             * ��ϵͳ���жϷ���ʱ����
             */
            if (OsKernelVariable.SchedLockTimes == 0U)
            {
                OsThreadSchedule();
            }
            state = eSuccess;
        }
    }
    OsCpuLeaveCritical(imask);
    return state;
}


/*************************************************************************************************
 *  ���ܣ�����ϵͳIdle������IDLE�̵߳���                                                         *
 *  ������(1) pEntry ϵͳIdle����                                                                *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclSetSysIdleEntry(TSysIdleEntry pEntry)
{
    TReg32 imask;
    OS_ASSERT((pEntry != (TSysIdleEntry)0), "");

    OsCpuEnterCritical(&imask);
    OsKernelVariable.SysIdleEntry = pEntry;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ�����ϵͳFault����                                                                      *
 *  ������(1) pEntry ϵͳFault����                                                               *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclSetSysFaultEntry(TSysFaultEntry pEntry)
{
    TReg32 imask;
    OS_ASSERT((pEntry != (TSysFaultEntry)0), "");

    OsCpuEnterCritical(&imask);
    OsKernelVariable.SysFaultEntry = pEntry;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ�����ϵͳWarning����                                                                    *
 *  ������(1) pEntry ϵͳWarning����                                                             *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclSetSysWarningEntry(TSysWarningEntry pEntry)
{
    TReg32 imask;
    OS_ASSERT((pEntry != (TSysWarningEntry)0), "");

    OsCpuEnterCritical(&imask);
    OsKernelVariable.SysWarningEntry = pEntry;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ����ϵͳ��ǰ�߳�ָ��                                                                   *
 *  ������(1) pThread2 ���ص�ǰ�߳�ָ��                                                          *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclGetCurrentThread(TThread** pThread2)
{
    TReg32 imask;
    OS_ASSERT((pThread2 != (TThread**)0), "");

    OsCpuEnterCritical(&imask);
    *pThread2 = OsKernelVariable.CurrentThread;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ����ϵͳ������ʱ�ӽ�����                                                               *
 *  ������(1) pJiffies ����ϵͳ������ʱ�ӽ�����                                                  *
 *        (2) pCycles  ����ϵͳ������ʱ���ֻ���                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclGetTimeStamp(TBase32* pCycles, TTimeTick* pJiffies)
{
    TReg32 imask;
    OS_ASSERT((pJiffies != (TTimeTick*)0), "");

    OsCpuEnterCritical(&imask);
    *pJiffies = OsKernelVariable.Jiffies;
    *pCycles  = OsKernelVariable.JiffyCycles;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  ���ܣ��ں���������                                                                           *
 *  ������(1) pUserEntry  Ӧ�ó�ʼ������                                                         *
 *        (2) pCpuEntry   ��������ʼ������                                                       *
 *        (3) pBoardEntry �弶�豸��ʼ������                                                     *
 *        (4) pTraceEntry �����������                                                           *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void TclStartKernel(TUserEntry pUserEntry,
                    TCpuSetupEntry pCpuEntry,
                    TBoardSetupEntry pBoardEntry,
                    TTraceEntry pTraceEntry)
{
    OS_ASSERT((pUserEntry  != (TUserEntry)0), "");
    OS_ASSERT((pCpuEntry   != (TCpuSetupEntry)0), "");
    OS_ASSERT((pBoardEntry != (TBoardSetupEntry)0), "");
    OS_ASSERT((pTraceEntry != (TTraceEntry)0), "");


    /* �رմ������ж� */
    OsCpuDisableInt();

    /* ��ʼ�������ں˲��� */
    memset(&OsKernelVariable, 0U, sizeof(TKernelVariable));
    OsKernelVariable.UserEntry       = pUserEntry;
    OsKernelVariable.CpuSetupEntry   = pCpuEntry;
    OsKernelVariable.BoardSetupEntry = pBoardEntry;
    OsKernelVariable.TraceEntry      = pTraceEntry;
    OsKernelVariable.SchedLockTimes  = 0U;
    OsKernelVariable.State           = OsOriginState;

    /* ��ʼ���̹߳���ģ����ں�ROOT�߳� */
    OsThreadModuleInit();

    /* ��ʼ���û���ʱ��ģ��Ͷ�ʱ���ػ��߳� */
#if (TCLC_TIMER_ENABLE)
    OsTimerModuleInit();
#endif

    /* ��ʼ���жϹ���ģ����ж��ػ��߳� */
#if (TCLC_IRQ_ENABLE)
    OsIrqModuleInit();
#endif

    /* ���ô������Ͱ弶��ʼ������ */
    OsKernelVariable.CpuSetupEntry();
    OsKernelVariable.BoardSetupEntry();

    /* �����ں�ROOT�߳� */
    OsCpuLoadRootThread();

    /* �򿪴������ж� */
    OsCpuEnableInt();

    /*
     * ���δ���Ӧ����Զ���ᱻִ�У������е��ˣ�˵��RTOS��ֲʱ�������⡣
     * ����Ĵ����𵽶������ã����⴦�������������״̬
     */
    OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
}


