/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_OBJECT_H
#define _TCL_OBJECT_H
#include "tcl.types.h"
#include "tcl.config.h"

/* ��������ö�ٶ��� */
enum ObjectTypeDef
{
    OsThreadObject = 0,
    OsTimerObject,
    OsSemaphoreObject,
    OsMutexObject,
    OsMailboxObject,
    OsMessageQueueObject,
    OsFlagObject
};
typedef enum ObjectTypeDef TObjectType;

/* �ں˶���ڵ�ṹ���� */
struct LinkNodeDef
{
    struct LinkNodeDef*  Prev;
    struct LinkNodeDef*  Next;
    struct LinkNodeDef** Handle;
    void*    Owner;
    TBase32* Data;
};
typedef struct LinkNodeDef TLinkNode;

/* �ں˶�������������ʱ�Ľڵ�λ�� */
typedef enum LinkPosDef
{
    OsLinkHead,
    OsLinkTail
} TLinkPos;

/* �ں˶���ṹ���� */
struct ObjectDef
{
    TBase32       ID;                                    /* �ں˶�����     */
    TObjectType   Type;                                  /* �ں˶�������     */
    TChar         Name[TCL_OBJ_NAME_LEN];                /* �ں˶�������     */
    void*         Owner;                                 /* �ں˶�������     */
    TLinkNode     LinkNode;                              /* �ں˶������ӽڵ� */
};
typedef struct ObjectDef TObject;

extern void OsObjQueueAddFifoNode(TLinkNode** pHandle2, TLinkNode* pNode, TLinkPos pos);
extern void OsObjQueueAddPriorityNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjQueueRemoveNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjListAddNode(TLinkNode** pHandle2, TLinkNode* pNode, TLinkPos pos);
extern void OsObjListRemoveNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjListAddPriorityNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjListAddDiffNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjListRemoveDiffNode(TLinkNode** pHandle2, TLinkNode* pNode);

#endif /* _TCL_OBJECT_H */

