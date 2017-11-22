#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH6_MAILBOX_EXAMPLE6)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* �û��̶߳��� */
static TThread ThreadLed1;
static TThread ThreadLed2;
static TThread ThreadLed3;
static TThread ThreadCTRL;

/* �û��߳�ջ���� */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed3Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* �û��ʼ����Ͷ��� */
typedef struct
{
    TIndex Index;
    TByte Value;
} TLedMail;

/* �û�������ʼ����� */
static TMailbox LedMailbox;
static TLedMail LedMail;


/* Led1���̵߳������� */
static void ThreadLed1Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail;

    while (eTrue)
    {
        /* Led3�߳���������ʽ�����ʼ����������ʼ����ݿ���Led3�ĵ�������Ϩ�� */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED1, pMail->Value);
        }
    }
}

/* Led2���̵߳������� */
static void ThreadLed2Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail;

    while (eTrue)
    {
        /* Led3�߳���������ʽ�����ʼ����������ʼ����ݿ���Led3�ĵ�������Ϩ�� */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED2, pMail->Value);
        }
    }
}

/* Led3�̵߳������� */
static void ThreadLed3Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail;

    while (eTrue)
    {
        /* Led3�߳���������ʽ�����ʼ����������ʼ����ݿ���Led3�ĵ�������Ϩ�� */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED3, pMail->Value);
        }
    }
}


/* CTRL�̵߳������� */
static void ThreadCtrlEntry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail;

    pMail = &LedMail;
    while (eTrue)
    {
        /* �����߳���ʱ1���㲥����Led�򿪵��ʼ� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        pMail->Index = LEDX;
        pMail->Value = LED_ON;
        state = TclBroadcastMail(&LedMailbox, (TMail*)(&pMail), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* �����߳���ʱ1���㲥����LedϨ����ʼ� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        pMail->Index = LEDX;
        pMail->Value = LED_OFF;
        state = TclBroadcastMail(&LedMailbox, (TMail*)(&pMail), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ������ */
    state =  TclCreateMailbox(&LedMailbox, "mbox", TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led1�豸�����߳� */
    state =  TclCreateThread(&ThreadLed1,  "thread led1",
                           &ThreadLed1Entry, (TArgument)0,
                           ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led2�豸�����߳� */
    state =  TclCreateThread(&ThreadLed2,  "thread led2",
                           &ThreadLed2Entry, (TArgument)0,
                           ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led3�豸�����߳� */
    state =  TclCreateThread(&ThreadLed3,  "thread led3",
                           &ThreadLed3Entry, (TArgument)0,
                           ThreadLed3Stack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��CTRL�߳� */
    state =  TclCreateThread(&ThreadCTRL,  "thread ctrl",
                           &ThreadCtrlEntry, (TArgument)0,
                           ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                           THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�1 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�2 */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�3 */
    state =  TclActivateThread(&ThreadLed3, &error);
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
