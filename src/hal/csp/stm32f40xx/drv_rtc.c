/**************************************************************************//**
* @file    drv_rtc.c
* @brief   RTC driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "os_supervise.h"
#include "os_power.h"
#include "os_time.h"

//-----------------------------------------------------------------------------
#define MDL_NAME                "drv_rtc"

#define RTC_STAMP               0xC01DC0FEUL

#ifdef RTC_CLOCK_SOURCE_LSI
#   define RTC_ASYNCH_PREDIV    0x7F
#   define RTC_SYNCH_PREDIV     0x0130
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
#   define RTC_ASYNCH_PREDIV    0x7F
#   define RTC_SYNCH_PREDIV     0x00FF
#endif

//-----------------------------------------------------------------------------
/// @brief   RTC initialize.
/// @return  #Status.
Status RTC_Init_(void);

static Status   RTC__Init(void* args_p);
static Status   RTC__DeInit(void* args_p);
static Status   RTC_LL_Init(void);
//static Status   RTC_LL_DeInit(void* args_p);
static Status   RTC_Open(void* args_p);
static Status   RTC_Close(void* args_p);
static Status   RTC_Read(void* data_in_p, Size size, void* args_p);
static Status   RTC_Write(void* data_out_p, Size size, void* args_p);
static Status   RTC_IoCtl(const U32 request_id, void* args_p);

static void     RTC_SRAM_BACKUP_Init(void);
static Status   RTC_AlarmInit(void);
static Status   RTC_WakeupInit(void);

static void     RTC_Reset(void);
static void     RTC_SRAM_RegistersReset(void);
static void     RTC_SRAM_BackupReset(void);
static void     RTC_CalendarReset(void);

//-----------------------------------------------------------------------------
RTC_HandleTypeDef rtc_handle;
HAL_IO FlagStatus TamperStatus = RESET;

const U32 rtc_backup_regs[HAL_RTC_BACKUP_REGS_MAX] = {
    RTC_BKP_DR0,  RTC_BKP_DR1,  RTC_BKP_DR2,
    RTC_BKP_DR3,  RTC_BKP_DR4,  RTC_BKP_DR5,
    RTC_BKP_DR6,  RTC_BKP_DR7,  RTC_BKP_DR8,
    RTC_BKP_DR9,  RTC_BKP_DR10, RTC_BKP_DR11,
    RTC_BKP_DR12, RTC_BKP_DR13, RTC_BKP_DR14,
    RTC_BKP_DR15, RTC_BKP_DR16, RTC_BKP_DR17,
    RTC_BKP_DR18, RTC_BKP_DR19
};

//-----------------------------------------------------------------------------
HAL_DriverItf* drv_rtc_v[DRV_ID_RTC_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_rtc = {
    .Init   = RTC__Init,
    .DeInit = RTC__DeInit,
    .Open   = RTC_Open,
    .Close  = RTC_Close,
    .Read   = RTC_Read,
    .Write  = RTC_Write,
    .IoCtl  = RTC_IoCtl
};

/*****************************************************************************/
Status RTC_Init_(void)
{
Status s = S_OK;
    HAL_MemSet(drv_rtc_v, 0x0, sizeof(drv_rtc_v));
    drv_rtc_v[DRV_ID_RTC] = &drv_rtc;
    //IF_STATUS(s = drv_rtc[DRV_ID_RTC]->Init()) { return s; }
    return s;
}

/*****************************************************************************/
Status RTC__Init(void* args_p)
{
Status s = S_OK;
    //HAL_LOG(L_INFO, "Init");
    RTC_LL_Init();
    /*##-2- Check if Data stored in BackUp register0: No Need to reconfigure RTC#*/
    /* Read the Back Up Register 0 Data */
    if (RTC_STAMP != HAL_RTCEx_BKUPRead(&rtc_handle, HAL_RTC_BKUP_REG_IS_VALID)) {
        /* Configure RTC Calendar */
        RTC_Reset();
    }
    RTC_AlarmInit();
    RTC_WakeupInit();
    //HAL_TRACE(L_INFO, S_STRING_GET(S_OK));
    return s;
}

/******************************************************************************/
Status RTC_LL_Init(void)
{
RCC_OscInitTypeDef        RCC_OscInitStruct;
RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
Status s = S_OK;
    /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
    /*##-1- Configue LSE as RTC clock soucre ###################################*/
    RCC_OscInitStruct.OscillatorType        = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState          = RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState              = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState              = RCC_LSI_OFF;
    if (HAL_OK != HAL_RCC_OscConfig(&RCC_OscInitStruct)) {
        HAL_ASSERT(OS_FALSE);
    }
    PeriphClkInitStruct.PeriphClockSelection= RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection   = RCC_RTCCLKSOURCE_LSE;
    if (HAL_OK != HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct)) {
        HAL_ASSERT(OS_FALSE);
    }
    /*##-2- Enable RTC peripheral Clocks #######################################*/
    /* Enable RTC Clock */
    __HAL_RCC_RTC_ENABLE();

    rtc_handle.Instance             = RTC;
    rtc_handle.Init.HourFormat      = RTC_HOURFORMAT_24;
    rtc_handle.Init.AsynchPrediv    = RTC_ASYNCH_PREDIV;
    rtc_handle.Init.SynchPrediv     = RTC_SYNCH_PREDIV;
    rtc_handle.Init.OutPut          = RTC_OUTPUT_DISABLE;
    rtc_handle.Init.OutPutPolarity  = RTC_OUTPUT_POLARITY_HIGH;
    rtc_handle.Init.OutPutType      = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_OK != HAL_RTC_Init(&rtc_handle)) {
        HAL_ASSERT(OS_FALSE);
    }
    RTC_SRAM_BACKUP_Init();
    if (HAL_OK != HAL_RTCEx_EnableBypassShadow(&rtc_handle)) {
        HAL_ASSERT(OS_FALSE);
    }
    return s;
}

/******************************************************************************/
void RTC_Reset(void)
{
    RTC_SRAM_RegistersReset();
    RTC_SRAM_BackupReset();
    RTC_CalendarReset();
    HAL_RTCEx_BKUPWrite(&rtc_handle, rtc_backup_regs[HAL_RTC_BKUP_REG_IS_VALID], RTC_STAMP);
}

/******************************************************************************/
void RTC_SRAM_RegistersReset(void)
{
    /* Store the content of BDCR register before the reset of Backup Domain */
    U32 tmpreg = (RCC->BDCR & ~(RCC_BDCR_RTCSEL));
    /* RTC Clock selection can be changed only if the Backup Domain is reset */
    __HAL_RCC_BACKUPRESET_FORCE();
    __HAL_RCC_BACKUPRESET_RELEASE();
    /* Restore the Content of BDCR register */
    RCC->BDCR = tmpreg;

    RTC_LL_Init();
}

/******************************************************************************/
void RTC_SRAM_BackupReset(void)
{
U32 uwIndex, uwErrorIndex = 0;
    /* Write to Backup SRAM with 32-Bit Data */
    for (uwIndex = 0x0; uwIndex < 0x1000; uwIndex += 4) {
        *(HAL_IO U32*) (BKPSRAM_BASE + uwIndex) = (U32)U32_MAX;
    }
    /* Check the written Data */
    for (uwIndex = 0x0; uwIndex < 0x1000; uwIndex += 4) {
        if ((*(HAL_IO U32*) (BKPSRAM_BASE + uwIndex)) != (U32)U32_MAX) {
            uwErrorIndex++;
        }
    }

    if (uwErrorIndex) {
        HAL_TRACE(L_WARNING, "\nBackup SRAM errors = %d", uwErrorIndex);
    }
}

/******************************************************************************/
void RTC_CalendarReset(void)
{
RTC_DateTypeDef date;
RTC_TimeTypeDef time;
    /*##-1- Configure the Date #################################################*/
    date.Year             = 0x00;
    date.Month            = RTC_MONTH_JANUARY;
    date.Date             = 0x00;
    date.WeekDay          = RTC_WEEKDAY_SATURDAY;

    if (HAL_OK != HAL_RTC_SetDate(&rtc_handle, &date, FORMAT_BCD)) {
        HAL_ASSERT(OS_FALSE);
    }
    /*##-2- Configure the Time #################################################*/
    time.Hours            = 0x00;
    time.Minutes          = 0x00;
    time.Seconds          = 0x00;
    time.TimeFormat       = RTC_HOURFORMAT12_AM;
    time.DayLightSaving   = RTC_DAYLIGHTSAVING_NONE;
    time.StoreOperation   = RTC_STOREOPERATION_RESET;

    if (HAL_OK != HAL_RTC_SetTime(&rtc_handle, &time, FORMAT_BCD)) {
        HAL_ASSERT(OS_FALSE);
    }
}

/******************************************************************************/
void RTC_SRAM_BACKUP_Init(void)
{
    __BKPSRAM_CLK_ENABLE();
    __BKPSRAM_CLK_SLEEP_ENABLE();
    if (HAL_OK != HAL_PWREx_EnableBkUpReg()) {
        HAL_ASSERT(OS_FALSE);
    }
}

/*****************************************************************************/
Status RTC_AlarmInit(void)
{
Status s = S_OK;
//    RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
//    RTC_AlarmCmd(RTC_Alarm_B, DISABLE);
//    /* Enable alarm A interrupt */
//    RTC_ITConfig(RTC_IT_ALRA, DISABLE);
//    RTC_ITConfig(RTC_IT_ALRB, DISABLE);
//    RTC_ClearITPendingBit(RTC_IT_ALRA);
//    RTC_ClearITPendingBit(RTC_IT_ALRB);
//    RTC_ClearFlag(RTC_FLAG_ALRAF);
//    RTC_ClearFlag(RTC_FLAG_ALRBF);
    return s;
}

/*****************************************************************************/
Status RTC_WakeupInit(void)
{
Status s = S_OK;
//    //HAL_LOG(L_INFO, "Wakeup init: ");
////    /* Disable the Wakeup detection */
//    RTC_WakeUpCmd(DISABLE);
//    RTC_ClearFlag(RTC_FLAG_WUTF);
////    /* Configure the RTC Wakeup Clock source and Counter (Wakeup event each 1 second) */
////    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
////    RTC_SetWakeUpCounter(0x7FF);
//    s = RTC_NVIC_WakeupInit();
////    /* Enable the Wakeup detection */
////    RTC_WakeUpCmd(ENABLE);
//    //HAL_TRACE_S(L_INFO, s);
    return s;
}

/*****************************************************************************/
Status RTC__DeInit(void* args_p)
{
Status s = S_OK;
    HAL_LOG(L_INFO, "DeInit: ");
    if (HAL_OK != HAL_RTC_DeInit(&rtc_handle)) {
        s = S_HARDWARE_ERROR;
    }
    __HAL_RCC_RTC_DISABLE();
    HAL_TRACE_S(L_INFO, s);
    return s;
}

/******************************************************************************/
Status RTC_Open(void* args_p)
{
Status s = S_OK;
    //TODO(A. Filyanov)
    return s;
}

/******************************************************************************/
Status RTC_Close(void* args_p)
{
Status s = S_OK;
    //TODO(A. Filyanov)
    return s;
}

/******************************************************************************/
/// @details RTC Backup SRAM read.
Status RTC_Read(void* data_in_p, Size size, void* args_p)
{
Status s = S_OK;
    HAL_IO U8* sram_bkup_p = (HAL_IO U8*)BKPSRAM_BASE;
    /* Read the SRAM Backup Data */
    while (size--) {
        U8* data_in_8p = (U8*)data_in_p;
        *data_in_8p++ = *sram_bkup_p++;
    }
    return s;
}

/******************************************************************************/
/// @details RTC Backup SRAM write.
Status RTC_Write(void* data_out_p, Size size, void* args_p)
{
Status s = S_OK;
    HAL_IO U8* sram_bkup_p = (HAL_IO U8*)BKPSRAM_BASE;
    {
        U8* data_out_tmp_p  = data_out_p;
        U32 size_tmp        = size;
        /* Write to Backup SRAM with 8-Bit Data */
        while (size_tmp--) {
            *sram_bkup_p++ = *data_out_tmp_p++;
        }
    }
    sram_bkup_p = (HAL_IO U8*)BKPSRAM_BASE;
    /* Check the written Data */
    while (size--) {
        U8* data_out_8p = (U8*)data_out_p;
        if (*sram_bkup_p++ != *data_out_8p++) { s = S_HARDWARE_ERROR; }
    }
    return s;
}

/******************************************************************************/
Status RTC_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
    switch (request_id) {
        case DRV_REQ_RTC_BKUP_REG_READ: {
            HAL_RTC_BackupRegRead* io_args_p = (HAL_RTC_BackupRegRead*)args_p;
            const U32 reg   = rtc_backup_regs[io_args_p->reg];
            U32* val_p      = io_args_p->val_p;
            *val_p          = HAL_RTCEx_BKUPRead(&rtc_handle, reg);
            s = S_OK;
            }
            break;
        case DRV_REQ_RTC_BKUP_REG_WRITE: {
            HAL_RTC_BackupRegWrite* io_args_p = (HAL_RTC_BackupRegWrite*)args_p;
            const U32 reg = rtc_backup_regs[io_args_p->reg];
            const U32 val = io_args_p->val;
            HAL_RTCEx_BKUPWrite(&rtc_handle, reg, val);
            if (val == HAL_RTCEx_BKUPRead(&rtc_handle, reg)) {
                s = S_OK;
            } else {
                s = S_HARDWARE_ERROR;
            }
            }
            break;
        case DRV_REQ_RTC_BKUP_REGS_READ:
            s = S_OK;
            break;
        case DRV_REQ_RTC_BKUP_REGS_WRITE:
            s = S_OK;
            break;
        case DRV_REQ_RTC_ALARM_A_SET:
            s = S_OK;
            break;
        case DRV_REQ_RTC_ALARM_B_SET:
            s = S_OK;
            break;
        case DRV_REQ_RTC_TIME_GET: {
            RTC_TimeTypeDef time;
            if (HAL_OK == HAL_RTC_GetTime(&rtc_handle, &time, FORMAT_BIN)) {
// Workaround {
//  * @note   Call HAL_RTC_GetDate() after HAL_RTC_GetTime() to unlock the values
//  *         in the higher-order calendar shadow registers.
                RTC_DateTypeDef date;
                HAL_RTC_GetDate(&rtc_handle, &date, FORMAT_BIN);
// Workaround }
                OS_DateTime* os_time_p = (OS_DateTime*)args_p;
                if (OS_NULL != os_time_p) {
                    os_time_p->hours        = time.Hours;
                    os_time_p->minutes      = time.Minutes;
                    os_time_p->seconds      = time.Seconds;
                    os_time_p->daylight     = time.DayLightSaving;
                    os_time_p->hourformat   = time.TimeFormat;
                    s = S_OK;
                } else { s = S_INVALID_PTR; }
            } else { s = S_HARDWARE_ERROR; }
            }
            break;
        case DRV_REQ_RTC_TIME_SET: {
            OS_DateTime* os_time_p = (OS_DateTime*)args_p;
            if (OS_NULL != os_time_p) {
                RTC_TimeTypeDef time;
                HAL_MemSet((void*)&time, 0, sizeof(time));
                time.Hours          = os_time_p->hours;
                time.Minutes        = os_time_p->minutes;
                time.Seconds        = os_time_p->seconds;
                time.DayLightSaving = os_time_p->daylight;
                time.TimeFormat     = os_time_p->hourformat;
                time.StoreOperation = RTC_STOREOPERATION_SET;
                if (HAL_OK == HAL_RTC_SetTime(&rtc_handle, &time, FORMAT_BIN)) {
                    s = S_OK;
                } else { s = S_HARDWARE_ERROR; }
            } else { s = S_INVALID_PTR; }
            }
            break;
        case DRV_REQ_RTC_DATE_GET: {
            RTC_DateTypeDef date;
            if (HAL_OK == HAL_RTC_GetDate(&rtc_handle, &date, FORMAT_BIN)) {
                OS_DateTime* os_date_p = (OS_DateTime*)args_p;
                if (OS_NULL != os_date_p) {
                    os_date_p->year     = date.Year + HAL_RTC_YEAR_BASE;
                    os_date_p->month    = date.Month;
                    os_date_p->weekday  = date.WeekDay;
                    os_date_p->day      = date.Date;
                    s = S_OK;
                } else { s = S_INVALID_PTR; }
            } else { s = S_HARDWARE_ERROR; }
            }
            break;
        case DRV_REQ_RTC_DATE_SET: {
            OS_DateTime* os_date_p = (OS_DateTime*)args_p;
            if (OS_NULL != os_date_p) {
                RTC_DateTypeDef date;
                HAL_MemSet((void*)&date, 0, sizeof(date));
                date.Year           = os_date_p->year - HAL_RTC_YEAR_BASE;
                date.Month          = os_date_p->month;
                date.WeekDay        = os_date_p->weekday;
                date.Date           = os_date_p->day;
                if (HAL_OK == HAL_RTC_SetDate(&rtc_handle, &date, FORMAT_BIN)) {
                    s = S_OK;
                } else { s = S_HARDWARE_ERROR; }
            } else { s = S_INVALID_PTR; }
            }
            break;
        case DRV_REQ_STD_POWER_SET:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    s = S_OK;
                    break;
                case PWR_OFF:
                case PWR_STOP:
                case PWR_SHUTDOWN:
                    s = S_OK;
                    break;
                default:
                    break;
            }
            break;
        default:
            s = S_INVALID_REQ_ID;
            break;
    }
    return s;
}

// RTC IRQ handlers ------------------------------------------------------------
/******************************************************************************/
