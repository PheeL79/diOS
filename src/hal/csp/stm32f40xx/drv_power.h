/**************************************************************************//**
* @file    drv_power.h
* @brief   Power driver.
* @author  A. Filyanov
******************************************************************************/
#ifndef _DRV_POWER_H_
#define _DRV_POWER_H_

//-----------------------------------------------------------------------------
/// @brief Power states.
enum {
    PWR_UNDEF,
    PWR_STARTUP,
    PWR_ON,
    PWR_OFF,
    PWR_SLEEP,
    PWR_STOP,
    PWR_STANDBY,
    PWR_HIBERNATE,
    PWR_SHUTDOWN,
    PWR_BATTERY,
    PWR_LAST
};

/// @brief Device voltage frequency scaling.
enum {
    DVFS_UNDEF,
    DVFS_LOW,
    DVFS_NORMAL,
    DVFS_MAX,
    DVFS_LAST
};

//-----------------------------------------------------------------------------
/// @brief Power system drivers.
enum {
    DRV_ID_POWER,
    DRV_ID_POWER_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_power_v[];

#endif // _DRV_POWER_H_