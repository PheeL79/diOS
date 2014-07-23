/***************************************************************************//**
* @file    osal.c
* @brief   OSAL.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "hal.h"
#include "osal.h"
#include "os_common.h"
#include "os_supervise.h"
#include "os_driver.h"
#include "os_memory.h"
#include "os_message.h"
#include "os_shell.h"
#include "os_power.h"
#include "os_list.h"
#include "os_time.h"
#include "os_file_system.h"
#include "os_environment.h"
#include "os_startup.h"
#include "task_sv.h"
#include "task_shell.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "osal"

//------------------------------------------------------------------------------
extern volatile HAL_Env hal_env;
volatile OS_Env os_env = {
    .hal_env_p      = &hal_env,
    .drv_stdio      = OS_NULL,
};
volatile BL is_idle;

//------------------------------------------------------------------------------
static Status OSAL_DriversCreate(void);

/******************************************************************************/
Status OSAL_Init(void)
{
extern Status OS_MemoryInit(void);
extern Status OS_SettingsInit(void);
extern Status OS_EnvInit(void);
extern Status OS_EventInit(void);
extern Status OS_TimeInit(void);
extern Status OS_TimerInit(void);
extern Status OS_DriverInit_(void);
extern Status OS_QueueInit(void);
extern Status OS_TaskInit_(void);
Status s;
    //Init OSAL.
    is_idle = OS_FALSE;
    IF_STATUS(s = OS_MemoryInit())      { return s; }
    IF_STATUS(s = OS_TimerInit())       { return s; }
    IF_STATUS(s = OS_TimeInit())        { return s; }
    IF_STATUS(s = OS_DriverInit_())     { return s; }
    IF_STATUS(s = OS_DebugInit())       { return s; }
    IF_STATUS(s = OS_QueueInit())       { return s; }
    IF_STATUS(s = OS_TaskInit_())       { return s; }
    IF_STATUS(s = OS_ShellInit())       { return s; }
    IF_STATUS(s = OS_EnvInit())         { return s; }
    IF_STATUS(s = OS_EventInit())       { return s; }
    IF_STATUS(s = OS_SettingsInit())    { return s; }
    IF_STATUS(s = OS_PowerInit())       { return s; }
    IF_STATUS(s = OSAL_DriversCreate()) { return s; }
    D_LOG(D_INFO, "OSAL init...");
    D_LOG(D_INFO, "-------------------------------");
#if (1 == OS_FILE_SYSTEM_ENABLED)
    IF_STATUS(s = OS_FileSystemInit())  { return s; }
#endif // OS_FILE_SYSTEM_ENABLED
    IF_STATUS(s = OS_LocaleSet(LOCALE_DEFAULT))                             { return s; }
    IF_STATUS(s = OS_LogLevelSet(OS_LOG_LEVEL_DEFAULT))                     { return s; }
    //Create environment variables.
    IF_STATUS(s = OS_EnvVariableSet("locale", LOCALE_DEFAULT))              { return s; }
    IF_STATUS(s = OS_EnvVariableSet("stdio", "USART6"))                     { return s; }
    IF_STATUS(s = OS_EnvVariableSet("log_level", OS_LOG_LEVEL_DEFAULT))     { return s; }
    IF_STATUS(s = OS_EnvVariableSet("log_file", OS_LOG_FILE_PATH))          { return s; }
    IF_STATUS(s = OS_EnvVariableSet("config_file", OS_SETTINGS_FILE_PATH))  { return s; }
    //Init environment variables.
    const OS_PowerState power = PWR_STARTUP;
    os_env.hal_env_p->power = power;
    //Create and start system tasks.
    IF_STATUS(s = OS_StartupInit())     { return s; }
    IF_STATUS(s = OS_StartupSystem())   { return s; }
    D_LOG(D_INFO, "-------------------------------");
    return s;
}

/******************************************************************************/
Status OSAL_DriversCreate(void)
{
Status s;
    //Create, init and open system drivers;
    const OS_DriverConfig drv_cfg = {
        .name       = "USART6",
        .itf_p      = drv_stdio_p,
        .prio_power = OS_PWR_PRIO_MAX - 1
    };
    IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&os_env.drv_stdio)) { return s; }
    IF_STATUS(s = OS_DriverInit(os_env.drv_stdio)) { return s; }
    IF_STATUS(s = OS_DriverOpen(os_env.drv_stdio, OS_NULL)) { return s; }
    return s;
}

/******************************************************************************/
void OS_SystemTickStart(void)
{
}

/******************************************************************************/
void OS_SystemTickStop(void)
{
}

///******************************************************************************/
OS_PowerState OS_PowerStateGet(void)
{
    return os_env.hal_env_p->power;
}

/******************************************************************************/
OS_DriverHd OS_DriverStdIoGet(void)
{
    return os_env.drv_stdio;
}

/******************************************************************************/
Locale OS_LocaleGet(void)
{
    return os_env.hal_env_p->locale;
}

/******************************************************************************/
Status OS_LocaleSet(ConstStrPtr locale_p)
{
    if (OS_NULL == locale_p) { return S_INVALID_REF; }
    if (!strcmp(LOCALE_STRING_EN, (char const*)locale_p)) {
        os_env.hal_env_p->locale = LOC_EN;
    } else if (!strcmp(LOCALE_STRING_RU, (char const*)locale_p)) {
        os_env.hal_env_p->locale = LOC_RU;
    } else { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
const HAL_DriverItf* OS_StdIoGet(void)
{
    return os_env.hal_env_p->stdio_p;
}

/******************************************************************************/
Status OS_StdIoSet(ConstStrPtr drv_name_p)
{
extern const HAL_DriverItf* OS_DriverItfGet(const OS_DriverHd dhd);

    if (OS_NULL == drv_name_p) { return S_INVALID_REF; }
    OS_DriverHd dhd = OS_DriverByNameGet(drv_name_p);
    if (OS_NULL == dhd) { return S_UNDEF_DRV; }
    os_env.hal_env_p->stdio_p = OS_DriverItfGet(dhd);
    return S_OK;
}

/******************************************************************************/
OS_LogLevel OS_LogLevelGet(void)
{
    return (OS_LogLevel)os_env.hal_env_p->log_level;
}

/******************************************************************************/
Status OS_LogLevelSet(ConstStrPtr log_level_p)
{
ConstStr none_str[]     = "none";
ConstStr critical_str[] = "critical";
ConstStr warning_str[]  = "warning";
ConstStr info_str[]     = "info";
ConstStr debug_str[]    = "debug";
OS_LogLevel level       = D_NONE;

    if (OS_NULL == log_level_p) { return S_INVALID_REF; }
    if (!strcmp((const char*)none_str, (const char*)log_level_p)) {
    } else if (!strcmp((const char*)critical_str, (const char*)log_level_p)) {
        level = D_CRITICAL;
    } else if (!strcmp((const char*)warning_str, (const char*)log_level_p)) {
        level = D_WARNING;
    } else if (!strcmp((const char*)info_str, (const char*)log_level_p)) {
        level = D_INFO;
    } else if (!strcmp((const char*)debug_str, (const char*)log_level_p)) {
        level = D_DEBUG;
    } else {
        return S_INVALID_VALUE;
    }
    os_env.hal_env_p->log_level = (LogLevel)level;
    return S_OK;
}

/******************************************************************************/
OS_SchedulerState OS_SchedulerStateGet(void)
{
OS_SchedulerState state = OS_SCHED_STATE_UNDEF;
    switch (xTaskGetSchedulerState()) {
        case taskSCHEDULER_NOT_STARTED:
            state = OS_SCHED_STATE_NOT_STARTED;
            break;
        case taskSCHEDULER_RUNNING:
            state = OS_SCHED_STATE_RUN;
            break;
        case taskSCHEDULER_SUSPENDED:
            state = OS_SCHED_STATE_SUSPEND;
            break;
        default:
            break;
    }
    return state;
}

/******************************************************************************/
Status OS_StorageItemCreate(const void* data_p, const U16 size, OS_StorageItem** item_pp)
{
    if (OS_NULL == item_pp) { return S_INVALID_REF; }
    OS_StorageItem* item_p = OS_Malloc(sizeof(OS_StorageItem));
    if (OS_NULL == item_p) { return S_NO_MEMORY; }
    item_p->mutex = OS_MutexCreate();
    if (OS_NULL == item_p->mutex) { return S_INVALID_REF; }
    *item_pp = item_p;
    item_p->data_p  = (void*)data_p;
    item_p->size    = size;
    item_p->owners  = 1;
    return S_OK;
}

/******************************************************************************/
Status OS_StorageItemDelete(OS_StorageItem* item_p)
{
    if (OS_NULL == item_p) { return S_INVALID_REF; }
    if (0 == --(item_p->owners)) {
        OS_MutexDelete(item_p->mutex);
        OS_Free(item_p->data_p);
        OS_Free(item_p);
    }
    return S_OK;
}

/******************************************************************************/
Status OS_StorageItemOwnerAdd(OS_StorageItem* item_p)
{
    if (OS_NULL == item_p) { return S_INVALID_REF; }
    if (item_p->owners >= OS_STORAGE_ITEM_OWNERS_MAX) { return S_OVERFLOW; }
    item_p->owners++;
    return S_OK;
}

/******************************************************************************/
Status OS_StorageItemLock(OS_StorageItem* item_p, const TimeMs timeout)
{
    if (OS_NULL == item_p) { return S_INVALID_REF; }
    return OS_MutexLock(item_p->mutex, timeout);
}

/******************************************************************************/
Status OS_StorageItemUnlock(OS_StorageItem* item_p)
{
    if (OS_NULL == item_p) { return S_INVALID_REF; }
    return OS_MutexUnlock(item_p->mutex);
}

/******************************************************************************/
void vApplicationStackOverflowHook(void);
void vApplicationStackOverflowHook(void)
{
    OS_ASSERT(OS_FALSE);
}

/******************************************************************************/
void vApplicationTickHook(void);
void vApplicationTickHook(void)
{
    OS_ASSERT(OS_FALSE);
}

/******************************************************************************/
void vApplicationIdleHook(void);
void vApplicationIdleHook(void)
{
    is_idle = OS_TRUE;
#if (0 == OS_TICKLESS_MODE_ENABLED)
    OS_POWER_STATE_SLEEP();
#endif // OS_TICKLESS_MODE_ENABLED
}

/******************************************************************************/
void vApplicationMallocFailedHook(void);
void vApplicationMallocFailedHook(void)
{
    OS_ASSERT(OS_FALSE);
}