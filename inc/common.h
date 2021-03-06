/***************************************************************************//**
* @file    common.h
* @brief   Common definitions.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _COMMON_H_
#define _COMMON_H_

#include "status.h"

//------------------------------------------------------------------------------
// Common definitions.
//------------------------------------------------------------------------------
#define KHZ                         1000UL
#define MHZ                         1000000UL

#define MS                          1000UL
#define US                          1000000UL

//ASCII
#define ASCII_ESC_LINE_FEED         "\n"

// Common macros.
// inline
#if defined(_MSC_VER)               /* Microsoft Compiler */
#   define INLINE
#   define WEAK
#elif defined(__GNUC__)             /* GNU Compiler */
#   define ALIGN_END                __attribute__ ((aligned (4)))
#   define ALIGN_BEGIN
#   define INLINE                   __inline
#   define RAM_FUNC                 __attribute__((section(".RamFunc")))
#   define WEAK                     __attribute__((weak))
#   define MEMORY_BARRIER()         asm volatile ("" ::: "memory")
#   ifndef MAX
#       define MAX(x, y)            ({ __typeof__ (x) _x = (x); __typeof__ (y) _y = (y); _x > _y ? _x : _y; })
#   endif
#   ifndef MIN
#       define MIN(x, y)            ({ __typeof__ (x) _x = (x); __typeof__ (y) _y = (y); _x < _y ? _x : _y; })
#   endif
#elif defined(__ICCARM__)           /* IAR Compiler */
#   define ALIGN_END
#   define ALIGN_BEGIN
#   define INLINE                   inline
#   define __inline                 INLINE
#   define INLINE_PRAGMA            #pragma inline
#   define INLINE_PRAGMA_FORCED     #pragma inline=forced
#   define RAM_FUNC                 __ramfunc
#   define WEAK                     __weak
#   ifndef MAX
#       define MAX(x, y)            (((x) > (y)) ? (x) : (y))
#   endif
#   ifndef MIN
#       define MIN(x, y)            (((x) < (y)) ? (x) : (y))
#   endif
#   define MEM_ALIGN_SIZE(size, align) (((size) + align - 1) & ~(align - 1))
#else
#endif

#define CONCAT(a, b)                a ## b
#define IS_OVERFLOW(x, y, t)        ((t)CONCAT(t, _MIN) < 0 ? IS_OVERFLOW_S(x, y, t) : IS_OVERFLOW_U(x, y, t))
#define IS_OVERFLOW_S(x, y, t)      (((t)(y) > 0 && (t)(x) > CONCAT(t, _MAX) - (t)(y)) || ((t)(y) < 0 && (t)(x) < CONCAT(t, _MIN) - (t)(y)))
#define IS_OVERFLOW_U(x, y, t)      (((t)(x) > (t)CONCAT(t, _MAX) - (t)(y)) || ((t)(x) < (t)CONCAT(t, _MIN) - (t)(y)))

// Bit operations.
// http://www.coranac.com/man/tonclib/group__grpCoreBit.htm
#define BIT(n)                      ((unsigned)1 << (n))                ///< Create value with bit n set.
#define	BIT_SHIFT(a, n)             ((a) << (n))                        ///< Shift left a by n.
#define BIT_SHIFT_RIGHT(a, n)       ((a) >> (n))                        ///< Shift right a by n.
#define BIT_MASK(len)               (BIT(len) - 1)                      ///< Create a bitmask len bits long.
#define BIT_SET(y, flag)            (y |= (flag))                       ///< Set the flag bits in word.
#define BIT_CLEAR(y, flag)          (y &= ~(flag))                      ///< Clear the flag bits in word.
#define BIT_SWAP(y, flag)           (y ^= (flag))                       ///< Flip the flag bits in word.
#define BIT_SIZE(t)                 (sizeof(t) * CHAR_BIT)              ///< Get bit size of the type.
#define BIT_TEST(y, flag)           (((y) & (flag)) == (flag))          ///< Test whether all the flag bits in word are set.
#define BF_MASK(shift, len)         (BIT_MASK(len) << (shift))          ///< Create a bitmask of length len starting at bit shift.
#define BF_GET(y, shift, len)       (((y) >> (shift)) & BIT_MASK(len))  ///< Retrieve a bitfield mask of length starting at bit shift from y.
#define BF_PREP(x, shift, len)      (((x) & BIT_MASK(len)) << (shift))  ///< Prepare a bitmask for insertion or combining.
#define BF_SET(y, x, shift, len)    (y = ((y) & ~BF_MASK(shift, len)) | BF_PREP(x, shift, len)) ///< Insert a new bitfield value x into y.

#if defined(CM3) || defined(CM4) || defined(CM4F)
#define BITBAND_SRAM(a, b)          ((SRAM_BB_BASE + (a - SRAM_BASE) * 32 + (b * 4)))       // Convert SRAM address
#define BITBAND_PERI(a, b)          ((PERIPH_BB_BASE + (a - PERIPH_BASE) * 32 + (b * 4)))   // Convert PERI address
#define BB_PERI(c, d)               *((HAL_IO U32*) ((PERIPH_BB_BASE + (U32)(&(c) - PERIPH_BASE) * 32 + (d * 4))))
#endif

// Get maximum value of the given type.
//(Plain integer types only!)
#define TYPE_VALUE_MAX(type)        BIT_MASK(BIT_SIZE(type))

// Get count of elements table (sizeof(type) never 0)
#define ITEMS_COUNT_GET(tab, item_type)     (sizeof(tab) / sizeof(item_type))

// Get count of variadic arguments.
#define __VA_NARG__(...) \
        (__VA_NARG_(_0, ## __VA_ARGS__, __RSEQ_N()) - 1)
#define __VA_NARG_(...) \
        __VA_ARG_N(__VA_ARGS__)
#define __VA_ARG_N( \
         _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,N,...) N
#define __RSEQ_N() \
        63, 62, 61, 60,                         \
        59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
        19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
         9,  8,  7,  6,  5,  4,  3,  2,  1,  0

#endif // _COMMON_H_
