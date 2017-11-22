#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH9_TIMER_CONFIG_EXAMPLE)

/* �����߳�ʱ��Ҫ�Ĳ��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (32U)

/* Led�߳̽ṹ��ջ */
static TThread ThreadLed;
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* �û���ʱ���ṹ */
static TTimer Led1Timer;
static TTimer Led2Timer;

static TBase32 mls1 = 1000;
static TBase32 mls2 = 1000;

/* �û���ʱ��1�Ļص����������1�룬������Ϩ��Led1 */
static void BlinkLed1(TArgument data, TBase32 cycles, TTimeTick ticks)
{
    static TIndex index = 0;
    if (index % 2)
    {
        EvbLedControl(LED1, LED_OFF);
    }
    else
    {
        EvbLedControl(LED1, LED_ON);
    }
    index++;
}

/* �û���ʱ��2�Ļص����������1�룬������Ϩ��Led2 */
static void BlinkLed2(TArgument data, TBase32 cycles, TTimeTick ticks)
{
    static TIndex index = 0;
    if (index % 2)
    {
        EvbLedControl(LED2, LED_OFF);
    }
    else
    {
        EvbLedControl(LED2, LED_ON);
    }
    index++;
}


/* �����尴���жϴ����� */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TError error;

    TBase32 keyid;

    keyid = EvbKeyScan();
    if (keyid == 1)
    {
        /* �ر��û���ʱ�� */
        state = TclStopTimer(&Led1Timer, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        state = TclStopTimer(&Led2Timer, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        mls1 *= 2;
        mls1 = (mls1 > 2000)? 2000:mls1;

        mls2 *= 2;
        mls2 = (mls2 > 2000)? 2000:mls2;

        /* �����û���ʱ�� */
        state = TclConfigTimer(&Led1Timer, TCLM_MLS2TICKS(mls1), (TPriority)5, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        state = TclConfigTimer(&Led2Timer, TCLM_MLS2TICKS(mls2), (TPriority)5, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");


        /* �����û���ʱ�� */
        state = TclStartTimer(&Led1Timer, 0U, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        state = TclStartTimer(&Led2Timer, 0U, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
    }
    else
    {
        /* �ر��û���ʱ�� */
        state = TclStopTimer(&Led1Timer, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        state = TclStopTimer(&Led2Timer, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        mls1 /= 2;
        mls1 = (mls1 < 100)? 100:mls1;

        mls2 /= 2;
        mls2 = (mls2 < 100)? 100:mls2;

        /* �����û���ʱ�� */
        state = TclConfigTimer(&Led1Timer, TCLM_MLS2TICKS(mls1), (TPriority)5, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        state = TclConfigTimer(&Led2Timer, TCLM_MLS2TICKS(mls2), (TPriority)5, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        /* �����û���ʱ�� */
        state = TclStartTimer(&Led1Timer, 0U, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        state = TclStartTimer(&Led2Timer, 0U, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
    }

    return TCLR_IRQ_DONE;
}


/* Led�߳������� */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;

    /* ��ʼ���û���ʱ��1 */
    state = TclCreateTimer(&Led1Timer,
                           "timer1",
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(mls1),
                           &BlinkLed1,
                           (TArgument)0,
                           (TPriority)5,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* ��ʼ���û���ʱ��2 */
    state = TclCreateTimer(&Led2Timer,
                           "timer2",
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(mls2),
                           &BlinkLed2,
                           (TArgument)0,
                           (TPriority)5,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");


    /* �����û���ʱ��1 */
    state = TclStartTimer(&Led1Timer, TCLM_MLS2TICKS(500), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* �����û���ʱ��2 */
    state = TclStartTimer(&Led2Timer, TCLM_MLS2TICKS(500), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");


    /* ���ú�KEY��ص��ⲿ�ж����� */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    while (eTrue)
    {
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ��Led�豸�����߳� */
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

    /* ����Led�豸�����߳� */
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
