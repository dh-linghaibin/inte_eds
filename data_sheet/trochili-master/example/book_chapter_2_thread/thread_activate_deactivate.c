/*  �̼߳��������API��ʾ */

#include "trochili.h"
#include "example.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE2)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES   (512)
#define THREAD_LED_PRIORITY      (5)
#define THREAD_LED_SLICE         (20)

#define THREAD_CTRL_STACK_BYTES  (512)
#define THREAD_CTRL_PRIORITY     (4)
#define THREAD_CTRL_SLICE        (20)

/* �û��̶߳��� */
static TThread ThreadLedOn;
static TThread ThreadLedOff;
static TThread ThreadCTRL;

/* �û��߳�ջ */
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
    TError error;
    TState state;

    while (eTrue)
    {
        /* ����Led�豸�����߳� */
        state = TclActivateThread(&ThreadLedOn, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* �����߳���ʱ1�� */
        state = TclDelayThread(TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* ����Led�豸�����߳� */
        state = TclDeactivateThread(&ThreadLedOn, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* ����Led�豸Ϩ���߳� */
        state = TclActivateThread(&ThreadLedOff, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* �����߳���ʱ1�� */
        state = TclDelayThread(TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* ����Led�豸Ϩ���߳� */
        state = TclDeactivateThread(&ThreadLedOff, &error);
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
    state = TclCreateThread(&ThreadLedOn, "thread led on",
                          &ThreadLedOnEntry, (TArgument)0,
                          ThreadLedOnStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�豸Ϩ���߳� */
    state = TclCreateThread(&ThreadLedOff, "thread led off",
                          &ThreadLedOffEntry, (TArgument)0,
                          ThreadLedOffStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��CTRL�߳� */
    state = TclCreateThread(&ThreadCTRL, "thread led ctrl",
                          &ThreadCtrlEntry, (TArgument)0,
                          ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����CTRL�߳� */
    state = TclActivateThread(&ThreadCTRL, &error);
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

