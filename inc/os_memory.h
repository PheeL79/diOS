/***************************************************************************//**
* @file    os_memory.h
* @brief   OS Memory.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_MEMORY_H_
#define _OS_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "typedefs.h"

/**
* \defgroup OS_Memory OS_Memory
* @{
*/
//------------------------------------------------------------------------------
typedef U32 OS_MemoryPool;

/// @brief   Memory description.
typedef struct {
    void*           addr;
    Size            size;
    Size            block_size;
    OS_MemoryPool   pool;
    ConstStrP       name_p;
} OS_MemoryDesc;

/// @brief   Memory statistics.
typedef struct {
    OS_MemoryDesc   desc;
    Size            used;
    Size            free;
} OS_MemoryStats;

//------------------------------------------------------------------------------
/// @brief      Common functions.

/// @brief      Allocate memory.
/// @param[in]  size            Allocation size (in bytes).
/// @return     Memory pointer.
/// @details    Tries to allocate memory in first memory pool of config.
void*           OS_Malloc(const Size size);

/// @brief      Allocate memory by pool.
/// @param[in]  size            Allocation size (in bytes).
/// @param[in]  pool            Memory pool.
/// @return     Pointer.
void*           OS_MallocEx(const Size size, const OS_MemoryPool pool);

/// @brief      Free allocated memory.
/// @param[in]  addr_p          Memory address.
/// @return     None.
void            OS_Free(void* addr_p);

/// @brief      Free allocated memory by pool.
/// @param[in]  addr_p          Memory address.
/// @param[in]  pool            Memory pool.
/// @return     None.
void            OS_FreeEx(void* addr_p, const OS_MemoryPool pool);

/// @brief      Flush memory caches.
/// @return     None.
#if defined(CM4F)
#define         OS_MemCacheFlush()      do {\
                                                __DMB();\
                                                __DSB();\
                                                __ISB();\
                                        } while (0)
#else
                                        void()
#endif // CM4F

///// @brief      Copy memory in bytes.
///// @param[out] dst_p           Destination address.
///// @param[in]  src_p           Source address.
///// @param[in]  size8           Size to copy (bytes).
///// @return     None.
//void            OS_MemCpy(void* dst_p, const void* src_p, Size size);
//
///// @brief      Copy memory in words.
///// @param[out] dst_p           Destination address.
///// @param[in]  src_p           Source address.
///// @param[in]  size32          Size to copy (words).
///// @return     None.
//void            OS_MemCpy32(void* dst_p, const void* src_p, Size size32);
//
///// @brief      Copy memory in bytes.
///// @param[out] dst_p           Destination address.
///// @param[in]  src_p           Source address.
///// @param[in]  size8           Size to copy (bytes).
///// @return     None.
//void            OS_MemMove(void* dst_p, const void* src_p, Size size);
//
///// @brief      Copy memory in words.
///// @param[out] dst_p           Destination address.
///// @param[in]  src_p           Source address.
///// @param[in]  size32          Size to copy (words).
///// @return     None.
//void            OS_MemMove32(void* dst_p, const void* src_p, Size size32);
//
///// @brief      Set memory by value.
///// @param[out] dst_p           Destination address.
///// @param[in]  value           Value.
///// @param[in]  size            Size to set(bytes).
///// @return     None.
//void            OS_MemSet(void* dst_p, const U8 value, Size size);

/// @brief      Get the next memory pool.
/// @param[in]  pool            Memory pool.
/// @return     Memory pool.
OS_MemoryPool   OS_MemoryPoolNextGet(const OS_MemoryPool pool);

/// @brief      Get the memory pool free size.
/// @param[in]  pool            Memory pool.
/// @return     #Size.
Size            OS_MemoryFreeGet(const OS_MemoryPool pool);

/// @brief      Get the memory pool usage statistics.
/// @param[in]  pool            Memory pool.
/// @param[out] mem_stats_p     Memory statistics.
/// @return     #Status.
Status          OS_MemoryStatsGet(const OS_MemoryPool pool, OS_MemoryStats* mem_stats_p);

//------------------------------------------------------------------------------
#ifdef USE_MPU
/**
* \addtogroup OS_MPU_Memory MPU specific types and functions.
* @{
*/
//#include "os_task.h"

//typedef MemoryRegion_t MemoryRegion;

/// @brief      Allocate memory with protection.
/// @param[in]  tid         Task id.
/// @param[in]  mem_regions Memory regions.
/// @return     Memory pointer.
//void*         OS_MPU_Malloc(const OS_TaskId tid, const MemoryRegion mem_regions);

/**@}*/ //OS_MPU_Memory

#endif // USE_MPU

/**
* \addtogroup OS_ISR_Memory ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Allocate memory.
/// @param[in]  size            Allocation size (in bytes).
/// @return     Memory pointer.
/// @details    Tries to allocate memory in first memory pool of config.
void*           OS_ISR_Malloc(const Size size);

/**@}*/ //OS_ISR_Memory

/**@}*/ //OS_Memory

#ifdef __cplusplus
}
#endif

#endif // _OS_MEMORY_H_
