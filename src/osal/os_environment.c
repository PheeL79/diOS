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
    ConstStrPtr             name_p;
    ConstStrPtr             value_p;
    OS_EnvVariableHandler   handler_p;
} OS_EnvVariable;

//------------------------------------------------------------------------------
static OS_List os_variables_list;
static OS_MutexHd os_env_mutex;

//------------------------------------------------------------------------------
Status OS_EnvInit(void);
static OS_ListItem* OS_EnvVariableListItemByNameGet(ConstStrPtr name_p);

/******************************************************************************/
Status OS_EnvInit(void)
{
    os_env_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_env_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_variables_list);
    if (OS_TRUE != OS_LIST_IS_INITIALISED(&os_variables_list)) { return S_INVALID_VALUE; }
    return S_OK;
}

/******************************************************************************/
OS_ListItem* OS_EnvVariableListItemByNameGet(ConstStrPtr name_p)
{
OS_ListItem* iter_li_p = OS_NULL;
    IF_STATUS_OK(OS_MutexRecursiveLock(os_env_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        iter_li_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_variables_list));
        if (OS_NULL == name_p) {
            goto exit;
        }
        if (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(iter_li_p)) {
            do {
                OS_EnvVariable* env_var_p = (OS_EnvVariable*)OS_LIST_ITEM_VALUE_GET(iter_li_p);
                if (!OS_STRCMP((const char*)env_var_p->name_p, (const char*)name_p)) {
                    goto exit;
                }
                iter_li_p = OS_LIST_ITEM_NEXT_GET(iter_li_p);
            } while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(iter_li_p));
        }
        iter_li_p = OS_NULL;
exit:
        OS_MutexRecursiveUnlock(os_env_mutex);
    }
    return iter_li_p;
}

/******************************************************************************/
OS_TaskHd OS_EnvVariableOwnerGet(ConstStrPtr variable_name_p)
{
    OS_LOG(D_DEBUG, "Env owner get: %s", variable_name_p);
    const OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
    if (OS_NULL == item_l_p) { return OS_NULL; } //Variable not exists.
    return (OS_TaskHd)OS_LIST_ITEM_OWNER_GET(item_l_p);
}

/******************************************************************************/
OS_EnvVariableHandler OS_EnvVariableHandlerGet(ConstStrPtr variable_name_p)
{
    OS_LOG(D_DEBUG, "Env owner get: %s", variable_name_p);
    const OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
    if (OS_NULL == item_l_p) { return OS_NULL; } //Variable not exists.
    const OS_EnvVariable* env_var_p = (OS_EnvVariable*)OS_LIST_ITEM_VALUE_GET(item_l_p);
    return env_var_p->handler_p;
}

/******************************************************************************/
ConstStrPtr OS_EnvVariableGet(ConstStrPtr variable_name_p)
{
    OS_LOG(D_DEBUG, "Env var get: %s", variable_name_p);
    const OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
    if (OS_NULL == item_l_p) { return OS_NULL; } //Variable not exists.
    const OS_EnvVariable* env_var_p = (OS_EnvVariable*)OS_LIST_ITEM_VALUE_GET(item_l_p);
    return env_var_p->value_p;
}

/******************************************************************************/
ConstStrPtr OS_EnvVariableNextGet(ConstStrPtr variable_name_p)
{
const OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
    if (OS_NULL == item_l_p) { return OS_NULL; }
    const OS_EnvVariable* env_var_p =
        (OS_EnvVariable*)OS_LIST_ITEM_VALUE_GET(OS_LIST_ITEM_NEXT_GET(item_l_p));
    if (OS_DELAY_MAX == (OS_Value)env_var_p) { return OS_NULL; } //No next variable.
    return env_var_p->name_p;
}

/******************************************************************************/
Status OS_EnvVariableSet(ConstStrPtr variable_name_p, ConstStrPtr variable_value_p,
                         const OS_EnvVariableHandler variable_handler_p)
{
Status s = S_OK;
    if ((OS_NULL == variable_name_p) || (OS_NULL == variable_value_p)) { return S_INVALID_REF; }
    HAL_LOG(D_DEBUG, "Env var set: %s, %s", variable_name_p, variable_value_p);
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_env_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_list protection;
        OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
        OS_EnvVariable* env_var_p;
        //Is variable already exists?
        if (OS_NULL == item_l_p) { //No. Create the new one.
            const SIZE variable_name_len = OS_STRLEN((char const*)variable_name_p) + 1;
            const SIZE variable_value_len= OS_STRLEN((char const*)variable_value_p) + 1;
            if ((0 == variable_name_len) || (0 == variable_value_len)) { s = S_INVALID_VALUE; goto error; }
            item_l_p = OS_ListItemCreate();
            env_var_p = (OS_EnvVariable*)OS_Malloc(sizeof(OS_EnvVariable));
            env_var_p->name_p  = (ConstStrPtr)OS_Malloc(variable_name_len);
            env_var_p->value_p = (ConstStrPtr)OS_Malloc(variable_value_len);
            if ((OS_NULL == item_l_p) || (OS_NULL == env_var_p)) { s = S_NO_MEMORY; goto error; }
            if ((OS_NULL == env_var_p->name_p) || (OS_NULL == env_var_p->value_p)) { s = S_NO_MEMORY; goto error; }
            OS_STRNCPY((char*)env_var_p->name_p,  (char const*)variable_name_p,  variable_name_len);
            OS_STRNCPY((char*)env_var_p->value_p, (char const*)variable_value_p, variable_value_len);
            OS_LIST_ITEM_VALUE_SET(item_l_p, (OS_Value)env_var_p);
            OS_LIST_ITEM_OWNER_SET(item_l_p, OS_TaskGet());
            OS_ListAppend(&os_variables_list, item_l_p);
        } else { //Yes.
            env_var_p = (OS_EnvVariable*)OS_LIST_ITEM_VALUE_GET(item_l_p);
            OS_Free((void*)env_var_p->value_p); //Delete old value.
            const SIZE variable_data_len = OS_STRLEN((char const*)variable_value_p) + 1;
            //Create the new one.
            env_var_p->value_p = (ConstStrPtr)OS_Malloc(variable_data_len);
            if (OS_NULL == env_var_p->value_p) { s = S_NO_MEMORY; goto error; }
            OS_STRNCPY((char*)env_var_p->value_p, (char const*)variable_value_p, variable_data_len);
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
Status OS_EnvVariableDelete(ConstStrPtr variable_name_p)
{
Status s;
    OS_LOG(D_DEBUG, "Env var del: %s", variable_name_p);
    IF_STATUS_OK(s = OS_MutexRecursiveLock(os_env_mutex, OS_TIMEOUT_MUTEX_LOCK)) {   // os_list protection;
        OS_ListItem* item_l_p = OS_EnvVariableListItemByNameGet(variable_name_p);
        if (OS_NULL == item_l_p) { return S_INVALID_REF; }
        OS_EnvVariable* env_var_p = (OS_EnvVariable*)OS_LIST_ITEM_VALUE_GET(item_l_p);
        OS_ListItemDelete(item_l_p);
        OS_Free((void*)env_var_p->value_p);
        OS_Free((void*)env_var_p->name_p);
        OS_Free(env_var_p);
        OS_MutexRecursiveUnlock(os_env_mutex);
    }
    return s;
}