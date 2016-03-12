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

#if defined(OS_MEDIA_VOL_SDCARD) && (HAL_SDIO_SD_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_m_sdcard"

#define LED_FS_ON()         OS_DriverWrite(args_init.drv_gpio, (void*)args_init.gpio_led, 0, (void*)ON)
#define LED_FS_OFF()        OS_DriverWrite(args_init.drv_gpio, (void*)args_init.gpio_led, 0, (void*)OFF)

//-----------------------------------------------------------------------------
/** @defgroup STM324xG_EVAL_SD_Exported_Constants
  * @{
  */
enum {
    SD_NOT_PRESENT,
    SD_PRESENT
};

enum {
    SD_POWER_STATE_OFF,
    SD_POWER_STATE_UP   = 2,
    SD_POWER_STATE_ON   = 3
};

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
static DrvMediaArgsInit args_init;

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
Status s = S_UNDEF;
    HAL_LOG(L_INFO, "Init: ");
    args_init = *(DrvMediaArgsInit*)args_p;
    /* Enable SDIO clock */
    HAL_SD_CLK_ENABLE();
    IF_STATUS(s = SDIO_LL_Init(args_p)) { return s; }
    /* Check if the SD card is plugged in the slot */
    if (OS_TRUE == SD_IsDetected()) {
        HAL_SD_CardInfoTypedef sd_card_info;

        sd_hd.Instance                = HAL_SD_ITF;
        sd_hd.Init.ClockEdge          = HAL_SDIO_SD_CLOCK_EDGE;
        sd_hd.Init.ClockBypass        = HAL_SDIO_SD_CLOCK_BYPASS;
        sd_hd.Init.ClockPowerSave     = HAL_SDIO_SD_CLOCK_POWER_SAVE;
        sd_hd.Init.BusWide            = HAL_SDIO_SD_BUS_WIDE;
        //Do _NOT_ use HardwareFlowControl due 2.9.1 SDIO HW flow control errata.
        sd_hd.Init.HardwareFlowControl= HAL_SDIO_SD_HW_FLOW_CONTROL;
        sd_hd.Init.ClockDiv           = HAL_SDIO_SD_CLOCK_DIV;
        if (SD_OK != HAL_SD_Init(&sd_hd, &sd_card_info)) { s = S_HARDWARE_ERROR; }

//TODO(A. Filyanov) Wrong bus width detection!
//        HAL_SD_CardStatusTypedef sd_card_status;
//        HAL_SD_GetCardStatus(&sd_hd, &sd_card_status);
//
//        if (4 == sd_card_status.DAT_BUS_WIDTH) {
        //Force switch to 4bit wide bus.
            if (SD_OK != HAL_SD_WideBusOperation_Config(&sd_hd, SDIO_BUS_WIDE_4B)) {
                s = S_HARDWARE_ERROR;
            }
//        }
    } else { s = S_HARDWARE_ERROR; }
    return s;
}

/*****************************************************************************/
Status SDIO_LL_Init(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStruct;
Status s = S_OK;
    /* Enable GPIOs clock */
    HAL_SD_GPIO_CLK_ENABLE();

    /* GPIO1 configuration */
    GPIO_InitStruct.Pin         = HAL_SD_GPIO1_PIN;
    GPIO_InitStruct.Mode        = HAL_SD_GPIO1_MODE;
    GPIO_InitStruct.Pull        = HAL_SD_GPIO1_PULL;
    GPIO_InitStruct.Speed       = HAL_SD_GPIO1_SPEED;
    GPIO_InitStruct.Alternate   = HAL_SD_GPIO1_ALT;
    HAL_GPIO_Init(HAL_SD_GPIO1_PORT, &GPIO_InitStruct);

    /* GPIO2 configuration */
    GPIO_InitStruct.Pin         = HAL_SD_GPIO2_PIN;
    GPIO_InitStruct.Mode        = HAL_SD_GPIO2_MODE;
    GPIO_InitStruct.Pull        = HAL_SD_GPIO2_PULL;
    GPIO_InitStruct.Speed       = HAL_SD_GPIO2_SPEED;
    GPIO_InitStruct.Alternate   = HAL_SD_GPIO2_ALT;
    HAL_GPIO_Init(HAL_SD_GPIO2_PORT, &GPIO_InitStruct);
//#ifndef HAL_MB_OLIMEX_STM32_P407 // HAL_MB_OLIMEX_STM32_P407 HAS NO SD DETECT PIN!
//    __SD_DETECT_GPIO_CLK_ENABLE();
//    /* SD Card detect pin configuration */
//    GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull      = GPIO_PULLUP;
//    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
//    GPIO_InitStruct.Pin       = SD_DETECT_PIN;
//    HAL_GPIO_Init(SD_DETECT_GPIO_PORT, &GPIO_InitStruct);
//#endif //HAL_MB_OLIMEX_STM32_P407

    /* Enable DMA clocks */
    HAL_SD_DMA_CLK();

    /* Configure DMA Rx parameters */
    sd_dma_rx_handle.Instance                   = HAL_SD_DMA_STREAM_RX;
    sd_dma_rx_handle.Init.Channel               = HAL_SD_DMA_CHAN_RX;
    sd_dma_rx_handle.Init.Direction             = HAL_SD_DMA_DIRECTION_RX;
    sd_dma_rx_handle.Init.PeriphInc             = HAL_SD_DMA_PERIPH_INC_RX;
    sd_dma_rx_handle.Init.MemInc                = HAL_SD_DMA_MEMORY_INC_RX;
#if (OS_FILE_SYSTEM_WORD_ACCESS)
    sd_dma_rx_handle.Init.MemDataAlignment      = HAL_SD_DMA_MEMORY_DATA_ALIGN_RX_WORD;
#else
    sd_dma_rx_handle.Init.MemDataAlignment      = HAL_SD_DMA_MEMORY_DATA_ALIGN_RX_BYTE;
#endif //(OS_FILE_SYSTEM_WORD_ACCESS)
    sd_dma_rx_handle.Init.PeriphDataAlignment   = HAL_SD_DMA_PERIPH_DATA_ALIGN_RX;
    sd_dma_rx_handle.Init.Mode                  = HAL_SD_DMA_MODE_RX;
    sd_dma_rx_handle.Init.Priority              = HAL_PRIO_DMA_SDIO_RX;
    sd_dma_rx_handle.Init.FIFOMode              = HAL_SD_DMA_FIFO_MODE_RX;
    sd_dma_rx_handle.Init.FIFOThreshold         = HAL_SD_DMA_FIFO_THRS_RX;
#if (OS_FILE_SYSTEM_WORD_ACCESS)
    sd_dma_rx_handle.Init.MemBurst              = HAL_SD_DMA_MEMORY_BURST_RX_WORD;
#else
    sd_dma_rx_handle.Init.MemBurst              = HAL_SD_DMA_MEMORY_BURST_RX_BYTE;
#endif //(OS_FILE_SYSTEM_WORD_ACCESS)
    sd_dma_rx_handle.Init.PeriphBurst           = HAL_SD_DMA_PERIPH_BURST_RX;
    /* Associate the DMA handle */
    __HAL_LINKDMA(&sd_hd, hdmarx, sd_dma_rx_handle);

    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&sd_dma_rx_handle);

    /* Configure the DMA stream */
    HAL_DMA_Init(&sd_dma_rx_handle);

    /* Configure DMA Tx parameters */
    sd_dma_tx_handle.Instance                   = HAL_SD_DMA_STREAM_TX;
    sd_dma_tx_handle.Init.Channel               = HAL_SD_DMA_CHAN_TX;
    sd_dma_tx_handle.Init.Direction             = DMA_MEMORY_TO_PERIPH;
    sd_dma_tx_handle.Init.PeriphInc             = HAL_SD_DMA_PERIPH_INC_TX;
    sd_dma_tx_handle.Init.MemInc                = HAL_SD_DMA_MEMORY_INC_TX;
#if (OS_FILE_SYSTEM_WORD_ACCESS)
    sd_dma_tx_handle.Init.MemDataAlignment      = HAL_SD_DMA_MEMORY_DATA_ALIGN_TX_WORD;
#else
    sd_dma_tx_handle.Init.MemDataAlignment      = HAL_SD_DMA_MEMORY_DATA_ALIGN_TX_BYTE;
#endif //(OS_FILE_SYSTEM_WORD_ACCESS)
    sd_dma_tx_handle.Init.PeriphDataAlignment   = HAL_SD_DMA_PERIPH_DATA_ALIGN_TX;
    sd_dma_tx_handle.Init.Mode                  = HAL_SD_DMA_MODE_TX;
    sd_dma_tx_handle.Init.Priority              = HAL_PRIO_DMA_SDIO_TX;
    sd_dma_tx_handle.Init.FIFOMode              = HAL_SD_DMA_FIFO_MODE_TX;
    sd_dma_tx_handle.Init.FIFOThreshold         = HAL_SD_DMA_FIFO_THRS_TX;
#if (OS_FILE_SYSTEM_WORD_ACCESS)
    sd_dma_tx_handle.Init.MemBurst              = HAL_SD_DMA_MEMORY_BURST_TX_WORD;
#else
    sd_dma_tx_handle.Init.MemBurst              = HAL_SD_DMA_MEMORY_BURST_TX_BYTE;
#endif //(OS_FILE_SYSTEM_WORD_ACCESS)
    sd_dma_tx_handle.Init.PeriphBurst           = HAL_SD_DMA_PERIPH_BURST_TX;
    /* Associate the DMA handle */
    __HAL_LINKDMA(&sd_hd, hdmatx, sd_dma_tx_handle);

    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&sd_dma_tx_handle);

    /* Configure the DMA stream */
    HAL_DMA_Init(&sd_dma_tx_handle);

    /* NVIC configuration for SDIO interrupts */
    HAL_NVIC_SetPriority(HAL_SD_IRQ, HAL_PRIO_IRQ_SDIO, 0);
    HAL_NVIC_EnableIRQ(HAL_SD_IRQ);

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(HAL_SD_DMA_IRQ_RX, HAL_PRIO_IRQ_DMA_SDIO_RX, 0);
    HAL_NVIC_EnableIRQ(HAL_SD_DMA_IRQ_RX);

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(HAL_SD_DMA_IRQ_TX, HAL_PRIO_IRQ_DMA_SDIO_TX, 0);
    HAL_NVIC_EnableIRQ(HAL_SD_DMA_IRQ_TX);
    return s;
}

/*****************************************************************************/
Status SDIO_DeInit_(void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_SD_DeInit(&sd_hd)) { s = S_HARDWARE_ERROR; }
    /* Peripheral clock disable */
    HAL_SD_CLK_DISABLE();

    HAL_GPIO_DeInit(HAL_SD_GPIO1_PORT, HAL_SD_GPIO1_PIN);
    HAL_GPIO_DeInit(HAL_SD_GPIO2_PORT, HAL_SD_GPIO2_PIN);

    /* Peripheral DMA DeInit*/
    HAL_DMA_DeInit(sd_hd.hdmarx);
    HAL_DMA_DeInit(sd_hd.hdmatx);
    return s;
}

/*****************************************************************************/
Status SDIO_Open(void* args_p)
{
Status s = S_OK;
    IF_STATUS(s = OS_DriverOpen(args_init.drv_gpio, OS_NULL)) {}
    return s;
}

/*****************************************************************************/
Status SDIO_Close(void* args_p)
{
Status s = S_OK;
    IF_OK(s = drv_media_sdcard.IoCtl(DRV_REQ_STD_SYNC, OS_NULL)) {
        IF_OK(s = OS_DriverClose(args_init.drv_gpio, OS_NULL)) {}
    }
    return s;
}

/******************************************************************************/
//Status SDIO_Read(void* data_in_p, Size size, void* args_p)
//{
//U32 sector = *(U32*)args_p;
//Status s = S_OK;
//    LED_FS_ON();
//    if (SD_OK != HAL_SD_ReadBlocks(&sd_hd, (U32*)data_in_p, (sector * HAL_SD_CARD_SECTOR_SIZE), HAL_SD_CARD_BLOCK_SIZE, size)) {}
//    LED_FS_OFF();
//    return s;
//}

/******************************************************************************/
//Status SDIO_Write(void* data_out_p, Size size, void* args_p)
//{
//U32 sector = *(U32*)args_p;
//Status s = S_OK;
//    LED_FS_ON();
//    if (SD_OK != HAL_SD_WriteBlocks(&sd_hd, (U32*)data_out_p, (sector * HAL_SD_CARD_SECTOR_SIZE), HAL_SD_CARD_BLOCK_SIZE, size)) {}
//    LED_FS_OFF();
//    return s;
//}

//https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=https%3a%2f%2fmy.st.com%2fpublic%2fSTe2ecommunities%2fmcu%2fLists%2fcortex_mx_stm32%2fST32f4discovery%2bsdcard%2bfatfs%20problem
/******************************************************************************/
Status SDIO_DMA_Read(void* data_in_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
HAL_SD_ErrorTypedef sd_status;
Status s = S_OK;
//    OS_LOG(L_DEBUG_1, "read 0x%X %6d %d", data_in_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
    LED_FS_ON();

#if (OS_FILE_SYSTEM_WORD_ACCESS)
    if ((U32)data_in_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(HAL_SD_CARD_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_OUT_OF_MEMORY; }
        while (size--) {
            U8* data_in_8p = (U8*)data_in_p;
            IF_STATUS(s = drv_media_sdcard.Read((U8*)scratch_p, 1, &sector)) { break; }
            OS_MemCpy(data_in_p, scratch_p, HAL_SD_CARD_BLOCK_SIZE);
            data_in_8p += HAL_SD_CARD_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }
#endif // (OS_FILE_SYSTEM_WORD_ACCESS)

    sd_status = HAL_SD_ReadBlocks_DMA(&sd_hd, (U32*)data_in_p, (sector * HAL_SD_CARD_SECTOR_SIZE), HAL_SD_CARD_BLOCK_SIZE, size);
    if (SD_OK == sd_status) {
        sd_status = HAL_SD_CheckReadOperation(&sd_hd, HAL_SD_TIMEOUT_TRANSFER); // Check if the Transfer is finished
        if (SD_OK != sd_status) {
            s = S_FS_TRANSFER_FAIL;
        }
    } else {
        s = S_FS_TRANSFER_FAIL;
    }
    LED_FS_OFF();
    return s;
}

/******************************************************************************/
Status SDIO_DMA_Write(void* data_out_p, Size size, void* args_p)
{
U32 sector = *(U32*)args_p;
HAL_SD_ErrorTypedef sd_status;
Status s = S_OK;
//    OS_LOG(L_DEBUG_1, "write 0x%X %6d %d", data_out_p, sector, size);
//TODO(A.Filyanov) Test context(ISR) before call.
    LED_FS_ON();

#if (OS_FILE_SYSTEM_WORD_ACCESS)
    if ((U32)data_out_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(HAL_SD_CARD_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_OUT_OF_MEMORY; }
        while (size--) {
            U8* data_out_8p = (U8*)data_out_p;
            OS_MemCpy(data_out_p, scratch_p, HAL_SD_CARD_BLOCK_SIZE);
            IF_STATUS(s = drv_media_sdcard.Write((U8*)scratch_p, 1, &sector)) { break; }
            data_out_8p += HAL_SD_CARD_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }
#endif //(OS_FILE_SYSTEM_WORD_ACCESS)

    sd_status = HAL_SD_WriteBlocks_DMA(&sd_hd, (U32*)data_out_p, (sector * HAL_SD_CARD_SECTOR_SIZE), HAL_SD_CARD_BLOCK_SIZE, size);
    if (SD_OK == sd_status) {
        sd_status = HAL_SD_CheckWriteOperation(&sd_hd, HAL_SD_TIMEOUT_TRANSFER); // Check if the Transfer is finished
        if (SD_OK != sd_status) {
            s = S_FS_TRANSFER_FAIL;
        }
    } else {
        s = S_FS_TRANSFER_FAIL;
    }
    LED_FS_OFF();
    return s;
}

/******************************************************************************/
Status SDIO_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
//TODO(A.Filyanov) Test context(ISR) before call.
//    OS_LOG(L_DEBUG_1, "ioctl req_id=%d", request_id);
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
                *(U32*)args_p = card_info.CardCapacity / HAL_SD_CARD_BLOCK_SIZE;
                s = S_OK;
            } else {
                *(U32*)args_p = 0;
            }
            }
            break;
        case DRV_REQ_MEDIA_SECTOR_SIZE_GET:
        case GET_SECTOR_SIZE:
            *(U16*)args_p = HAL_SD_CARD_SECTOR_SIZE;
            s = S_OK;
            break;
        case DRV_REQ_MEDIA_BLOCK_SIZE_GET:
        case GET_BLOCK_SIZE:
            *(U16*)args_p = HAL_SD_CARD_BLOCK_SIZE;
            s = S_OK;
            break;
        case CTRL_TRIM: {
            //TODO(A. Filyanov) Check SDHC capability!
            U32 start_sector= ((U32*)args_p)[0] * HAL_SD_CARD_SECTOR_SIZE;
            U32 end_sector  = ((U32*)args_p)[1] * HAL_SD_CARD_SECTOR_SIZE;
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
#ifndef HAL_MB_OLIMEX_STM32_P407 // HAL_MB_OLIMEX_STM32_P407 HAS NO SD DETECT PIN!
    /* Check SD card detect pin */
    if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(SD_DETECT_GPIO_PORT, SD_DETECT_PIN)) {
        return OS_FALSE;
    }
#endif //HAL_MB_OLIMEX_STM32_P407
    return OS_TRUE;
}

// IRQ handlers ---------------------------------------------------------------
/*****************************************************************************/
void HAL_SD_IRQ_HANDLER(void);
void HAL_SD_IRQ_HANDLER(void)
{
    HAL_SD_IRQHandler(&sd_hd);
}

/*****************************************************************************/
void HAL_SD_DMA_IRQ_HANDLER_RX(void);
void HAL_SD_DMA_IRQ_HANDLER_RX(void)
{
    HAL_DMA_IRQHandler(sd_hd.hdmarx);
}

/*****************************************************************************/
void HAL_SD_DMA_IRQ_HANDLER_TX(void);
void HAL_SD_DMA_IRQ_HANDLER_TX(void)
{
    HAL_DMA_IRQHandler(sd_hd.hdmatx);
}

#endif //defined(OS_MEDIA_VOL_SDCARD) && (HAL_SDIO_SD_ENABLED)