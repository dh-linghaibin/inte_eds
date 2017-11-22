#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH4_SEMAPHORE_EXAMPLE2)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

/* �û��߳�ջ���� */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];

/* �û��̶߳��� */
static TThread ThreadLed1;
static TThread ThreadLed2;

/* �û��ź������� */
static TSemaphore LedSemaphore1;
static TSemaphore LedSemaphore2;


/* Led�߳�1�������� */
static void ThreadLed1Entry(TArgument arg)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led�߳�1��������ʽ��ȡ�ź���1������ɹ��͵���Led1*/
        state = TclObtainSemaphore(&LedSemaphore1, TCLO_IPC_WAIT,
                                   0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        EvbLedControl(LED1, LED_ON);

        /* Led�߳�1��ʱ1���ر�Led1 */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        EvbLedControl(LED1, LED_OFF);

        /* Led�߳�1�Է�������ʽ�ͷ��ź���2 */
        state = TclReleaseSemaphore(&LedSemaphore2, TCLO_IPC_DEFAULT,
                                    0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* Led�߳�2�������� */
static void ThreadLed2Entry(TArgument arg)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led�߳�2����Led2 */
        EvbLedControl(LED2, LED_ON);

        /* Led�߳�2��ʱ1���ر�Led2 */
        state =TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        EvbLedControl(LED2, LED_OFF);

        /* Led�߳�2�Է�������ʽ�ͷ��ź���1 */
        state =TclReleaseSemaphore(&LedSemaphore1, TCLO_IPC_DEFAULT,
                                   0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led�߳�2��������ʽ��ȡ�ź���2 */
        state = TclObtainSemaphore(&LedSemaphore2, TCLO_IPC_WAIT,
                                   0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ���ź���1 */
    state = TclCreateSemaphore(&LedSemaphore1, "semaphore1", 0, 1, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ���ź���2 */
    state = TclCreateSemaphore(&LedSemaphore2, "semaphore2", 0, 1, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led�߳�1 */
    state = TclCreateThread(&ThreadLed1, "thread led1",
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�߳�2 */
    state = TclCreateThread(&ThreadLed2, "thread led2",
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY + 1, THREAD_LED_SLICE,
                          &error);
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
