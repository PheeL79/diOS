/**************************************************************************//**
* @file    drv_media_sdcard.c
* @brief   SDIO SD media driver.
* @author  A. Filyanov
******************************************************************************/
#include <string.h>
#include "hal.h"
#include "diskio.h"
#include "os_common.h"
#include "os_debug.h"
#include "os_driver.h"
#include "os_memory.h"
#include "os_file_system.h"

#if defined(OS_MEDIA_VOL_SDCARD) && (1 == SDIO_SD_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_m_sdcard"

//-----------------------------------------------------------------------------
/** @defgroup STM324xG_EVAL_SD_Exported_Constants
  * @{
  */
#define HAL_TIMEOUT_SD_TRANSFER         ((uint32_t)100000000)
#define HAL_TIMEOUT_USB_TRANSFER        HAL_TIMEOUT_SD_TRANSFER

#define SD_PRESENT                      ((uint8_t)0x01)
#define SD_NOT_PRESENT                  ((uint8_t)0x00)

#define SD_POWER_STATE_OFF              0x00
#define SD_POWER_STATE_UP               0x02
#define SD_POWER_STATE_ON               0x03

/* DMA definitions for SD DMA transfer */
#define SD_NVIC_IRQ_PRIORITY            (OS_PRIORITY_INT_MIN - 1)
#define SD_NVIC_DMA_IRQ_PRIORITY        (OS_PRIORITY_INT_MIN)
#define __DMAx_TxRx_CLK_ENABLE          __DMA2_CLK_ENABLE
#define SD_DMAx_Tx_CHANNEL              DMA_CHANNEL_4
#define SD_DMAx_Rx_CHANNEL              DMA_CHANNEL_4
#define SD_DMAx_Tx_STREAM               DMA2_Stream6
#define SD_DMAx_Rx_STREAM               DMA2_Stream3
#define SD_DMAx_Tx_IRQn                 DMA2_Stream6_IRQn
#define SD_DMAx_Rx_IRQn                 DMA2_Stream3_IRQn
#define SD_DMAx_Tx_IRQHandler           DMA2_Stream6_IRQHandler
#define SD_DMAx_Rx_IRQHandler           DMA2_Stream3_IRQHandler

#ifndef OLIMEX_STM32_P407 // OLIMEX_STM32_P407 HAS NO SD DETECT PIN!
#define SD_DETECT_PIN                   GPIO_PIN_13
#define SD_DETECT_GPIO_PORT             GPIOH
#define __SD_DETECT_GPIO_CLK_ENABLE()   __GPIOH_CLK_ENABLE()
#define SD_DETECT_IRQn                  EXTI15_10_IRQn
#define SD_DetectIRQHandler()           HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13)
#endif // OLIMEX_STM32_P407

//-----------------------------------------------------------------------------
static Status SDIO_Init_(void* args_p);
static Status SDIO_DeInit_(void* args_p);
static Status SDIO_LL_Init(void* args_p);
//static Status SDIO_LL_DeInit(void* args_p);
static Status SDIO_Open(void* args_p);
static Status SDIO_Close(void* args_p);
//static Status SDIO_Read(void* data_in_p, Size size, void* args_p);
//static Status SDIO_Write(void* data_out_p, Size size, void* args_p);
static Status SDIO_DMA_Read(void* data_in_p, Size size, void* args_p);
static Status SDIO_DMA_Write(void* data_out_p, Size size, void* args_p);
static Status SDIO_IoCtl(const U32 request_id, void* args_p);

static Bool SD_IsDetected(void);

//-----------------------------------------------------------------------------
static SD_HandleTypeDef sd_hd;
static DMA_HandleTypeDef sd_dma_rx_handle;
static DMA_HandleTypeDef sd_dma_tx_handle;
static OS_DriverHd drv_led_fs;

//-----------------------------------------------------------------------------
HAL_DriverItf drv_media_sdcard = {
    .Init   = SDIO_Init_,
    .DeInit = SDIO_DeInit_,
    .Open   = SDIO_Open,
    .Close  = SDIO_Close,
    .Read   = SDIO_DMA_Read,
    .Write  = SDIO_DMA_Write,
    .IoCtl  = SDIO_IoCtl
};

/*****************************************************************************/
Status SDIO_Init_(void* args_p)
{
Status s;
    HAL_LOG(D_INFO, "Init: ");
    drv_led_fs = *(OS_DriverHd*)args_p;
    /* Enable SDIO clock */
    __SDIO_CLK_ENABLE();
    IF_STATUS(s = SDIO_LL_Init(args_p)) { return s; }
    /* Check if the SD card is plugged in the slot */
    if (OS_TRUE == SD_IsDetected()) {
        HAL_SD_CardInfoTypedef sd_card_info;

        sd_hd.Instance                = SDIO;
        sd_hd.Init.ClockEdge          = SDIO_CLOCK_EDGE_RISING;
        sd_hd.Init.ClockBypass        = SDIO_CLOCK_BYPASS_DISABLE;
        sd_hd.Init.ClockPowerSave     = SDIO_CLOCK_POWER_SAVE_DISABLE;
        sd_hd.Init.BusWide            = SDIO_BUS_WIDE_1B;
        //Do _NOT_ use HardwareFlowControl due 2.9.1 SDIO HW flow control errata.
        sd_hd.Init.HardwareFlowControl= SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
        sd_hd.Init.ClockDiv           = SDIO_TRANSFER_CLK_DIV;
        if (SD_OK != HAL_SD_Init(&sd_hd, &sd_card_info)) { s = S_HARDWARE_FAULT; }

//TODO(A. Filyanov) Wrong bus width detection!
//        HAL_SD_CardStatusTypedef sd_card_status;
//        HAL_SD_GetCardStatus(&sd_hd, &sd_card_status);
//
//        if (4 == sd_card_status.DAT_BUS_WIDTH) {
            if (SD_OK != HAL_SD_WideBusOperation_Config(&sd_hd, SDIO_BUS_WIDE_4B)) {
                s = S_HARDWARE_FAULT;
            }
//        }
    } else { s = S_HARDWARE_FAULT; }
    return s;
}

/*****************************************************************************/
Status SDIO_LL_Init(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStruct;
    /* Enable GPIOs clock */
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();

    /* Common GPIO configuration */
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;

    /* GPIOC configuration */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* GPIOD configuration */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
#ifndef OLIMEX_STM32_P407 // OLIMEX_STM32_P407 HAS NO SD DETECT PIN!
    __SD_DETECT_GPIO_CLK_ENABLE();
    /* SD Card detect pin configuration */
    GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pin       = SD_DETECT_PIN;
    HAL_GPIO_Init(SD_DETECT_GPIO_PORT, &GPIO_InitStruct);
#endif // OLIMEX_STM32_P407

    /* Enable DMA2 clocks */
    __DMAx_TxRx_CLK_ENABLE();

    /* Configure DMA Rx parameters */
    sd_dma_rx_handle.Init.Channel             = SD_DMAx_Rx_CHANNEL;
    sd_dma_rx_handle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    sd_dma_rx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
    sd_dma_rx_handle.Init.MemInc              = DMA_MINC_ENABLE;
    sd_dma_rx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    sd_dma_rx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    sd_dma_rx_handle.Init.Mode                = DMA_PFCTRL;
    sd_dma_rx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    sd_dma_rx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    sd_dma_rx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    sd_dma_rx_handle.Init.MemBurst            = DMA_MBURST_INC4;
    sd_dma_rx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;

    sd_dma_rx_handle.Instance = SD_DMAx_Rx_STREAM;

    /* Associate the DMA handle */
    __HAL_LINKDMA(&sd_hd, hdmarx, sd_dma_rx_handle);

    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&sd_dma_rx_handle);

    /* Configure the DMA stream */
    HAL_DMA_Init(&sd_dma_rx_handle);

    /* Configure DMA Tx parameters */
    sd_dma_tx_handle.Init.Channel             = SD_DMAx_Tx_CHANNEL;
    sd_dma_tx_handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    sd_dma_tx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
    sd_dma_tx_handle.Init.MemInc              = DMA_MINC_ENABLE;
    sd_dma_tx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    sd_dma_tx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    sd_dma_tx_handle.Init.Mode                = DMA_PFCTRL;
    sd_dma_tx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    sd_dma_tx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    sd_dma_tx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    sd_dma_tx_handle.Init.MemBurst            = DMA_MBURST_INC4;
    sd_dma_tx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;

    sd_dma_tx_handle.Instance = SD_DMAx_Tx_STREAM;

    /* Associate the DMA handle */
    __HAL_LINKDMA(&sd_hd, hdmatx, sd_dma_tx_handle);

    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&sd_dma_tx_handle);

    /* Configure the DMA stream */
    HAL_DMA_Init(&sd_dma_tx_handle);

    /* NVIC configuration for SDIO interrupts */
    HAL_NVIC_SetPriority(SDIO_IRQn, SD_NVIC_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(SD_DMAx_Rx_IRQn, SD_NVIC_DMA_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SD_DMAx_Rx_IRQn);

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(SD_DMAx_Tx_IRQn, SD_NVIC_DMA_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SD_DMAx_Tx_IRQn);
    return S_OK;
}

/*****************************************************************************/
Status SDIO_DeInit_(void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_SD_DeInit(&sd_hd)) { s = S_HARDWARE_FAULT; }
    /* Peripheral clock disable */
    __SDIO_CLK_DISABLE();

    /**SDIO GPIO Configuration
    PC8     ------> SDIO_D0
    PC9     ------> SDIO_D1
    PC10    ------> SDIO_D2
    PC11    ------> SDIO_D3
    PC12    ------> SDIO_CK
    PD2     ------> SDIO_CMD
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    /* Peripheral DMA DeInit*/
    HAL_DMA_DeInit(sd_hd.hdmarx);
    HAL_DMA_DeInit(sd_hd.hdmatx);
    return s;
}

/*****************************************************************************/
Status SDIO_Open(void* args_p)
{
Status s = S_OK;
    IF_STATUS(s = OS_DriverOpen(drv_led_fs, OS_NULL)) {}
    return s;
}

/*****************************************************************************/
Status SDIO_Close(void* args_p)
{
Status s = S_OK;
    IF_OK(s = drv_media_sdcard.IoCtl(DRV_REQ_STD_SYNC, OS_NULL)) {
        IF_OK(s = OS_DriverClose(drv_led_fs, OS_NULL)) {}
    }
    return s;
}

/******************************************************************************/
//Status SDIO_Read(void* data_in_p, Size size, void* args_p)
//{
//U32 sector = *(U32*)args_p;
//State led_fs_state = ON;
//Status s = S_OK;
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    if (SD_OK != HAL_SD_ReadBlocks(&sd_hd, (U32*)data_in_p, (sector * SD_CARD_SECTOR_Size), SD_CARD_BLOCK_Size, size)) {}
//    led_fs_state = OFF;
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    return s;
//}

/******************************************************************************/
//Status SDIO_Write(void* data_out_p, Size size, void* args_p)
//{
//U32 sector = *(U32*)args_p;
//State led_fs_state = ON;
//Status s = S_OK;
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    if (SD_OK != HAL_SD_WriteBlocks(&sd_hd, (U32*)data_out_p, (sector * SD_CARD_SECTOR_Size), SD_CARD_BLOCK_Size, size)) {}
//    led_fs_state = OFF;
//    OS_DriverWrite(drv_led_fs, &led_fs_state, 1, OS_NULL);
//    return s;
//}

//https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=https%3a%2f%2fmy.st.com%2fpublic%2fSTe2ecommunities%2fmcu%2fLists%2fcortex_mx_stm32%2fST32f4discovery%2bsdcard%2bfatfs%20problem
/******************************************************************************/
Status SDIO_DMA_Read(void* data_in_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
HAL_SD_ErrorTypedef sd_status;
State state = ON;
Status s = S_OK;
//    OS_LOG(D_DEBUG, "read 0x%X %6d %d", data_in_p, sector, size);
    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
    if ((U32)data_in_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(SD_CARD_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            U8* data_in_8p = (U8*)data_in_p;
            IF_STATUS(s = drv_media_sdcard.Read((U8*)scratch_p, 1, &sector)) { break; }
            OS_MemCpy(data_in_p, scratch_p, SD_CARD_BLOCK_SIZE);
            data_in_8p += SD_CARD_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

    sd_status = HAL_SD_ReadBlocks_DMA(&sd_hd, (U32*)data_in_p, (sector * SD_CARD_SECTOR_SIZE), SD_CARD_BLOCK_SIZE, size);
    if (SD_OK == sd_status) {
        sd_status = HAL_SD_CheckReadOperation(&sd_hd, HAL_TIMEOUT_SD_TRANSFER); // Check if the Transfer is finished
        if (SD_OK != sd_status) {
            s = S_FS_TRANSFER_FAIL;
        }
    } else {
        s = S_FS_TRANSFER_FAIL;
    }
    state = OFF;
    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status SDIO_DMA_Write(void* data_out_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
HAL_SD_ErrorTypedef sd_status;
State state = ON;
Status s = S_OK;
//    OS_LOG(D_DEBUG, "write 0x%X %6d %d", data_out_p, sector, size);
    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
    if ((U32)data_out_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(SD_CARD_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            U8* data_out_8p = (U8*)data_out_p;
            OS_MemCpy(data_out_p, scratch_p, SD_CARD_BLOCK_SIZE);
            IF_STATUS(s = drv_media_sdcard.Write((U8*)scratch_p, 1, &sector)) { break; }
            data_out_8p += SD_CARD_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

    sd_status = HAL_SD_WriteBlocks_DMA(&sd_hd, (U32*)data_out_p, (sector * SD_CARD_SECTOR_SIZE), SD_CARD_BLOCK_SIZE, size);
    if (SD_OK == sd_status) {
        sd_status = HAL_SD_CheckWriteOperation(&sd_hd, HAL_TIMEOUT_SD_TRANSFER); // Check if the Transfer is finished
        if (SD_OK != sd_status) {
            s = S_FS_TRANSFER_FAIL;
        }
    } else {
        s = S_FS_TRANSFER_FAIL;
    }
    state = OFF;
    OS_DriverWrite(drv_led_fs, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status SDIO_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
//    OS_LOG(D_DEBUG, "ioctl req_id=%d", request_id);
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET: {
            HAL_StatusTypeDef hal_status = HAL_OK;
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    if (SD_POWER_STATE_OFF == SDIO_GetPowerState(sd_hd.Instance)) {
                        hal_status = SDIO_PowerState_ON(sd_hd.Instance);
                    }
                    break;
                case PWR_OFF: {
                    if (SD_POWER_STATE_ON == SDIO_GetPowerState(sd_hd.Instance)) {
                        hal_status = SDIO_PowerState_OFF(sd_hd.Instance);
                    } else if (SD_POWER_STATE_UP == SDIO_GetPowerState(sd_hd.Instance)) {
                        while (SD_POWER_STATE_ON != SDIO_GetPowerState(sd_hd.Instance)) {};
                        hal_status = SDIO_PowerState_OFF(sd_hd.Instance);
                    }
                    }
                    break;
                default:
                    break;
            }
            if (HAL_OK == hal_status) {
                s = S_OK;
            } else {
                s = S_FS_UNDEF;
            }
            }
            break;
        case CTRL_POWER: {
            HAL_StatusTypeDef hal_status = HAL_OK;
            //Power Get
            //....
            //Power Set
            const U8 power_state = (*(U8*)args_p) & 0xFF;
                switch (power_state) {
                    case PWR_ON:
                        hal_status = SDIO_PowerState_ON(sd_hd.Instance);
                        break;
                    case PWR_OFF:
                        hal_status = SDIO_PowerState_OFF(sd_hd.Instance);
                        break;
                    default:
                        break;
                }
                if (HAL_OK == hal_status) {
                    s = S_OK;
                } else {
                    s = S_FS_UNDEF;
                }
            }
            break;
        case DRV_REQ_STD_SYNC:
        case CTRL_SYNC:
            while (SD_TRANSFER_OK != HAL_SD_GetStatus(&sd_hd)) {};
            s = S_OK;
            break;
        case DRV_REQ_MEDIA_STATUS_GET:
            if (OS_TRUE != SD_IsDetected()) {
                s = S_FS_NOT_READY;
            } else {
                s = S_OK;
            }
            break;
        case DRV_REQ_MEDIA_SECTOR_COUNT_GET:
        case GET_SECTOR_COUNT: {
            HAL_SD_CardInfoTypedef card_info;

            if (SD_OK == HAL_SD_Get_CardInfo(&sd_hd, &card_info)) {
                *(U32*)args_p = card_info.CardCapacity / SD_CARD_BLOCK_SIZE;
                s = S_OK;
            } else {
                *(U32*)args_p = 0;
            }
            }
            break;
        case DRV_REQ_MEDIA_SECTOR_SIZE_GET:
        case GET_SECTOR_SIZE:
            *(U16*)args_p = SD_CARD_SECTOR_SIZE;
            s = S_OK;
            break;
        case DRV_REQ_MEDIA_BLOCK_SIZE_GET:
        case GET_BLOCK_SIZE:
            *(U16*)args_p = SD_CARD_BLOCK_SIZE;
            s = S_OK;
            break;
        case CTRL_ERASE_SECTOR: {
            //TODO(A. Filyanov) Check SDHC capability!
            U32 start_sector= ((U32*)args_p)[0] * SD_CARD_SECTOR_SIZE;
            U32 end_sector  = ((U32*)args_p)[1] * SD_CARD_SECTOR_SIZE;
            if (SD_OK != HAL_SD_Erase(&sd_hd, start_sector, end_sector)) {
                s = S_FS_UNDEF;
            } else {
                s = S_OK;
            }
            }
            break;
        default:
            s = S_FS_UNDEF;
            break;
    }
    return s;
}

/*****************************************************************************/
Bool SD_IsDetected(void)
{
#ifndef OLIMEX_STM32_P407 // OLIMEX_STM32_P407 HAS NO SD DETECT PIN!
    /* Check SD card detect pin */
    if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(SD_DETECT_GPIO_PORT, SD_DETECT_PIN)) {
        return OS_FALSE;
    }
#endif // OLIMEX_STM32_P407
    return OS_TRUE;
}

// SDIO IRQ handlers -----------------------------------------------------------
/*****************************************************************************/
void SDIO_IRQHandler(void);
void SDIO_IRQHandler(void)
{
    HAL_SD_IRQHandler(&sd_hd);
}

/*****************************************************************************/
void SD_DMAx_Rx_IRQHandler(void);
void SD_DMAx_Rx_IRQHandler(void)
{
    HAL_DMA_IRQHandler(sd_hd.hdmarx);
}

/*****************************************************************************/
void SD_DMAx_Tx_IRQHandler(void);
void SD_DMAx_Tx_IRQHandler(void)
{
    HAL_DMA_IRQHandler(sd_hd.hdmatx);
}

#endif //defined(OS_MEDIA_VOL_SDCARD) && (1 == SDIO_SD_ENABLED)