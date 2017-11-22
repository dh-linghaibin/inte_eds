/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_MEMORY_POOL_H
#define _TCLC_MEMORY_POOL_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.memory.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_POOL_ENABLE))

#define OS_MEM_PAGE_TAGS  ((TCLC_MEMORY_POOL_PAGES + 31U) >> 5u)

/* �ڴ�ؿ��ƿ�ṹ */
struct MemPoolDef
{
    TProperty Property;                   /* �ڴ�ҳ������                      */
    TChar*    PageAddr;                   /* ��������ڴ����ʼ��ַ            */
    TBase32   PageSize;                   /* �ڴ�ҳ��С                        */
    TBase32   PageNbr;                    /* �ڴ�ҳ��Ŀ                        */
    TBase32   PageAvail;                  /* �����ڴ�ҳ��Ŀ                    */
    TBase32   PageTags[OS_MEM_PAGE_TAGS];    /* �ڴ�ҳ�Ƿ���ñ��                */
    TLinkNode* PageList;                   /* �����ڴ�ҳ����ͷָ��              */
};
typedef struct MemPoolDef TMemPool;


extern TState TclCreateMemoryPool(TMemPool* pPool, void* pAddr, TBase32 pages, TBase32 pgsize, TError* pError);
extern TState TclDeleteMemoryPool(TMemPool* pPool, TError* pError);
extern TState TclMallocPoolMemory (TMemPool* pPool, void** pAddr2, TError* pError);
extern TState TclFreePoolMemory (TMemPool* pPool, void* pAddr, TError* pError);
#endif

#endif /* _TCLC_MEMORY_POOL_H  */

