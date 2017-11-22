#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH8_FLAGS_EXAMPLE1)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* �û��߳�ջ���� */
static TBase32 ThreadLedOnStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLedOffStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* �û��̶߳��� */
static TThread ThreadLedOn;
static TThread ThreadLedOff;
static TThread ThreadCTRL;

/* �û��¼���� */
static TFlags LedFlags;
#define LED1_ON_FLG  (0x1<<0)
#define LED2_ON_FLG  (0x1<<1)
#define LED3_ON_FLG  (0x1<<2)

#define LED1_OFF_FLG (0x1<<4)
#define LED2_OFF_FLG (0x1<<5)
#define LED3_OFF_FLG (0x1<<6)


/* Led�̵߳������� */
static void ThreadLedOnEntry(TArgument arg)
{
    TState state;
    TError error;
    TBitMask pattern;
    TOption option;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�¼����õ������Led */
        pattern = LED1_ON_FLG | LED2_ON_FLG | LED3_ON_FLG;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;

        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if (state == eSuccess)
        {
            if (pattern & LED1_ON_FLG)
            {
                EvbLedControl(LED1, LED_ON);
            }

            if (pattern & LED2_ON_FLG)
            {
                EvbLedControl(LED2, LED_ON);
            }

            if (pattern & LED3_ON_FLG)
            {
                EvbLedControl(LED3, LED_ON);
            }
        }
    }
}


/* Led�̵߳������� */
static void ThreadLedOffEntry(TArgument arg)
{
    TState state;
    TError error;
    TBitMask pattern;
    TOption option;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�¼����õ���Ϩ��Led */
        pattern = LED1_OFF_FLG | LED2_OFF_FLG | LED3_OFF_FLG;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;

        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if (state == eSuccess)
        {
            if (pattern & LED1_OFF_FLG)
            {
                EvbLedControl(LED1, LED_OFF);
            }

            if (pattern & LED2_OFF_FLG)
            {
                EvbLedControl(LED2, LED_OFF);
            }

            if (pattern & LED3_OFF_FLG)
            {
                EvbLedControl(LED3, LED_OFF);
            }
        }
    }
}

/* CTRL�̵߳������� */
static void ThreadCtrlEntry(TArgument arg)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* CTRL�߳��ͷ��¼���� */
        state =  TclSendFlags(&LedFlags, LED1_ON_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL�߳���ʱ1�� */
        state =  TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL�߳��ͷ��¼���� */
        state =  TclSendFlags(&LedFlags, LED2_ON_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL�߳���ʱ1�� */
        state =   TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL�߳��ͷ��¼���� */
        state =  TclSendFlags(&LedFlags, LED3_ON_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL�߳���ʱ1�� */
        state =  TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL�߳��ͷ��¼���� */
        state =  TclSendFlags(&LedFlags, LED1_OFF_FLG | LED2_OFF_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL�߳���ʱ1�� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL�߳��ͷ��¼���� */
        state =  TclSendFlags(&LedFlags, LED3_OFF_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL�߳���ʱ1�� */
        state =  TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ���¼���� */
    state = TclCreateFlags(&LedFlags, "flag", TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLedOn, "thread led on",
                           &ThreadLedOnEntry, (TArgument)0,
                           ThreadLedOnStack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

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

    /* ����Led�߳� */
    state = TclActivateThread(&ThreadLedOn, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    state = TclActivateThread(&ThreadLedOff, &error);
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
