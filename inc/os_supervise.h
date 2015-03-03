#ifndef _OS_SUPERVISE_H_
#define _OS_SUPERVISE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_common.h"
#include "task.h"

/**
* \defgroup OS_Supervise OS_Supervise
* @{
*/
//------------------------------------------------------------------------------
enum {
    OS_SCHED_STATE_UNDEF,
    OS_SCHED_STATE_NOT_STARTED,
    OS_SCHED_STATE_RUN,
    OS_SCHED_STATE_SUSPEND,
    OS_SCHED_STATE_LAST
};
typedef U8 OS_SchedulerState;

//------------------------------------------------------------------------------
/// @brief      Enter critical section.
/// @return     None.
#define         OS_CriticalSectionEnter         portENTER_CRITICAL

/// @brief      Exit critical section.
/// @return     None.
#define         OS_CriticalSectionExit          portEXIT_CRITICAL

/// @brief      Start scheduler.
/// @return     None.
#define         OS_SchedulerStart               vTaskStartScheduler

/// @brief      Suspend scheduler.
/// @return     None.
#define         OS_SchedulerSuspend             vTaskSuspendAll

/// @brief      Resume scheduler.
/// @return     None.
#define         OS_SchedulerResume              xTaskResumeAll

/// @brief      Force task context switch.
/// @return     None.
#define         OS_ContextSwitchForce           taskYIELD

/// @brief      Switch to user mode.
/// @return     None.
#define         OS_UserMode                     portSWITCH_TO_USER_MODE

/// @brief      Get OS scheduler state.
/// @return     #OS_SchedulerState.
OS_SchedulerState OS_SchedulerStateGet(void);

/// @brief      Start system tick.
/// @return     None.
void            OS_SystemTickStart(void);

/// @brief      Stop system tick.
/// @return     None.
void            OS_SystemTickStop(void);

/**
* \addtogroup OS_ISR_Supervise ISR specific functions.
* @{
*/
//------------------------------------------------------------------------------
/// @brief      Force task context switch from ISR.
/// @return     None.
#define         OS_ISR_ContextSwitchForce(x)    portYIELD_FROM_ISR(x)

/**@}*/ //OS_ISR_Supervise

/**@}*/ //OS_Supervise

#ifdef __cplusplus
}
#endif

#endif // _OS_SUPERVISE_H_
