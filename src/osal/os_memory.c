/***************************************************************************//**
* @file    os_memory.c
* @brief   OS Memory.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "tlsf.h"
#include "hal.h"
#include "os_common.h"
#include "os_supervise.h"
#include "os_mutex.h"
#include "os_memory.h"

//------------------------------------------------------------------------------
static OS_MutexHd os_mem_mutex;

/******************************************************************************/
void vPortMemoryInit(void);
void vPortMemoryInit(void)
{
    OS_MemoryDesc* mem_desc_p = (OS_MemoryDesc*)&memory_cfg_v[0];
    do {
        if ((OS_MemoryDesc*)&memory_cfg_v[OS_MEM_HEAP_SYS] != mem_desc_p) { //init system pool last!
            init_memory_pool(mem_desc_p->size, (void*)mem_desc_p->addr);
            //OS_LOG(D_DEBUG, "heap: %5d; tlsf: %d\r\n", get_used_size((void*)mem_desc_p->addr), get_max_size((void*)mem_desc_p->addr));
        }
    } while (OS_MEM_LAST != (++mem_desc_p)->type);
    // Init system mempool
    mem_desc_p = (OS_MemoryDesc*)&memory_cfg_v[OS_MEM_HEAP_SYS];
    init_memory_pool(mem_desc_p->size, (void*)mem_desc_p->addr);
}

/******************************************************************************/
void* pvPortMalloc(size_t xWantedSize);
void* pvPortMalloc(size_t xWantedSize)
{
void* pvReturn;
    OS_CriticalSectionEnter(); {
	    pvReturn = tlsf_malloc(xWantedSize);
        //OS_LOG(D_DEBUG, "heap: %5d; malloc: 0x%X, %d\r\n", get_used_size(&heap), pvReturn, xWantedSize);
    } OS_CriticalSectionExit();
	return pvReturn;
}

/******************************************************************************/
void vPortFree(void* pv);
void vPortFree(void* pv)
{
	if (pv)	{
        OS_CriticalSectionEnter(); {
		    tlsf_free(pv);
            //OS_LOG(D_DEBUG, "heap: %5d; free  : 0x%X\r\n", get_used_size(&heap), pv);
        } OS_CriticalSectionExit();
	}
}

/******************************************************************************/
Status OS_MemoryInit(void);
Status OS_MemoryInit(void)
{
    vPortMemoryInit();
    os_mem_mutex = OS_MutexCreate();
    if (OS_NULL == os_mem_mutex) { return S_INVALID_REF; }
    return S_OK;
}

/******************************************************************************/
#pragma inline
static OS_MemoryDesc* OS_MemoryDescriptorGet(const OS_MemoryType mem_type);
OS_MemoryDesc* OS_MemoryDescriptorGet(const OS_MemoryType mem_type)
{
register OS_MemoryDesc* mem_desc_p = (OS_MemoryDesc*)&memory_cfg_v[0];

    do {
        if (mem_type == mem_desc_p->type) {
            return mem_desc_p;
        }
    } while (OS_MEM_LAST != (++mem_desc_p)->type);
    return OS_NULL;
}

/******************************************************************************/
void* OS_Malloc(const U32 size)
{
static const OS_MemoryDesc* mem_desc_p = (OS_MemoryDesc*)&memory_cfg_v[OS_MEM_HEAP_SYS];
void* p = OS_NULL;
//    // Trying to allocate wanted amount of memory in each descriptor.
//    do {
//        p = malloc_ex(size, (void*)mem_desc_p->addr);
//        // if memory successfully alocated...
//        if (OS_NULL != p) {
//            // ...return pointer;
//            break;
//        }
//        // ...otherwise - next descriptor.
//    } while (OS_MEM_LAST != (++mem_desc_p)->type);
    IF_STATUS_OK(OS_MutexLock(os_mem_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        p = malloc_ex(size, (void*)mem_desc_p->addr);
        OS_MutexUnlock(os_mem_mutex);
    }
    return p;
}

/******************************************************************************/
void* OS_MallocEx(const U32 size, const OS_MemoryType mem_type)
{
const OS_MemoryDesc* mem_desc_p = OS_MemoryDescriptorGet(mem_type);
void* p = OS_NULL;
    if (mem_desc_p) {
        IF_STATUS_OK(OS_MutexLock(os_mem_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
            p = malloc_ex(size, (void*)mem_desc_p->addr);
            OS_MutexUnlock(os_mem_mutex);
        }
    }
    return p;
}

/******************************************************************************/
//void* OS_MPU_Malloc(const TaskId id, const MemoryRegion mem_regions);
//{
//}

/******************************************************************************/
void OS_Free(void* addr_p)
{
    if (addr_p) {
        IF_STATUS_OK(OS_MutexLock(os_mem_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
            tlsf_free(addr_p);
            OS_MutexUnlock(os_mem_mutex);
        }
    }
}

/******************************************************************************/
void OS_FreeEx(void* addr_p, const OS_MemoryType mem_type)
{
const OS_MemoryDesc* mem_desc_p = OS_MemoryDescriptorGet(mem_type);
    if (mem_desc_p) {
        if (addr_p) {
            IF_STATUS_OK(OS_MutexLock(os_mem_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
                free_ex(addr_p, (void*)mem_desc_p->addr);
                OS_MutexUnlock(os_mem_mutex);
            }
        }
    } else {
        //OS_LOG(D_DEBUG, "ERROR: Unknown memory type!");
    }
}

/******************************************************************************/
void OS_MemCacheFlush(void)
{
#if defined(CM4F)
    __DMB();    //Memory barrier.
	__DSB();    //Data barrier.
	__ISB();    //Instructions barrier.
#endif // CM4F
}

///******************************************************************************/
//void OS_MemCpy(void* dst_p, const void* src_p, SIZE size)
//{
//    memcpy(dst_p, src_p, size);
//    //DMA_MemCpy8(dst_p, src_p, size8);
//}
//
///******************************************************************************/
//void OS_MemCpy32(void* dst_p, const void* src_p, SIZE size32)
//{
//    memcpy(dst_p, src_p, (size32 * sizeof(U32)));
//    //DMA_MemCpy32(dst_p, src_p, size32);
//}
//
///******************************************************************************/
//void OS_MemMove(void* dst_p, const void* src_p, SIZE size)
//{
//    memmove(dst_p, src_p, size);
//}
//
///******************************************************************************/
//void OS_MemMove32(void* dst_p, const void* src_p, SIZE size32)
//{
//    memmove(dst_p, src_p, (size32 * sizeof(U32)));
//}
//
///******************************************************************************/
//void OS_MemSet(void* dst_p, const U8 value, SIZE size)
//{
//    memset(dst_p, value, size);
//}

/******************************************************************************/
OS_MemoryType OS_MemoryTypeHeapNextGet(const OS_MemoryType mem_type)
{
OS_MemoryDesc* memory_cfg_p = (OS_MemoryDesc*)&memory_cfg_v;

    if (OS_MEM_UNDEF == mem_type) {
        //Return first item.
        return memory_cfg_p->type;
    }
    while (OS_MEM_LAST != memory_cfg_p->type) {
        if (mem_type == memory_cfg_p->type) {
            ++memory_cfg_p;
            if (OS_MEM_LAST == memory_cfg_p->type) {
                return OS_MEM_UNDEF;
            }
            return memory_cfg_p->type;
        }
        ++memory_cfg_p;
    }
    return OS_MEM_UNDEF;
}

/******************************************************************************/
Status OS_MemoryStatGet(const OS_MemoryType mem_type, OS_MemoryStat* mem_stat_p)
{
OS_MemoryDesc* memory_cfg_p = (OS_MemoryDesc*)&memory_cfg_v[0];
    while (memory_cfg_p->type != mem_type) {
        if (OS_MEM_LAST == memory_cfg_p->type) {
            return S_UNDEF_PARAMETER;
        }
        ++memory_cfg_p;
    }
    OS_MEMCPY((void*)&mem_stat_p->desc, memory_cfg_p, sizeof(OS_MemoryDesc));
    mem_stat_p->used = get_used_size((void*)mem_stat_p->desc.addr);
    mem_stat_p->free = mem_stat_p->desc.size - mem_stat_p->used;
    return S_OK;
}