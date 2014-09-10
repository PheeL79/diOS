/**************************************************************************//**
* @file    drv_sdio_sd.c
* @brief   SDIO SD driver.
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

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_sdio_sd"

//-----------------------------------------------------------------------------
#if (1 == SDIO_SD_ENABLED)
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
#define SD_NVIC_IRQ_PRIORITY            (OS_PRIORITY_MIN + 1)
#define SD_NVIC_DMA_IRQ_PRIORITY        (OS_PRIORITY_MIN + 2)
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

#endif // SDIO_SD_ENABLED

//-----------------------------------------------------------------------------
/// @brief   SDIO initialization.
/// @return  #Status.
Status SDIO__Init(void);

static Status SDIO_Init_(void* args_p);
static Status SDIO_DMA_Init(void);
static Status SDIO_GPIO_Init(void);
static Status SDIO_NVIC_Init(void);
static Status SDIO_DeInit_(void* args_p);
static Status SDIO_Open(void* args_p);
static Status SDIO_Close(void* args_p);
static Status SDIO_Read(U8* data_in_p, U32 size, void* args_p);
static Status SDIO_Write(U8* data_out_p, U32 size, void* args_p);
static Status SDIO_DMA_Read(U8* data_in_p, U32 size, void* args_p);
static Status SDIO_DMA_Write(U8* data_out_p, U32 size, void* args_p);
static Status SDIO_IoCtl(const U32 request_id, void* args_p);

static BL SD_IsDetected(void);

//-----------------------------------------------------------------------------
static SD_HandleTypeDef sd_handle;
static DMA_HandleTypeDef sd_dma_rx_handle;
static DMA_HandleTypeDef sd_dma_tx_handle;
static OS_DriverHd drv_led_sd;
HAL_DriverItf* drv_sdio_v[DRV_ID_SDIO_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_sdio_sd = {
    .Init   = SDIO_Init_,
    .DeInit = SDIO_DeInit_,
    .Open   = SDIO_Open,
    .Close  = SDIO_Close,
    .Read   = SDIO_DMA_Read,
    .Write  = SDIO_DMA_Write,
    .IoCtl  = SDIO_IoCtl
};

/*****************************************************************************/
Status SDIO__Init(void)
{
    HAL_MemSet(drv_sdio_v, 0x0, sizeof(drv_sdio_v));
    drv_sdio_v[DRV_ID_SDIO_SD] = &drv_sdio_sd;
    return S_OK;
}

/*****************************************************************************/
Status SDIO_Init_(void* args_p)
{
Status s;
    HAL_LOG(D_INFO, "Init: ");
    /* Enable SDIO clock */
    __SDIO_CLK_ENABLE();
    IF_STATUS(s = SDIO_GPIO_Init()) { return s; }
    IF_STATUS(s = SDIO_DMA_Init())  { return s; }
    IF_STATUS(s = SDIO_NVIC_Init()) { return s; }
    /* Check if the SD card is plugged in the slot */
    if (OS_TRUE == SD_IsDetected()) {
        HAL_SD_CardInfoTypedef sd_card_info;

        sd_handle.Instance                = SDIO;
        sd_handle.Init.ClockEdge          = SDIO_CLOCK_EDGE_RISING;
        sd_handle.Init.ClockBypass        = SDIO_CLOCK_BYPASS_DISABLE;
        sd_handle.Init.ClockPowerSave     = SDIO_CLOCK_POWER_SAVE_DISABLE;
        sd_handle.Init.BusWide            = SDIO_BUS_WIDE_1B;
        //Do _NOT_ use HardwareFlowControl due 2.9.1 SDIO HW flow control errata.
        sd_handle.Init.HardwareFlowControl= SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
        sd_handle.Init.ClockDiv           = SDIO_TRANSFER_CLK_DIV;
        if (SD_OK != HAL_SD_Init(&sd_handle, &sd_card_info)) { s = S_HARDWARE_FAULT; }

//TODO(A. Filyanov) Wrong bus width detection!
//        HAL_SD_CardStatusTypedef sd_card_status;
//        HAL_SD_GetCardStatus(&sd_handle, &sd_card_status);
//
//        if (4 == sd_card_status.DAT_BUS_WIDTH) {
            if (SD_OK != HAL_SD_WideBusOperation_Config(&sd_handle, SDIO_BUS_WIDE_4B)) {
                s = S_HARDWARE_FAULT;
            }
//        }
    } else { s = S_HARDWARE_FAULT; }
    return s;
}

/*****************************************************************************/
Status SDIO_GPIO_Init(void)
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
    return S_OK;
}

/*****************************************************************************/
Status SDIO_DMA_Init(void)
{
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
    __HAL_LINKDMA(&sd_handle, hdmarx, sd_dma_rx_handle);

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
    __HAL_LINKDMA(&sd_handle, hdmatx, sd_dma_tx_handle);

    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&sd_dma_tx_handle);

    /* Configure the DMA stream */
    HAL_DMA_Init(&sd_dma_tx_handle);
    return S_OK;
}

/*****************************************************************************/
Status SDIO_NVIC_Init(void)
{
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
    if (HAL_OK != HAL_SD_DeInit(&sd_handle)) { s = S_HARDWARE_FAULT; }
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
    HAL_DMA_DeInit(sd_handle.hdmarx);
    HAL_DMA_DeInit(sd_handle.hdmatx);
    return s;
}

/*****************************************************************************/
Status SDIO_Open(void* args_p)
{
    drv_led_sd = *(OS_DriverHd*)args_p;
    return S_OK;
}

/*****************************************************************************/
Status SDIO_Close(void* args_p)
{
Status s = S_OK;
    s = drv_sdio_sd.IoCtl(DRV_REQ_STD_SYNC, OS_NULL);
    return s;
}

/******************************************************************************/
Status SDIO_Read(U8* data_in_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
State led_sd_state = ON;
Status s = S_OK;
    OS_DriverWrite(drv_led_sd, &led_sd_state, 1, OS_NULL);
    if (SD_OK != HAL_SD_ReadBlocks(&sd_handle, (U32*)data_in_p, (sector * SD_CARD_SECTOR_SIZE), SD_CARD_BLOCK_SIZE, size)) {}
    led_sd_state = OFF;
    OS_DriverWrite(drv_led_sd, &led_sd_state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status SDIO_Write(U8* data_out_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
State led_sd_state = ON;
Status s = S_OK;
    OS_DriverWrite(drv_led_sd, &led_sd_state, 1, OS_NULL);
    if (SD_OK != HAL_SD_WriteBlocks(&sd_handle, (U32*)data_out_p, (sector * SD_CARD_SECTOR_SIZE), SD_CARD_BLOCK_SIZE, size)) {}
    led_sd_state = OFF;
    OS_DriverWrite(drv_led_sd, &led_sd_state, 1, OS_NULL);
    return s;
}

//https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=https%3a%2f%2fmy.st.com%2fpublic%2fSTe2ecommunities%2fmcu%2fLists%2fcortex_mx_stm32%2fST32f4discovery%2bsdcard%2bfatfs%20problem
/******************************************************************************/
Status SDIO_DMA_Read(U8* data_in_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
HAL_SD_ErrorTypedef sd_status;
State state = ON;
Status s = S_OK;

    OS_LOG(D_DEBUG, "read 0x%X %6d %d", data_in_p, sector, size);
    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    if ((U32)data_in_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(SD_CARD_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            IF_STATUS(s = drv_sdio_sd.Read((U8*)scratch_p, 1, &sector)) { break; }
            OS_MemCpy(data_in_p, scratch_p, SD_CARD_BLOCK_SIZE);
            data_in_p += SD_CARD_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

    sd_status = HAL_SD_ReadBlocks_DMA(&sd_handle, (U32*)data_in_p, (sector * SD_CARD_SECTOR_SIZE), SD_CARD_BLOCK_SIZE, size);
    if (SD_OK == sd_status) {
        sd_status = HAL_SD_CheckReadOperation(&sd_handle, (U32)HAL_TIMEOUT_SD_TRANSFER); // Check if the Transfer is finished
        if (SD_OK != sd_status) {
            s = S_FS_TRANSFER_FAIL;
        }
    } else {
        s = S_FS_TRANSFER_FAIL;
    }
    state = OFF;
    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status SDIO_DMA_Write(U8* data_out_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
HAL_SD_ErrorTypedef sd_status;
State state = ON;
Status s = S_OK;

    OS_LOG(D_DEBUG, "write 0x%X %6d %d", data_out_p, sector, size);
    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    if ((U32)data_out_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(SD_CARD_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            OS_MemCpy(data_out_p, scratch_p, SD_CARD_BLOCK_SIZE);
            IF_STATUS(s = drv_sdio_sd.Write((U8*)scratch_p, 1, &sector)) { break; }
            data_out_p += SD_CARD_BLOCK_SIZE;
            ++sector;
        }
        OS_Free(scratch_p);
        return s;
    }

    sd_status = HAL_SD_WriteBlocks_DMA(&sd_handle, (U32*)data_out_p, (sector * SD_CARD_SECTOR_SIZE), SD_CARD_BLOCK_SIZE, size);
    if (SD_OK == sd_status) {
        sd_status = HAL_SD_CheckWriteOperation(&sd_handle, (U32)HAL_TIMEOUT_SD_TRANSFER); // Check if the Transfer is finished
        if (SD_OK != sd_status) {
            s = S_FS_TRANSFER_FAIL;
        }
    } else {
        s = S_FS_TRANSFER_FAIL;
    }
    state = OFF;
    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    return s;
}

/******************************************************************************/
Status SDIO_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET: {
            HAL_StatusTypeDef hal_status = HAL_OK;
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    if (SD_POWER_STATE_OFF == SDIO_GetPowerState(sd_handle.Instance)) {
                        hal_status = SDIO_PowerState_ON(sd_handle.Instance);
                    }
                    break;
                case PWR_OFF: {
                    if (SD_POWER_STATE_ON == SDIO_GetPowerState(sd_handle.Instance)) {
                        hal_status = SDIO_PowerState_OFF(sd_handle.Instance);
                    } else if (SD_POWER_STATE_UP == SDIO_GetPowerState(sd_handle.Instance)) {
                        while (SD_POWER_STATE_ON != SDIO_GetPowerState(sd_handle.Instance)) {};
                        hal_status = SDIO_PowerState_OFF(sd_handle.Instance);
                    }
                    }
                    break;
                default:
                    break;
            }
            if (HAL_OK != hal_status) { s = S_FS_UNDEF; }
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
                        hal_status = SDIO_PowerState_ON(sd_handle.Instance);
                        break;
                    case PWR_OFF:
                        hal_status = SDIO_PowerState_OFF(sd_handle.Instance);
                        break;
                    default:
                        break;
                }
                if (HAL_OK != hal_status) { s = S_FS_UNDEF; }
            }
            break;
        case DRV_REQ_STD_SYNC:
        case CTRL_SYNC:
            while (SD_TRANSFER_OK != HAL_SD_GetStatus(&sd_handle)) {};
            break;
        case DRV_REQ_FS_MEDIA_STATUS_GET:
            if (OS_TRUE != SD_IsDetected()) {
                s = S_FS_NOT_READY;
            }
            break;
        case GET_SECTOR_COUNT: {
            HAL_SD_CardInfoTypedef card_info;

            if (SD_OK == HAL_SD_Get_CardInfo(&sd_handle, &card_info)) {
                *(U32*)args_p = card_info.CardCapacity / SD_CARD_BLOCK_SIZE;
            } else {
                *(U32*)args_p = 0;
            }
            }
            break;
        case GET_SECTOR_SIZE:
            *(U16*)args_p = SD_CARD_SECTOR_SIZE;
            break;
        case GET_BLOCK_SIZE:
            *(U32*)args_p = SD_CARD_BLOCK_SIZE;
            break;
        case CTRL_ERASE_SECTOR: {
            //TODO(A. Filyanov) Check SDHC capability!
            U32 start_sector= ((U32*)args_p)[0] * SD_CARD_SECTOR_SIZE;
            U32 end_sector  = ((U32*)args_p)[1] * SD_CARD_SECTOR_SIZE;
            if (SD_OK != HAL_SD_Erase(&sd_handle, start_sector, end_sector)) {
                s = S_FS_UNDEF;
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
BL SD_IsDetected(void)
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
    HAL_SD_IRQHandler(&sd_handle);
}

/*****************************************************************************/
void SD_DMAx_Rx_IRQHandler(void);
void SD_DMAx_Rx_IRQHandler(void)
{
    HAL_DMA_IRQHandler(sd_handle.hdmarx);
}

/*****************************************************************************/
void SD_DMAx_Tx_IRQHandler(void);
void SD_DMAx_Tx_IRQHandler(void)
{
    HAL_DMA_IRQHandler(sd_handle.hdmatx);
}
