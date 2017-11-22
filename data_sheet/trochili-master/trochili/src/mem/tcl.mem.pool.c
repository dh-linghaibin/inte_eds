/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "string.h"

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.debug.h"
#include "tcl.cpu.h"
#include "tcl.mem.pool.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_POOL_ENABLE))
/*************************************************************************************************
 *  ����: ��ʼ���ڴ�ҳ��                                                                         *
 *  ����: (1) pPool      �ڴ�ҳ�ؽṹ��ַ                                                        *
 *        (2) pAddr      �ڴ����������ַ                                                        *
 *        (3) pages      �ڴ�����ڴ�ҳ��Ŀ                                                      *
 *        (4) pgsize     �ڴ�ҳ��С                                                              *
 *        (5) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclCreateMemoryPool(TMemPool* pPool, void* pAddr, TBase32 pages, TBase32 pgsize, TError* pError)
{
    TState state = eFailure;
    TError error = OS_MEM_ERR_FAULT;
    TReg32 imask;
    TIndex index;
    TChar* pTemp;

    OS_ASSERT((pPool != (TMemPool*)0), "");
    OS_ASSERT((pAddr != (void*)0), "");
    OS_ASSERT((pages != 0U), "");
    OS_ASSERT((pgsize > 0U), "");
    OS_ASSERT((pages <= TCLC_MEMORY_POOL_PAGES), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (!(pPool->Property & OS_MEM_PROP_READY))
    {
        /* ��ձ�������ڴ�ռ� */
        memset(pAddr, 0U, pages * pgsize);

        /* ���������ڴ�ҳ���� */
        pTemp = (TChar*)pAddr;
        for (index = 0; index < pages; index++)
        {
            OsObjListAddNode(&(pPool->PageList), (TLinkNode*)pTemp, OsLinkTail);
            pTemp += pgsize;
        }

        /* ���������ڴ涼���ڿɷ���״̬ */
        for (index = 0; index < OS_MEM_PAGE_TAGS; index++)
        {
            pPool->PageTags[index] = ~0U;
        }
        pPool->PageAddr  = pAddr;
        pPool->PageAvail = pages;
        pPool->PageNbr   = pages;
        pPool->PageSize  = pgsize;
        pPool->Property  = OS_MEM_PROP_READY;

        error = OS_MEM_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: �����ڴ��                                                                             *
 *  ����: (1) pPool      �ڴ�ؽṹ��ַ                                                          *
 *        (2) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclDeleteMemoryPool(TMemPool* pPool, TError* pError)
{
    TReg32 imask;
    TState state = eFailure;
    TError error = OS_MEM_ERR_UNREADY;

    OS_ASSERT((pPool != (TMemPool*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);
    if (pPool->Property & OS_MEM_PROP_READY)
    {
        memset(pPool->PageAddr, 0U, pPool->PageNbr * pPool->PageSize);
        memset(pPool, 0U, sizeof(TMemPool));
        error = OS_MEM_ERR_NONE;
        state = eSuccess;
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ���ڴ������������ڴ�                                                                 *
 *  ����: (1) pPool      �ڴ�ؽṹ��ַ                                                          *
 *        (2) pAddr2     �������뵽���ڴ��ָ�����                                              *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclMallocPoolMemory(TMemPool* pPool, void** pAddr2, TError* pError)
{
    TState state = eFailure;
    TError error = OS_MEM_ERR_UNREADY;
    TReg32 imask;
    TIndex x;
    TIndex y;
    TIndex index;
    TChar* pTemp;

    OS_ASSERT((pPool != (TMemPool*)0), "");
    OS_ASSERT((pAddr2 != (void**)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pPool->Property & OS_MEM_PROP_READY)
    {
        /* ����ڴ�ش��ڿ����ڴ�ҳ */
        if (pPool->PageAvail > 0U)
        {
            /* �����ڴ�ҳ�����ȥ */
            pTemp = (TChar*)(pPool->PageList);
            OsObjListRemoveNode(&(pPool->PageList), (TLinkNode*)pTemp);
            pPool->PageAvail--;
            *pAddr2 = (void*)pTemp;

            /* ��Ǹ��ڴ�ҳ�Ѿ������� */
            index = (pTemp - pPool->PageAddr)/(pPool->PageSize);
            y = (index >> 5U);
            x = (index & 0x1f);
            pPool->PageTags[y]  &= ~(0x1 << x);

            /* ��ո��ڴ�ҳ���� */
            memset((void*)pTemp, 0U, pPool->PageSize);

            error = OS_MEM_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_MEM_ERR_NO_MEM;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  ����: ���ڴ�����ͷ��ڴ�                                                                     *
 *  ����: (1) pPool      �ڴ�ؽṹ��ַ                                                          *
 *        (2) pAddr      ���ͷ��ڴ�ĵ�ַ                                                        *
 *        (3) pError     ��ϸ���ý��                                                            *
 *  ����: (1) eSuccess   �����ɹ�                                                                *
 *        (2) eFailure   ����ʧ��                                                                *
 *  ˵����                                                                                       *
 *************************************************************************************************/
TState TclFreePoolMemory (TMemPool* pPool, void* pAddr, TError* pError)
{
    TState state = eFailure;
    TError error = OS_MEM_ERR_UNREADY;
    TReg32 imask;
    TIndex index;
    TChar* pTemp;
    TBase32 x;
    TBase32 y;
    TBase32 tag;

    OS_ASSERT((pPool != (TMemPool*)0), "");
    OS_ASSERT((pAddr != (void*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pPool->Property & OS_MEM_PROP_READY)
    {
        /* ����ڴ��ȷʵ���ڴ�ҳ�������ȥ��δ���� */
        if (pPool->PageAvail < pPool->PageNbr)
        {
            /* ����ͷŵ��ڴ��ַ�Ƿ���Ĵ��ں��ʵĿ���ʼ��ַ�ϡ�
               �˴�����Ҫ�󱻹�����ڴ�ռ������������ */
            index = ((TChar*)pAddr - pPool->PageAddr) / (pPool->PageSize);
            //  index = (index < pPool->PageNbr) ? index : pPool->PageNbr;
            pTemp = pPool->PageAddr + index * pPool->PageSize;

            /* ����õ�ַ����������ȷʵ�Ǵ���ĳ���ڴ�ҳ���׵�ַ */
            if (pTemp == (TChar*)pAddr)
            {
                /* ����ڴ�ҳ�����ǣ������ٴ��ͷ��Ѿ��ͷŹ����ڴ�ҳ��ַ */
                y = (index >> 5U);
                x = (index & 0x1f);
                tag = pPool->PageTags[y] & (0x1 << x);
                if (tag == 0U)
                {
                    /* ��ո��ڴ�ҳ */
                    memset(pAddr, 0U, pPool->PageSize);

                    /* �ջظõ�ַ���ڴ�ҳ */
                    OsObjListAddNode(&(pPool->PageList), (TLinkNode*)pAddr, OsLinkTail);
                    pPool->PageAvail++;

                    /* ��Ǹ��ڴ�ҳ���Ա����� */
                    pPool->PageTags[y] |= (0x1 << x);

                    error = OS_MEM_ERR_NONE;
                    state = eSuccess;
                }
                else
                {
                    error = OS_MEM_ERR_DBL_FREE;
                }
            }
            else
            {
                error = OS_MEM_ERR_BAD_ADDR;
            }
        }
        else
        {
            error = OS_MEM_ERR_POOL_FULL;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#endif

