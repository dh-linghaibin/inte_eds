#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH6_MAILBOX_EXAMPLE4)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

/* �û��̶߳��� */
static TThread ThreadLed1;
static TThread ThreadLed2;

/* �û��߳�ջ���� */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];

/* �û��ʼ����Ͷ��� */
typedef struct
{
    TIndex Index;
    TByte Value;
} TLedMail;


static TMailbox Led1Mailbox;
static TMailbox Led2Mailbox;
static TLedMail Led1Mail;
static TLedMail Led2Mail;


/* Led1�̵߳������� */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;

    TLedMail* pMail1;
    TLedMail* pMail2;

    pMail2 = &Led2Mail;
    while (eTrue)
    {
        /* Led1�߳���������ʽ����Led1�����ʼ� */
        state = TclReceiveMail(&Led1Mailbox, (TMail*)(&pMail1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1�߳̿���Led1�ĵ�����Ϩ�� */
        EvbLedControl(pMail1->Index, pMail1->Value);

        /* Led1�߳���ʱ1�� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led1�߳���������ʽ����Led2�����ʼ� */
        pMail2->Index = LED2;
        pMail2->Value = LED_ON;
        state = TclSendMail(&Led2Mailbox, (TMail*)(&pMail2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1�߳���������ʽ����Led1�����ʼ� */
        state = TclReceiveMail(&Led1Mailbox, (TMail*)(&pMail1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1�߳̿���Led1�ĵ�����Ϩ�� */
        EvbLedControl(pMail1->Index, pMail1->Value);

        /* Led1�߳���ʱ1�� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led1�߳���������ʽ����Led2Ϩ���ʼ� */
        pMail2->Index = LED2;
        pMail2->Value = LED_OFF;
        state = TclSendMail(&Led2Mailbox, (TMail*)(&pMail2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}

/* Led2�̵߳������� */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;

    TLedMail* pMail1;
    TLedMail* pMail2;

    pMail1 = &Led1Mail;
    while (eTrue)
    {
        /* Led2�߳���������ʽ����Led1�����ʼ� */
        pMail1->Index = LED1;
        pMail1->Value = LED_ON;
        state = TclSendMail(&Led1Mailbox, (TMail*)(&pMail1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led2�߳���������ʽ����Led2�����ʼ� */
        state = TclReceiveMail(&Led2Mailbox, (TMail*)(&pMail2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led2�߳̿���Led2�ĵ�����Ϩ�� */
        EvbLedControl(pMail2->Index, pMail2->Value);

        /* Led2�߳���������ʽ����Led1Ϩ���ʼ� */
        pMail1->Index = LED1;
        pMail1->Value = LED_OFF;
        state = TclSendMail(&Led1Mailbox, (TMail*)(&pMail1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led2�߳���������ʽ����Led2�����ʼ� */
        state = TclReceiveMail(&Led2Mailbox, (TMail*)(&pMail2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1�߳̿���Led1�ĵ�����Ϩ�� */
        EvbLedControl(pMail2->Index, pMail2->Value);
    }
}

/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ��Led1���� */	
	  state = TclCreateMailbox(&Led1Mailbox, "mbox1", TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");
	
  	/* ��ʼ��Led2���� */
    state = TclCreateMailbox(&Led2Mailbox, "mbox2", TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led1�豸�����߳� */
    state = TclCreateThread(&ThreadLed1, "thread led1",
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led2�豸�����߳� */
    state =  TclCreateThread(&ThreadLed2, "thread led2",
                           &ThreadLed2Entry, (TArgument)0,
                           ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY + 1, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�豸�����߳� */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    state = TclActivateThread(&ThreadLed2, &error);
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
