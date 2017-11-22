/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MUTEX_H
#define _TCL_MUTEX_H

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"
#include "tcl.ipc.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MUTEX_ENABLE))

/* �����ź����ṹ���� */
struct MutexDef
{
    TProperty Property;      /* �������̵߳ĵ��Ȳ��Ե��������� */
    TThread*  Owner;         /* ռ�л����ź������߳�ָ��       */
    TBase32   Nest;          /* �����ź���Ƕ�׼������         */
    TPriority Priority;      /* ceiling value                  */
    TIpcQueue Queue;         /* �����ź������߳���������       */
    TLinkNode LockNode;      /* ������ɻ���������             */
    TObject   Object;
};
typedef struct MutexDef TMutex;

extern TState TclCreateMutex(TMutex* pMutex, TChar* pName, TProperty property, TPriority priority, TError* pError);
extern TState TclDeleteMutex(TMutex* pMutex, TError* pError);
extern TState TclLockMutex(TMutex* pMutex, TOption option, TTimeTick timeo, TError* pError);
extern TState TclFreeMutex(TMutex* pMutex, TError* pError);
extern TState TclResetMutex(TMutex* pMutex, TError* pError);
extern TState TclFlushMutex(TMutex* pMutex, TError* pError);
#endif

#endif /*_TCL_MUTEX_H*/

