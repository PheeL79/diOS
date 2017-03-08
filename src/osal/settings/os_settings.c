/***************************************************************************//**
* @file    os_settings.c
* @brief   OS Settings.
* @author  A. Filyanov
*******************************************************************************/
#include "minIni.h"
#include "os_debug.h"
#include "os_settings.h"

//------------------------------------------------------------------------------
#define MDL_NAME            "settings"
#undef  MDL_STATUS_ITEMS
#define MDL_STATUS_ITEMS    &status_sett_tab[0]

//------------------------------------------------------------------------------
const StatusItem status_sett_tab[] = {
//file system
    {"Sett undef status"},
    {"Sett item read fail"},
    {"Sett item write fail"},
};

/******************************************************************************/
Status OS_SettingsInit(void)
{
Status s = S_OK;
    //D_LOG(L_INFO, "Init: ");
    //D_TRACE_S(L_INFO, s);
    return s;
}

/******************************************************************************/
Status OS_SettingsDeInit(void)
{
Status s = S_OK;
    //D_LOG(L_INFO, "DeInit: ");
    //D_TRACE_S(L_INFO, s);
    return s;
}

/******************************************************************************/
Status OS_SettingsDelete(ConstStrP file_path_p, ConstStrP section_p, ConstStrP key_p)
{
    OS_LOG(L_DEBUG_1, "Sett del: %s, sect: %s, key: %s", file_path_p, section_p, key_p);
    return OS_SettingsWrite(file_path_p, section_p, key_p, OS_NULL);
}

/******************************************************************************/
Status OS_SettingsRead(ConstStrP file_path_p, ConstStrP section_p, ConstStrP key_p, StrP value_p)
{
Status s = S_OK;
    if (!ini_gets((char const*)section_p, (char const*)key_p,
                  OS_SETTINGS_VALUE_DEFAULT, (char*)value_p, OS_SETTINGS_VALUE_LEN, (const char*)file_path_p)) {
        s = S_SETT_READ;
        OS_LOG_S(L_WARNING, s);
    }
    OS_LOG(L_DEBUG_1, "Sett read: %s,\nsect: %s, key: %s, val: %s", file_path_p, section_p, key_p, value_p);
    return s;
}

/******************************************************************************/
Status OS_SettingsWrite(ConstStrP file_path_p, ConstStrP section_p, ConstStrP key_p, ConstStrP value_p)
{
Status s = S_OK;
    OS_LOG(L_DEBUG_1, "Sett write: %s,\nsect: %s, key: %s, val: %s", file_path_p, section_p, key_p, value_p);
    if (!ini_puts((char const*)section_p, (char const*)key_p, (char*)value_p, (const char*)file_path_p)) {
        s = S_SETT_WRITE;
        OS_LOG_S(L_WARNING, s);
    }
    return s;
}

/******************************************************************************/
Status OS_SettingsItemsRead(ConstStrP file_path_p, OS_SettingsItem items[])
{
OS_SettingsItem* items_it = &items[0];
Status s = S_IS_EMPTY;
    OS_ASSERT_DEBUG(items_it);
    while (items_it++) {
        if (OS_NULL == items_it->key_p) { break; }
        IF_STATUS(s = OS_SettingsRead(file_path_p, items_it->section_p, items_it->key_p, &items_it->value[0])) { break; }
    }
    return s;
}

/******************************************************************************/
Status OS_SettingsItemsWrite(ConstStrP file_path_p, OS_SettingsItem items[])
{
OS_SettingsItem* items_it = &items[0];
Status s = S_IS_EMPTY;
    OS_ASSERT_DEBUG(items_it);
    while (items_it++) {
        if (OS_NULL == items_it->key_p) { break; }
        IF_STATUS(s = OS_SettingsWrite(file_path_p, items_it->section_p, items_it->key_p, &items_it->value[0])) { break; }
    }
    return s;
}

#if (OS_SETTINGS_BROWSE_ENABLED)
/******************************************************************************/
//int SettingsCallback(const char* section, const char* key, const char* value, const void* userdata)
//{
//    return ()
//}
#endif // (OS_SETTINGS_BROWSE_ENABLED)
