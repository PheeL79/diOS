/***************************************************************************//**
* @file    os_settings.h
* @brief   OS Settings.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_SETTINGS_H_
#define _OS_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_file_system.h"

/**
* \defgroup OS_Settings OS_Settings
* @{
*/
//------------------------------------------------------------------------------
#if (1 == OS_SETTINGS_BROWSE_ENABLED)
typedef Status (*OS_SettingsCallback)(ConstStrPtr section_p, ConstStrPtr key_p, ConstStrPtr value_p, const void* args_p);
#endif // OS_SETTINGS_BROWSE_ENABLED

typedef enum {
    S_SETT_UNDEF = S_MODULE,
    S_SETT_READ,
    S_SETT_WRITE,
} OS_SettingsStatus;

typedef struct {
    ConstStrPtr section_p;
    ConstStrPtr key_p;
    Str         value[OS_SETTINGS_VALUE_LEN];
} OS_SettingsItem;

//------------------------------------------------------------------------------
/// @brief      Initialise the settings.
/// @return     #Status.
Status          OS_SettingsInit(void);

/// @brief      Deinitialise the settings.
/// @return     #Status.
Status          OS_SettingsDeInit(void);

/// @brief      Delete settings item.
/// @param[in]  file_path_p     Path to the settings file.
/// @param[in]  section_p       Settings section.
/// @param[in]  key_p           Key item.
/// @return     #Status.
Status          OS_SettingsDelete(ConstStrPtr file_path_p, ConstStrPtr section_p, ConstStrPtr key_p);

/// @brief      Read settings item.
/// @param[in]  file_path_p     Path to the settings file.
/// @param[in]  key_p           Key item.
/// @param[out] value_p         Key value.
/// @return     #Status.
Status          OS_SettingsRead(ConstStrPtr file_path_p, ConstStrPtr section_p, ConstStrPtr key_p, StrPtr value_p);

/// @brief      Write settings item.
/// @param[in]  file_path_p     Path to the settings file.
/// @param[in]  section_p       Settings section.
/// @param[in]  key_p           Key item.
/// @param[in]  value_p         Key value.
/// @return     #Status.
Status          OS_SettingsWrite(ConstStrPtr file_path_p, ConstStrPtr section_p, ConstStrPtr key_p, ConstStrPtr value_p);

/// @brief      Read settings items.
/// @param[in]  file_path_p     Path to the settings file.
/// @param[out] items[]         Items vector.
/// @return     #Status.
Status          OS_SettingsItemsRead(ConstStrPtr file_path_p, OS_SettingsItem items[]);

/// @brief      Write settings items.
/// @param[in]  file_path_p     Path to the settings file.
/// @param[in]  items[]         Items vector.
/// @return     #Status.
Status          OS_SettingsItemsWrite(ConstStrPtr file_path_p, OS_SettingsItem items[]);

#if (1 == OS_SETTINGS_BROWSE_ENABLED)
//Status OS_SettingsBrowse(OS_SettingsCallback callback_func_p, );
#endif // OS_SETTINGS_BROWSE_ENABLED

/**@}*/ //OS_Settings

#ifdef __cplusplus
}
#endif

#endif // _OS_SETTINGS_H_
