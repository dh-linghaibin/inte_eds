#include "tcl.gd32f190.h"
#include "colibri190bsp.h"

/* ����ʹ���������ϵ��豸 */
void EvbSetupEntry(void)
{
   EvbUart1Config();
   EvbLedConfig();
   EvbKeyConfig();
}


void EvbTraceEntry(const char* str)
{
    EvbUart1WriteStr(str);
}


