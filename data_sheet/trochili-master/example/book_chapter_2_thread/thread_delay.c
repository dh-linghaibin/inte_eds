/*  �߳���ʱAPI��ʾ */

#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE4)

/* �����߳�ʱ��Ҫ�Ĳ��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (65535)

/* �̶߳��塢�߳�ջ���� */
static TThread ThreadLed;
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* Led�̵߳��߳������� */
static void ThreadLedEntry(TArgument data)
{
    TError error;
    TState state;
    TThread* pThread;

    /* ����2Ϊ�̵߳��ں˲��������ں��Զ�����Ϊ��ǰ�̵߳ĵ�ַ */
    pThread = (TThread*)data;
    while (eTrue)
    {
        EvbLedControl(LED1, LED_ON);
        state = TclDelayThread(pThread, TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        EvbLedControl(LED1, LED_OFF);
        state = TclDelayThread(pThread, TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}

/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLed,
                          &ThreadLedEntry,
                          (TArgument)(&ThreadLed),
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
    /* ע������ں˺����������ں� */
    TclStartKernel(&AppSetupEntry,
                   &OsCpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    return 1;
}

#endif


