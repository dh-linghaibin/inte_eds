#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE4)

/* �����߳�ʱ��Ҫ�Ĳ��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (65535)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (65535)

/* �̶߳��塢�߳�ջ���� */
static TThread ThreadLed1;
static TThread ThreadLed2;
static TThread ThreadCtrl;
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCtrlStack[THREAD_LED_STACK_BYTES/4];

/* Led�̵߳��߳������� */
static void ThreadLed1Entry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        state = TclDelayThread(TCLM_SEC2TICKS(1), &error);
        if(state == eSuccess)
        {
            EvbLedControl(LED1, LED_OFF);
        }

        state = TclDelayThread(TCLM_SEC2TICKS(1), &error);
        if(state == eSuccess)
        {
            EvbLedControl(LED1, LED_ON);
        }
    }
}


/* Led�̵߳��߳������� */
static void ThreadLed2Entry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        state = TclDelayThread(TCLM_SEC2TICKS(200), &error);
        if(state == eSuccess)
        {
            EvbLedControl(LED2, LED_OFF);
        }

        state = TclDelayThread(TCLM_SEC2TICKS(200), &error);
        if(state == eSuccess)
        {
            EvbLedControl(LED2, LED_ON);
        }
    }
}


/* Led�̵߳��߳������� */
static void ThreadCtrlEntry(TArgument data)
{
    TError error;
    TState state;

    static int i = 0;

    while (eTrue)
    {
        i = 0xfffff;
        while (i--);
        state = TclUndelayThread(&ThreadLed2, &error);
        state = state;
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    /* ��ʼ��Led1�����߳� */
    state = TclCreateThread(&ThreadLed1,
                            "thread led1",
                            &ThreadLed1Entry,
                            (TArgument)(&ThreadLed1),
                            ThreadLed1Stack,
                            THREAD_LED_STACK_BYTES,
                            THREAD_LED_PRIORITY,
                            THREAD_LED_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led2�����߳� */
    state = TclCreateThread(&ThreadLed2,
                            "thread led2",
                            &ThreadLed2Entry,
                            (TArgument)(&ThreadLed2),
                            ThreadLed2Stack,
                            THREAD_LED_STACK_BYTES,
                            THREAD_LED_PRIORITY,
                            THREAD_LED_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led2�����߳� */
    state = TclCreateThread(&ThreadCtrl,
                            "thread ctrl",
                            &ThreadCtrlEntry,
                            (TArgument)(&ThreadLed2),
                            ThreadCtrlStack,
                            THREAD_CTRL_STACK_BYTES,
                            THREAD_CTRL_PRIORITY,
                            THREAD_CTRL_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led1�����߳� */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");


    /* ����Led2�����߳� */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�豸�����߳� */
    state = TclActivateThread(&ThreadCtrl, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
}


/* ������BOOT֮������main�����������ṩ */
int main(void)
{
    /* ע������ں˺����������ں� */
    TclStartKernel(&AppSetupEntry,
                   &OsCpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    return 1;
}

#endif

