/***************************************************************************//**
* @file    os_environment.c
* @brief   OS Environment.
* @author  A. Filyanov
*******************************************************************************/
#include <string.h>
#include "os_common.h"
#include "os_memory.h"
#include "os_list.h"
#include "os_debug.h"
#include "os_mutex.h"
#include "os_environment.h"

//------------------------------------------------------------------------------
#define MDL_NAME            "environment"

//------------------------------------------------------------------------------
typedef struct {
    ConstStrP               name_p;
    ConstStrP               value_p;
    OS_EnvVariableHandler   handler_p;
} OS_EnvVariable;

//------------------------------------------------------------------------------
static OS_List os_variables_list;
static OS_MutexHd os_env_mutex;

//------------------------------------------------------------------------------
Status OS_EnvInit(void);
static OS_ListItem* OS_EnvVariableListItemByNameGet(ConstStrP name_p);

/******************************************************************************/
Status OS_EnvInit(void)
{
    os_env_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_env_mutex) { return S_INVALID_PTR; }
    OS_ListInit(&os_variables_list);
    if (OS_TRUE != OS_ListIsInitialised(&os_variables_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
OS_ListItem* OS_EnvVariableListItemByNameGet(ConstStrP name_p)
{
OS_ListItem* iter_li_p = OS_NULL;
    IF_OK(OS_MutexRecursiveLock(os_env_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_variables_list));
        if (OS_NULL == name_p) {
            goto exit;
        }
        if (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p)) {
            do {
                OS_EnvVariable* env_var_p = (OS_EnvVariable*)OS_ListItemValueGet(iter_li_p);
                if (!OS_StrCmp((const char*)env_var_p->name_p, (const char*)name_p)) {
                    goto exit;
                }
                iter_li_p = OS_ListItemNextGet(iter_li_p);
            } while (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p));
        }
        iter_li_p = OS_NULL;
exit:
        OS_MutexRecursiveUnlock(os_env_mutex);
    }
    return iter_li_p;
}

/******************************************************************************/
OS_TaskHd OS_EnvVariableOwnerGet(ConstStrP variable_name_p)
{
    OS_LOG(L_DEBUG_1, "Env owner get: %s", variable_name_p);
    const OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
    if (OS_NULL == item_l_p) { return OS_NULL; } //Variable not exists.
    return (OS_TaskHd)OS_ListItemOwnerGet(item_l_p);
}

/******************************************************************************/
OS_EnvVariableHandler OS_EnvVariableHandlerGet(ConstStrP variable_name_p)
{
    OS_LOG(L_DEBUG_1, "Env owner get: %s", variable_name_p);
    const OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
    if (OS_NULL == item_l_p) { return OS_NULL; } //Variable not exists.
    const OS_EnvVariable* env_var_p = (OS_EnvVariable*)OS_ListItemValueGet(item_l_p);
    return env_var_p->handler_p;
}

/******************************************************************************/
ConstStrP OS_EnvVariableGet(ConstStrP variable_name_p)
{
    OS_LOG(L_DEBUG_1, "Env var get: %s", variable_name_p);
    const OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
    if (OS_NULL == item_l_p) { return OS_NULL; } //Variable not exists.
    const OS_EnvVariable* env_var_p = (OS_EnvVariable*)OS_ListItemValueGet(item_l_p);
    return env_var_p->value_p;
}

/******************************************************************************/
ConstStrP OS_EnvVariableNextGet(ConstStrP variable_name_p)
{
const OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
    if (OS_NULL == item_l_p) { return OS_NULL; }
    const OS_EnvVariable* env_var_p =
        (OS_EnvVariable*)OS_ListItemValueGet(OS_ListItemNextGet(item_l_p));
    if (OS_DELAY_MAX == (OS_Value)env_var_p) { return OS_NULL; } //No next variable.
    return env_var_p->name_p;
}

/******************************************************************************/
Status OS_EnvVariableSet(ConstStrP variable_name_p, ConstStrP variable_value_p,
                         const OS_EnvVariableHandler variable_handler_p)
{
Status s = S_OK;
    if ((OS_NULL == variable_name_p) || (OS_NULL == variable_value_p)) { return S_INVALID_PTR; }
    HAL_LOG(L_DEBUG_1, "Env var set: %s, %s", variable_name_p, variable_value_p);
    IF_OK(s = OS_MutexRecursiveLock(os_env_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_list protection;
        OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
        OS_EnvVariable* env_var_p;
        //Is variable already exists?
        if (OS_NULL == item_l_p) { //No. Create the new one.
            const Size variable_name_len = OS_StrLen((char const*)variable_name_p) + 1;
            const Size variable_value_len= OS_StrLen((char const*)variable_value_p) + 1;
            if ((0 == variable_name_len) || (0 == variable_value_len)) { s = S_INVALID_VALUE; goto error; }
            item_l_p = OS_ListItemCreate();
            env_var_p = (OS_EnvVariable*)OS_Malloc(sizeof(OS_EnvVariable));
            env_var_p->name_p  = (ConstStrP)OS_Malloc(variable_name_len);
            env_var_p->value_p = (ConstStrP)OS_Malloc(variable_value_len);
            if ((OS_NULL == item_l_p) || (OS_NULL == env_var_p)) { s = S_OUT_OF_MEMORY; goto error; }
            if ((OS_NULL == env_var_p->name_p) || (OS_NULL == env_var_p->value_p)) { s = S_OUT_OF_MEMORY; goto error; }
            OS_StrNCpy((char*)env_var_p->name_p,  (char const*)variable_name_p,  variable_name_len);
            OS_StrNCpy((char*)env_var_p->value_p, (char const*)variable_value_p, variable_value_len);
            OS_ListItemValueSet(item_l_p, (OS_Value)env_var_p);
            OS_ListItemOwnerSet(item_l_p, OS_TaskGet());
            OS_ListAppend(&os_variables_list, item_l_p);
        } else { //Yes.
            env_var_p = (OS_EnvVariable*)OS_ListItemValueGet(item_l_p);
            OS_Free((void*)env_var_p->value_p); //Delete old value.
            const Size variable_data_len = OS_StrLen((char const*)variable_value_p) + 1;
            //Create the new one.
            env_var_p->value_p = (ConstStrP)OS_Malloc(variable_data_len);
            if (OS_NULL == env_var_p->value_p) { s = S_OUT_OF_MEMORY; goto error; }
            OS_StrNCpy((char*)env_var_p->value_p, (char const*)variable_value_p, variable_data_len);
        }
error:
        IF_STATUS(s) {
            OS_Free((void*)env_var_p->value_p);
            OS_Free((void*)env_var_p->name_p);
            OS_Free(env_var_p);
            OS_ListItemDelete(item_l_p);
        } else {
            if ((OS_NULL != env_var_p->handler_p) ||
                (OS_NULL != variable_handler_p)) {
                if (OS_NULL != variable_handler_p) {
                    env_var_p->handler_p = variable_handler_p;
                }
                s = env_var_p->handler_p(env_var_p->value_p);
            }
        }
        OS_MutexRecursiveUnlock(os_env_mutex);
    }
    return s;
}

/******************************************************************************/
Status OS_EnvVariableDelete(ConstStrP variable_name_p)
{
Status s;
    OS_LOG(L_DEBUG_1, "Env var del: %s", variable_name_p);
    IF_OK(s = OS_MutexRecursiveLock(os_env_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_list protection;
        OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
        if (OS_NULL == item_l_p) { return S_INVALID_PTR; }
        OS_EnvVariable* env_var_p = (OS_EnvVariable*)OS_ListItemValueGet(item_l_p);
        OS_ListItemDelete(item_l_p);
        OS_Free((void*)env_var_p->value_p);
        OS_Free((void*)env_var_p->name_p);
        OS_Free(env_var_p);
        OS_MutexRecursiveUnlock(os_env_mutex);
    }
    return s;
}