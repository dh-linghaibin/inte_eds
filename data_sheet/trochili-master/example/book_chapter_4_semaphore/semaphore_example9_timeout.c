#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH4_SEMAPHORE_EXAMPLE9)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

/* �û��߳�ջ���� */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* �û��̶߳��� */
TThread ThreadLed;

/* �û��ź������� */
static TSemaphore LedSemaphore;

/* Led�̵߳������� */
static void ThreadLedEntry(TArgument arg)
{
    TState state;
    TError error;

    while (eTrue)
    {
        state = eSuccess;
        error = TCLE_IPC_NONE;

        /* Led�߳���������ʽ��ȡ�ź������õ������Led */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT| TCLO_IPC_TIMEO,
                                   TCLM_SEC2TICKS(1), &error);
        if ((state == eFailure) && (error == TCLE_IPC_TIMEO))
        {
            EvbLedControl(LED1, LED_ON);
        }

        state = eSuccess;
        error = TCLE_IPC_NONE;

        /* Led�߳���������ʽ��ȡ�ź������õ���Ϩ��Led */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT| TCLO_IPC_TIMEO,
                                   TCLM_SEC2TICKS(1), &error);
        if ((state == eFailure) && (error == TCLE_IPC_TIMEO))
        {
            EvbLedControl(LED1, LED_OFF);
        }
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ���ź��� */
    state = TclCreateSemaphore(&LedSemaphore, "led semaphore", 0, 1,
                               TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLed, "led thread",
                            &ThreadLedEntry, (TArgument)0,
                            ThreadLedStack, THREAD_LED_STACK_BYTES,
                            THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳� */
    state = TclActivateThread(&ThreadLed, &error);
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

