#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH13_BOARD_TEST_EXAMPLE)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES         (515)
#define THREAD_LED_PRIORITY            (5)
#define THREAD_LED_SLICE               (20)


/* �û��߳�ջ���� */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* �û��̶߳��� */
static TThread ThreadLed;

/* �û���ʱ���ṹ */
static TTimer LedTimer1;
static TTimer LedTimer2;

/* �û���ʱ��1�Ļص����������1�룬������Ϩ��Led1 */
static void Blink1(TArgument data, TBase32 cycles, TTimeTick ticks)
{
    static int index = 0;

    if (index % 2)
    {
        EvbLedControl(LED1, LED_OFF);
        TclTrace("LED1 OFF\r\n");
    }
    else
    {
        EvbLedControl(LED1, LED_ON);
        TclTrace("LED1 ON\r\n");
    }
    index++;
}

/* �û���ʱ��1�Ļص����������1�룬������Ϩ��Led1 */
static void Blink2(TArgument data, TBase32 cycles, TTimeTick ticks)
{
    static int index = 0;

    if (index % 2)
    {
        EvbLedControl(LED2, LED_OFF);
        TclTrace("LED2 OFF\r\n");
    }
    else
    {
        EvbLedControl(LED2, LED_ON);
        TclTrace("LED2 ON\r\n");
    }
    index++;
}

/* �����尴���жϴ����� */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TError error;

    TBase32 keyid;
    static TBase32 index1 = 1;
    static TBase32 index2 = 1;

    keyid = EvbKeyScan();
    if (keyid == 1)
    {
        if (index1 % 2)
        {
            /* �����û���ʱ�� */
            state = TclStartTimer(&LedTimer1, 0U, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
        }
        else
        {
            /* �ر��û���ʱ�� */
            state = TclStopTimer(&LedTimer1, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
        }
        index1++;
    }
    else
    {
        if (index2 % 2)
        {
            /* �����û���ʱ�� */
            state = TclStartTimer(&LedTimer2, 0U, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
        }
        else
        {
            /* �ر��û���ʱ�� */
            state = TclStopTimer(&LedTimer2, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
        }
        index2++;
    }

    return TCLR_IRQ_DONE;
}


/* Led�̵߳������� */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
        EvbLedControl(LED3, LED_ON);
        TclTrace("LED3 ON\r\n");

        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
        EvbLedControl(LED3, LED_OFF);
        TclTrace("LED3 OFF\r\n");
    }
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

    /* ��ʼ���û���ʱ�� */
    state = TclCreateTimer(&LedTimer1,"timer1", TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(1000),
                           &Blink1, (TArgument)0, (TPriority)5, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* ��ʼ���û���ʱ�� */
    state = TclCreateTimer(&LedTimer2, "timer2", TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(1000),
                           &Blink2, (TArgument)0, (TPriority)5, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* ��ʼ��Led�߳� */
    state = TclCreateThread(&ThreadLed, "thread led",
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
    /* ע������ں˺����������ں� */
    TclStartKernel(&AppSetupEntry,
                   &OsCpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    return 1;
}

#endif
