/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_SEMAPHORE_H
#define _TCL_SEMAPHORE_H

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"
#include "tcl.ipc.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_SEMAPHORE_ENABLE))

/* �����ź�������ֵ�ṹ���� */
struct SemaphoreDef
{
    TProperty Property;         /* �������̵߳ĵ��Ȳ��Ե���������     */
    TBase32   Value;            /* �����ź����ĵ�ǰ��ֵ               */
    TBase32   LimitedValue;     /* �����ź����������ֵ               */
    TBase32   InitialValue;     /* �����ź����ĳ�ʼ��ֵ               */
    TIpcQueue Queue;            /* �ź������߳���������               */
    TObject   Object;
};
typedef struct SemaphoreDef TSemaphore;

extern TState TclCreateSemaphore(TSemaphore* pSemaphore, TChar* pName,
                                 TBase32 value, TBase32 mvalue,
                                 TProperty property, TError* pError);
extern TState TclDeleteSemaphore(TSemaphore* pSemaphore, TError* pError);
extern TState TclResetSemaphore(TSemaphore* pSemaphore, TError* pError);
extern TState TclFlushSemaphore(TSemaphore* pSemaphore, TError* pError);
extern TState TclObtainSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo,
                                 TError* pError);
extern TState TclReleaseSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo,
                                  TError* pError);
extern TState TclIsrReleaseSemaphore(TSemaphore* pSemaphore, TError* pError);

#endif

#endif /*_TCL_SEMAPHORE_H*/

