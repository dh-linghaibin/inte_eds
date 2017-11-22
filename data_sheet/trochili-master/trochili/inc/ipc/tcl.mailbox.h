/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MAILBOX_H
#define _TCL_MAILBOX_H

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"
#include "tcl.ipc.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MAILBOX_ENABLE))

/* ����״̬���� */
typedef enum
{
    OsMailboxEmpty,                  /* �������ݿ�              */
    OsMailboxFull                    /* ����������              */
} TMailboxStatus;

/* �ʼ��ṹ���� */
typedef void* TMail;

/* ����ṹ���� */
struct MailboxDef
{
    TProperty      Property;         /* �̵߳ĵ��Ȳ��Ե��������� */
    TMail          Mail;             /* ������ʼ�����           */
    TMailboxStatus Status;           /* �����״̬               */
    TIpcQueue      Queue;            /* ������߳���������       */
    TObject        Object;
};
typedef struct MailboxDef TMailbox;


extern TState TclCreateMailbox(TMailbox* pMailbox, TChar* pName, TProperty property, TError* pError);
extern TState TclDeleteMailbox(TMailbox* pMailbox, TError* pError);
extern TState TclReceiveMail(TMailbox* pMailbox, TMail* pMail2, TOption option,
                             TTimeTick timeo, TError* pError);
extern TState TclSendMail(TMailbox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                          TError* pError);
extern TState TclIsrSendMail(TMailbox* pMailbox, TMail* pMail2, TOption option, TError* pError);
extern TState TclBroadcastMail(TMailbox* pMailbox, TMail* pMail2, TError* pError);
extern TState TclResetMailbox(TMailbox* pMailbox, TError* pError);
extern TState TclFlushMailbox(TMailbox* pMailbox, TError* pError);

#endif

#endif /* _TOCHILI_MAILBOX_H */

