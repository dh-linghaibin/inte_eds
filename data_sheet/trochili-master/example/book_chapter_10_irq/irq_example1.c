#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH10_IRQ_ISR_EXAMPLE)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)


/* �û��߳�ջ���� */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* �û��̶߳��� */
static TThread ThreadLed;

/* �û��ź������� */
static TSemaphore LedSemaphore;

/* Led�̵߳������� */
static void ThreadLedEntry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�ź���������ɹ������Led */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            TCLM_ASSERT((error == TCLE_IPC_NONE), "");
            EvbLedControl(LED1, LED_ON);
        }

        /* Led�߳���������ʽ��ȡ�ź���������ɹ���Ϩ��Led */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            TCLM_ASSERT((error == TCLE_IPC_NONE), "");
            EvbLedControl(LED1, LED_OFF);
        }
    }
}


/* �����尴���жϴ����� */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TError error;
	
    if (EvbKeyScan())
    {
        /* ISR�Է�������ʽ(����)�ͷ��ź��� */
        state = TclIsrReleaseSemaphore(&LedSemaphore, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IRQ_NONE), "");
    }
    return TCLR_IRQ_DONE;
}

/* �û�Ӧ�ó�����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ���ú�KEY��ص��ⲿ�ж����� */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    /* ��ʼ���ź��� */
    state = TclCreateSemaphore(&LedSemaphore, "semaphore", 0, 1, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led�߳� */
    state = TclCreateThread(&ThreadLed,
	                        "thread led",
                          &ThreadLedEntry,
                          (TArgument)0,
                          ThreadLedStack,
                          THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY,
                          THREAD_LED_SLICE,
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
