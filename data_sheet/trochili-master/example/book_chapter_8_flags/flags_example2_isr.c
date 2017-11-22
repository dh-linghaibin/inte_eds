#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH8_FLAGS_EXAMPLE2)

/* �û��̲߳��� */
#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* �û��߳�ջ���� */
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* �û��̶߳��� */
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
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;
    TBitMask pattern;
    TOption option;

    while (eTrue)
    {
        /* Led�߳���������ʽ��ȡ�¼����õ���Ϩ��Led */
        pattern = LED1_OFF_FLG | LED1_ON_FLG;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;

        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if (state == eSuccess)
        {
            if (pattern & LED1_OFF_FLG)
            {
                EvbLedControl(LED1, LED_OFF);
            }

            if (pattern & LED1_ON_FLG)
            {
                EvbLedControl(LED1, LED_ON);
            }
        }
    }
}


/* �����尴���жϴ����� */
static TBitMask EvbKeyISR(TArgument data)
{
    static TBase32 cmd = 0;
    TError error;

    if (EvbKeyScan())
    {
        /* Key ISR�Է�������ʽ(����)�����¼���� */
        cmd++;
        if (cmd % 2)
        {
            TclSendFlags(&LedFlags, LED1_OFF_FLG, &error);
        }
        else
        {
            TclSendFlags(&LedFlags, LED1_ON_FLG, &error);
        }
    }

    return 0;
}

/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ���ú�KEY��ص��ⲿ�ж����� */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    /* ��ʼ���¼���� */
    state = TclCreateFlags(&LedFlags, "flag", TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��CTRL�߳� */
    state = TclCreateThread(&ThreadCTRL, "thread ctrl",
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
