#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH4_SEMAPHORE_EXAMPLE4)

#define THREAD_SYNC_ENABLE (1)

/* �û��̲߳��� */
#define THREAD_UART_STACK_BYTES  (512)
#define THREAD_UART_PRIORITY     (5)
#define THREAD_UART_SLICE        (100)

/* �û��߳�ջ���� */
static TBase32 ThreadUartLowCaseStack[THREAD_UART_STACK_BYTES/4];
static TBase32 ThreadUartUpCaseStack[THREAD_UART_STACK_BYTES/4];

/* �û��̶߳��� */
static TThread ThreadUartLowCase;
static TThread ThreadUartUpCase;

/* �û��ź������� */
static TSemaphore UartSemaphore;

/* �ַ���ӡ������ͨ��BSP���ַ��������������Ĵ��� */
static void PrintfString(char* str)
{
    TState state;
    TError error;

    /* ��ǰ�߳���������ʽ��ȡ�ź������ɹ���ͨ��BSP��ӡ�ַ��� */
#if (THREAD_SYNC_ENABLE)
    state = TclObtainSemaphore(&UartSemaphore, TCLO_IPC_WAIT, 0, &error);
    if (state == eSuccess)
    {
        TclTrace(str);
    }
    /* �ַ�����ȫ��ӡ�󣬵�ǰ�߳��Է�������ʽ�ͷ��ź��� */
    TclReleaseSemaphore(&UartSemaphore, 0, 0, &error);
#else
    error = error;
    state = state;
    TclTrace(str);
#endif
}

/* ��ӡ��д�ַ����̵߳������� */
static void ThreadUartLowCaseEntry(TArgument arg)
{
    while (eTrue)
    {
        PrintfString("ABCDDEFGHIJK\r\n");
    }
}

/* ��ӡСд�ַ����̵߳������� */
static void ThreadUartUpCaseEntry(TArgument arg)
{
    while (eTrue)
    {
        PrintfString("abcdefghijk\r\n");
    }
}


/* �û�Ӧ�ó�����ں��� */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* ��ʼ���ź��� */
    state = TclCreateSemaphore(&UartSemaphore, "uart semaphore", 1, 1, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* ��ʼ��UART�豸�����߳� */
    state = TclCreateThread(&ThreadUartLowCase, "thread lowcase",
                          &ThreadUartLowCaseEntry, (TArgument)0,
                          ThreadUartLowCaseStack, THREAD_UART_STACK_BYTES,
                          THREAD_UART_PRIORITY, THREAD_UART_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ��ʼ��UART�豸�����߳� */
    state = TclCreateThread(&ThreadUartUpCase, "thread upcase",
                          &ThreadUartUpCaseEntry, (TArgument)0,
                          ThreadUartUpCaseStack, THREAD_UART_STACK_BYTES,
                          THREAD_UART_PRIORITY, THREAD_UART_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����UARTСд�߳� */
    state = TclActivateThread(&ThreadUartLowCase, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* ����UART��д�߳� */
    state = TclActivateThread(&ThreadUartUpCase, &error);
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
