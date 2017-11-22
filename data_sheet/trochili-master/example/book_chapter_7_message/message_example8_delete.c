#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE8)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
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

/* Led1�̵߳������� */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led1�߳���������ʽ������Ϣ�����������Ϣ���б����������Led1 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED1,  LED_ON);
        }

        /* Led1�߳���������ʽ������Ϣ�����������Ϣ���б�������Ϩ��Led1 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED1,  LED_OFF);
        }
    }
}

/* Led2���̵߳������� */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led2�߳���������ʽ������Ϣ�����������Ϣ���б����������Led2 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED2, LED_ON);
        }

        /* Led2�߳���������ʽ������Ϣ�����������Ϣ���б�������Ϩ��Led2 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED2, LED_OFF);
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
        /* Led3�߳���������ʽ������Ϣ�����������Ϣ���б����������Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED3,  LED_ON);
        }

        /* Led3�߳���������ʽ������Ϣ�����������Ϣ���б�������Ϩ��Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED3, LED_OFF);
        }
    }
}

/* CTRL�̵߳������� */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* CTRL�߳���ʱ1�� */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* ������Ϣ���� */
        state = TclDeleteMsgQueue(&LedMQ, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* ���ú����Ϣ������Ҫ�ٴγ�ʼ������ʹ�� */
        state = TclCreateMsgQueue(&LedMQ, "queue renewal", (void**)(&LedMsgPool),
                                MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
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

    /* ��ʼ��Led�߳�1 */
    state = TclCreateThread(&ThreadLed1, "thread led1",
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�߳�2 */
    state = TclCreateThread(&ThreadLed2, "thread led2",
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�߳�3 */
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

