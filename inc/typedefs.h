/***************************************************************************//**
* @file    typedefs.h
* @brief   Type definitions.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#include <limits.h>

//------------------------------------------------------------------------------
#define QUOTED(a)                       #a

// packed structs
#if defined(_MSC_VER)
#   define PACKED(pack_val, _struct_)   __pragma (pack(push, pack_val)) _struct_; __pragma (pack(pop))
#elif defined(__GNUC__)
#   define PACKED(pack_val, _struct_)   _Pragma (QUOTED(pack(pack_val))) _struct_ __attribute__((__aligned__(pack_val))); _Pragma ("pack()")
#elif defined(__ICCARM__)
#   define PACKED(pack_val, _struct_)   _Pragma (QUOTED(pack(push, pack_val))) _struct_; _Pragma ("pack(pop)")
#endif

//------------------------------------------------------------------------------
// common types
#if defined(CM4F) || defined(STM32F40XX)
    #include <stdlib.h>
    #include <stdint.h>

#ifndef ptrdiff_t
    typedef void*               Ptr;
#else
    typedef ptrdiff_t           Ptr;
#endif //ptrdiff_t
    typedef size_t              Size;

    typedef float               Float;
    typedef double              Double;

    typedef int                 Int;
    typedef unsigned int        UInt;

    typedef long                Long;
    typedef unsigned long       ULong;

    typedef uint8_t             Bool;

    // plain types
    typedef uint8_t             U8;
    typedef uint16_t            U16;
    typedef uint32_t            U32;
    typedef uint64_t            U64;

    typedef int8_t              S8;
    typedef int16_t             S16;
    typedef int32_t             S32;
    typedef int64_t             S64;

    // fast types
    typedef uint_fast8_t        UF8;
    typedef uint_fast16_t       UF16;
    typedef uint_fast32_t       UF32;
    typedef uint_fast64_t       UF64;

    typedef int_fast8_t         SF8;
    typedef int_fast16_t        SF16;
    typedef int_fast32_t        SF32;
    typedef int_fast64_t        SF64;
#else
#   error "typedefs.h: Undefined platform!"
#endif

// limits
#define U8_MAX                      ((U8)255)
#define S8_MAX                      ((S8)127)
#define S8_MIN                      ((S8)-128)
#define U16_MAX                     ((U16)65535u)
#define S16_MAX                     ((S16)32767)
#define S16_MIN                     ((S16)-32768)
#define U32_MAX                     ((U32)4294967295uL)
#define S32_MAX                     ((S32)2147483647)
#define S32_MIN                     ((S32)-2147483648)

#define HAL_NULL                    ((void*)0)
#define OS_NULL                     HAL_NULL
#ifndef NULL
#   define NULL                     HAL_NULL
#endif

#ifndef false
#   define false                    (0 == 1)
#   define FALSE                    false
#   define HAL_FALSE                FALSE
#   define OS_FALSE                 HAL_FALSE
#   define OS_FALSE_STR             "false"
#endif

#ifndef true
#   define true                     (1 == 1)
#   define TRUE                     true
#   define HAL_TRUE                 TRUE
#   define OS_TRUE                  HAL_TRUE
#   define OS_TRUE_STR              "true"
#endif

typedef enum {
    SORT_ASCENDING = 1,
    SORT_DESCENDING
} SortDirection;

typedef enum {
    OFF = 1,
    ON
} State;

typedef enum {
    DIR_IN = 1,
    DIR_OUT,
    DIR_BI
} Direction;

typedef enum {
    UNLOCKED = 1,
    LOCKED
} MutexState;

typedef enum {
    DISCONNECTED = 1,
    CONNECTED
} ConnectState;

typedef enum {
    LOC_EN,
    LOC_RU,
    LOC_LAST
} Locale;

typedef S16 Status;

// String type
typedef char Str;                       ///< String (zero ended)
typedef Str* StrP;                      ///< Pointer to string (zero ended)
typedef const Str ConstStr;             ///< Constant string (zero ended)
typedef ConstStr* ConstStrP;            ///< Pointer to constant string (zero ended)

typedef struct {
    ConstStrP locale[LOC_LAST];
//    ConstStrP en;                       ///< String English.
//    ConstStrP ru;                       ///< String Russian.
} LocaleString;

PACKED(PACK_VAL_PROTO,
typedef struct {
    U16 year;
    U8  month;
    U8  weekday;
    U8  day;
    U8  daylight;
    U8  hours;
    U8  minutes;
    U8  seconds;
    U8  hourformat;
} Time);

typedef struct {
    U8  data[6];
} DeviceId;

typedef struct {
    U16 maj;                    ///< Major revision.
    U16 min;                    ///< Minor revision.
    U16 id;                     ///< Device id.
    U16 padding;                ///< Reserved/padding.
} DeviceRevision;

/// @brief   Application version.
PACKED(PACK_VAL_PROTO,
typedef struct {
    U8  maj;                    ///< Major version.
    U8  min;                    ///< Minor version.
    U16 bld;                    ///< Build number.
    Str rev[11];                ///< Revision hash.
    U8  lbl;                    ///< Label.
} Version);

/// @brief   Motherboard description.
PACKED(PACK_VAL_PROTO,
typedef struct {
    DeviceRevision  revision;   ///< Device hardware revision.
} MbDesc);

/// @brief   Device description.
PACKED(PACK_VAL_PROTO,
typedef struct {
    Version         version;    ///< Application version.
    MbDesc          mb;         ///< Motherboard description.
} DeviceDesc);

/// @brief   Device description union.
/// @details Device description union no more than data[] length!
typedef union {
    U8 data[64];
    DeviceDesc device_description;
} DeviceDescUnion;

/// @brief   Device state.
typedef struct {
    ConnectState    connection;                 ///< Device connection state.
    DeviceDescUnion description;                ///< Device description.
} DeviceState;

#endif // _TYPEDEFS_H_
