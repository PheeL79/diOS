/***************************************************************************//**
* @file    os_startup.c
* @brief   OS Startup.
* @author  A. Filyanov
*******************************************************************************/
#include "os_list.h"
#include "os_task.h"
#include "os_memory.h"
#include "os_startup.h"

//------------------------------------------------------------------------------
static OS_List os_startup_list;

/******************************************************************************/
Status OS_StartupInit(void)
{
    OS_ListInit(&os_startup_list);
    if (OS_TRUE != OS_ListIsInitialised(&os_startup_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
Status OS_StartupDeInit(void)
{
    OS_ListClear(&os_startup_list);
    return S_OK;
}

/******************************************************************************/
Status OS_StartupApplication(void)
{
OS_ListItem* iter_li_p;
Status s = S_OK;

    OS_LOG(D_INFO, "OS startup run...");
    // Create startup tasks.
    for (iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_startup_list));
         OS_DELAY_MAX != (OS_Value)OS_ListItemValueGet(iter_li_p);
         iter_li_p = OS_ListItemNextGet(iter_li_p)) {
        const OS_TaskConfig* task_cfg_p = (const OS_TaskConfig*)OS_ListItemOwnerGet(iter_li_p);
        OS_TaskHd thd = OS_NULL;
        IF_STATUS(s = OS_TaskCreate(OS_NULL, task_cfg_p, &thd)) {
            OS_ASSERT(OS_FALSE);
        }
        OS_ListItemValueSet(iter_li_p, (OS_Value)thd);
    }
    return s;
}

/******************************************************************************/
Status OS_StartupSystem(void)
{
extern const OS_TaskConfig task_sv_cfg, task_log_cfg, task_shell_cfg;
const OS_PowerState power = OS_PowerStateGet();
Status s;
    IF_STATUS(s = task_sv_cfg.func_power(OS_NULL, power)) { return s; }
    IF_STATUS(s = OS_StartupTaskAdd(&task_log_cfg)) { return s; }
#if (USBH_ENABLED) || (USBD_ENABLED)
    extern const OS_TaskConfig task_usb_cfg;
    IF_STATUS(s = OS_StartupTaskAdd(&task_usb_cfg)) { return s; }
#endif //(USBH_ENABLED) || (USBD_ENABLED)
#if (OS_FILE_SYSTEM_ENABLED)
    extern const OS_TaskConfig task_fs_cfg;
    IF_STATUS(s = OS_StartupTaskAdd(&task_fs_cfg)) { return s; }
#endif //(OS_FILE_SYSTEM_ENABLED)
#if (OS_AUDIO_ENABLED)
    extern const OS_TaskConfig task_audio_cfg;
    IF_STATUS(s = OS_StartupTaskAdd(&task_audio_cfg)) { return s; }
#endif //(OS_AUDIO_ENABLED)
    IF_STATUS(s = OS_StartupTaskAdd(&task_shell_cfg)) { return s; }
    return s;
}

/******************************************************************************/
Status OS_StartupTaskAdd(const OS_TaskConfig* task_cfg_p)
{
OS_ListItem* item_l_p = OS_ListItemCreate();
Status s = S_OK;
    if (OS_NULL == item_l_p) { return S_NO_MEMORY; }
    OS_ListItemValueSet(item_l_p, (OS_Value)OS_NULL);
    OS_ListItemOwnerSet(item_l_p, (OS_Owner)task_cfg_p);
    OS_ListAppend(&os_startup_list, item_l_p);
    return s;
}