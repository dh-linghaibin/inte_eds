/* �߳�ʱ��Ƭ�޸�API��ʾ */
#include "trochili.h"
#include "example.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE7)

/* �����߳�ʱ��Ҫ�Ĳ��� */
#define THREAD_LED_STACK_BYTES   (512)
#define THREAD_LED_PRIORITY      (5)
#define THREAD_LED_SLICE         (100)
#define THREAD_LED_SLICE_MAX     (400)
#define THREAD_CTRL_STACK_BYTES  (256*2)
#define THREAD_CTRL_PRIORITY     (5)
#define THREAD_CTRL_SLICE        (100)

/* �̶߳��� */
static TThread ThreadLedOn;
static TThread ThreadLedOff;
static TThread ThreadCTRL;

/* �߳�ջ���� */
static TBase32 ThreadLedOnStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLedOffStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* �߳�LedOn���̺߳��� */
static void ThreadLedOnEntry(TArgument data)
{
    while (eTrue)
    {
        EvbLedControl(LED1, LED_ON);
    }
}

/* �߳�LedOff���̺߳��� */
static void ThreadLedOffEntry(TArgument data)
{
    while (eTrue)
    {
        EvbLedControl(LED1, LED_OFF);
    }
}

/* �߳�CTRL���̺߳��� */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;

    static TTimeTick ticks = THREAD_LED_SLICE;
    while (eTrue)
    {
        state = TclSetThreadSlice(&ThreadLedOn, ticks, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
        if (ticks < THREAD_LED_SLICE_MAX)
        {
            ticks += THREAD_LED_SLICE;
        }
        else
        {
            ticks = THREAD_LED_SLICE;
        }

        state = TclYieldThread(&error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}

/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ��CTRL�߳� */
    state = TclCreateThread(&ThreadCTRL, "thread ctrl",
                          &ThreadCtrlEntry, (TArgument)0,
                          ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLedOn, "thread led on",
                          &ThreadLedOnEntry, (TArgument)0,
                          ThreadLedOnStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLedOff, "thread led off",
                          &ThreadLedOffEntry, (TArgument)0,
                          ThreadLedOffStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����CTRL�߳� */
    state = TclActivateThread(&ThreadCTRL, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�豸�����߳� */
    state = TclActivateThread(&ThreadLedOn, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�豸�����߳� */
    state = TclActivateThread(&ThreadLedOff, &error);
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
