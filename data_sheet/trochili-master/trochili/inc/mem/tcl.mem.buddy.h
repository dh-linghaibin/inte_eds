/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_MEMORY_BUDDY_H
#define _TCLC_MEMORY_BUDDY_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.memory.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_BUDDY_ENABLE))

#define OS_MEM_BUDDY_PAGE_TAGS  ((TCLC_MEMORY_BUDDY_PAGES + 31U) >> 5U)
#define OS_MEM_BUDDY_NODE_TAGS (TCLC_MEMORY_BUDDY_PAGES * 2U - 1U)

typedef struct MemBuddyDef
{
    TProperty Property;                       /* �ڴ�ҳ������                      */
    TChar*    PageAddr;                       /* ��������ڴ����ʼ��ַ            */
    TBase32   PageSize;                       /* �ڴ�ҳ��С                        */
    TBase32   PageNbr;                        /* �ڴ�ҳ��Ŀ                        */
    TBase32   PageAvail;                      /* �����ڴ�ҳ��Ŀ                    */
    TBitMask  PageTags[OS_MEM_BUDDY_PAGE_TAGS];  /* �ڴ�ҳ�Ƿ���ñ��                */
    TBase32   NodeNbr;
    TByte     NodeTags[OS_MEM_BUDDY_NODE_TAGS];
} TMemBuddy;


extern TState TclCreateMemoryBuddy(TMemBuddy* pBuddy, TChar* pAddr, TBase32 pages, TBase32 pagesize, TError* pError);
extern TState TclDeleteMemoryBuddy(TMemBuddy* pBuddy, TError* pError);
extern TState TclMallocBuddyMemory(TMemBuddy* pBuddy, TBase32 length, void** pAddr2, TError* pError);
extern TState TclFreeBuddyMemory(TMemBuddy* pBuddy,  void* pAddr, TError* pError);

#endif

#endif /* _TCLC_MEMORY_BUDDY_H  */

