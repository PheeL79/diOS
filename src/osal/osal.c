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
#include "os_mailbox.h"
#include "os_shell.h"
#include "os_power.h"
#include "os_list.h"
#include "os_time.h"
#include "os_file_system.h"
#include "os_environment.h"
#include "os_startup.h"

//-----------------------------------------------------------------------------
#define MDL_NAME    "osal"

//------------------------------------------------------------------------------
extern volatile HAL_Env hal_env;
volatile OS_Env os_env = {
    .hal_env_p      = &hal_env,
    .drv_stdin      = OS_NULL,
    .drv_stdout     = OS_NULL,
    .drv_rtc        = OS_NULL,
#if (OS_AUDIO_ENABLED)
    .volume         = OS_AUDIO_VOLUME_MIN
#endif // (OS_AUDIO_ENABLED)
};
volatile Bool is_idle;

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
#if (OS_TIMERS_ENABLED)
extern Status OS_TimerInit(void);
#endif //(OS_TIMERS_ENABLED)
extern Status OS_DriverInit_(void);
extern Status OS_QueueInit(void);
extern Status OS_TaskInit_(void);
#if (OS_AUDIO_ENABLED)
extern Status OS_AudioInit(void);
#endif // (OS_AUDIO_ENABLED)
#if (OS_NETWORK_ENABLED)
extern Status OS_NetworkInit(void);
#endif // (OS_NETWORK_ENABLED)
Status s;
    //Init OSAL.
    HAL_CRITICAL_SECTION_ENTER();
    is_idle = OS_FALSE;
    // uxCriticalNesting = 0; !!! Variables are created before OS Engine scheduler is started! Affects on drivers interrupts!
    IF_STATUS(s = OS_MemoryInit())      { return s; }
#if (OS_TIMERS_ENABLED)
    IF_STATUS(s = OS_TimerInit())       { return s; }
#endif //(OS_TIMERS_ENABLED)
    IF_STATUS(s = OS_TimeInit())        { return s; }
    IF_STATUS(s = OS_DriverInit_())     { return s; }
    IF_STATUS(s = OS_DebugInit())       { return s; }
    IF_STATUS(s = OS_QueueInit())       { return s; }
    IF_STATUS(s = OS_TaskInit_())       { return s; }
    IF_STATUS(s = OS_ShellInit())       { return s; }
    IF_STATUS(s = OS_EnvInit())         { return s; }
#if (OS_EVENTS_ENABLED)
    IF_STATUS(s = OS_EventInit())       { return s; }
#endif //(OS_EVENTS_ENABLED)
    IF_STATUS(s = OS_SettingsInit())    { return s; }
    IF_STATUS(s = OS_PowerInit())       { return s; }
    IF_STATUS(s = OSAL_DriversCreate()) { return s; }
    HAL_CRITICAL_SECTION_EXIT();
    HAL_LOG(D_INFO, "OSAL init...");
    HAL_LOG(D_INFO, "-------------------------------");
#if (OS_FILE_SYSTEM_ENABLED)
    IF_STATUS(s = OS_FileSystemInit())  { return s; }
#endif // OS_FILE_SYSTEM_ENABLED
#if (OS_AUDIO_ENABLED)
    IF_STATUS(s = OS_AudioInit())       { return s; }
#endif //(OS_AUDIO_ENABLED)
#if (OS_NETWORK_ENABLED)
    IF_STATUS(s = OS_NetworkInit())     { return s; }
#endif //(OS_NETWORK_ENABLED)
    //Create environment variables.
    IF_STATUS(s = OS_EnvVariableSet("locale", HAL_LOCALE_DEFAULT, OS_LocaleSet))            { return s; }
//    IF_STATUS(s = OS_EnvVariableSet("stdio", "USART6", OS_StdIoSet))                    { return s; }
    IF_STATUS(s = OS_EnvVariableSet("log_level", OS_LOG_LEVEL_DEFAULT, OS_LogLevelSet)) { return s; }
    IF_STATUS(s = OS_EnvVariableSet("log_file", OS_LOG_FILE_PATH, OS_NULL))             { return s; }
    IF_STATUS(s = OS_EnvVariableSet("config_file", OS_SETTINGS_FILE_PATH, OS_NULL))     { return s; }
#if (OS_FILE_SYSTEM_ENABLED)
    IF_STATUS(s = OS_EnvVariableSet("media_automount", "on", OS_NULL))                  { return s; }
#endif // OS_FILE_SYSTEM_ENABLED
#if (OS_AUDIO_ENABLED)
    Str volume_str[4];
    if (0 > OS_SNPrintF(volume_str, sizeof(volume_str), "%u", OS_AUDIO_OUT_VOLUME_DEFAULT)) {
        return S_INVALID_VALUE;
    }
    IF_STATUS(s = OS_EnvVariableSet("volume", volume_str, OS_VolumeSet)) {
        if (S_INVALID_PTR != s) { return s; } //Ignore first attempt. No audio devices are created so far.
    }
#endif //(OS_AUDIO_ENABLED)
    //Init environment variables.
    const OS_PowerState power = PWR_STARTUP;
    os_env.hal_env_p->power = power;
    //Create and start system tasks.
    IF_STATUS(s = OS_StartupInit())     { return s; }
    IF_STATUS(s = OS_StartupSystem())   { return s; }
    HAL_LOG(D_INFO, "-------------------------------");
    return s;
}

/******************************************************************************/
Status OSAL_DriversCreate(void)
{
Status s;
    //Create, init and open system drivers;
    {
        const OS_DriverConfig drv_cfg = {
            .name       = "USART6",
            .itf_p      = drv_stdio_p,
            .prio_power = OS_PWR_PRIO_MAX - 1,
        };
        OS_DriverHd drv_stdio;
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&drv_stdio)) { return s; }
        HAL_DriverItf drv_itf;
        OS_MemMov(&drv_itf, drv_stdio_p, sizeof(drv_itf));
        drv_itf.Write = USART6_DMA_Write;
        IF_STATUS(s = OS_DriverItfSet(drv_stdio, &drv_itf)) { return s; }
        IF_STATUS(s = OS_DriverInit(drv_stdio, OS_NULL)) { return s; }
        IF_STATUS(s = OS_DriverOpen(drv_stdio, OS_NULL)) { return s; }
        os_env.drv_stdin = drv_stdio;
        os_env.drv_stdout= os_env.drv_stdin;
    }

    {
        const OS_DriverConfig drv_cfg = {
            .name       = "RTC",
            .itf_p      = drv_rtc_v[DRV_ID_RTC],
            .prio_power = OS_PWR_PRIO_DEFAULT
        };
        IF_STATUS(s = OS_DriverCreate(&drv_cfg, (OS_DriverHd*)&os_env.drv_rtc)) { return s; }
        IF_STATUS(s = OS_DriverInit(os_env.drv_rtc, OS_NULL)) { return s; }
        IF_STATUS(s = OS_DriverOpen(os_env.drv_rtc, OS_NULL)) { return s; }
    }
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
OS_DriverHd OS_DriverStdInGet(void)
{
    return os_env.drv_stdin;
}

/******************************************************************************/
OS_DriverHd OS_DriverStdOutGet(void)
{
    return os_env.drv_stdout;
}

/******************************************************************************/
OS_DriverHd OS_DriverRtcGet(void)
{
    return os_env.drv_rtc;
}

#if (OS_AUDIO_ENABLED)
/******************************************************************************/
OS_AudioVolume OS_VolumeGet(void)
{
    return os_env.volume;
}

/******************************************************************************/
Status OS_VolumeSet(ConstStrP volume_p)
{
    os_env.volume = (OS_AudioVolume)OS_StrToUL((const char*)volume_p, OS_NULL, 10);
    OS_AudioDeviceHd dev_hd = OS_AudioDeviceDefaultGet(DIR_OUT);
    if (OS_NULL == dev_hd) {
        return S_INVALID_PTR;
    }
    return OS_AudioVolumeSet(dev_hd, os_env.volume);
}
#endif //(OS_AUDIO_ENABLED)

/******************************************************************************/
Locale OS_LocaleGet(void)
{
    return os_env.hal_env_p->locale;
}

/******************************************************************************/
Status OS_LocaleSet(ConstStrP locale_p)
{
    if (OS_NULL == locale_p) { return S_INVALID_PTR; }
    if (!OS_StrCmp(HAL_LOCALE_STRING_EN, (char const*)locale_p)) {
        os_env.hal_env_p->locale = LOC_EN;
    } else if (!OS_StrCmp(HAL_LOCALE_STRING_RU, (char const*)locale_p)) {
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
Status OS_StdIoSet(ConstStrP drv_name_p)
{
    if (OS_NULL == drv_name_p) { return S_INVALID_PTR; }
    OS_DriverHd dhd = OS_DriverByNameGet(drv_name_p);
    if (OS_NULL == dhd) { return S_INVALID_DRIVER; }
    os_env.hal_env_p->stdio_p = OS_DriverItfGet(dhd);
    return S_OK;
}

/******************************************************************************/
OS_LogLevel OS_LogLevelGet(void)
{
    return (OS_LogLevel)os_env.hal_env_p->log_level;
}

/******************************************************************************/
Status OS_LogLevelSet(ConstStrP log_level_p)
{
ConstStr none_str[]     = "none";
ConstStr critical_str[] = "critical";
ConstStr warning_str[]  = "warning";
ConstStr info_str[]     = "info";
ConstStr debug_str[]    = "debug";
OS_LogLevel level       = D_NONE;

    if (OS_NULL == log_level_p) { return S_INVALID_PTR; }
    if (!OS_StrCmp((const char*)none_str, (const char*)log_level_p)) {
    } else if (!OS_StrCmp((const char*)critical_str, (const char*)log_level_p)) {
        level = D_CRITICAL;
    } else if (!OS_StrCmp((const char*)warning_str, (const char*)log_level_p)) {
        level = D_WARNING;
    } else if (!OS_StrCmp((const char*)info_str, (const char*)log_level_p)) {
        level = D_INFO;
    } else if (!OS_StrCmp((const char*)debug_str, (const char*)log_level_p)) {
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
    if (OS_NULL == item_pp) { return S_INVALID_PTR; }
    OS_StorageItem* item_p = OS_Malloc(sizeof(OS_StorageItem));
    if (OS_NULL == item_p) { return S_OUT_OF_MEMORY; }
    item_p->mutex = OS_MutexCreate();
    if (OS_NULL == item_p->mutex) { return S_INVALID_PTR; }
    *item_pp = item_p;
    item_p->data_p  = (void*)data_p;
    item_p->size    = size;
    item_p->owners  = 1;
    return S_OK;
}

/******************************************************************************/
Status OS_StorageItemDelete(OS_StorageItem* item_p)
{
    if (OS_NULL == item_p) { return S_INVALID_PTR; }
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
    if (OS_NULL == item_p) { return S_INVALID_PTR; }
    if (item_p->owners >= OS_STORAGE_ITEM_OWNERS_MAX) { return S_OVERFLOW; }
    item_p->owners++;
    return S_OK;
}

/******************************************************************************/
Status OS_StorageItemLock(OS_StorageItem* item_p, const OS_TimeMs timeout)
{
    if (OS_NULL == item_p) { return S_INVALID_PTR; }
    return OS_MutexLock(item_p->mutex, timeout);
}

/******************************************************************************/
Status OS_StorageItemUnlock(OS_StorageItem* item_p)
{
    if (OS_NULL == item_p) { return S_INVALID_PTR; }
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
}

/******************************************************************************/
void vApplicationMallocFailedHook(void);
void vApplicationMallocFailedHook(void)
{
    OS_ASSERT(OS_FALSE);
}
