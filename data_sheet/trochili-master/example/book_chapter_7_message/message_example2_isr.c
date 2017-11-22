#include "example.h"
#include "trochili.h"


#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE2)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
#define THREAD_LED_SLICE        (20)

/* �û��̶߳��� */
static TThread ThreadLed;

/* �û��߳�ջ���� */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

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

/* Led�̵߳������� */
static void ThreadLedEntry(TArgument data)
{
    TError error;
    TState state;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led�߳���������ʽ������Ϣ������ɹ�������Ϣ��
        ��������������Ϩ��Led */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg),
                                  TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(pMsg->Index, pMsg->Value);
        }
    }
}

/* �����尴���жϴ����� */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
	TError error;
	
    TLedMsg* pMsg = &LedMsg;
    static TBase32 turn = 0;

    if (EvbKeyScan())
    {
        if (turn % 2)
        {
            /* KeyISR����Led��������Ϣ */
            pMsg->Index = LED1;
            pMsg->Value = LED_ON;
            state = TclIsrSendMessage(&LedMQ, (TMessage*)(&pMsg), TCLO_IPC_UARGENT, &error);
            TCLM_ASSERT((state == eSuccess), "");
        }
        else
        {
            /* KeyISR����LedϨ�����Ϣ */
            pMsg->Index = LED1;
            pMsg->Value = LED_OFF;
            state = TclIsrSendMessage(&LedMQ, (TMessage*)(&pMsg), (TOption)0, &error);
            TCLM_ASSERT((state == eSuccess), "");
        }
        turn++;
    }

    return 0;
}


/* �û�Ӧ�ó�����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ���ú�KEY��ص��ⲿ�ж����� */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    /* ��ʼ����Ϣ���� */
    state = TclCreateMsgQueue(&LedMQ, "queue", (void**)(&LedMsgPool),
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

    /* ����Led�߳� */
    state = TclActivateThread(&ThreadLed, &error);
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

