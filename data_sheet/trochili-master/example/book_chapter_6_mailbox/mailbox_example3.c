#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH6_MAILBOX_EXAMPLE3)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* �û��̶߳��� */
static TThread ThreadLed;
static TThread ThreadCTRL;

/* �û��߳�ջ���� */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* �û��ʼ����Ͷ��� */
typedef struct
{
    TIndex Index;
    TByte Value;
} TLedMail;


/* �û����䡢�ʼ����ź������� */
static TMailbox LedMailbox;
static TLedMail LedMail;
static TSemaphore LedSemaphore;

/* Led�̵߳������� */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;
    TLedMail* pMail;

    while (eTrue)
    {
        /* Led�߳���������ʽ�����ʼ� */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            /* Led�߳̿���Led�ĵ�����Ϩ�� */
            EvbLedControl(pMail->Index, pMail->Value);

            /* Led�߳���ʱ1�� */
            state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

            /* Led�߳��ͷ��ź��� */
            state = TclReleaseSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        }
    }
}

/* CTRL�̵߳������� */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;
    TLedMail* pMail = &LedMail;

    while (eTrue)
    {
        /* �����߳��Է�������ʽ���Ϳ���Led�������ʼ� */
        pMail->Value = LED_ON;
        pMail->Index = LED1;
        state = TclSendMail(&LedMailbox, (TMail*)(&pMail), 0, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* �����߳��Է�������ʽ��ȡ�ź��� */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* �����߳��Է�������ʽ���Ϳ���LedϨ����ʼ� */
        pMail->Value = LED_OFF;
        pMail->Index = LED1;
        state = TclSendMail(&LedMailbox, (TMail*)(&pMail), 0, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* �����߳���������ʽ��ȡ�ź��� */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* �û�Ӧ�ó�����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ���ź��������� */
    state = TclCreateMailbox(&LedMailbox, "mbox", TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    state = TclCreateSemaphore(&LedSemaphore, "semaphore", 0, 1, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLed, "thread led",
                          &ThreadLedEntry,
                          (TArgument)0,
                          ThreadLedStack,
                          THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY,
                          THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��CTRL�߳� */
    state = TclCreateThread(&ThreadCTRL, "thread ctrl",
                          &ThreadCtrlEntry,
                          (TArgument)0,
                          ThreadCTRLStack,
                          THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY,
                          THREAD_CTRL_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳� */
    state = TclActivateThread(&ThreadLed, &error);
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
