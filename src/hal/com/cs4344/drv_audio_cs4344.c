/**************************************************************************//**
* @file    drv_audio_cs4344.c
* @brief   CS4344 audio driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "com/cs4344/drv_audio_cs4344.h"
#include "os_power.h"
#include "os_debug.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_cs4344"

#define CS4344_I2Sx_FORCE_RESET()           __SPI3_FORCE_RESET()
#define CS4344_I2Sx_RELEASE_RESET()         __SPI3_RELEASE_RESET()

/* I2S peripheral configuration defines */
#define CS4344_I2Sx                         SPI3
#define CS4344_I2Sx_CLK_ENABLE()            __SPI3_CLK_ENABLE()
#define CS4344_I2Sx_SCK_SD_WS_AF            GPIO_AF6_SPI3
#define CS4344_I2Sx_WS_CLK_ENABLE()         __GPIOA_CLK_ENABLE()
#define CS4344_I2Sx_SCK_SD_CLK_ENABLE()     __GPIOB_CLK_ENABLE()
#define CS4344_I2Sx_MCK_CLK_ENABLE()        __GPIOC_CLK_ENABLE()
#define CS4344_I2Sx_WS_PIN                  GPIO_PIN_15
#define CS4344_I2Sx_SCK_PIN                 GPIO_PIN_3
#define CS4344_I2Sx_SD_PIN                  GPIO_PIN_5
#define CS4344_I2Sx_MCK_PIN                 GPIO_PIN_7
#define CS4344_I2Sx_WS_GPIO_PORT            GPIOA
#define CS4344_I2Sx_SCK_SD_GPIO_PORT        GPIOB
#define CS4344_I2Sx_MCK_GPIO_PORT           GPIOC

/* I2S DMA Stream definitions */
#define CS4344_I2Sx_DMAx_CLK_ENABLE()       __DMA1_CLK_ENABLE()
#define CS4344_I2Sx_DMAx_STREAM             DMA1_Stream5
#define CS4344_I2Sx_DMAx_CHANNEL            DMA_CHANNEL_0
#define CS4344_I2Sx_DMAx_IRQn               DMA1_Stream5_IRQn
#define CS4344_I2Sx_DMAx_PERIPH_DATA_SIZE   DMA_PDATAALIGN_HALFWORD
#define CS4344_I2Sx_DMAx_MEM_DATA_SIZE      DMA_PDATAALIGN_HALFWORD
#define DMA_MAX_SZE                         U16_MAX

#define CS4344_I2Sx_DMAx_IRQHandler         DMA1_Stream5_IRQHandler

//------------------------------------------------------------------------------
static Status   CS4344_Init(void* args_p);
static Status   CS4344_DeInit(void* args_p);
static Status   CS4344_LL_Init(void* args_p);
//static Status   CS4344_LL_DeInit(void* args_p);
static Status   CS4344_Open(void* args_p);
static Status   CS4344_Close(void* args_p);
static Status   CS4344_DMA_Write(void* data_out_p, SIZE size, void* args_p);
static Status   CS4344_IoCtl(const U32 request_id, void* args_p);

static Status   VolumeSet(const OS_AudioVolume volume);
static S8       FrequencyIdxGet(const OS_AudioFreq freq);
static Status   FrequencySet(const OS_AudioFreq freq);
static void     VolumeApply(void* data_out_p, SIZE size, const OS_AudioVolume volume);

//------------------------------------------------------------------------------
/* These PLL parameters are valide when the f(VCO clock) = 1Mhz */
static const U32 i2s_freq_v[] = { 8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000 };
static const U32 i2s_plln_v[] = { 256, 429, 213, 429, 426, 271, 258, 344 };
static const U32 i2s_pllr_v[] = { 5, 4, 4, 4, 4, 6, 3, 1 };

static I2S_HandleTypeDef        i2s_hd;
static DMA_HandleTypeDef        dma_tx_hd;
//static DMA_HandleTypeDef        dma_rx_hd;
static volatile OS_AudioFreq    cs4344_freq;
static volatile OS_AudioVolume  cs4344_volume;

HAL_DriverItf drv_audio_cs4344 = {
    .Init   = CS4344_Init,
    .DeInit = CS4344_DeInit,
    .Open   = CS4344_Open,
    .Close  = CS4344_Close,
    .Read   = OS_NULL,
    .Write  = CS4344_DMA_Write,
    .IoCtl  = CS4344_IoCtl
};

/******************************************************************************/
Status CS4344_Init(void* args_p)
{
const CS4344_DrvAudioArgsInit* drv_args_p = (CS4344_DrvAudioArgsInit*)args_p;
Status s = S_UNDEF;
    //HAL_LOG(D_INFO, "Init: ");
    if (OS_NULL == drv_args_p) { return s = S_INVALID_REF; }
    IF_STATUS(s = FrequencySet(drv_args_p->freq))   { return s; } //CS4344_LL_Init();
    IF_STATUS(s = VolumeSet(drv_args_p->volume))    { return s; }
    return s;
}

/******************************************************************************/
Status CS4344_LL_Init(void* args_p)
{
const OS_AudioFreq audio_freq = *(OS_AudioFreq*)args_p;
GPIO_InitTypeDef GPIO_InitStruct;
    /* Disable I2S block */
    __HAL_I2S_DISABLE(&i2s_hd);

    /* Enable I2S clock */
    CS4344_I2Sx_CLK_ENABLE();

    /* Enable SCK, SD and WS GPIO clock */
    CS4344_I2Sx_WS_CLK_ENABLE();
    CS4344_I2Sx_SCK_SD_CLK_ENABLE();

    /* CODEC_I2S pins configuration: WS, SCK and SD pins */
    GPIO_InitStruct.Pin         = CS4344_I2Sx_SCK_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate   = CS4344_I2Sx_SCK_SD_WS_AF;
    HAL_GPIO_Init(CS4344_I2Sx_SCK_SD_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin         = CS4344_I2Sx_SD_PIN;
    HAL_GPIO_Init(CS4344_I2Sx_SCK_SD_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin         = CS4344_I2Sx_WS_PIN;
    HAL_GPIO_Init(CS4344_I2Sx_WS_GPIO_PORT, &GPIO_InitStruct);

    /* Enable MCK GPIO clock */
    CS4344_I2Sx_MCK_CLK_ENABLE();

    /* CODEC_I2S pins configuration: MCK pin */
    GPIO_InitStruct.Pin         = CS4344_I2Sx_MCK_PIN;
    HAL_GPIO_Init(CS4344_I2Sx_MCK_GPIO_PORT, &GPIO_InitStruct);

    /* Enable the DMA clock */
    CS4344_I2Sx_DMAx_CLK_ENABLE();

    /* Configure the dma_tx_hd handle parameters */
    dma_tx_hd.Init.Channel              = CS4344_I2Sx_DMAx_CHANNEL;
    dma_tx_hd.Init.Direction            = DMA_MEMORY_TO_PERIPH;
    dma_tx_hd.Init.PeriphInc            = DMA_PINC_DISABLE;
    dma_tx_hd.Init.MemInc               = DMA_MINC_ENABLE;
    dma_tx_hd.Init.PeriphDataAlignment  = CS4344_I2Sx_DMAx_PERIPH_DATA_SIZE;
    dma_tx_hd.Init.MemDataAlignment     = CS4344_I2Sx_DMAx_MEM_DATA_SIZE;
    dma_tx_hd.Init.Mode                 = DMA_NORMAL;
    dma_tx_hd.Init.Priority             = DMA_PRIORITY_HIGH;
    dma_tx_hd.Init.FIFOMode             = DMA_FIFOMODE_ENABLE;
    dma_tx_hd.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;
    dma_tx_hd.Init.MemBurst             = DMA_MBURST_SINGLE;
    dma_tx_hd.Init.PeriphBurst          = DMA_PBURST_SINGLE;

    dma_tx_hd.Instance = CS4344_I2Sx_DMAx_STREAM;

    /* Associate the DMA handle */
    __HAL_LINKDMA(&i2s_hd, hdmatx, dma_tx_hd);

    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&dma_tx_hd);

    /* Configure the DMA Stream */
    HAL_DMA_Init(&dma_tx_hd);

    /* I2S DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(CS4344_I2Sx_DMAx_IRQn, OS_PRIORITY_INT_MIN, 0);
    HAL_NVIC_EnableIRQ(CS4344_I2Sx_DMAx_IRQn);
    /* Initialize the i2s_hd Instance parameter */
    i2s_hd.Instance = CS4344_I2Sx;

    i2s_hd.Init.Mode        = I2S_MODE_MASTER_TX;
    i2s_hd.Init.Standard    = I2S_STANDARD_PHILIPS;
    i2s_hd.Init.DataFormat  = I2S_DATAFORMAT_16B;
    i2s_hd.Init.AudioFreq   = audio_freq;
    i2s_hd.Init.CPOL        = I2S_CPOL_LOW;
    i2s_hd.Init.MCLKOutput  = I2S_MCLKOUTPUT_ENABLE;

    /* Init the I2S */
    if (HAL_OK != HAL_I2S_Init(&i2s_hd)) { return S_HARDWARE_FAULT; }
    return S_OK;
}

/******************************************************************************/
Status CS4344_DeInit(void* args_p)
{
    if (HAL_OK != HAL_I2S_DeInit(&i2s_hd)) { return S_HARDWARE_FAULT; }
    /*##-1- Reset peripherals ##################################################*/
    CS4344_I2Sx_FORCE_RESET();
    CS4344_I2Sx_RELEASE_RESET();

    HAL_GPIO_DeInit(CS4344_I2Sx_MCK_GPIO_PORT, CS4344_I2Sx_MCK_PIN);
    HAL_GPIO_DeInit(CS4344_I2Sx_SCK_SD_GPIO_PORT, CS4344_I2Sx_SCK_PIN);
    HAL_GPIO_DeInit(CS4344_I2Sx_SCK_SD_GPIO_PORT, CS4344_I2Sx_SD_PIN);
    HAL_GPIO_DeInit(CS4344_I2Sx_WS_GPIO_PORT, CS4344_I2Sx_WS_PIN);

    /*##-3- Disable the DMA Streams ############################################*/
    /* De-Initialize the DMA Stream associate to transmission process */
    HAL_DMA_DeInit(i2s_hd.hdmatx);

    /*##-5- Disable the NVIC for DMA ###########################################*/
    HAL_NVIC_DisableIRQ(CS4344_I2Sx_DMAx_IRQn);
    return S_OK;
}

/******************************************************************************/
Status CS4344_Open(void* args_p)
{
    //TODO(A. Filyanov)
    return S_OK;
}

/******************************************************************************/
Status CS4344_Close(void* args_p)
{
    //TODO(A. Filyanov)
    return S_OK;
}

/******************************************************************************/
Status CS4344_DMA_Write(void* data_out_p, SIZE size, void* args_p)
{
Status s = S_UNDEF;
    VolumeApply(data_out_p, size, cs4344_volume);
    while (HAL_I2S_STATE_READY != HAL_I2S_GetState(&i2s_hd)) {};
    /* Update the Media layer and enable it for play */
    if (HAL_OK != HAL_I2S_Transmit_DMA(&i2s_hd, (U16*)data_out_p, (size / sizeof(U16)))) { s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status CS4344_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
    switch (request_id) {
// Standard driver's requests.
        case DRV_REQ_STD_SYNC:
            break;
        case DRV_REQ_STD_POWER_SET:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    break;
                case PWR_OFF:
                case PWR_SLEEP:
                case PWR_STOP:
                case PWR_STANDBY:
                case PWR_HIBERNATE:
                case PWR_SHUTDOWN:
                    break;
                default:
                    break;
            }
            break;
// Audio driver's requests.
        case DRV_REQ_AUDIO_PLAY:
            break;
        case DRV_REQ_AUDIO_STOP:
            if (HAL_OK != HAL_I2S_DMAStop(&i2s_hd))     { s = S_HARDWARE_FAULT; }
            break;
        case DRV_REQ_AUDIO_PAUSE:
            if (HAL_OK != HAL_I2S_DMAPause(&i2s_hd))    { s = S_HARDWARE_FAULT; }
            break;
        case DRV_REQ_AUDIO_RESUME:
            if (HAL_OK != HAL_I2S_DMAResume(&i2s_hd))   { s = S_HARDWARE_FAULT; }
            break;
        case DRV_REQ_AUDIO_SEEK:
            break;
        case DRV_REQ_AUDIO_MUTE_SET:
            break;
        case DRV_REQ_AUDIO_VOLUME_GET:
            *(OS_AudioVolume*)args_p = cs4344_volume;
            break;
        case DRV_REQ_AUDIO_VOLUME_SET:
            s = VolumeSet(*(OS_AudioVolume*)args_p);
            break;
        case DRV_REQ_AUDIO_FREQUENCY_GET:
            *(OS_AudioFreq*)args_p = cs4344_freq;
            break;
        case DRV_REQ_AUDIO_FREQUENCY_SET:
            s = FrequencySet(*(OS_AudioFreq*)args_p);
            break;
// Device driver's requests.
        default:
            HAL_LOG_S(D_WARNING, S_UNDEF_REQ_ID);
            s = S_OK;
            break;
    }
    return s;
}

/******************************************************************************/
void VolumeApply(void* data_out_p, SIZE size, const OS_AudioVolume volume)
{
    HAL_ASSERT_VALUE(OS_NULL != data_out_p);
    if ((I2S_DATAFORMAT_16B         == i2s_hd.Init.DataFormat) ||
        (I2S_DATAFORMAT_16B_EXTENDED== i2s_hd.Init.DataFormat)) {
        while (--size) {
            U16* data_out_16p = (U16*)data_out_p;
            const U16 sample = (*data_out_16p / OS_AUDIO_VOLUME_MAX) * volume;
            *data_out_16p++ = sample;
        }
    } else if ((I2S_DATAFORMAT_24B == i2s_hd.Init.DataFormat) ||
               (I2S_DATAFORMAT_32B == i2s_hd.Init.DataFormat)) {
        size *= 2;
        while (--size) {
            U32* data_out_32p = (U32*)data_out_p;
            const U32 sample = (*data_out_32p / OS_AUDIO_VOLUME_MAX) * volume;
            *data_out_32p++ = sample;
        }
    } else { OS_LOG_S(D_WARNING, S_INVALID_VALUE); }
}

/******************************************************************************/
Status VolumeSet(const OS_AudioVolume volume)
{
Status s = S_UNDEF;
    cs4344_volume = volume;
    return s = S_OK;
}

/******************************************************************************/
S8 FrequencyIdxGet(const OS_AudioFreq freq)
{
    for (SIZE idx = 0; idx < ITEMS_COUNT_GET(i2s_freq_v, i2s_freq_v[0]); ++idx) {
        if (freq == i2s_freq_v[idx]) {
            return idx;
        }
    }
    return S8_MAX;
}

/******************************************************************************/
Status FrequencySet(const OS_AudioFreq freq)
{
RCC_PeriphCLKInitTypeDef RCC_ExCLKInitStruct;
const S8 freq_idx = FrequencyIdxGet(freq);
Status s = S_UNDEF;
    if (S8_MAX == freq_idx) { return S_INVALID_VALUE; }
    HAL_RCCEx_GetPeriphCLKConfig(&RCC_ExCLKInitStruct);
    /* I2S clock config
    PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) x (PLLI2SN/PLLM)
    I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR */
    RCC_ExCLKInitStruct.PeriphClockSelection= RCC_PERIPHCLK_I2S;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SN      = i2s_plln_v[freq_idx];
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SR      = i2s_pllr_v[freq_idx];
    HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
    IF_STATUS(s = CS4344_DeInit(OS_NULL))       { return s; }
    IF_STATUS(s = CS4344_LL_Init((void*)&freq)) { return s; }
    cs4344_freq = freq;
    return s;
}

/******************************************************************************/
//TODO(A. Filyanov) Move to I2S peripheral driver.
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (CS4344_I2Sx == hi2s->Instance) {
        //emit a signal
    }
}

/******************************************************************************/
//TODO(A. Filyanov) Move to I2S peripheral driver.
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    OS_ASSERT(OS_FALSE);
}

// I2S IRQ handlers-------------------------------------------------------------
/******************************************************************************/
void CS4344_I2Sx_DMAx_IRQHandler(void);
void CS4344_I2Sx_DMAx_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(CS4344_I2Sx_DMAx_IRQn);
    HAL_DMA_IRQHandler(i2s_hd.hdmatx);
}
