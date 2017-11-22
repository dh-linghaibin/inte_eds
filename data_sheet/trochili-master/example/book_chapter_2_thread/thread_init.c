
#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE1)

#define THREAD_LED_STACK_BYTES (512)
#define THREAD_LED_PRIORITY    (5)
#define THREAD_LED_SLICE       (20)

static TThread ThreadLed;
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];


static void delay(TBase32 count)
{
    while (count--)
        ;
}

/* �߳�Led����ں��� */
static void ThreadLedEntry(TArgument data)
{
    while (eTrue)
    {
        delay(0x8FFFFF);
        EvbLedControl(LED1, LED_ON);

        delay(0x8FFFFF);
        EvbLedControl(LED1, LED_OFF);

        delay(0x8FFFFF);
        EvbLedControl(LED2, LED_ON);

        delay(0x8FFFFF);
        EvbLedControl(LED2, LED_OFF);

        delay(0x8FFFFF);
        EvbLedControl(LED3, LED_ON);

        delay(0x8FFFFF);
        EvbLedControl(LED3, LED_OFF);
    }
}

/* �û�Ӧ����ں��� */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    /* ��ʼ��Led�豸�����߳� */
    state = TclCreateThread(&ThreadLed, "thread led",
                          &ThreadLedEntry, (TArgument)0,
                          ThreadLedStack,  THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
	
    /* ����Led�����߳� */
    state =TclActivateThread(&ThreadLed, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    TclTrace("example start!\r\n");
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
