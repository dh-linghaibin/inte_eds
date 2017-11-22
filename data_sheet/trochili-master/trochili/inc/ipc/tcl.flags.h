/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_FLAGS_H
#define _TCL_FLAGS_H

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"
#include "tcl.ipc.h"

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))

/* �¼���ǽṹ���� */
struct FlagsDef
{
    TProperty Property;  /* �̵߳ĵ��Ȳ��Ե���������   */
    TBitMask  Value;     /* �¼���ǵĵ�ǰ�¼���       */
    TIpcQueue Queue;     /* �¼���ǵ��߳���������     */
    TObject   Object;
};
typedef struct FlagsDef TFlags;


extern TState TclCreateFlags(TFlags* pFlags, TChar* pName, TProperty property, TError* pError);
extern TState TclDeleteFlags(TFlags* pFlags, TError* pError);
extern TState TclSendFlags(TFlags* pFlags, TBitMask pattern, TError* pError);
extern TState TclReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option,
                              TTimeTick timeo, TError* pError);
extern TState TclResetFlags(TFlags* pFlags, TError* pError);
extern TState TclFlushFlags(TFlags* pFlags,  TError* pError);
#endif

#endif /* _TCL_FLAGS_H */

