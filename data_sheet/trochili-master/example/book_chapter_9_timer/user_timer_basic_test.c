#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH9_TIMER_BASIC_EXAMPLE)

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
static TTimer Led3Timer;

/* �û���ʱ��1�Ļص����������1�룬������Ϩ��Led1 */
static void BlinkLed1(TArgument data, TTimeTick ticks)
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
static void BlinkLed2(TArgument data, TTimeTick ticks)
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

/* �û���ʱ��3�Ļص����������1�룬������Ϩ��Led3 */
static void BlinkLed3(TArgument data, TTimeTick ticks)
{
    static TIndex index = 0;
    if (index % 2)
    {
        EvbLedControl(LED3, LED_OFF);
    }
    else
    {
        EvbLedControl(LED3, LED_ON);
    }
    index++;
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
                           TCLM_MLS2TICKS(1000),
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
                           TCLM_MLS2TICKS(1000),
                           &BlinkLed2,
                           (TPriority)5,
                           (TArgument)0,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* ��ʼ���û���ʱ��3 */
    state = TclCreateTimer(&Led3Timer,
                           "timer3",
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(1000),
                           &BlinkLed3,
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

    /* �����û���ʱ��3 */
    state = TclStartTimer(&Led3Timer, TCLM_MLS2TICKS(500), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

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
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    return 1;
}

#endif
