/* �߳����ȼ��޸�API��ʾ */

#include "trochili.h"
#include "example.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE6)

/* �����߳�ʱ��Ҫ�Ĳ��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (0xFFFFFFFF)

/* �̶߳��� */
static TThread ThreadLed1;
static TThread ThreadLed2;
/* �߳�ջ���� */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];

/* �߳������ò������𵽿�תЧ�� */
static void delay(TBase32 count)
{
    while(count--);
}


/* �߳�Led1����ں��� */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;

    TByte turn = 0;
    while (eTrue)
    {
        EvbLedControl(LED1, LED_ON);
        delay(0x8FFFFF);
        EvbLedControl(LED1, LED_OFF);
        delay(0x8FFFFF);
        turn ++;
        if (turn == 2)
        {
            state = TclSetThreadPriority(&ThreadLed2, THREAD_LED_PRIORITY - 1, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
            turn = 0;
        }
    }
}


/* �߳�Led2����ں��� */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;

    TByte turn = 0;
    while (eTrue)
    {
        EvbLedControl(LED2, LED_ON);
        delay(0x8FFFFF);
        EvbLedControl(LED2, LED_OFF);
        delay(0x8FFFFF);
        turn ++;
        if (turn == 2)
        {
            state = TclSetThreadPriority(&ThreadLed2, THREAD_LED_PRIORITY + 1, &error );
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
            turn = 0;
        }
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ��Led1�豸�����߳� */
    state = TclCreateThread(&ThreadLed1, "thread led1",
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led2�豸�����߳� */
    state = TclCreateThread(&ThreadLed2, "thread led2",
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY + 1, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led1�豸�����߳� */
    state = TclActivateThread(&ThreadLed1, &error);

    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led2�豸�����߳� */
    state = TclActivateThread(&ThreadLed2, &error);
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
