#ifndef _OS_SUPERVISE_H_
#define _OS_SUPERVISE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_common.h"
#include "task.h"
#include "typedefs.h"

//------------------------------------------------------------------------------
#define OS_CriticalSectionEnter()   portENTER_CRITICAL()
#define OS_CriticalSectionExit()    portEXIT_CRITICAL()
#define OS_SchedulerStart()         vTaskStartScheduler()
#define OS_SchedulerSuspend()       vTaskSuspendAll()
#define OS_SchedulerResume()        xTaskResumeAll()
#define OS_ContextSwitchForce()     taskYIELD()
#define OS_UserMode()               portSWITCH_TO_USER_MODE()

enum {
    OS_SCHED_STATE_UNDEF,
    OS_SCHED_STATE_NOT_STARTED,
    OS_SCHED_STATE_RUN,
    OS_SCHED_STATE_SUSPEND,
    OS_SCHED_STATE_LAST
};
typedef U8 OS_SchedulerState;

//------------------------------------------------------------------------------
OS_SchedulerState   OS_SchedulerStateGet(void);
void                OS_SystemTickStart(void);
void                OS_SystemTickStop(void);

#ifdef __cplusplus
}
#endif

#endif // _OS_SUPERVISE_H_
