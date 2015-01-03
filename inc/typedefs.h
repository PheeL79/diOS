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
// limits
    #define U8_MIN              0x0
    #define U8_MAX              0xFFU
    #define U16_MIN             0x0
    #define U16_MAX             0xFFFFU
    #define U32_MIN             0x0
    #define U32_MAX             0xFFFFFFFFUL
    #define U64_MIN             0x0
    #define U64_MAX             0xFFFFFFFFFFFFFFFFUL

    #define S8_MIN              0x80
    #define S8_MAX              0x7F
    #define S16_MIN             0x8000
    #define S16_MAX             0x7FFF
    #define S32_MIN             0x80000000
    #define S32_MAX             0x7FFFFFFF
    #define S64_MIN             0x8000000000000000
    #define S64_MAX             0x7FFFFFFFFFFFFFFF

// common types
#if defined(CM4F) || defined(STM32F40XX)
    #include <stdlib.h>
    #include <stdint.h>

    #define DEV_ID_SIZE         6

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
    U8  data[DEV_ID_SIZE];
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
