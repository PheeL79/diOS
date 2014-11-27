/***************************************************************************//**
* @file    os_audio.c
* @brief   OS Audio.
* @author  A. Filyanov
*******************************************************************************/
#include "hal.h"
#include "drv_audio.h"
#include "os_debug.h"
#include "os_list.h"
#include "os_mutex.h"
#include "os_memory.h"
#include "os_audio.h"
#include "os_task_audio.h"

#if (1 == OS_AUDIO_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "audio"

//------------------------------------------------------------------------------
typedef struct {
    Str                 name[OS_AUDIO_DEVICE_NAME_LEN];
    OS_DriverHd         dhd;
    OS_AudioDeviceCaps  caps;
    OS_AudioDeviceStats stats;
} OS_AudioDeviceConfigDyn;

//------------------------------------------------------------------------------
static Status AudioDeviceCapsTest(const OS_AudioDeviceHd dev_hd, const OS_AudioInfo info, Direction dir);

//------------------------------------------------------------------------------
static OS_List os_audio_list;
static OS_MutexHd os_audio_mutex;
static volatile OS_AudioDeviceHd def_in_dev_hd;
static volatile OS_AudioDeviceHd def_out_dev_hd;

/******************************************************************************/
#pragma inline
static OS_AudioDeviceConfigDyn* OS_AudioDeviceConfigDynGet(const OS_AudioDeviceHd dev_hd);
OS_AudioDeviceConfigDyn* OS_AudioDeviceConfigDynGet(const OS_AudioDeviceHd dev_hd)
{
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    const OS_ListItem* item_l_p = (OS_ListItem*)dev_hd;
    OS_AudioDeviceConfigDyn* cfg_dyn_p = (OS_AudioDeviceConfigDyn*)OS_ListItemValueGet(item_l_p);
    OS_ASSERT_VALUE(OS_DELAY_MAX != (OS_Value)cfg_dyn_p);
    return cfg_dyn_p;
}

/******************************************************************************/
Status OS_AudioInit(void)
{
Status s = S_OK;
    HAL_LOG(D_INFO, "Init");
    OS_AudioDeviceDefaultSet(OS_NULL, DIR_IN);
    OS_AudioDeviceDefaultSet(OS_NULL, DIR_OUT);
    os_audio_mutex = OS_MutexRecursiveCreate();
    if (OS_NULL == os_audio_mutex) { return S_INVALID_REF; }
    OS_ListInit(&os_audio_list);
    if (OS_TRUE != OS_ListIsInitialised(&os_audio_list)) { return S_INVALID_VALUE; }
    return s;
}

/******************************************************************************/
Status OS_AudioDeInit(void)
{
Status s = S_OK;
    //TODO(A. Filyanov) Loop and delete over existed media handles.
    return s;
}

/******************************************************************************/
Status OS_AudioDeviceCreate(const OS_AudioDeviceConfig* cfg_p, OS_AudioDeviceHd* dev_hd_p)
{
Status s = S_UNDEF;
    if (OS_NULL == cfg_p) { return S_INVALID_REF; }
    OS_ListItem* item_l_p = OS_ListItemCreate();
    if (OS_NULL == item_l_p) { return S_NO_MEMORY; }
    OS_AudioDeviceConfigDyn* cfg_dyn_p = OS_Malloc(sizeof(OS_AudioDeviceConfigDyn));
    if (OS_NULL == cfg_dyn_p) {
        OS_ListItemDelete(item_l_p);
        return S_NO_MEMORY;
    }
    IF_STATUS(s = OS_DriverCreate(cfg_p->drv_cfg_p, &cfg_dyn_p->dhd)) {
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
        return s;
    }
    OS_MemSet(&cfg_dyn_p->stats, 0, sizeof(OS_AudioDeviceStats));
    OS_StrNCpy(cfg_dyn_p->name, (const char*)cfg_p->name, sizeof(cfg_dyn_p->name));
    cfg_dyn_p->caps = cfg_p->caps;
    OS_ListItemValueSet(item_l_p, (OS_Value)cfg_dyn_p);
    OS_ListItemOwnerSet(item_l_p, (OS_Owner)OS_TaskGet());
    IF_OK(s = OS_MutexRecursiveLock(os_audio_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListAppend(&os_audio_list, item_l_p);
        OS_MutexRecursiveUnlock(os_audio_mutex);
    }
    if (OS_NULL != dev_hd_p) {
        *dev_hd_p = (OS_AudioDeviceHd)item_l_p;
    }
//error:
    IF_STATUS(s) {
        Status s_drv;
        IF_STATUS(s_drv = OS_DriverDelete(cfg_dyn_p->dhd)) { s = s_drv; }
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
    }
    return s;
}

/******************************************************************************/
Status OS_AudioDeviceDelete(const OS_AudioDeviceHd dev_hd)
{
Status s = S_UNDEF;
    IF_OK(s = OS_MutexRecursiveLock(os_audio_mutex, OS_TIMEOUT_MUTEX_LOCK)) {  // os_list protection;
        OS_ListItem* item_l_p = (OS_ListItem*)dev_hd;
        OS_AudioDeviceConfigDyn* cfg_dyn_p = (OS_AudioDeviceConfigDyn*)OS_ListItemValueGet(item_l_p);
        s = OS_DriverDelete(cfg_dyn_p->dhd);
        OS_Free(cfg_dyn_p);
        OS_ListItemDelete(item_l_p);
        OS_MutexRecursiveUnlock(os_audio_mutex);
    }
    return s;
}

/*****************************************************************************/
Status OS_AudioDeviceInit(const OS_AudioDeviceHd dev_hd, void* args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    OS_LOG(D_DEBUG, "Audio device init: %s", OS_AudioDeviceNameGet(dev_hd));
    const OS_AudioDeviceConfigDyn* cfg_dyn_p = OS_AudioDeviceConfigDynGet(dev_hd);
    const OS_DriverHd drv_audio_dev_hd = cfg_dyn_p->dhd;
    IF_STATUS(s = OS_DriverInit(drv_audio_dev_hd, args_p)) { OS_LOG_S(D_WARNING, s); }
    return s;
}

/*****************************************************************************/
Status OS_AudioDeviceDeInit(const OS_AudioDeviceHd dev_hd)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    OS_LOG(D_DEBUG, "Audio device deinit: %s", OS_AudioDeviceNameGet(dev_hd));
    const OS_AudioDeviceConfigDyn* cfg_dyn_p = OS_AudioDeviceConfigDynGet(dev_hd);
    const OS_DriverHd drv_audio_dev_hd = cfg_dyn_p->dhd;
    IF_STATUS(s = OS_DriverDeInit(drv_audio_dev_hd, OS_NULL)) {
        OS_LOG_S(D_WARNING, s);
    }
    return s;
}

/*****************************************************************************/
Status OS_AudioDeviceOpen(const OS_AudioDeviceHd dev_hd, void* args_p)
{
Status s = S_UNDEF;
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    OS_LOG(D_DEBUG, "Audio device open: %s", OS_AudioDeviceNameGet(dev_hd));
    const OS_AudioDeviceConfigDyn* cfg_dyn_p = OS_AudioDeviceConfigDynGet(dev_hd);
    const OS_DriverHd drv_audio_dev_hd = cfg_dyn_p->dhd;
    IF_OK(s = OS_DriverOpen(drv_audio_dev_hd, args_p)) {
        // Set default in audio device.
        if (OS_NULL != cfg_dyn_p->caps.input_p) {
            const Direction dir = DIR_IN;
            if (OS_NULL == OS_AudioDeviceDefaultGet(dir)) {
                IF_OK(s = OS_AudioDeviceDefaultSet(dev_hd, dir)) {
                }
            }
        }
        // Set default out audio device.
        if (OS_NULL != cfg_dyn_p->caps.output_p) {
            const Direction dir = DIR_OUT;
            if (OS_NULL == OS_AudioDeviceDefaultGet(dir)) {
                IF_OK(s = OS_AudioDeviceDefaultSet(dev_hd, dir)) {
                }
            }
        }
        const OS_TaskHd audio_thd = OS_TaskByNameGet(OS_DAEMON_NAME_AUDIO);
        if (audio_thd != OS_TaskGet()) { //Avoid self-connection.
            IF_OK(s = OS_TasksConnect(audio_thd, OS_THIS_TASK)) {
            }
        }
    }
    IF_STATUS(s) { OS_LOG_S(D_WARNING, s); }
    return s;
}

/*****************************************************************************/
Status OS_AudioDeviceClose(const OS_AudioDeviceHd dev_hd)
{
Status s;
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    OS_LOG(D_DEBUG, "Audio device close: %s", OS_AudioDeviceNameGet(dev_hd));
    const OS_AudioDeviceConfigDyn* cfg_dyn_p = OS_AudioDeviceConfigDynGet(dev_hd);
    const OS_DriverHd drv_audio_dev_hd = cfg_dyn_p->dhd;
    IF_OK(s = OS_DriverIoCtl(drv_audio_dev_hd, DRV_REQ_AUDIO_STOP, OS_NULL)) {
        IF_OK(s = OS_DriverClose(drv_audio_dev_hd, OS_NULL)) {
            if (1 > OS_DriverOwnersCountGet(drv_audio_dev_hd)) {
                // Set null default in audio device.
                if (OS_NULL != cfg_dyn_p->caps.input_p) {
                    const Direction dir = DIR_IN;
                    if (dev_hd == OS_AudioDeviceDefaultGet(dir)) {
                        IF_OK(s = OS_AudioDeviceDefaultSet(OS_NULL, dir)) {
                        }
                    }
                }
                // Set null default out audio device.
                if (OS_NULL != cfg_dyn_p->caps.output_p) {
                    const Direction dir = DIR_OUT;
                    if (dev_hd == OS_AudioDeviceDefaultGet(dir)) {
                        IF_OK(s = OS_AudioDeviceDefaultSet(OS_NULL, dir)) {
                        }
                    }
                }
            }
            const OS_TaskHd audio_thd = OS_TaskByNameGet(OS_DAEMON_NAME_AUDIO);
            IF_OK(s = OS_TasksDisconnect(audio_thd, OS_THIS_TASK)) {
            }
        }
    }
    IF_STATUS(s) { OS_LOG_S(D_WARNING, s); }
    return s;
}

/*****************************************************************************/
OS_AudioDeviceHd OS_AudioDeviceDefaultGet(const Direction dir)
{
    if (DIR_IN == dir) {
        return def_in_dev_hd;
    } else if (DIR_OUT == dir) {
        return def_out_dev_hd;
    } else {
        return OS_NULL;
    }
}

/*****************************************************************************/
Status OS_AudioDeviceDefaultSet(const OS_AudioDeviceHd dev_hd, const Direction dir)
{
    if (DIR_IN == dir) {
        def_in_dev_hd   = dev_hd;
    } else if (DIR_OUT == dir) {
        def_out_dev_hd  = dev_hd;
    } else {
        return S_INVALID_VALUE;
    }
    return S_OK;
}

/*****************************************************************************/
Status OS_AudioDeviceIoSetup(const OS_AudioDeviceHd dev_hd, const OS_AudioInfo info, const Direction dir)
{
Status s = S_UNDEF;
    if (OS_NULL == dev_hd) { return S_INVALID_REF; }
    IF_OK(s = AudioDeviceCapsTest(dev_hd, info, dir)) {
        const OS_AudioDeviceConfigDyn* cfg_dyn_p = OS_AudioDeviceConfigDynGet(dev_hd);
        U32 drv_request_id;
        if (DIR_IN == dir) {
            drv_request_id = DRV_REQ_AUDIO_INPUT_SETUP;
        } else if (DIR_OUT == dir) {
            drv_request_id = DRV_REQ_AUDIO_OUTPUT_SETUP;
        } else { return s = S_INVALID_VALUE; }
        s = OS_DriverIoCtl(cfg_dyn_p->dhd, drv_request_id, (void*)&info);
    }
    return s;
}

/*****************************************************************************/
Status OS_AudioDeviceCapsGet(const OS_AudioDeviceHd dev_hd, OS_AudioDeviceCaps* caps_p)
{
    if (OS_NULL == dev_hd) { return S_INVALID_REF; }
    if (OS_NULL == caps_p) { return S_INVALID_REF; }
    const OS_AudioDeviceConfigDyn* cfg_dyn_p = OS_AudioDeviceConfigDynGet(dev_hd);
    *caps_p = cfg_dyn_p->caps;
    return S_OK;
}

/******************************************************************************/
Status AudioDeviceCapsTest(const OS_AudioDeviceHd dev_hd, const OS_AudioInfo info, const Direction dir)
{
OS_AudioDeviceCaps caps;
Status s = S_UNDEF;
    if (OS_NULL == dev_hd) { return s = S_INVALID_REF; }
    IF_OK(s = OS_AudioDeviceCapsGet(dev_hd, &caps)) {
        const OS_AudioSampleRate* sample_rates_vp;
        const OS_AudioSampleBits* sample_bits_vp;
        OS_AudioChannels channels;

        if (DIR_IN == dir) {
            if (OS_NULL == caps.input_p)  { return s = S_INVALID_REF; }
            sample_rates_vp = caps.input_p->sample_rates_vp;
            sample_bits_vp  = caps.input_p->sample_bits_vp;
            channels        = caps.input_p->channels;
        } else if (DIR_OUT == dir) {
            if (OS_NULL == caps.output_p)  { return s = S_INVALID_REF; }
            sample_rates_vp = caps.output_p->sample_rates_vp;
            sample_bits_vp  = caps.output_p->sample_bits_vp;
            channels        = caps.output_p->channels;
        } else { return s = S_INVALID_VALUE; }
        //Test sample rate.
        Size i = 0;
        for (; 0 != sample_rates_vp[i]; ++i) {
            if (info.sample_rate == sample_rates_vp[i]) {
                break;
            }
        }
        if (0 == sample_rates_vp[i]) {
            return s = S_UNSUPPORTED;
        }
        //Test bits rate.
        i = 0;
        for (; 0 != sample_bits_vp[i]; ++i) {
            if (info.sample_bits == sample_bits_vp[i]) {
                break;
            }
        }
        if (0 == sample_bits_vp[i]) {
            return s = S_UNSUPPORTED;
        }
        //Test channels.
        if (info.channels != channels) {
            return s = S_UNSUPPORTED;
        }
    }
    return s;
}

/******************************************************************************/
ConstStrPtr OS_AudioDeviceNameGet(const OS_AudioDeviceHd dev_hd)
{
    if (OS_NULL == dev_hd) { return OS_NULL; }
    const OS_AudioDeviceConfigDyn* cfg_dyn_p = OS_AudioDeviceConfigDynGet(dev_hd);
    return cfg_dyn_p->name;
}

/*****************************************************************************/
Status OS_AudioVolumeGet(const OS_AudioDeviceHd dev_hd, OS_AudioVolume* volume_p)
{
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    OS_ASSERT_VALUE(OS_NULL != volume_p);
    const OS_DriverHd dhd = OS_AudioDeviceConfigDynGet(dev_hd)->dhd;
    return OS_DriverIoCtl(dhd, DRV_REQ_AUDIO_VOLUME_GET, (void*)volume_p);
}

/*****************************************************************************/
Status OS_AudioVolumeSet(const OS_AudioDeviceHd dev_hd, const OS_AudioVolume volume)
{
OS_AudioVolume dev_volume = volume;
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    if (dev_volume < OS_AUDIO_VOLUME_MIN) {
        dev_volume = OS_AUDIO_VOLUME_MIN;
    }
    if (dev_volume > OS_AUDIO_VOLUME_MAX) {
        dev_volume = OS_AUDIO_VOLUME_MAX;
    }
    const OS_DriverHd dhd = OS_AudioDeviceConfigDynGet(dev_hd)->dhd;
    return OS_DriverIoCtl(dhd, DRV_REQ_AUDIO_VOLUME_SET, (void*)&dev_volume);
}

/*****************************************************************************/
Status OS_AudioPlay(const OS_AudioDeviceHd dev_hd, void* data_p, Size size)
{
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    const OS_DriverHd dhd = OS_AudioDeviceConfigDynGet(dev_hd)->dhd;
    return OS_DriverWrite(dhd, data_p, size, OS_NULL);
}

/*****************************************************************************/
Status OS_AudioStop(const OS_AudioDeviceHd dev_hd)
{
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    const OS_DriverHd dhd = OS_AudioDeviceConfigDynGet(dev_hd)->dhd;
    return OS_DriverIoCtl(dhd, DRV_REQ_AUDIO_STOP, OS_NULL);
}

/*****************************************************************************/
Status OS_AudioPause(const OS_AudioDeviceHd dev_hd)
{
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    const OS_DriverHd dhd = OS_AudioDeviceConfigDynGet(dev_hd)->dhd;
    return OS_DriverIoCtl(dhd, DRV_REQ_AUDIO_PAUSE, OS_NULL);
}

/*****************************************************************************/
Status OS_AudioResume(const OS_AudioDeviceHd dev_hd)
{
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    const OS_DriverHd dhd = OS_AudioDeviceConfigDynGet(dev_hd)->dhd;
    return OS_DriverIoCtl(dhd, DRV_REQ_AUDIO_RESUME, OS_NULL);
}

/*****************************************************************************/
Status OS_AudioSeek(const OS_AudioDeviceHd dev_hd, const Size offset)
{
    OS_ASSERT_VALUE(OS_NULL != dev_hd);
    const OS_DriverHd dhd = OS_AudioDeviceConfigDynGet(dev_hd)->dhd;
    return OS_DriverIoCtl(dhd, DRV_REQ_AUDIO_SEEK, (void*)&offset);
}

/******************************************************************************/
OS_AudioDeviceHd OS_AudioDeviceNextGet(const OS_AudioDeviceHd dev_hd)
{
OS_ListItem* iter_li_p = (OS_ListItem*)dev_hd;
    IF_OK(OS_MutexRecursiveLock(os_audio_mutex, OS_TIMEOUT_MUTEX_LOCK)) {    // os_list protection;
        if (OS_NULL == iter_li_p) {
            iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(&os_audio_list));
            if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) {
                iter_li_p = OS_NULL;
            }
        } else {
            if (OS_DELAY_MAX != OS_ListItemValueGet(iter_li_p)) {
                iter_li_p = OS_ListItemNextGet(iter_li_p);
                if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) {
                    iter_li_p = OS_NULL;
                }
            } else {
                iter_li_p = OS_NULL;
            }
        }
        OS_MutexRecursiveUnlock(os_audio_mutex);
    } else {
        iter_li_p = OS_NULL;
    }
    return (OS_AudioDeviceHd)iter_li_p;
}

#endif // (1 == OS_AUDIO_ENABLED)