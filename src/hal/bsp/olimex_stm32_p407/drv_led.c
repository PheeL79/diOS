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

static Status LED_PulseInit(void);
static Status LED_PulseDeInit(void);
static Status LED_PulseOpen(void* args_p);
static Status LED_PulseClose(void);
static Status LED_PulseWrite(U8* data_out_p, U32 size, void* args_p);
static Status LED_PulseIoCtl(const U32 request_id, void* args_p);

static Status LED_FsInit(void);
static Status LED_FsDeInit(void);
static Status LED_FsOpen(void* args_p);
static Status LED_FsClose(void);
static Status LED_FsWrite(U8* data_out_p, U32 size, void* args_p);
static Status LED_FsIoCtl(const U32 request_id, void* args_p);

static Status LED_AssertInit(void);

static Status LED_UserInit(void);
static Status LED_UserDeInit(void);
static Status LED_UserOpen(void* args_p);
static Status LED_UserClose(void);
static Status LED_UserWrite(U8* data_out_p, U32 size, void* args_p);
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
    memset(drv_led_v, 0x0, sizeof(drv_led_v));
    drv_led_v[DRV_ID_LED_PULSE] = &drv_led_pulse;
    drv_led_v[DRV_ID_LED_FS]    = &drv_led_fs;
    drv_led_v[DRV_ID_LED_ASSERT]= &drv_led_assert;
    drv_led_v[DRV_ID_LED_USER]  = &drv_led_user;
    return S_OK;
}

/*****************************************************************************/
Status LED_PulseInit(void)
{
GPIO_InitTypeDef GPIO_InitStructure;

    D_LOG(D_INFO, "LED Pulse Init: ");
    GPIO_StructInit(&GPIO_InitStructure);
    /* Enable the GPIOF clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOF, GPIO_Pin_6);
    D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status LED_PulseDeInit(void)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_PulseOpen(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_PulseClose(void)
{
    return S_OK;
}

/******************************************************************************/
Status LED_PulseWrite(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    while (size--) {
        if (ON == *data_out_p) {
            GPIO_SetBits(GPIOF, GPIO_Pin_6);
        } else {
            GPIO_ResetBits(GPIOF, GPIO_Pin_6);
        }
    }
    return s;
}

/******************************************************************************/
Status LED_PulseIoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_POWER:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    break;
                case PWR_OFF:
                case PWR_SHUTDOWN:
                case PWR_STOP:
                case PWR_STANDBY: {
                    const State led_state = OFF;
                    LED_PulseWrite((U8*)&led_state, 1, OS_NULL);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            D_LOG_S(D_WARNING, S_UNDEF_REQ_ID);
            break;
    }
    return s;
}

/*****************************************************************************/
Status LED_FsInit(void)
{
GPIO_InitTypeDef GPIO_InitStructure;

    D_LOG(D_INFO, "LED FS Init: ");
    GPIO_StructInit(&GPIO_InitStructure);
    /* Enable the GPIOF clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOF, GPIO_Pin_7);
    D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status LED_FsDeInit(void)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_FsOpen(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_FsClose(void)
{
    return S_OK;
}

/******************************************************************************/
Status LED_FsWrite(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    while (size--) {
        if (ON == *data_out_p) {
            GPIO_SetBits(GPIOF, GPIO_Pin_7);
        } else {
            GPIO_ResetBits(GPIOF, GPIO_Pin_7);
        }
    }
    return s;
}

/******************************************************************************/
Status LED_FsIoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_POWER:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    break;
                case PWR_OFF:
                case PWR_SHUTDOWN:
                case PWR_STOP:
                case PWR_STANDBY: {
                    const State led_state = OFF;
                    LED_FsWrite((U8*)&led_state, 1, OS_NULL);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            D_LOG_S(D_WARNING, S_UNDEF_REQ_ID);
            break;
    }
    return s;
}

/*****************************************************************************/
Status LED_AssertInit(void)
{
GPIO_InitTypeDef GPIO_InitStructure;

    D_LOG(D_INFO, "LED Assert Init: ");
    GPIO_StructInit(&GPIO_InitStructure);
    /* Enable the GPIOF clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOF, GPIO_Pin_8);
    D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status LED_UserInit(void)
{
GPIO_InitTypeDef GPIO_InitStructure;

    D_LOG(D_INFO, "LED User Init: ");
    GPIO_StructInit(&GPIO_InitStructure);
    /* Enable the GPIOF clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOF, GPIO_Pin_9);
    D_TRACE_S(D_INFO, S_OK);
    return S_OK;
}

/*****************************************************************************/
Status LED_UserDeInit(void)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_UserOpen(void* args_p)
{
    return S_OK;
}

/*****************************************************************************/
Status LED_UserClose(void)
{
    return S_OK;
}

/******************************************************************************/
Status LED_UserWrite(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    while (size--) {
        if (ON == *data_out_p) {
            GPIO_SetBits(GPIOF, GPIO_Pin_9);
        } else {
            GPIO_ResetBits(GPIOF, GPIO_Pin_9);
        }
    }
    return s;
}

/******************************************************************************/
Status LED_UserIoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_POWER:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    break;
                case PWR_OFF:
                case PWR_SHUTDOWN:
                case PWR_STOP:
                case PWR_STANDBY: {
                    const State led_state = OFF;
                    LED_UserWrite((U8*)&led_state, 1, OS_NULL);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            D_LOG_S(D_WARNING, S_UNDEF_REQ_ID);
            break;
    }
    return s;
}