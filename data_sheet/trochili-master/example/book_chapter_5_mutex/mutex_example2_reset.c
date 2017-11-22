#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH5_MUTEX_EXAMPLE2)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (8)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* �û��̶߳��� */
static TThread ThreadLed1;
static TThread ThreadLed2;
static TThread ThreadLed3;
static TThread ThreadCTRL;

/* �û��߳�ջ���� */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed3Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* �û����������� */
static TMutex LedMutex;

/* �û����������ȼ��컨�� */
#define LED_MUTEX_PRIORITY   (4)

/* Led�߳�1�������� */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�������������ֻ�����reset�����Led1 */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED1, LED_ON);
        }

        /* Led�߳���������ʽ��ȡ�������������ֻ�����reset��Ϩ��Led1 */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED1, LED_OFF);
        }
    }
}


/* Led�߳�2�������� */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�������������ֻ�����reset�����Led2 */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED2, LED_ON);
        }

        /* Led�߳���������ʽ��ȡ�������������ֻ�����reset��Ϩ��Led2 */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED2, LED_OFF);
        }
    }
}


/* Led�߳�3�������� */
static void ThreadLed3Entry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�������������ֻ�����reset�����Led3 */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED3, LED_ON);
        }

        /* Led�߳���������ʽ��ȡ�������������ֻ�����reset��Ϩ��Led3 */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED3, LED_OFF);
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
        /* CTRL�߳����Ȼ�ȡ������ */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL�߳���ʱ1���RESET������ */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        state = TclResetMutex(&LedMutex, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ�������� */
    state = TclCreateMutex(&LedMutex, "mutex", TCLP_IPC_DEFAULT, LED_MUTEX_PRIORITY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led1�豸�����߳� */
    state = TclCreateThread(&ThreadLed1, "thread led1",
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led2�豸�����߳� */
    state = TclCreateThread(&ThreadLed2, "thread led2",
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led3�豸�����߳� */
    state = TclCreateThread(&ThreadLed3, "thread led3",
                          &ThreadLed3Entry, (TArgument)0,
                          ThreadLed3Stack, THREAD_LED_STACK_BYTES,
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

    /* ����Led�߳�1 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�2 */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�3 */
    state = TclActivateThread(&ThreadLed3, &error);
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

