#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH5_MUTEX_EXAMPLE4)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (8)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* �û��̶߳��� */
static TThread ThreadLed;
static TThread ThreadCTRL;

/* �û��߳�ջ���� */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* �û����������� */
static TMutex LedMutex;

/* �û����������ȼ��컨�� */
#define LED_MUTEX_PRIORITY   (4)

/* Led�̵߳������� */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�������������ֻ��������ú����Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, (TTimeTick)0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED1, LED_ON);
        }

        /* Led�߳���������ʽ��ȡ�������������ֻ��������ú�Ϩ��Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, (TTimeTick)0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED1, LED_OFF);
        }
    }
}


/* CTRL�̵߳������� */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* CTRL�̳߳�ʼ����������Ȼ����ǰ��û����� */
        state = TclCreateMutex(&LedMutex, "mutex", TCLP_IPC_DEFAULT, LED_MUTEX_PRIORITY, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL�߳���ʱ1�룬�˺�Led�̲߳ŵõ����л��� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL�߳����û����� */
        state = TclDeleteMutex(&LedMutex, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLed, "thread led",
                          &ThreadLedEntry, (TArgument)0,
                          ThreadLedStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��CTRL�߳� */
    state = TclCreateThread(&ThreadCTRL, "thread ctrl",
                          &ThreadCtrlEntry, (TArgument)0,
                          ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳� */
    state = TclActivateThread(&ThreadLed, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����CTRL�߳� */
    state = TclActivateThread(&ThreadCTRL, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
}


/* ������BOOT֮������main�����������ṩ */
int main(void)
{
    /* ע������ں˺���,�����ں� */
    TclStartKernel(&AppSetupEntry,
                   &OsCpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);
    return 1;
}


#endif

