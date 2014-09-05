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
    if (OS_TRUE != OS_LIST_IS_INITIALISED(&os_startup_list)) { return S_INVALID_VALUE; }
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
    // Create and suspend startup tasks.
    for (iter_li_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_startup_list));
         OS_DELAY_MAX != (OS_Value)OS_LIST_ITEM_VALUE_GET(iter_li_p);
         iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p)) {
        const OS_TaskConfig* task_cfg_p = (const OS_TaskConfig*)OS_LIST_ITEM_OWNER_GET(iter_li_p);
        OS_TaskHd thd = OS_NULL;
        IF_STATUS_OK(s = task_cfg_p->func_power(task_cfg_p->args_p, PWR_STARTUP)) {
            IF_STATUS(s = OS_TaskCreate(task_cfg_p, &thd)) {
                OS_ASSERT(OS_FALSE);
            }
            OS_TaskSuspend(thd);
        }
        OS_LIST_ITEM_VALUE_SET(iter_li_p, (OS_Value)thd);
    }
    // Run tasks.
    for (iter_li_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_startup_list));
         OS_DELAY_MAX != (OS_Value)OS_LIST_ITEM_VALUE_GET(iter_li_p);
         iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p)) {
        const OS_TaskHd thd = (const OS_TaskHd)OS_LIST_ITEM_VALUE_GET(iter_li_p);
        if (OS_NULL != thd) {
            OS_TaskResume(thd);
        }
    }
    return s;
}

/******************************************************************************/
Status OS_StartupSystem(void)
{
extern const OS_TaskConfig task_sv_cfg, task_log_cfg, task_shell_cfg;
const OS_PowerState power = OS_PowerStateGet();
Status s;
    IF_STATUS(s = task_sv_cfg.func_power(task_sv_cfg.args_p, power)) { return s; }
    IF_STATUS(s = OS_StartupTaskAdd(&task_log_cfg))   { return s; }
    IF_STATUS(s = OS_StartupTaskAdd(&task_shell_cfg)) { return s; }
#if (1 == USBH_ENABLED)
    extern const OS_TaskConfig task_usbhd_cfg;
    IF_STATUS(s = OS_StartupTaskAdd(&task_usbhd_cfg)) { return s; }
#endif // USBH_ENABLED
    return s;
}

/******************************************************************************/
Status OS_StartupTaskAdd(const OS_TaskConfig* task_cfg_p)
{
OS_ListItem* item_l_p = OS_ListItemCreate();
Status s = S_OK;
    if (OS_NULL == item_l_p) { return S_NO_MEMORY; }
    OS_LIST_ITEM_VALUE_SET(item_l_p, (OS_Value)OS_NULL);
    OS_LIST_ITEM_OWNER_SET(item_l_p, (OS_Owner)task_cfg_p);
    OS_ListAppend(&os_startup_list, item_l_p);
    return s;
}