#include "example.h"

#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE1)
#include "trochili.h"

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
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

/* �û���Ϣ���Ͷ��� */
typedef struct
{
    TBase32 Index;
    TBase32 Value;
} TLedMsg;

/* �û���Ϣ���ж��� */
#define MQ_POOL_LEN (32)
static void* LedMsgPool[MQ_POOL_LEN];
static TMsgQueue LedMQ;

/* �û���Ϣ���� */
static TLedMsg LedMsg;

/* Led�̵߳������� */
static void ThreadLedEntry(TArgument arg)
{
    TState state;
    TError error;

    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led�߳���������ʽ������Ϣ������ɹ�������Ϣ����������������Ϩ��Led */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg),
                                  TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(pMsg->Index, pMsg->Value);
        }
    }
}


/* CTRL�̵߳������� */
static void ThreadCtrlEntry(TArgument arg)
{
    TState state;
    TError error;

    TLedMsg* pMsg = &LedMsg;
    while (eTrue)
    {
        /* ���س�����������ʽ����Led��������Ϣ */
        pMsg->Index = LED1;
        pMsg->Value = LED_ON;
        state =TclSendMessage(&LedMQ, (TMessage*)(&pMsg), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* ���س�����ʱ1�� */
        state =TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* ���س�����������ʽ����LedϨ�����Ϣ */
        pMsg->Index = LED1;
        pMsg->Value = LED_OFF;
        state =TclSendMessage(&LedMQ, (TMessage*)(&pMsg), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* ���س�����ʱ1�� */
        state =TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}


/* �û�Ӧ�ó�����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ����Ϣ���� */
    state =TclCreateMsgQueue(&LedMQ, "queue",(void**)(&LedMsgPool),
                           MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLed, "thread led",
                          &ThreadLedEntry, (TArgument)0,
                          ThreadLedStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��CTRL�߳� */
    state = TclCreateThread(&ThreadCTRL, "thread ctrl",
                          &ThreadCtrlEntry, (TArgument)0,
                          ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳� */
    state =TclActivateThread(&ThreadLed, &error);
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
