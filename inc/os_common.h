#ifndef _OS_COMMON_H_
#define _OS_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "os_config.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include "task.h"

//------------------------------------------------------------------------------
#define OS_DELAY_MAX            portMAX_DELAY
#define OS_BLOCK                OS_DELAY_MAX
#define OS_NO_BLOCK             ((U32)0)

//------------------------------------------------------------------------------
typedef portBASE_TYPE           OS_BaseType;
typedef TickType_t              OS_Value;
typedef TickType_t              OS_Status;
typedef TaskHandle_t            OS_Owner;

typedef enum {
    OS_MSG_PRIO_NORMAL,
    OS_MSG_PRIO_HIGH
} OS_MessagePrio;

#ifdef __cplusplus
}
#endif

#endif // _OS_COMMON_H_
