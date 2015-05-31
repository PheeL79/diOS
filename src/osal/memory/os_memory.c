/***************************************************************************//**
* @file    os_memory.c
* @brief   OS Memory.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "tlsf.h"
#include "hal.h"
#include "os_common.h"
#include "os_debug.h"
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
        }
    } while (OS_MEM_LAST != (++mem_desc_p)->pool);
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
        } OS_CriticalSectionExit();
	}
}

/******************************************************************************/
Status OS_MemoryInit(void);
Status OS_MemoryInit(void)
{
    vPortMemoryInit();
    os_mem_mutex = OS_MutexCreate();
    if (OS_NULL == os_mem_mutex) { return S_INVALID_PTR; }
    return S_OK;
}

/******************************************************************************/
static OS_MemoryDesc* OS_MemoryDescriptorGet(const OS_MemoryPool pool);
INLINE OS_MemoryDesc* OS_MemoryDescriptorGet(const OS_MemoryPool pool)
{
register OS_MemoryDesc* mem_desc_p = (OS_MemoryDesc*)&memory_cfg_v[0];

    do {
        if (pool == mem_desc_p->pool) {
            return mem_desc_p;
        }
    } while (OS_MEM_LAST != (++mem_desc_p)->pool);
    return OS_NULL;
}

/******************************************************************************/
void* OS_Malloc(const Size size)
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
//    } while (OS_MEM_LAST != (++mem_desc_p)->pool);
    IF_OK(OS_MutexLock(os_mem_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
        p = malloc_ex(size, (void*)mem_desc_p->addr);
        OS_MutexUnlock(os_mem_mutex);
    }
    return p;
}

/******************************************************************************/
void* OS_MallocEx(const Size size, const OS_MemoryPool pool)
{
const OS_MemoryDesc* mem_desc_p = OS_MemoryDescriptorGet(pool);
void* p = OS_NULL;
    if (mem_desc_p) {
        IF_OK(OS_MutexLock(os_mem_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
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
        IF_OK(OS_MutexLock(os_mem_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
            tlsf_free(addr_p);
            OS_MutexUnlock(os_mem_mutex);
        }
    }
}

/******************************************************************************/
void OS_FreeEx(void* addr_p, const OS_MemoryPool pool)
{
const OS_MemoryDesc* mem_desc_p = OS_MemoryDescriptorGet(pool);
    if (mem_desc_p) {
        if (addr_p) {
            IF_OK(OS_MutexLock(os_mem_mutex, OS_TIMEOUT_MUTEX_LOCK)) {
                free_ex(addr_p, (void*)mem_desc_p->addr);
                OS_MutexUnlock(os_mem_mutex);
            }
        }
    } else {
        OS_LOG(D_WARNING, "Unknown memory type!");
    }
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
OS_MemoryPool OS_MemoryPoolNextGet(const OS_MemoryPool pool)
{
OS_MemoryDesc* memory_cfg_p = (OS_MemoryDesc*)&memory_cfg_v;

    if (OS_MEM_UNDEF == pool) {
        //Return first item.
        return memory_cfg_p->pool;
    }
    while (OS_MEM_LAST != memory_cfg_p->pool) {
        if (pool == memory_cfg_p->pool) {
            ++memory_cfg_p;
            if (OS_MEM_LAST == memory_cfg_p->pool) {
                return OS_MEM_UNDEF;
            }
            return memory_cfg_p->pool;
        }
        ++memory_cfg_p;
    }
    return OS_MEM_UNDEF;
}

/******************************************************************************/
Size OS_MemoryFreeGet(const OS_MemoryPool pool)
{
OS_MemoryDesc* memory_cfg_p = (OS_MemoryDesc*)&memory_cfg_v[0];
    while (memory_cfg_p->pool != pool) {
        if (OS_MEM_LAST == memory_cfg_p->pool) {
            return U32_MAX;
        }
        ++memory_cfg_p;
    }
    return (memory_cfg_p->size - get_used_size((void*)memory_cfg_p->addr));
}

/******************************************************************************/
Status OS_MemoryStatsGet(const OS_MemoryPool pool, OS_MemoryStats* mem_stats_p)
{
OS_MemoryDesc* memory_cfg_p = (OS_MemoryDesc*)&memory_cfg_v[0];
    while (memory_cfg_p->pool != pool) {
        if (OS_MEM_LAST == memory_cfg_p->pool) {
            return S_INVALID_ARG;
        }
        ++memory_cfg_p;
    }
    OS_MemCpy((void*)&mem_stats_p->desc, memory_cfg_p, sizeof(OS_MemoryDesc));
    mem_stats_p->used = get_used_size((void*)mem_stats_p->desc.addr);
    mem_stats_p->free = mem_stats_p->desc.size - mem_stats_p->used;
    return S_OK;
}

//------------------------------------------------------------------------------
/// @brief ISR specific functions.

/******************************************************************************/
void* OS_ISR_Malloc(const Size size)
{
static const OS_MemoryDesc* mem_desc_p = (OS_MemoryDesc*)&memory_cfg_v[OS_MEM_HEAP_SYS];
void* p = OS_NULL;
Status s = OS_ISR_MutexLock(os_mem_mutex);

    if ((S_OK == s) || (1 == s)) {
        p = malloc_ex(size, (void*)mem_desc_p->addr);
        OS_ISR_MutexUnlock(os_mem_mutex);
    }
    return p;
}