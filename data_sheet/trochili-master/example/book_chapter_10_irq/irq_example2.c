#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH10_IRQ_DAEMON_EXAMPLE)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES         (512)
#define THREAD_LED_PRIORITY            (5)
#define THREAD_LED_SLICE               (20)

/* �û��߳�ջ���� */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed3Stack[THREAD_LED_STACK_BYTES/4];

/* �û��̶߳��� */
static TThread ThreadLed1;
static TThread ThreadLed2;
static TThread ThreadLed3;

/* �û��ź������� */
static TSemaphore LedSemaphore1;
static TSemaphore LedSemaphore2;

/* IRQ������� */
static TIrq  irq1;
static TIrq  irq2;

/* �����尴��IRQ�ص����� */
static void IrqCallback(TArgument arg)
{
    TState state;
    TError error;
    TSemaphore* pSemaphore;

    pSemaphore = (TSemaphore*)arg;
    state = TclReleaseSemaphore(pSemaphore, TCLO_IPC_DEFAULT, 0U, &error);
    TCLM_ASSERT((state == eSuccess), "");
}


/* �����尴���жϴ����� */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TError error;
    int key;

    key = EvbKeyScan();
    if (key == 1)
    {
        state = TclPostIRQ(&irq1,
                           IrqCallback,
                           (TArgument)(&LedSemaphore1),
                           (TPriority)5,
                           &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

        return TCLR_IRQ_DAEMON;
    }
    else if (key == 2)
    {
        state = TclPostIRQ(&irq2,
                           IrqCallback,
                           (TArgument)(&LedSemaphore2),
                           (TPriority)5,
                           &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

        return TCLR_IRQ_DAEMON;
    }
    else
    {
        return TCLR_IRQ_DONE;
    }
}


/* Led1�̵߳������� */
static void ThreadLed1Entry(TBase32 arg0)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�ź���������ɹ������Led */
        state = TclObtainSemaphore(&LedSemaphore1, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED1, LED_ON);

        /* Led�߳���������ʽ��ȡ�ź���������ɹ���Ϩ��Led */
        state = TclObtainSemaphore(&LedSemaphore1, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED1, LED_OFF);
    }
}


/* Led2�̵߳������� */
static void ThreadLed2Entry(TBase32 arg0)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�ź���������ɹ������Led */
        state = TclObtainSemaphore(&LedSemaphore2, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED2, LED_ON);

        /* Led�߳���������ʽ��ȡ�ź���������ɹ���Ϩ��Led */
        state = TclObtainSemaphore(&LedSemaphore2, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED2, LED_OFF);
    }
}


/* Led3�̵߳������� */
static void ThreadLed3Entry(TBase32 arg0)
{
    TState state;
    TError error;

    while (eTrue)
    {
        EvbLedControl(LED3, LED_ON);
        state = TclDelayThread(TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        EvbLedControl(LED3, LED_OFF);
        state = TclDelayThread(TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}

/* �û�Ӧ�ó�����ں��� */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    /* ��ʼ���ź���1 */
    state = TclCreateSemaphore(&LedSemaphore1, "semaphore1", 0, 1, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ���ź���2 */
    state = TclCreateSemaphore(&LedSemaphore2, "semaphore2", 0, 1, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ���ú�KEY��ص��ⲿ�ж����� */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    /* ��ʼ��Led1�߳� */
    state = TclCreateThread(&ThreadLed1,
                            "thread led1",
                            &ThreadLed1Entry,
                            (TArgument)0,
                            ThreadLed1Stack,
                            THREAD_LED_STACK_BYTES,
                            THREAD_LED_PRIORITY,
                            THREAD_LED_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led2�߳� */
    state = TclCreateThread(&ThreadLed2,
                            "thread led2",
                            &ThreadLed2Entry,
                            (TArgument)0,
                            ThreadLed2Stack,
                            THREAD_LED_STACK_BYTES,
                            THREAD_LED_PRIORITY,
                            THREAD_LED_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led3�߳� */
    state = TclCreateThread(&ThreadLed3,
                            "thread led3",
                            &ThreadLed3Entry,
                            (TArgument)0,
                            ThreadLed3Stack,
                            THREAD_LED_STACK_BYTES,
                            THREAD_LED_PRIORITY,
                            THREAD_LED_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led1�߳� */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led2�߳� */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led3�߳� */
    state = TclActivateThread(&ThreadLed3, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
}


/* ������BOOT֮������main�����������ṩ */
int main(void)
{
    /* ע������ں˺���, �����ں� */
    TclStartKernel(&AppSetupEntry,
                   &OsCpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    return 1;
}

#endif

