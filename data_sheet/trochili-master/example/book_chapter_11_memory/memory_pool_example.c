#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH11_MEMORY_POOL_EXAMPLE)

/* �û��̲߳��� */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define DATA_BLOCK_BYTES  THREAD_LED_STACK_BYTES


/* �û��̶߳��� */
static TThread* pThreadLed1;
static TThread* pThreadLed2;
static TThread* pThreadLed3;


/* �û��߳�ջ���� */
static TBase32* pThreadStack1;
static TBase32* pThreadStack2;
static TBase32* pThreadStack3;


static TByte DataMemory[DATA_BLOCK_BYTES * 6];
static TMemPool DataMemoryPool;

/* Led1\Led2\led3�̵߳������� */
static void ThreadLedEntry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
        EvbLedControl(data, LED_ON);

        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
        EvbLedControl(data, LED_OFF);
    }
}

/* �û�Ӧ�ó�����ں��� */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    state = TclCreateMemoryPool(&DataMemoryPool, (void*)DataMemory, 6, DATA_BLOCK_BYTES, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadLed1), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadLed2), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadLed3), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadStack1), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadStack2), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadStack3), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    /* ��ʼ��Led�߳�1 */
    state = TclCreateThread(pThreadLed1, "thread led1",
                          &ThreadLedEntry, (TArgument)LED1,
                          pThreadStack1, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�߳�2*/
    state = TclCreateThread(pThreadLed2, "thread led2",
                          &ThreadLedEntry,(TArgument)LED2,
                          pThreadStack2, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��Led�߳�3 */
    state = TclCreateThread(pThreadLed3, "thread led3",
                          &ThreadLedEntry, (TArgument)LED3,
                          pThreadStack3, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);

    /* ����Led�߳�1 */
    state = TclActivateThread(pThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�2 */
    state = TclActivateThread(pThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����Led�߳�3 */
    state = TclActivateThread(pThreadLed3, &error);
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

