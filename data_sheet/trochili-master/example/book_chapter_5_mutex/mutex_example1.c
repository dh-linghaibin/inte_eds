#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH5_MUTEX_EXAMPLE1)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (8)
#define THREAD_LED_SLICE        (0xff)

/* �û��̶߳��� */
static TThread ThreadLedOn;
static TThread ThreadLedOff;

/* �û��߳�ջ���� */
static TBase32 ThreadLedOnStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLedOffStack[THREAD_LED_STACK_BYTES/4];

/* �û����������� */
static TMutex LedMutex;

/* �û����������ȼ��컨�� */
#define LED_MUTEX_PRIORITY   (4)

/* Led�����̵߳������� */
static void ThreadLedOnEntry(TArgument data)
{
    TError error;
    TState state;
    while (eTrue)
    {
        /* Led�����߳���������ʽ����������������ɹ������Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED1, LED_ON);

        /* Led�����߳���ʱ1�� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led�����߳��ͷŻ����� */
        state = TclFreeMutex(&LedMutex, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}

/* LedϨ���̵߳������� */
static void ThreadLedOffEntry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        /* LedϨ���߳���������ʽ����������������ɹ���Ϩ��Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        EvbLedControl(LED1, LED_OFF);

        /* LedϨ���߳���ʱ1�� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* LedϨ���߳��ͷŻ����� */
        state = TclFreeMutex(&LedMutex, &error);
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

    /* ��ʼ��Led�����߳� */
    state = TclCreateThread(&ThreadLedOn, "thread led on",
                          &ThreadLedOnEntry, (TArgument)0,
                          ThreadLedOnStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, 
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��LedϨ���߳� */
    state = TclCreateThread(&ThreadLedOff, "thread led off",
                          &ThreadLedOffEntry, (TArgument)0,
                          ThreadLedOffStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY + 1, THREAD_LED_SLICE, 
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�����߳� */
    state = TclActivateThread(&ThreadLedOn, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����LedϨ���߳� */
    state = TclActivateThread(&ThreadLedOff, &error);
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
