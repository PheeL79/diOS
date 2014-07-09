/**************************************************************************//**
* @file    drv_rtc.c
* @brief   RTC driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "stm32f4xx_rtc.h"
#include "os_supervise.h"
#include "os_power.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_rtc"

#define RTC_STAMP           0xC01DC0FEUL

//-----------------------------------------------------------------------------
/// @brief   RTC initialize.
/// @return  #Status.
Status RTC_Init_(void);

static Status   RTC__Init(void);
static Status   RTC__DeInit(void);
static Status   RTC_Open(void* args_p);
static Status   RTC_Close(void);
static Status   RTC_Read(U8* data_in_p, U32 size, void* args_p);
static Status   RTC_Write(U8* data_out_p, U32 size, void* args_p);
static Status   RTC_IoCtl(const U32 request_id, void* args_p);

static void     RTC_NVIC_Init(void);
static void     RTC_SRAM_BACKUP_Init(void);
static Status   RTC_AlarmInit(void);
static Status   RTC_WakeupInit(void);
static Status   RTC_NVIC_WakeupInit(void);

static void     RTC_LowLevelInit(void);
static void     RTC_Reset(void);
static void     RTC_SRAM_RegistersReset(void);
static void     RTC_SRAM_BackupReset(void);
static void     RTC_CalendarReset(void);

//-----------------------------------------------------------------------------
const U32 rtc_backup_regs[RTC_BACKUP_REGS_MAX] = {
    RTC_BKP_DR0, RTC_BKP_DR1, RTC_BKP_DR2,
    RTC_BKP_DR3, RTC_BKP_DR4, RTC_BKP_DR5,
    RTC_BKP_DR6, RTC_BKP_DR7, RTC_BKP_DR8,
    RTC_BKP_DR9, RTC_BKP_DR10, RTC_BKP_DR11,
    RTC_BKP_DR12, RTC_BKP_DR13, RTC_BKP_DR14,
    RTC_BKP_DR15, RTC_BKP_DR16, RTC_BKP_DR17,
    RTC_BKP_DR18,  RTC_BKP_DR19
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
    memset(drv_rtc_v, 0x0, sizeof(drv_rtc_v));
    drv_rtc_v[DRV_ID_RTC] = &drv_rtc;
    return S_OK;
}

/*****************************************************************************/
Status RTC__Init(void)
{
    //D_LOG(D_INFO, "Init");
    OS_CriticalSectionEnter(); {
        /* Enable the PWR APB1 Clock Interface */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        if (RTC_STAMP != RTC_ReadBackupRegister(rtc_backup_regs[HAL_RTC_BKUP_REG_IS_VALID])) {
            RTC_Reset();
        } else {
            RTC_LowLevelInit();
        }
        //Readout time and date values from shadow register to fill it with the actual ones.
        RTC->TR;
        RTC->DR;
        RTC_AlarmInit();
        RTC_WakeupInit();
        RTC_NVIC_Init();
        //RTC_BypassShadowCmd(ENABLE);
    } OS_CriticalSectionExit();
    //D_TRACE(D_INFO, S_STRING_GET(S_OK));
    return S_OK;
}

/******************************************************************************/
void RTC_NVIC_Init(void)
{
NVIC_InitTypeDef NVIC_InitStructure;
    //D_LOG(D_INFO, "NVIC Init: ");
    NVIC_StructInit(&NVIC_InitStructure);
    // RTC Alarm IRQ channel configuration
    NVIC_InitStructure.NVIC_IRQChannel                  = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MIN;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Configure interrupts for RTC.
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    RTC_ClearITPendingBit(RTC_IT_ALRB);
    //D_TRACE_S(D_INFO, S_OK);
}

/******************************************************************************/
void RTC_LowLevelInit(void)
{
    /* Enable the PWR APB1 Clock Interface */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);
#ifdef RTC_LSI
    /* The RTC Clock may varies due to LSI frequency dispersion. */
    /* Enable the LSI OSC */
    RCC_LSICmd(ENABLE);
    /* Wait till LSI is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {};
    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#elif RTC_LSE
    /* Enable the LSE OSC */
    RCC_LSEConfig(RCC_LSE_ON);
    /* Wait till LSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {};
    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#else
#   error "drv_rtc.c: Please select the RTC Clock source!"
#endif // RTC_CLOCK_SOURCE
    /* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
    __IO U32 uwSynchPrediv = 0xFF;
    __IO U32 uwAsynchPrediv= 0x7F;
    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();
    RTC_InitTypeDef RTC_InitStructure;
    RTC_InitStructure.RTC_AsynchPrediv  = uwAsynchPrediv;
    RTC_InitStructure.RTC_SynchPrediv   = uwSynchPrediv;
    RTC_InitStructure.RTC_HourFormat    = RTC_HourFormat_24;
    RTC_Init(&RTC_InitStructure);
    RTC_SRAM_BACKUP_Init();
}

/******************************************************************************/
void RTC_Reset(void)
{
    RTC_SRAM_RegistersReset();
    RTC_SRAM_BackupReset();
    RTC_CalendarReset();
    RTC_WriteBackupRegister(rtc_backup_regs[HAL_RTC_BKUP_REG_IS_VALID], RTC_STAMP);
}

/******************************************************************************/
void RTC_SRAM_RegistersReset(void)
{
    RCC_BackupResetCmd(ENABLE);
    RCC_BackupResetCmd(DISABLE);
    RTC_LowLevelInit();
}

/******************************************************************************/
void RTC_SRAM_BackupReset(void)
{
U32 uwIndex, uwErrorIndex = 0;
    /* Write to Backup SRAM with 32-Bit Data */
    for (uwIndex = 0x0; uwIndex < 0x1000; uwIndex += 4) {
        *(__IO U32*) (BKPSRAM_BASE + uwIndex) = (U32)U32_MAX;
        //OS_ContextSwitchForce();
    }
    /* Check the written Data */
    for (uwIndex = 0x0; uwIndex < 0x1000; uwIndex += 4) {
        if ((*(__IO U32*) (BKPSRAM_BASE + uwIndex)) != (U32)U32_MAX) {
            uwErrorIndex++;
        }
        //OS_ContextSwitchForce();
    }

    if (uwErrorIndex) {
        D_TRACE(D_WARNING, "\nBackup SRAM errors = %d", uwErrorIndex);
    }
}

/******************************************************************************/
void RTC_CalendarReset(void)
{
RTC_TimeTypeDef RTC_TimeStructure;
RTC_DateTypeDef RTC_DateStructure;
    /* Set the Time */
    RTC_TimeStructure.RTC_Hours         = 0x00;
    RTC_TimeStructure.RTC_Minutes       = 0x00;
    RTC_TimeStructure.RTC_Seconds       = 0x00;
    /* Set the Date */
    /* Set the date: Friday January 11th 2013 */
    RTC_DateStructure.RTC_Month         = RTC_Month_January;
    RTC_DateStructure.RTC_Date          = 0x01;
    RTC_DateStructure.RTC_Year          = 0x00;
    RTC_DateStructure.RTC_WeekDay       = RTC_Weekday_Saturday;
    /* Calendar Configuration */
    /* Set Current Time and Date */
    RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
    RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
    //Reset the daylight savings.
    RTC_DayLightSavingConfig(RTC_DayLightSaving_ADD1H, RTC_StoreOperation_Set);
    RTC_DayLightSavingConfig(RTC_DayLightSaving_SUB1H, RTC_StoreOperation_Set);
}

/******************************************************************************/
void RTC_SRAM_BACKUP_Init(void)
{
    /* Enable BKPRAM Clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
    /* Enable the Backup SRAM low power Regulator to retain it's content in VBAT mode */
    PWR_BackupRegulatorCmd(ENABLE);
    /* Wait until the Backup SRAM low power Regulator is ready */
    while (PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET) {};
}

/*****************************************************************************/
Status RTC_AlarmInit(void)
{
Status s = S_OK;
    RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
    RTC_AlarmCmd(RTC_Alarm_B, DISABLE);
    /* Enable alarm A interrupt */
    RTC_ITConfig(RTC_IT_ALRA, DISABLE);
    RTC_ITConfig(RTC_IT_ALRB, DISABLE);
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    RTC_ClearITPendingBit(RTC_IT_ALRB);
    RTC_ClearFlag(RTC_FLAG_ALRAF);
    RTC_ClearFlag(RTC_FLAG_ALRBF);
    return s;
}

/*****************************************************************************/
Status RTC_WakeupInit(void)
{
Status s = S_OK;

    //D_LOG(D_INFO, "Wakeup init: ");
//    /* Disable the Wakeup detection */
    RTC_WakeUpCmd(DISABLE);
    RTC_ClearFlag(RTC_FLAG_WUTF);
//    /* Configure the RTC Wakeup Clock source and Counter (Wakeup event each 1 second) */
//    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
//    RTC_SetWakeUpCounter(0x7FF);
    s = RTC_NVIC_WakeupInit();
//    /* Enable the Wakeup detection */
//    RTC_WakeUpCmd(ENABLE);
    //D_TRACE_S(D_INFO, s);
    return s;
}

/*****************************************************************************/
Status RTC_NVIC_WakeupInit(void)
{
EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

    //D_LOG(D_INFO, "NVIC Wakeup Init: ");
//    EXTI_StructInit(&EXTI_InitStructure);
//    EXTI_InitStructure.EXTI_Line    = EXTI_Line13;
//    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Event;
//    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&EXTI_InitStructure);

    NVIC_StructInit(&NVIC_InitStructure);
    // RTC Wakeup IRQ channel configuration
    NVIC_InitStructure.NVIC_IRQChannel                  = RTC_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MAX;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Clear Wakeup pin Event(WUTF) pending flag */
    RTC_ClearFlag(RTC_FLAG_WUTF);
    /* Enable the Wakeup interrupt */
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    /* Enable The external line22 interrupt */
    EXTI_ClearITPendingBit(EXTI_Line22);
    /* Clear Wakeup pin interrupt pending bit */
    RTC_ClearITPendingBit(RTC_IT_WUT);
    return S_OK;
}

/*****************************************************************************/
Status RTC__DeInit(void)
{
Status s = S_OK;
    D_LOG(D_INFO, "DeInit: ");
    if (ERROR == RTC_DeInit()) {
        s = S_HARDWARE_FAULT;
    }
    D_TRACE_S(D_INFO, s);
    return s;
}

/******************************************************************************/
Status RTC_Open(void* args_p)
{
    //TODO(A. Filyanov)
    return S_OK;
}

/******************************************************************************/
Status RTC_Close(void)
{
    //TODO(A. Filyanov)
    return S_OK;
}

/******************************************************************************/
/// @details RTC Backup SRAM read.
Status RTC_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    __IO U8* sram_bkup_p = (__IO U8*)BKPSRAM_BASE;
    /* Read the SRAM Backup Data */
    while (size--) {
        *data_in_p++ = *sram_bkup_p++;
    }
    return s;
}

/******************************************************************************/
/// @details RTC Backup SRAM write.
Status RTC_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    __IO U8* sram_bkup_p = (__IO U8*)BKPSRAM_BASE;
    {
        U8* data_out_tmp_p  = data_out_p;
        U32 size_tmp        = size;
        /* Write to Backup SRAM with 8-Bit Data */
        while (size_tmp--) {
            *sram_bkup_p++ = *data_out_tmp_p++;
        }
    }
    sram_bkup_p = (__IO U8*)BKPSRAM_BASE;
    /* Check the written Data */
    while (size--) {
        if (*sram_bkup_p++ != *data_out_p++) { s = S_HARDWARE_FAULT; }
    }
    return s;
}

/******************************************************************************/
Status RTC_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_RTC_BKUP_REG_READ: {
            HAL_RTC_BackupRegRead* io_args_p = (HAL_RTC_BackupRegRead*)args_p;
            const U32 reg   = rtc_backup_regs[io_args_p->reg];
            U32* val_p      = io_args_p->val_p;
            *val_p = RTC_ReadBackupRegister(reg);
            }
            break;
        case DRV_REQ_RTC_BKUP_REG_WRITE: {
            HAL_RTC_BackupRegWrite* io_args_p = (HAL_RTC_BackupRegWrite*)args_p;
            const U32 reg = rtc_backup_regs[io_args_p->reg];
            const U32 val = io_args_p->val;
            RTC_WriteBackupRegister(reg, val);
            if (val != RTC_ReadBackupRegister(reg)) { s = S_HARDWARE_FAULT; }
            }
            break;
        case DRV_REQ_RTC_BKUP_REGS_READ:
            break;
        case DRV_REQ_RTC_BKUP_REGS_WRITE:
            break;
        case DRV_REQ_RTC_ALARM_A_SET:
            break;
        case DRV_REQ_RTC_ALARM_B_SET:
            break;
        case DRV_REQ_RTC_TIME_GET:
            break;
        case DRV_REQ_RTC_TIME_SET:
            break;
        case DRV_REQ_RTC_DATE_GET:
            break;
        case DRV_REQ_RTC_DATE_SET:
            break;
        case DRV_REQ_STD_POWER:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    break;
                case PWR_OFF:
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

// RTC IRQ handlers ------------------------------------------------------------
/******************************************************************************/
void RTC_WKUP_IRQHandler(void);
void RTC_WKUP_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line22);
    if (RESET != RTC_GetFlagStatus(RTC_FLAG_WUTF)) {
        RTC_ClearITPendingBit(RTC_IT_WUT);
        /* Clear Wakeup pin Event pending flag */
        RTC_ClearFlag(RTC_FLAG_WUTF);
        /* Disable Wakeup pin */
        RTC_WakeUpCmd(DISABLE);
        /* Enable Wakeup pin */
        RTC_WakeUpCmd(ENABLE);
//        if (NULL != wakeup_irq_callback_func) {
//            wakeup_irq_callback_func();
//        }
    }
}

/******************************************************************************/
void RTC_Alarm_IRQHandler(void);
void RTC_Alarm_IRQHandler(void)
{
    if (RESET != RTC_GetITStatus(RTC_IT_ALRA)) {
        RTC_ClearITPendingBit(RTC_IT_ALRA);
    } else if (RESET != RTC_GetITStatus(RTC_IT_ALRB)) {
        RTC_ClearITPendingBit(RTC_IT_ALRB);
    }
}