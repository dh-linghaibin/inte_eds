#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE6)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (8)
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

/* �û���Ϣ���Ͷ��� */
typedef struct
{
    TBase32 Index;
    TBase32 Value;
} TLedMsg;

/* �û���Ϣ���ж��� */
#define MQ_POOL_LEN (32)
static TMsgQueue LedMQ;
static void* LedMsgPool[MQ_POOL_LEN];

/* �û���Ϣ���� */
static TLedMsg LedMsg;

/* Led1�̵߳������� */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led3�߳���������ʽ������Ϣ��������Ϣ��������������Ϩ��Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED1, pMsg->Value);
        }
    }
}

/* Led2�̵߳������� */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led3�߳���������ʽ������Ϣ��������Ϣ��������������Ϩ��Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED2, pMsg->Value);
        }
    }
}

/* Led3�̵߳������� */
static void ThreadLed3Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led3�߳���������ʽ������Ϣ��������Ϣ��������������Ϩ��Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED3, pMsg->Value);
        }
    }
}

/* CTRL�̵߳������� */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg = &LedMsg;

    while (eTrue)
    {
        /* CTRL�߳���ʱ1�룬Ȼ��㲥��Ϣ��������Led������ */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        pMsg->Index = LEDX;
        pMsg->Value = LED_ON;
        TclBroadcastMessage(&LedMQ, (TMessage*)&pMsg, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL�߳���ʱ1�룬Ȼ��㲥��Ϣ��������Led��Ϩ�� */
        state =TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        pMsg->Index = LEDX;
        pMsg->Value = LED_OFF;
        state = TclBroadcastMessage(&LedMQ, (TMessage*)&pMsg, &error);
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
    state = TclCreateMsgQueue(&LedMQ, "queue", (void**)(&LedMsgPool),
                            MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��Led�豸1�����߳� */
    state = TclCreateThread(&ThreadLed1,  "thread led1",
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�豸2�����߳� */
    state = TclCreateThread(&ThreadLed2,  "thread led2",
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�豸3�����߳� */
    state = TclCreateThread(&ThreadLed3, "thread led3",
                          &ThreadLed3Entry, (TArgument)0,
                          ThreadLed3Stack, THREAD_LED_STACK_BYTES,
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

    /* ����Led�߳�1 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�2 */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�3 */
    state = TclActivateThread(&ThreadLed3, &error);
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


