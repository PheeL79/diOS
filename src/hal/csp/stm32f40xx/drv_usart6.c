/**************************************************************************//**
* @file    drv_usart6.c
* @brief   USART6 driver.
* @author  A. Filyanov
******************************************************************************/
#include <stdarg.h>
#include "hal.h"

#include "os_supervise.h"
#include "os_time.h"
#include "os_signal.h"
#include "os_message.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_usart6"

#define USARTx                          USART6
#define USARTx_CLK_ENABLE()             __USART6_CLK_ENABLE()
#define DMAx_CLK_ENABLE()               __DMA2_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()     __GPIOG_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()     __GPIOC_CLK_ENABLE()

#define USARTx_FORCE_RESET()            __USART6_FORCE_RESET()
#define USARTx_RELEASE_RESET()          __USART6_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                   GPIO_PIN_6
#define USARTx_TX_GPIO_PORT             GPIOC
#define USARTx_TX_AF                    GPIO_AF8_USART6
#define USARTx_RX_PIN                   GPIO_PIN_9
#define USARTx_RX_GPIO_PORT             GPIOG
#define USARTx_RX_AF                    GPIO_AF8_USART6

/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_CHANNEL           DMA_CHANNEL_5
#define USARTx_TX_DMA_STREAM            DMA2_Stream7
#define USARTx_RX_DMA_CHANNEL           DMA_CHANNEL_5
#define USARTx_RX_DMA_STREAM            DMA2_Stream2

/* Definition for USARTx's NVIC */
#define USARTx_IRQn                     USART6_IRQn
#define USARTx_IRQHandler               USART6_IRQHandler
#define USARTx_DMA_TX_IRQn              DMA2_Stream7_IRQn
#define USARTx_DMA_RX_IRQn              DMA2_Stream2_IRQn
#define USARTx_DMA_TX_IRQHandler        DMA2_Stream7_IRQHandler
#define USARTx_DMA_RX_IRQHandler        DMA2_Stream2_IRQHandler

//------------------------------------------------------------------------------
static Status   USART6_Init(void* args_p);
static Status   USART6_DeInit(void* args_p);
static Status   USART6_LL_Init(void* args_p);
//static Status   USART6_LL_DeInit(void* args_p);
static Status   USART6_Open(void* args_p);
static Status   USART6_Close(void* args_p);
static Status   USART6_Read(void* data_in_p, SIZE size, void* args_p);
static Status   USART6_Write(void* data_out_p, SIZE size, void* args_p);
//static Status   USART6_IT_Read(void* data_in_p, SIZE size, void* args_p);
//static Status   USART6_IT_Write(void* data_out_p, SIZE size, void* args_p);
static Status   USART6_DMA_Read(void* data_in_p, SIZE size, void* args_p);
//static Status   USART6_DMA_Write(void* data_out_p, SIZE size, void* args_p);
static Status   USART6_IoCtl(const U32 request_id, void* args_p);

//------------------------------------------------------------------------------
static UART_HandleTypeDef   uart_hd;
static DMA_HandleTypeDef    dma_tx_hd;
static DMA_HandleTypeDef    dma_rx_hd;

/*static is excluded for stdio*/ HAL_DriverItf drv_usart6 = {
    .Init   = USART6_Init,
    .DeInit = USART6_DeInit,
    .Open   = USART6_Open,
    .Close  = USART6_Close,
    .Read   = USART6_Read,
    .Write  = USART6_Write,
    .IoCtl  = USART6_IoCtl
};

/******************************************************************************/
Status USART6_Init(void* args_p)
{
Status s = S_OK;
    //HAL_LOG(D_INFO, "Init: ");
    IF_STATUS(s = USART6_LL_Init(args_p)) { return s; }
    /* Enable USARTx clock */
    USARTx_CLK_ENABLE();
    /*##-1- Configure the UART peripheral ######################################*/
    /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
    /* UART6 configured as follow:
      - Word Length = 8 Bits
      - Stop Bit    = One Stop bit
      - Parity      = NO parity
      - BaudRate    = 115200 baud
      - Hardware flow control disabled (RTS and CTS signals) */
    uart_hd.Instance        = USARTx;
    uart_hd.Init.BaudRate   = 115200;
    uart_hd.Init.WordLength = UART_WORDLENGTH_8B;
    uart_hd.Init.StopBits   = UART_STOPBITS_1;
    uart_hd.Init.Parity     = UART_PARITY_NONE;
    uart_hd.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    uart_hd.Init.Mode       = UART_MODE_TX_RX;

    if (HAL_OK == HAL_UART_Init(&uart_hd)) {
        /* Enable the UART Data Register not empty Interrupt */
        __HAL_UART_ENABLE_IT(&uart_hd, UART_IT_RXNE);
    } else { s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USART6_LL_Init(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStruct;
    /* Enable GPIO clock */
    USARTx_TX_GPIO_CLK_ENABLE();
    USARTx_RX_GPIO_CLK_ENABLE();
      /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin         = USARTx_TX_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull        = GPIO_PULLUP;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate   = USARTx_TX_AF;

    HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin         = USARTx_RX_PIN;
    GPIO_InitStruct.Alternate   = USARTx_RX_AF;

    HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

    /* Enable DMA2 clock */
    DMAx_CLK_ENABLE();
      /*##-3- Configure the DMA streams ##########################################*/
    /* Configure the DMA handler for Transmission process */
    dma_tx_hd.Instance                 = USARTx_TX_DMA_STREAM;

    dma_tx_hd.Init.Channel             = USARTx_TX_DMA_CHANNEL;
    dma_tx_hd.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    dma_tx_hd.Init.PeriphInc           = DMA_PINC_DISABLE;
    dma_tx_hd.Init.MemInc              = DMA_MINC_ENABLE;
    dma_tx_hd.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_tx_hd.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    dma_tx_hd.Init.Mode                = DMA_NORMAL;
    dma_tx_hd.Init.Priority            = DMA_PRIORITY_LOW;
    dma_tx_hd.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    dma_tx_hd.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dma_tx_hd.Init.MemBurst            = DMA_MBURST_INC4;
    dma_tx_hd.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&dma_tx_hd);

    /* Associate the initialized DMA handle to the UART handle */
    __HAL_LINKDMA(&uart_hd, hdmatx, dma_tx_hd);

    /* Configure the DMA handler for reception process */
    dma_rx_hd.Instance                 = USARTx_RX_DMA_STREAM;

    dma_rx_hd.Init.Channel             = USARTx_RX_DMA_CHANNEL;
    dma_rx_hd.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    dma_rx_hd.Init.PeriphInc           = DMA_PINC_DISABLE;
    dma_rx_hd.Init.MemInc              = DMA_MINC_ENABLE;
    dma_rx_hd.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_rx_hd.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    dma_rx_hd.Init.Mode                = DMA_NORMAL;
    dma_rx_hd.Init.Priority            = DMA_PRIORITY_HIGH;
    dma_rx_hd.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    dma_rx_hd.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dma_rx_hd.Init.MemBurst            = DMA_MBURST_INC4;
    dma_rx_hd.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&dma_rx_hd);

    /* Associate the initialized DMA handle to the the UART handle */
    __HAL_LINKDMA(&uart_hd, hdmarx, dma_rx_hd);

    /*##-3- Configure the NVIC for IRQ #########################################*/
    /* NVIC configuration for interrupt (USARTx) */
    HAL_NVIC_SetPriority(USARTx_IRQn, OS_PRIORITY_INT_MIN, 0);
    HAL_NVIC_EnableIRQ(USARTx_IRQn);

    /*##-4- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt (USARTx_TX) */
    HAL_NVIC_SetPriority(USARTx_DMA_TX_IRQn, OS_PRIORITY_INT_MIN, 1);
    HAL_NVIC_EnableIRQ(USARTx_DMA_TX_IRQn);

    /* NVIC configuration for DMA transfer complete interrupt (USARTx_RX) */
    HAL_NVIC_SetPriority(USARTx_DMA_RX_IRQn, OS_PRIORITY_INT_MIN, 0);
    HAL_NVIC_EnableIRQ(USARTx_DMA_RX_IRQn);
    return S_OK;
}

/******************************************************************************/
Status USART6_DeInit(void* args_p)
{
    if (HAL_OK != HAL_UART_DeInit(&uart_hd)) { return S_HARDWARE_FAULT; }
    /*##-1- Reset peripherals ##################################################*/
    USARTx_FORCE_RESET();
    USARTx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks #################################*/
    /* Configure UART Tx as alternate function  */
    HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
    /* Configure UART Rx as alternate function  */
    HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);

    /*##-3- Disable the DMA Streams ############################################*/
    /* De-Initialize the DMA Stream associate to transmission process */
    HAL_DMA_DeInit(&dma_tx_hd);
    /* De-Initialize the DMA Stream associate to reception process */
    HAL_DMA_DeInit(&dma_rx_hd);

    /*##-4- Disable the NVIC for IRQ ###########################################*/
    HAL_NVIC_DisableIRQ(USARTx_IRQn);
    /*##-5- Disable the NVIC for DMA ###########################################*/
    HAL_NVIC_DisableIRQ(USARTx_DMA_TX_IRQn);
    HAL_NVIC_DisableIRQ(USARTx_DMA_RX_IRQn);
    return S_OK;
}

/******************************************************************************/
Status USART6_Open(void* args_p)
{
    //TODO(A. Filyanov)
    return S_OK;
}

/******************************************************************************/
Status USART6_Close(void* args_p)
{
    //TODO(A. Filyanov)
    return S_OK;
}

/******************************************************************************/
Status USART6_Read(void* data_in_p, SIZE size, void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_UART_Receive(&uart_hd, data_in_p, size, HAL_TIMEOUT_DRIVER)) { s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USART6_Write(void* data_out_p, SIZE size, void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_UART_Transmit(&uart_hd, data_out_p, size, HAL_TIMEOUT_DRIVER)) { s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USART6_DMA_Read(void* data_in_p, SIZE size, void* args_p)
{
Status s = S_OK;
    /*##-4- Wait for the end of the transfer ###################################*/
    while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_hd)) {};
    /*##-3- Put UART peripheral in reception process ###########################*/
    /* Any data received will be stored in "RxBuffer" buffer : the number max of
     data received is 10 */
    if (HAL_OK != HAL_UART_Receive_DMA(&uart_hd, data_in_p, size)) {
        /* Transfer error in reception process */
        s = S_HARDWARE_FAULT;
    }
    return s;
}

/******************************************************************************/
Status USART6_DMA_Write(void* data_out_p, SIZE size, void* args_p)
{
Status s = S_OK;
    /*##-4- Wait for the end of the transfer ###################################*/
    while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_hd)) {};
    /*##-5- Send the received Buffer ###########################################*/
    if (HAL_OK != HAL_UART_Transmit_DMA(&uart_hd, data_out_p, size)) {
        /* Transfer error in transmission process */
        s = S_HARDWARE_FAULT;
    }
    return s;
}

/******************************************************************************/
Status USART6_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_SYNC:
            while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_hd)) {};
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
                    while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_hd)) {};
                    break;
                default:
                    break;
            }
            break;
        default:
            HAL_LOG_S(D_WARNING, S_UNDEF_REQ_ID);
            break;
    }
    return s;
}

// USART IRQ handlers----------------------------------------------------------
/******************************************************************************/
/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream
  *         used for USART data transmission
  */
void USARTx_IRQHandler(void);
void USARTx_IRQHandler(void)
{
    HAL_UART_IRQHandler(&uart_hd);

    extern OS_QueueHd stdin_qhd;
    const OS_SignalData sig_data = (U16)(USARTx->DR & (U16)0x01FF);
    const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_USART6, OS_SIG_STDIN, sig_data);
    if (1 == OS_ISR_SignalSend(stdin_qhd, signal, OS_MSG_PRIO_NORMAL)) {
        OS_ContextSwitchForce();
    }
}

/******************************************************************************/
/**
  * @brief  This function handles DMA interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream
  *         used for USART data transmission
  */
void USARTx_DMA_RX_IRQHandler(void);
void USARTx_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(uart_hd.hdmarx);
}

/******************************************************************************/
/**
  * @brief  This function handles DMA interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream
  *         used for USART data reception
  */
void USARTx_DMA_TX_IRQHandler(void);
void USARTx_DMA_TX_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(USARTx_DMA_TX_IRQn);
    HAL_DMA_IRQHandler(uart_hd.hdmatx);
}