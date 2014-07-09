/**************************************************************************//**
* @file    drv_sdio.c
* @brief   SDIO driver.
* @author  A. Filyanov
* @warning Do _NOT_ use HardwareFlowControl due 2.9.1 SDIO HW flow control errata.
******************************************************************************/
#include <string.h>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_sdio.h"
#include "stm324xg_eval_sdio_sd.h"
#include "hal.h"
#include "diskio.h"
#include "os_debug.h"
#include "os_driver.h"
#include "os_memory.h"
#include "os_supervise.h"
#include "os_file_system.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_sdio"

//-----------------------------------------------------------------------------
#define SDIO_FIFO_ADDRESS               ((uint32_t)0x40012C80)
/**
  * @brief  SDIO Intialization Frequency (400KHz max)
  */
#define SDIO_INIT_CLK_DIV               ((uint8_t)0x76)
/**
  * @brief  SDIO Data Transfer Frequency (25MHz max)
  */
#define SDIO_TRANSFER_CLK_DIV           ((uint8_t)0x0)

#ifndef OLIMEX_STM32_P407
// THESE ARE NOT PRESENT ON THE OLIMEX BOARD!!!
#define SD_CP_PIN                       GPIO_Pin_3
#define SD_CP_PORT                      GPIOD
#define SD_CP_CLK                       RCC_AHB1Periph_GPIOD
#define SD_CP_SOURCE                    GPIO_PinSource3

#define SD_WP_PIN                       GPIO_Pin_4
#define SD_WP_PORT                      GPIOE
#define SD_WP_CLK                       RCC_AHB1Periph_GPIOE
#define SD_WP_SOURCE                    GPIO_PinSource4
#endif // OLIMEX_STM32_P407

#ifdef OLIMEX_STM32_P407
#if (1 == SD_SDIO_ENABLED) // MMC mode

#define SD_CMD_PIN                      GPIO_Pin_2
#define SD_CMD_PORT                     GPIOD
#define SD_CMD_CLK                      RCC_AHB1Periph_GPIOD
#define SD_CMD_SOURCE                   GPIO_PinSource2

#define SD_D0_PIN                       GPIO_Pin_8
#define SD_D0_PORT                      GPIOC
#define SD_D0_CLK                       RCC_AHB1Periph_GPIOC
#define SD_D0_SOURCE                    GPIO_PinSource8

#define SD_D1_PIN                       GPIO_Pin_9
#define SD_D1_PORT                      GPIOC
#define SD_D1_CLK                       RCC_AHB1Periph_GPIOC
#define SD_D1_SOURCE                    GPIO_PinSource9

#define SD_D2_PIN                       GPIO_Pin_10
#define SD_D2_PORT                      GPIOC
#define SD_D2_CLK                       RCC_AHB1Periph_GPIOC
#define SD_D2_SOURCE                    GPIO_PinSource10

#define SD_D3_PIN                       GPIO_Pin_11
#define SD_D3_PORT                      GPIOC
#define SD_D3_CLK                       RCC_AHB1Periph_GPIOC
#define SD_D3_SOURCE                    GPIO_PinSource11

#define SD_CLK_PIN                      GPIO_Pin_12
#define SD_CLK_PORT                     GPIOC
#define SD_CLK_CLK                      RCC_AHB1Periph_GPIOC
#define SD_CLK_SOURCE                   GPIO_PinSource12

#define SD_SDIO_DMA                     DMA2
#define SD_SDIO_DMA_CLK                 RCC_AHB1Periph_DMA2

#ifdef SD_SDIO_DMA_STREAM3
#   define SD_SDIO_DMA_STREAM           DMA2_Stream3
#   define SD_SDIO_DMA_CHANNEL          DMA_Channel_4
#   define SD_SDIO_DMA_FLAG_FEIF        DMA_FLAG_FEIF3
#   define SD_SDIO_DMA_FLAG_DMEIF       DMA_FLAG_DMEIF3
#   define SD_SDIO_DMA_FLAG_TEIF        DMA_FLAG_TEIF3
#   define SD_SDIO_DMA_FLAG_HTIF        DMA_FLAG_HTIF3
#   define SD_SDIO_DMA_FLAG_TCIF        DMA_FLAG_TCIF3
#   define SD_SDIO_DMA_IRQn             DMA2_Stream3_IRQn
#   define SD_SDIO_DMA_IRQHANDLER       DMA2_Stream3_IRQHandler
#elif defined SD_SDIO_DMA_STREAM6
#   define SD_SDIO_DMA_STREAM           DMA2_Stream6
#   define SD_SDIO_DMA_CHANNEL          DMA_Channel_4
#   define SD_SDIO_DMA_FLAG_FEIF        DMA_FLAG_FEIF6
#   define SD_SDIO_DMA_FLAG_DMEIF       DMA_FLAG_DMEIF6
#   define SD_SDIO_DMA_FLAG_TEIF        DMA_FLAG_TEIF6
#   define SD_SDIO_DMA_FLAG_HTIF        DMA_FLAG_HTIF6
#   define SD_SDIO_DMA_FLAG_TCIF        DMA_FLAG_TCIF6
#   define SD_SDIO_DMA_IRQn             DMA2_Stream6_IRQn
#   define SD_SDIO_DMA_IRQHANDLER       DMA2_Stream6_IRQHandler
#endif /* SD_SDIO_DMA_STREAM3 */

#else // SPI mode

/**
 * @brief MMC/SD Card Card chip select
 */
#define MMC_CS_PORT                     GPIOD
#define MMC_CS_CLK                      RCC_AHB1Periph_GPIOD
#define MMC_CS_PIN                      GPIO_Pin_2
#define MMC_CS_EXTI_LINE                EXTI_Line2
#define MMC_CS_PORT_SOURCE              GPIO_PortSourceGPIOD
#define MMC_CS_PIN_SOURCE               GPIO_PinSource2
#define MMC_CS_IRQn                     EXTI2_IRQn

/**
 * @brief MMC/SD Card SPI
 */
#define MMC_SPI                         SPI3   /* SPI pins are remapped by software */
#define MMC_SPI_CLK                     RCC_APB1Periph_SPI3
#define MMC_SPI_GPIO                    GPIOC
#define MMC_SPI_GPIO_CLK                RCC_AHB1Periph_GPIOC
#define MMC_PIN_SCK                     GPIO_Pin_10
#define MMC_PIN_SCK_SOURCE              GPIO_PinSource10
#define MMC_PIN_MISO                    GPIO_Pin_11
#define MMC_PIN_MISO_SOURCE             GPIO_PinSource11
#define MMC_PIN_MOSI                    GPIO_Pin_12
#define MMC_PIN_MOSI_SOURCE             GPIO_PinSource12

/** @addtogroup STM324xG_EVAL_SDIO_SD
  * @brief      This file provides all the SD Card driver firmware functions.
  * @{
  */

/** @defgroup STM324xG_EVAL_SDIO_SD_Private_Types
  * @{
  */
/**
  * @}
  */


/** @defgroup STM324xG_EVAL_SDIO_SD_Private_Defines
  * @{
  */

/**
  * @brief  SDIO Static flags, TimeOut, FIFO Address
  */
#define NULL 0
#define SDIO_STATIC_FLAGS               ((uint32_t)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((uint32_t)0x00010000)

/**
  * @brief  Mask for errors Card Status R1 (OCR Register)
  */
#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

/**
  * @brief  Masks for R6 Response
  */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define SD_STD_CAPACITY                 ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)

#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)

#define SD_DATATIMEOUT                  ((uint32_t)0xFFFFFFFF)
#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)

/**
  * @brief  Command Class Supported
  */
#define SD_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)

/**
  * @brief  Following commands are SD Card Specific commands.
  *         SDIO_APP_CMD should be sent before sending these commands.
  */
#define SDIO_SEND_IF_COND               ((uint32_t)0x00000008)

/**
  * @}
  */

/** @defgroup STM324xG_EVAL_SDIO_SD_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup STM324xG_EVAL_SDIO_SD_Private_Variables
  * @{
  */

#endif // SD_SDIO_ENABLED

#endif // OLIMEX_STM32_P407

//-----------------------------------------------------------------------------
/// @brief   SDIO initialization.
/// @return  #Status.
Status SDIO_Init_(void);

static Status SDIO__Init(void);
static Status SDIO_NVIC_Init(void);
static Status SDIO__DeInit(void);
static Status SDIO_Open(void* args_p);
static Status SDIO_Close(void);
static Status SDIO_Read(U8* data_in_p, U32 size, void* args_p);
static Status SDIO_Write(U8* data_out_p, U32 size, void* args_p);
static Status SDIO_IoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
static OS_DriverHd drv_led_sd;
HAL_DriverItf* drv_sdio_v[DRV_ID_SDIO_LAST];

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_sdio = {
    .Init   = SDIO__Init,
    .DeInit = SDIO__DeInit,
    .Open   = SDIO_Open,
    .Close  = SDIO_Close,
    .Read   = SDIO_Read,
    .Write  = SDIO_Write,
    .IoCtl  = SDIO_IoCtl
};

/*****************************************************************************/
Status SDIO_Init_(void)
{
    memset(drv_sdio_v, 0x0, sizeof(drv_sdio_v));
    drv_sdio_v[DRV_ID_SDIO] = &drv_sdio;
    return S_OK;
}

/*****************************************************************************/
Status SDIO__Init(void)
{
Status s;
    D_LOG(D_INFO, "Init: ");
    OS_CriticalSectionEnter(); {
        const SD_Error sd_err = SD_Init();
        if (SD_OK == sd_err) {
            s = SDIO_NVIC_Init();
            D_TRACE_S(D_INFO, s);
        } else {
            if (SD_CMD_RSP_TIMEOUT == sd_err) {
                s = S_TIMEOUT;
                D_TRACE(D_INFO, "Timeout (no media?)");
            } else {
                s = S_HARDWARE_FAULT;
                D_TRACE_S(D_INFO, s);
            }
        }
    } OS_CriticalSectionExit();
    return s;
}

/*****************************************************************************/
void SD_LowLevel_Init(void);
void SD_LowLevel_Init(void)
{
GPIO_InitTypeDef    GPIO_InitStructure;
    // GPIO enable clock
    RCC_AHB1PeriphClockCmd(/*SD_CP_CLK | SD_WP_CLK |*/ SD_CMD_CLK | SD_CLK_CLK | \
                           SD_D0_CLK | SD_D1_CLK | SD_D2_CLK | SD_D3_CLK, ENABLE);
    GPIO_StructInit(&GPIO_InitStructure);
#ifndef OLIMEX_STM32_P407
    //THESE ARE NOT PRESENT OPN THE OLIMEX BOARD!!!
    // Init CP pin
    GPIO_InitStructure.GPIO_Pin     = SD_CP_PIN;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(SD_CP_PORT, &GPIO_InitStructure);

    // Init WP pin
    GPIO_InitStructure.GPIO_Pin     = SD_WP_PIN;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(SD_WP_PORT, &GPIO_InitStructure);
#endif // OLIMEX_STM32_P407

    // Configure PC.08, PC.09, PC.10, PC.11, PC.12 pin: D0, D1, D2, D3, CLK pin
    GPIO_InitStructure.GPIO_Pin     = SD_D0_PIN | SD_D1_PIN | SD_D2_PIN | SD_D3_PIN | SD_CLK_PIN;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Configure PD.02 CMD line
    GPIO_InitStructure.GPIO_Pin     = SD_CMD_PIN;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    //GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(SD_CMD_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(SD_CMD_PORT, SD_CMD_SOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_CLK_PORT, SD_CLK_SOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_D0_PORT, SD_D0_SOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_D1_PORT, SD_D1_SOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_D2_PORT, SD_D2_SOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_D3_PORT, SD_D3_SOURCE, GPIO_AF_SDIO);

    /* Enable the SDIO AHB Clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);

    // Enable the DMA2 Clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
}

/*****************************************************************************/
Status SDIO_NVIC_Init(void)
{
NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_StructInit(&NVIC_InitStructure);
    // SDIO Interrupt ENABLE
    NVIC_InitStructure.NVIC_IRQChannel                      = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = SD_SDIO_IRQ_PRIO;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // DMA2 STREAMx Interrupt ENABLE
    NVIC_InitStructure.NVIC_IRQChannel                      = SD_SDIO_DMA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = SD_SDIO_DMA_IRQ_PRIO;
    NVIC_Init(&NVIC_InitStructure);
    return S_OK;
}

/*****************************************************************************/
Status SDIO__DeInit(void)
{
    SD_DeInit();
    return S_OK;
}

/*****************************************************************************/
void SD_LowLevel_DeInit(void);
void SD_LowLevel_DeInit(void)
{
GPIO_InitTypeDef GPIO_InitStructure;
    /*!< Disable SDIO Clock */
    SDIO_ClockCmd(DISABLE);
    /*!< Set Power State to OFF */
    SDIO_SetPowerState(SDIO_PowerState_OFF);
    /*!< DeInitializes the SDIO peripheral */
    SDIO_DeInit();
    /* Disable the SDIO APB2 Clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, DISABLE);

    GPIO_PinAFConfig(SD_CMD_PORT, SD_CMD_SOURCE, GPIO_AF_MCO);
    GPIO_PinAFConfig(SD_CLK_PORT, SD_CLK_SOURCE, GPIO_AF_MCO);
    GPIO_PinAFConfig(SD_D0_PORT, SD_D0_SOURCE, GPIO_AF_MCO);
    GPIO_PinAFConfig(SD_D1_PORT, SD_D1_SOURCE, GPIO_AF_MCO);
    GPIO_PinAFConfig(SD_D2_PORT, SD_D2_SOURCE, GPIO_AF_MCO);
    GPIO_PinAFConfig(SD_D3_PORT, SD_D3_SOURCE, GPIO_AF_MCO);

    /* Configure PC.08, PC.09, PC.10, PC.11 pins: D0, D1, D2, D3 pins */
    GPIO_InitStructure.GPIO_Pin     = SD_D0_PIN | SD_D1_PIN | SD_D2_PIN | SD_D3_PIN | SD_CLK_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure PD.02 CMD line */
    GPIO_InitStructure.GPIO_Pin = SD_CMD_PIN;
    GPIO_Init(SD_CMD_PORT, &GPIO_InitStructure);
}

/*****************************************************************************/
Status SDIO_Open(void* args_p)
{
    drv_led_sd = *(OS_DriverHd*)args_p;
    return S_OK;
}

/*****************************************************************************/
Status SDIO_Close(void)
{
    return S_OK;
}

//https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=https%3a%2f%2fmy.st.com%2fpublic%2fSTe2ecommunities%2fmcu%2fLists%2fcortex_mx_stm32%2fST32f4discovery%2bsdcard%2bfatfs%20problem
/******************************************************************************/
Status SDIO_Read(U8* data_in_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
SD_Error sd_status;
State state = ON;
Status s = S_OK;

    OS_LOG(D_DEBUG, "read 0x%X %6d %d", data_in_p, sector, size);
    if (SD_PRESENT != SD_Detect()) {
        s = S_FS_NOT_READY;
        return s;
    }
    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    if ((U32)data_in_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(SD_SDIO_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            IF_STATUS(s = SDIO_Read((U8*)scratch_p, 1, &sector)) { break; }
            ++sector;
            OS_MemCpy32(data_in_p, scratch_p, SD_SDIO_BLOCK_SIZE / 4);
            data_in_p += SD_SDIO_BLOCK_SIZE;
            OS_ContextSwitchForce();
        }
        OS_Free(scratch_p);
        return s;
    }

    sd_status = SD_ReadMultiBlocks(data_in_p, (sector * SD_CARD_SECTOR_SIZE), SD_SDIO_BLOCK_SIZE, size); // 4GB Compliant(?)
    if (SD_OK == sd_status) {
        SDTransferState transf_state;
        OS_ContextSwitchForce();
        sd_status = SD_WaitReadOperation(); // Check if the Transfer is finished
        if (SD_OK == sd_status) {
            while (SD_TRANSFER_BUSY == (transf_state = SD_GetStatus())) {
                OS_ContextSwitchForce();
            } // BUSY, OK (DONE), ERROR (FAIL)
            if (SD_TRANSFER_ERROR == transf_state) {
                s = S_FS_TRANSFER_FAIL;
            }
        } else {
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
Status SDIO_Write(U8* data_out_p, U32 size, void* args_p)
{
U32 sector = *(U32*)args_p;
SD_Error sd_status;
State state = ON;
Status s = S_OK;

    OS_LOG(D_DEBUG, "write 0x%X %6d %d", data_out_p, sector, size);
    if (SD_PRESENT != SD_Detect()) {
        s = S_FS_NOT_READY;
        return s;
    }
    OS_DriverWrite(drv_led_sd, &state, 1, OS_NULL);
    if ((U32)data_out_p & 0x03) { // DMA Alignment failure, do single up to aligned buffer
        U32* scratch_p = (U32*)OS_Malloc(SD_SDIO_BLOCK_SIZE); // Alignment assured
        if (OS_NULL == scratch_p) { return S_NO_MEMORY; }
        while (size--) {
            IF_STATUS(s = SDIO_Write((U8*)scratch_p, 1, &sector)) { break; }
            ++sector;
            OS_MemCpy32(data_out_p, scratch_p, SD_SDIO_BLOCK_SIZE / 4);
            data_out_p += SD_SDIO_BLOCK_SIZE;
            OS_ContextSwitchForce();
        }
        OS_Free(scratch_p);
        return s;
    }

    sd_status = SD_WriteMultiBlocks(data_out_p, (sector * SD_CARD_SECTOR_SIZE), SD_SDIO_BLOCK_SIZE, size); // 4GB Compliant(?)
    if (SD_OK == sd_status) {
        SDTransferState transf_state;
        OS_ContextSwitchForce();
        sd_status = SD_WaitWriteOperation(); // Check if the Transfer is finished
        if (SD_OK == sd_status) {
            while (SD_TRANSFER_BUSY == (transf_state = SD_GetStatus())) {
                OS_ContextSwitchForce();
            } // BUSY, OK (DONE), ERROR (FAIL)
            if (SD_TRANSFER_ERROR == transf_state) {
                s = S_FS_TRANSFER_FAIL;
            }
        } else {
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
        case DRV_REQ_STD_POWER: {
            SD_Error sd_status = SD_OK;
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    if (SDIO_PowerState_OFF == SDIO_GetPowerState()) {
                        sd_status = SD_PowerON();
                    }
                    break;
                case PWR_OFF: {
                    const U32 SDIO_PowerState_UP = 0x2;
                    if (SDIO_PowerState_ON == SDIO_GetPowerState()) {
                        sd_status = SD_PowerOFF();
                    } else if (SDIO_PowerState_UP == SDIO_GetPowerState()) {
                        while (SDIO_PowerState_ON != SDIO_GetPowerState()) {
                            OS_ContextSwitchForce();
                        }
                        sd_status = SD_PowerOFF();
                    }
                    }
                    break;
                default:
                    break;
            }
            if (SD_OK != sd_status) { s = S_FS_UNDEF; }
            }
            break;
        case CTRL_POWER: {
            SD_Error sd_status = SD_OK;
            //Power Get
            //....
            //Power Set
            const U8 power_state = (*(U8*)args_p) & 0xFF;
                switch (power_state) {
                    case PWR_ON:
                        sd_status = SD_PowerON();
                        break;
                    case PWR_OFF:
                        sd_status = SD_PowerOFF();
                        break;
                    default:
                        break;
                }
                if (SD_OK != sd_status) { s = S_FS_UNDEF; }
            }
            break;
        case DRV_REQ_STD_SYNC:
        case CTRL_SYNC:
            while (SD_TRANSFER_OK != SD_GetStatus());
            break;
        case GET_SECTOR_COUNT: {
            SD_CardInfo card_info;

            if (SD_OK == SD_GetCardInfo(&card_info)) {
                *(U32*)args_p = card_info.CardCapacity / SD_SDIO_BLOCK_SIZE;
            } else {
                *(U32*)args_p = 0;
            }
            }
            break;
        case GET_SECTOR_SIZE:
            *(U16*)args_p = SD_CARD_SECTOR_SIZE;
            break;
        case GET_BLOCK_SIZE:
            *(U32*)args_p = SD_SDIO_BLOCK_SIZE;
            break;
        case CTRL_ERASE_SECTOR: {
            //TODO(A. Filyanov) Check SDHC capability!
            U32 start_sector= ((U32*)args_p)[0] * SD_CARD_SECTOR_SIZE;
            U32 end_sector  = ((U32*)args_p)[1] * SD_CARD_SECTOR_SIZE;
            if (SD_OK != SD_Erase(start_sector, end_sector)) {
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

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
  * @param  BufferSRC: pointer to the source buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
/*****************************************************************************/
void SD_LowLevel_DMA_TxConfig(uint32_t* BufferSRC, uint32_t BufferSize);
void SD_LowLevel_DMA_TxConfig(uint32_t* BufferSRC, uint32_t BufferSize)
{
DMA_InitTypeDef SDDMA_InitStructure;

    DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF |\
                  SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);
    /* DMA2 Stream3  or Stream6 disable */
    DMA_Cmd(SD_SDIO_DMA_STREAM, DISABLE);
    /* DMA2 Stream3  or Stream6 Config */
    DMA_DeInit(SD_SDIO_DMA_STREAM);

    SDDMA_InitStructure.DMA_Channel = SD_SDIO_DMA_CHANNEL;
    SDDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
    SDDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
    SDDMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    SDDMA_InitStructure.DMA_BufferSize = BufferSize;
    SDDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    SDDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    SDDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    SDDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    SDDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    SDDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    SDDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    SDDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    SDDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
    SDDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
    DMA_Init(SD_SDIO_DMA_STREAM, &SDDMA_InitStructure);
    DMA_ITConfig(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
    DMA_FlowControllerConfig(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
    /* DMA2 Stream3  or Stream6 enable */
    DMA_Cmd(SD_SDIO_DMA_STREAM, ENABLE);
}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
  * @param  BufferDST: pointer to the destination buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
/*****************************************************************************/
void SD_LowLevel_DMA_RxConfig(uint32_t* BufferDST, uint32_t BufferSize);
void SD_LowLevel_DMA_RxConfig(uint32_t* BufferDST, uint32_t BufferSize)
{
DMA_InitTypeDef SDDMA_InitStructure;

    DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF |\
                  SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);
    /* DMA2 Stream3  or Stream6 disable */
    DMA_Cmd(SD_SDIO_DMA_STREAM, DISABLE);
    /* DMA2 Stream3 or Stream6 Config */
    DMA_DeInit(SD_SDIO_DMA_STREAM);

    SDDMA_InitStructure.DMA_Channel = SD_SDIO_DMA_CHANNEL;
    SDDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
    SDDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferDST;
    SDDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    SDDMA_InitStructure.DMA_BufferSize = BufferSize;
    SDDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    SDDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    SDDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    SDDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    SDDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    SDDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    SDDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    SDDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    SDDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
    SDDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
    DMA_Init(SD_SDIO_DMA_STREAM, &SDDMA_InitStructure);
    DMA_ITConfig(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
    DMA_FlowControllerConfig(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
    /* DMA2 Stream3 or Stream6 enable */
    DMA_Cmd(SD_SDIO_DMA_STREAM, ENABLE);
}

// SDIO IRQ handlers -----------------------------------------------------------
/*****************************************************************************/
void SDIO_IRQHandler(void);
void SDIO_IRQHandler(void)
{
    /* Process All SDIO Interrupt Sources */
    SD_ProcessIRQSrc();
}

/*****************************************************************************/
void SD_SDIO_DMA_IRQHANDLER(void);
void SD_SDIO_DMA_IRQHANDLER(void)
{
    /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
    SD_ProcessDMAIRQ();
}