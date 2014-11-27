/**************************************************************************//**
* @file    drv_led.c
* @brief   LEDs driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "os_power.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_led"

//-----------------------------------------------------------------------------
/// @brief      Init LEDs.
/// @return     #Status.
Status          LED_Init_(void);

static Status LED_PulseInit(void* args_p);
static Status LED_PulseDeInit(void* args_p);
static Status LED_PulseOpen(void* args_p);
static Status LED_PulseClose(void* args_p);
static Status LED_PulseWrite(void* data_out_p, Size size, void* args_p);
static Status LED_PulseIoCtl(const U32 request_id, void* args_p);

static Status LED_FsInit(void* args_p);
static Status LED_FsDeInit(void* args_p);
static Status LED_FsOpen(void* args_p);
static Status LED_FsClose(void* args_p);
static Status LED_FsWrite(void* data_out_p, Size size, void* args_p);
static Status LED_FsIoCtl(const U32 request_id, void* args_p);

static Status LED_AssertInit(void* args_p);

static Status LED_UserInit(void* args_p);
static Status LED_UserDeInit(void* args_p);
static Status LED_UserOpen(void* args_p);
static Status LED_UserClose(void* args_p);
static Status LED_UserWrite(void* data_out_p, Size size, void* args_p);
static Status LED_UserIoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_led_v[DRV_ID_LED_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_led_pulse = {
    .Init   = LED_PulseInit,
    .DeInit = LED_PulseDeInit,
    .Open   = LED_PulseOpen,
    .Close  = LED_PulseClose,
    .Read   = OS_NULL,
    .Write  = LED_PulseWrite,
    .IoCtl  = LED_PulseIoCtl
};

static HAL_DriverItf drv_led_fs = {
    .Init   = LED_FsInit,
    .DeInit = LED_FsDeInit,
    .Open   = LED_FsOpen,
    .Close  = LED_FsClose,
    .Read   = OS_NULL,
    .Write  = LED_FsWrite,
    .IoCtl  = LED_FsIoCtl
};

static HAL_DriverItf drv_led_assert = {
    .Init   = LED_AssertInit,
    .DeInit = OS_NULL,
    .Open   = OS_NULL,
    .Close  = OS_NULL,
    .Read   = OS_NULL,
    .Write  = OS_NULL,
    .IoCtl  = OS_NULL
};

static HAL_DriverItf drv_led_user = {
    .Init   = LED_UserInit,
    .DeInit = LED_UserDeInit,
    .Open   = LED_UserOpen,
    .Close  = LED_UserClose,
    .Read   = OS_NULL,
    .Write  = LED_UserWrite,
    .IoCtl  = LED_UserIoCtl
};

/*****************************************************************************/
Status LED_Init_(void)
{
Status s = S_UNDEF;
    HAL_MemSet(drv_led_v, 0x0, sizeof(drv_led_v));
    drv_led_v[DRV_ID_LED_PULSE] = &drv_led_pulse;
    drv_led_v[DRV_ID_LED_FS]    = &drv_led_fs;
    drv_led_v[DRV_ID_LED_ASSERT]= &drv_led_assert;
    drv_led_v[DRV_ID_LED_USER]  = &drv_led_user;
    for (Size i = 0; i < ITEMS_COUNT_GET(drv_led_v, drv_led_v[0]); ++i) {
        //Ignore specific driver(s).
        if ((DRV_ID_LED_PULSE == i) ||
            (DRV_ID_LED_FS == i)) { continue; }
        if (OS_NULL != drv_led_v[i]) {
            IF_STATUS(s = drv_led_v[i]->Init(OS_NULL)) { return s; }
        }
    }
    return S_OK;
}

/*****************************************************************************/
Status LED_PulseInit(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStructure;

    HAL_LOG(D_INFO, "LED Pulse Init: ");
    __GPIOF_CLK_ENABLE();
    GPIO_InitStructure.Pin      = GPIO_PIN_6;
    GPIO_InitStructure.Mode     = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull     = GPIO_NOPULL;
    GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status LED_PulseDeInit(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_PulseOpen(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_PulseClose(void* args_p)
{
    return S_OK;
}

/******************************************************************************/
Status LED_PulseWrite(void* data_out_p, Size size, void* args_p)
{
Status s = S_OK;
    if (ON == *(State*)data_out_p) {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
    }
    return s;
}

/******************************************************************************/
Status LED_PulseIoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    s = S_OK;
                    break;
                case PWR_OFF:
                case PWR_SHUTDOWN:
                case PWR_STOP:
                case PWR_STANDBY: {
                    const State led_state = OFF;
                    s = LED_PulseWrite((U8*)&led_state, 1, OS_NULL);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            s = S_UNDEF_REQ_ID;
            break;
    }
    return s;
}

/*****************************************************************************/
Status LED_FsInit(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStructure;

    HAL_LOG(D_INFO, "LED FS Init: ");
    __GPIOF_CLK_ENABLE();
    GPIO_InitStructure.Pin      = GPIO_PIN_7;
    GPIO_InitStructure.Mode     = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull     = GPIO_NOPULL;
    GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status LED_FsDeInit(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_FsOpen(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_FsClose(void* args_p)
{
    return S_OK;
}

/******************************************************************************/
Status LED_FsWrite(void* data_out_p, Size size, void* args_p)
{
Status s = S_OK;
    if (ON == *(State*)data_out_p) {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
    }
    return s;
}

/******************************************************************************/
Status LED_FsIoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    s = S_OK;
                    break;
                case PWR_OFF:
                case PWR_SHUTDOWN:
                case PWR_STOP:
                case PWR_STANDBY: {
                    const State led_state = OFF;
                    s = LED_FsWrite((U8*)&led_state, 1, OS_NULL);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            s = S_UNDEF_REQ_ID;
            break;
    }
    return s;
}

/*****************************************************************************/
Status LED_AssertInit(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStructure;

    HAL_LOG(D_INFO, "LED Assert Init: ");
    __GPIOF_CLK_ENABLE();
    GPIO_InitStructure.Pin      = GPIO_PIN_8;
    GPIO_InitStructure.Mode     = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull     = GPIO_NOPULL;
    GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status LED_UserInit(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStructure;

    HAL_LOG(D_INFO, "LED User Init: ");
    __GPIOF_CLK_ENABLE();
    GPIO_InitStructure.Pin      = GPIO_PIN_9;
    GPIO_InitStructure.Mode     = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull     = GPIO_NOPULL;
    GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status LED_UserDeInit(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_UserOpen(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_UserClose(void* args_p)
{
    return S_OK;
}

/******************************************************************************/
Status LED_UserWrite(void* data_out_p, Size size, void* args_p)
{
Status s = S_OK;
    if (ON == *(State*)data_out_p) {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
    }
    return s;
}

/******************************************************************************/
Status LED_UserIoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    s = S_OK;
                    break;
                case PWR_OFF:
                case PWR_SHUTDOWN:
                case PWR_STOP:
                case PWR_STANDBY: {
                    const State led_state = OFF;
                    s = LED_UserWrite((U8*)&led_state, 1, OS_NULL);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            s = S_UNDEF_REQ_ID;
            break;
    }
    return s;
}