/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.kernel.h"
#include "tcl.debug.h"


/*************************************************************************************************
 *  ���ܣ��ں˳������������ֹͣ��������                                                         *
 *  ������(1) pNote  ��ʾ�ַ���                                                                  *
 *        (2) pFile  ������ļ���                                                                *
 *        (3) pFunc  ����ĺ�����                                                                *
 *        (4) line   ����Ĵ�����                                                                *
 *  ���أ���                                                                                     *
 *  ˵���������ں���ϵ�����������ʱ��ȡ���ж�                                                 *
 *************************************************************************************************/
void OsDebugPanic(const char* pNote, const char* pFile, const char* pFunc, int line)
{
    OsCpuDisableInt();

    OsKernelVariable.DBGLog.File = pFile;
    OsKernelVariable.DBGLog.Func = pFunc;
    OsKernelVariable.DBGLog.Line = line;
    OsKernelVariable.DBGLog.Note = pNote;

    if (OsKernelVariable.TraceEntry != (TTraceEntry)0)
    {
        OsKernelVariable.TraceEntry(pNote);
    }
    if (OsKernelVariable.SysFaultEntry != (TSysFaultEntry)0)
    {
        OsKernelVariable.SysFaultEntry(&OsKernelVariable);
    }

    while (eTrue);
}


/*************************************************************************************************
 *  ���ܣ��ں˳��ַ�������������о���                                                           *
 *  ������(1) pNote  ��ʾ�ַ���                                                                  *
 *  ���أ���                                                                                     *
 *  ˵���������ں���ϵ�����������ʱ��ȡ���ж�                                                 *
 *************************************************************************************************/
void OsDebugWarning(const char* pNote)
{
    TReg32 imask;

    OsCpuEnterCritical(&imask);
    if (OsKernelVariable.TraceEntry != (TTraceEntry)0)
    {
        OsKernelVariable.TraceEntry(pNote);
    }
    OsCpuLeaveCritical(imask);

    if (OsKernelVariable.SysWarningEntry != (TSysWarningEntry)0)
    {
        OsKernelVariable.SysWarningEntry(&OsKernelVariable);
    }

}
