/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.debug.h"
#include "tcl.object.h"

/*************************************************************************************************
 *  ���ܣ����������ȳ����򽫽ڵ���뵽ָ����˫��ѭ������                                         *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *        (3) pos      ���׶�β���                                                              *
 *  ���أ���                                                                                     *
 *  ˵������                                                                                     *
 *************************************************************************************************/
void OsObjQueueAddFifoNode(TLinkNode** pHandle2, TLinkNode* pNode, TLinkPos pos)
{
    OS_ASSERT((pNode->Handle == (TLinkNode**)0), "");

    if (*pHandle2)
    {
        /* ������в��գ���ѽڵ�׷���ڶ�β */
        pNode->Prev = (*pHandle2)->Prev;
        pNode->Prev->Next = pNode;
        pNode->Next = *pHandle2;
        pNode->Next->Prev = pNode;
        if (pos == OsLinkHead)
        {
            (*pHandle2) = (*pHandle2)->Prev;
        }
    }
    else
    {
        /* �����ʼ����һ���ڵ��ͷ�ڵ�ָ�� */
        *pHandle2 = pNode;
        pNode->Prev = pNode;
        pNode->Next = pNode;
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  ���ܣ����������ȼ�������򽫽ڵ���뵽ָ����˫��ѭ������                                     *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsObjQueueAddPriorityNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    TLinkNode* pTemp = (TLinkNode*)0;

    OS_ASSERT((pNode->Handle == (TLinkNode**)0), "");

    /* �������Ƿ�Ϊ�� */
    if (*pHandle2)
    {
        pTemp = (*pHandle2);

        /* �½ڵ��ͷ������ȼ�����(��ʱ��Ҫ����headָ��)  */
        if (*(pTemp->Data) > *(pNode->Data))
        {
            (*pHandle2) = pNode;
        }
        else
        {
            /*
             * �ڶ����������½ڵ��λ�ã������½ڵ�����ȼ������нڵ�����ȼ�����
             * (��ʱ����Ҫ����headָ��)
             */
            pTemp = pTemp->Next;
            while ((*(pTemp->Data) <= *(pNode->Data)) && (pTemp != (*pHandle2)))
            {
                pTemp = pTemp->Next;
            }
        }

        /* �����½ڵ㵽���� */
        pNode->Prev = pTemp->Prev;
        pNode->Prev->Next = pNode;
        pNode->Next = pTemp;
        pNode->Next->Prev = pNode;
    }
    else
    {
        (*pHandle2) = pNode;
        pNode->Prev = pNode;
        pNode->Next = pNode;
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  ���ܣ����ڵ��ָ����˫��ѭ���������Ƴ�                                                       *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *        (3) pos      ���׶�β���                                                              *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsObjQueueRemoveNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    OS_ASSERT((pNode->Handle == pHandle2), "");

    /* ����Ƿ������ֻ��һ���ڵ㣬����ǾͰ�ͷ���ָ����� */
    if (pNode->Prev == pNode)
    {
        *pHandle2 = (TLinkNode*)0;
    }
    else
    {
        /* ��������в�ֹһ���ڵ㣬��ѵ�ǰ�ڵ�ɾ�� */
        pNode->Prev->Next = pNode->Next;
        pNode->Next->Prev = pNode->Prev;

        /* ������ڵ���ͷ��㣬�����ͷ���ָ�룬�������� */
        if (pNode == *pHandle2)
        {
            *pHandle2 = pNode->Next;
        }
    }

    /* ��սڵ�ǰ���������Ϣ */
    pNode->Next = (TLinkNode*)0;
    pNode->Prev = (TLinkNode*)0;
    pNode->Handle = (TLinkNode**)0;
}


/*************************************************************************************************
 *  ���ܣ����ڵ���뵽˫�������ָ��λ��                                                         *
 *  ��Χ��ȫ��                                                                                   *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsObjListAddNode(TLinkNode** pHandle2, TLinkNode* pNode, TLinkPos pos)
{
    TLinkNode* pTail;
    OS_ASSERT((pNode->Handle == (TLinkNode**)0), "");

    /* ����������и��ڵ� */
    if (*pHandle2)
    {
        if (pos == OsLinkHead)
        {
            pNode->Next = *pHandle2;
            pNode->Prev = (TLinkNode*)0;
            (*pHandle2)->Prev = pNode;
            *pHandle2 = pNode;
        }
        else
        {
            pTail= *pHandle2;
            while(pTail->Next)
            {
                pTail = pTail->Next;
            }

            pNode->Next = (TLinkNode*)0;
            pNode->Prev = pTail;
            pTail->Next = pNode;
        }
    }
    /* ���������û�нڵ� */
    else
    {
        *pHandle2 = pNode;
        pNode->Next = (TLinkNode*)0;
        pNode->Prev = (TLinkNode*)0;
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  ���ܣ����������ȼ�������򽫽ڵ���뵽ָ����˫������                                         *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsObjListAddPriorityNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    TLinkNode* pCursor = (TLinkNode*)0;
    TLinkNode* pTail = (TLinkNode*)0;

    /* �������Ϊ�գ����½ڵ���Ϊͷ��� */
    if ((*pHandle2) == (TLinkNode*)0)
    {
        *pHandle2 = pNode;
        pNode->Next = (TLinkNode*)0;
        pNode->Prev = (TLinkNode*)0;
    }
    else
    {
        /* ����������У����Һ���λ�� */
        pCursor = *pHandle2;
        while (pCursor != (TLinkNode*)0)
        {
            pTail = pCursor;
            if (*(pNode->Data) >= *(pCursor->Data))
            {
                pCursor = pCursor->Next;
            }
            else
            {
                break;
            }
        }

        /* �α겻Ϊ��˵�����������ҵ����ʵĽڵ㣬��Ҫ���½ڵ���뵽�ýڵ�֮ǰ */
        if (pCursor != (TLinkNode*)0)
        {
            /* ���������ͷ�ڵ� */
            if (pCursor->Prev == (TLinkNode*)0)
            {
                *pHandle2 = pNode;
                pCursor->Prev = pNode;
                pNode->Prev   = (TLinkNode*)0;
                pNode->Next    = pCursor;
            }
            /* ��������������ڵ�(�����м�Ľڵ����β�ڵ�) */
            else
            {
                pNode->Prev         = pCursor->Prev;
                pNode->Next         = pCursor;
                pCursor->Prev->Next = pNode;
                pCursor->Prev       = pNode;
            }
        }
        /* ����α�Ϊ��˵��û���ҵ����ʵĽڵ㣬����ֻ�ܰ��½ڵ����������� */
        else
        {
            pTail->Next = pNode;
            pNode->Prev = pTail;
            pNode->Next = (TLinkNode*)0;
        }
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  ���ܣ����ڵ��ָ����˫���������Ƴ�                                                           *
 *  ��Χ��ȫ��                                                                                   *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsObjListRemoveNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    OS_ASSERT((pNode->Handle == pHandle2), "");

    /* ���������ֻ��һ���ڵ� */
    if ((pNode->Prev == (TLinkNode*)0) && (pNode->Next == (TLinkNode*)0))
    {
        *pHandle2 = (TLinkNode*)0;
    }
    /* ����������ж���ڵ㲢����Ҫɾ������β���Ľڵ� */
    else if ((pNode->Prev != (TLinkNode*)0) && (pNode->Next == (TLinkNode*)0))
    {
        pNode->Prev->Next = (TLinkNode*)0;
    }
    /* ����������ж���ڵ㲢����Ҫɾ������ͷ���Ľڵ� */
    else if ((pNode->Prev == (TLinkNode*)0) && (pNode->Next != (TLinkNode*)0))
    {
        pNode->Next->Prev = (TLinkNode*)0;
        *pHandle2 = pNode->Next;
    }
    /* ����������ж���ڵ㲢����Ҫɾ�������в��Ľڵ� */
    else
    {
        pNode->Prev->Next = pNode->Next;
        pNode->Next->Prev = pNode->Prev;
    }

    /* ���ñ�ɾ���ڵ������ */
    pNode->Next = (TLinkNode*)0;
    pNode->Prev = (TLinkNode*)0;
    pNode->Handle = (TLinkNode**)0;
}


/*************************************************************************************************
 *  ���ܣ����ڵ���뵽ָ����˫��������                                                         *
 *  ��Χ��ȫ��                                                                                   *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����˫���ѭ������                                                                         *
 *************************************************************************************************/
void OsObjListAddDiffNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    TLinkNode* pCursor = (TLinkNode*)0;
    TLinkNode* pTail = (TLinkNode*)0;
    OS_ASSERT((pNode->Handle == (TLinkNode**)0), "");

    /* ����������Ϊ�գ����½ڵ���Ϊͷ��� */
    if ((*pHandle2) == (TLinkNode*)0)
    {
        (*pHandle2) = pNode;
        pNode->Next = (TLinkNode*)0;
        pNode->Prev = (TLinkNode*)0;
    }
    /* ����������У����Һ���λ�� */
    else
    {
        pCursor = *pHandle2;
        while (pCursor != (TLinkNode*)0)
        {
            pTail = pCursor;
            if (*(pNode->Data) >= *(pCursor->Data))
            {
                *(pNode->Data) -= *(pCursor->Data);
                pCursor = pCursor->Next;
            }
            else
            {
                *(pCursor->Data) -= *(pNode->Data);
                break;
            }
        }

        /* �α겻Ϊ��˵�����������ҵ����ʵĽڵ㣬��Ҫ���½ڵ���뵽�ýڵ�֮ǰ */
        if (pCursor != (TLinkNode*)0)
        {
            /* ���������ͷ�ڵ� */
            if (pCursor->Prev == (TLinkNode*)0)
            {
                *pHandle2 = pNode;
                pCursor->Prev = pNode;
                pNode->Prev = (TLinkNode*)0;
                pNode->Next = pCursor;
            }
            /* ��������������ڵ�(�����м�Ľڵ����β�ڵ�) */
            else
            {
                pNode->Prev = pCursor->Prev;
                pNode->Next = pCursor;
                pCursor->Prev->Next = pNode;
                pCursor->Prev = pNode;
            }
        }
        /* ����α�Ϊ��˵��û���ҵ����ʵĽڵ㣬����ֻ�ܰ��½ڵ����������� */
        else
        {
            pTail->Next = pNode;
            pNode->Prev = pTail;
            pNode->Next = (TLinkNode*)0;
        }
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  ���ܣ����ڵ��ָ����˫�����������Ƴ�                                                       *
 *  ��Χ��ȫ��                                                                                   *
 *  ������(1) pHandle2 ָ������ͷָ���ָ��                                                      *
 *        (2) pNode    �ڵ�ָ��                                                                  *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void OsObjListRemoveDiffNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    OS_ASSERT((pNode->Handle == pHandle2), "");

    /* ������������ֻ��һ���ڵ� */
    if ((pNode->Next == (TLinkNode*)0) && (pNode->Prev == (TLinkNode*)0))
    {
        *pHandle2 = (TLinkNode*)0;
    }
    /* ���ɾ�����������β�ڵ� */
    else if (pNode->Next == (TLinkNode*)0)
    {
        pNode->Prev->Next = (TLinkNode*)0;
    }
    /* ���ɾ�����������ͷ�ڵ� */
    else if (pNode->Prev == (TLinkNode*)0)
    {
        *(pNode->Next->Data) += *(pNode->Data);
        *pHandle2 = pNode->Next;
        pNode->Next->Prev = (TLinkNode*)0;
    }
    /* ���ɾ�����������м�Ľڵ� */
    else
    {
        *(pNode->Next->Data) += *(pNode->Data);
        pNode->Next->Prev = pNode->Prev;
        pNode->Prev->Next = pNode->Next;
    }

    /* �ýڵ��������ɾ�� */
    pNode->Prev = (TLinkNode*)0;
    pNode->Next = (TLinkNode*)0;
    pNode->Handle = (TLinkNode**)0;
}

