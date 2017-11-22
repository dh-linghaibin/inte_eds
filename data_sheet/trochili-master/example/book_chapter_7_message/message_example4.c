#include "example.h"
#include "trochili.h"


#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE4)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
#define THREAD_LED_SLICE        (20)

/* �û��̶߳��� */
static TThread ThreadLed1;
static TThread ThreadLed2;

/* �û��߳�ջ���� */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];

/* �û���Ϣ���Ͷ��� */
typedef struct
{
    TBase32 Index;
    TBase32 Value;
} TLedMsg;

/* �û���Ϣ���ж��� */
#define MQ_POOL_LEN (32)
static TMsgQueue LedMQ1;
static TMsgQueue LedMQ2;
static void* Led1MsgPool[MQ_POOL_LEN];
static void* Led2MsgPool[MQ_POOL_LEN];

/* �û���Ϣ���� */
static TLedMsg LedMsg1;
static TLedMsg LedMsg2;

/* Led1���̵߳������� */
static void ThreadLed1Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMsg* pMsg1 = &LedMsg1;
    TLedMsg* pMsg2 = &LedMsg2;

    while (eTrue)
    {
        /* Led1�߳���������ʽ����Led1������Ϣ */
        pMsg2->Index = LED2;
        pMsg2->Value = LED_ON;
        state = TclSendMessage(&LedMQ2, (void**)(&pMsg2), TCLO_IPC_WAIT,  0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1�߳���������ʽ������Ϣ��������Ϣ��������������Ϩ��Led1 */
        state = TclReceiveMessage(&LedMQ1, (void**)(&pMsg1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(pMsg1->Index, pMsg1->Value);

        pMsg2->Index = LED2;
        pMsg2->Value = LED_OFF;
        state = TclSendMessage(&LedMQ2, (void**)(&pMsg2), TCLO_IPC_WAIT,  0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1�߳���������ʽ������Ϣ��������Ϣ��������������Ϩ��Led1 */
        state = TclReceiveMessage(&LedMQ1, (void**)(&pMsg1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(pMsg1->Index, pMsg1->Value);
    }
}


/* Led2�̵߳������� */
static void ThreadLed2Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMsg* pMsg1 = &LedMsg1;
    TLedMsg* pMsg2 = &LedMsg2;

    while (eTrue)
    {
        /* Led2�߳���������ʽ������Ϣ��������Ϣ��������������Ϩ��Led2 */
        state = TclReceiveMessage(&LedMQ2, (void**)(&pMsg2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(pMsg2->Index, pMsg2->Value);

        /* Led2�߳���ʱ1�� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led2�߳���������ʽ����Led1������Ϣ */
        pMsg1->Index = LED1;
        pMsg1->Value = LED_ON;
        state = TclSendMessage(&LedMQ1, (void**)(&pMsg1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led2�߳���������ʽ������Ϣ��������Ϣ��������������Ϩ��Led2 */
        state = TclReceiveMessage(&LedMQ2, (void**)(&pMsg2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(pMsg2->Index, pMsg2->Value);

        /* Led2�߳���ʱ1�� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led2�߳���������ʽ����Led1Ϩ����Ϣ */
        pMsg1->Index = LED1;
        pMsg1->Value = LED_OFF;
        state = TclSendMessage(&LedMQ1, (void**)(&pMsg1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ����Ϣ���� */
    state = TclCreateMsgQueue(&LedMQ1, "queue1", (void**)(&Led1MsgPool),
                    MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    state = TclCreateMsgQueue(&LedMQ2, "queue2", (void**)(&Led2MsgPool),
                    MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led�豸�����߳�1 */
    state = TclCreateThread(&ThreadLed1,  "thread led1",
                  &ThreadLed1Entry, (TArgument)0,
                  ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                  THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                  &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�豸�����߳�2 */
    state = TclCreateThread(&ThreadLed2, "thread led2",
                  &ThreadLed2Entry, (TArgument)0,
                  ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                  THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                  &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�1 */
    state = TclActivateThread(&ThreadLed1,&error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�2 */
    state = TclActivateThread(&ThreadLed2,&error);
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
