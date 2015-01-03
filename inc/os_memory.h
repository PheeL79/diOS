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
typedef U32 OS_MemoryType;

/// @brief   Memory description.
typedef struct {
    UInt            addr;
    Size            size;
    Size            block_size;
    OS_MemoryType   type;
    ConstStrP       name_p;
} OS_MemoryDesc;

/// @brief   Memory statistics.
typedef struct {
    OS_MemoryDesc   desc;
    Size            used;
    Size            free;
} OS_MemoryStats;

//------------------------------------------------------------------------------
/// @brief   Common functions.
#define malloc(s)   OS_Malloc(s)
#define free(p)     OS_Free(p)

#define OS_MemSet   HAL_MemSet
#define OS_MemCmp   HAL_MemCmp
#define OS_MemCpy   HAL_MemCpy
#define OS_MemCpy32(dst_p, src_p, size) OS_MemCpy(dst_p, src_p, ((size) * sizeof(U32)))
#define OS_MemMov   HAL_MemMov
#define OS_MemMov32(dst_p, src_p, size) OS_MemMov(dst_p, src_p, ((size) * sizeof(U32)))

/// @brief      Allocate memory.
/// @param[in]  size            Allocation size (in bytes).
/// @return     Memory pointer.
/// @details    Tries to allocate memory in first memory pool of config.
void*           OS_Malloc(const Size size);

/// @brief      Allocate memory by type.
/// @param[in]  size            Allocation size (in bytes).
/// @param[in]  mem_type        Memory type.
/// @return     Memory pointer.
void*           OS_MallocEx(const Size size, const OS_MemoryType mem_type);

/// @brief      Free allocated memory.
/// @param[in]  addr_p          Memory address.
/// @return     None.
void            OS_Free(void* addr_p);

/// @brief      Free allocated memory by type.
/// @param[in]  addr_p          Memory address.
/// @param[in]  mem_type        Memory type.
/// @return     None.
void            OS_FreeEx(void* addr_p, const OS_MemoryType mem_type);

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

/// @brief      Get the next memory heap type.
/// @param[in]  mem_type        Memory type.
/// @return     Memory type.
OS_MemoryType   OS_MemoryTypeHeapNextGet(const OS_MemoryType mem_type);

/// @brief      Get the memory heap usage statistics.
/// @param[in]  mem_type        Memory type.
/// @param[out] mem_stats_p     Memory statistics.
/// @return     #Status.
Status          OS_MemoryStatsGet(const OS_MemoryType mem_type, OS_MemoryStats* mem_stats_p);

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
