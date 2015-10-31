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
#include "os_mailbox.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_usart6"

//------------------------------------------------------------------------------
static Status   USART6_Init(void* args_p);
static Status   USART6_DeInit(void* args_p);
static Status   USART6_LL_Init(void* args_p);
//static Status   USART6_LL_DeInit(void* args_p);
static Status   USART6_Open(void* args_p);
static Status   USART6_Close(void* args_p);
static Status   USART6_Read(void* data_in_p, Size size, void* args_p);
static Status   USART6_Write(void* data_out_p, Size size, void* args_p);
//static Status   USART6_IT_Read(void* data_in_p, Size size, void* args_p);
//static Status   USART6_IT_Write(void* data_out_p, Size size, void* args_p);
static Status   USART6_DMA_Read(void* data_in_p, Size size, void* args_p);
//static Status   USART6_DMA_Write(void* data_out_p, Size size, void* args_p);
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
Status s = S_UNDEF;
    //HAL_LOG(L_INFO, "Init: ");
    IF_STATUS(s = USART6_LL_Init(args_p)) { return s; }
    /* Enable USARTx clock */
    HAL_USART_DEBUG_CLK_ENABLE();
    /*##-1- Configure the UART peripheral ######################################*/
    uart_hd.Instance        = HAL_USART_DEBUG_ITF;
    uart_hd.Init.BaudRate   = HAL_USART_DEBUG_BAUD_RATE;
    uart_hd.Init.WordLength = HAL_USART_DEBUG_WORD_LENGTH;
    uart_hd.Init.StopBits   = HAL_USART_DEBUG_STOP_BITS;
    uart_hd.Init.Parity     = HAL_USART_DEBUG_PARITY;
    uart_hd.Init.HwFlowCtl  = HAL_USART_DEBUG_HW_FLOW_CONTROL;
    uart_hd.Init.Mode       = HAL_USART_DEBUG_MODE;

    if (HAL_OK == HAL_UART_Init(&uart_hd)) {
        /* Enable the UART Data Register not empty Interrupt */
        __HAL_UART_ENABLE_IT(&uart_hd, UART_IT_RXNE);
    } else { s = S_HARDWARE_ERROR; }
    return s;
}

/******************************************************************************/
Status USART6_LL_Init(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStruct;
Status s = S_OK;
    /* Enable GPIO clock */
    HAL_USART_DEBUG_GPIO_CLK_TX_ENABLE();
    HAL_USART_DEBUG_GPIO_CLK_RX_ENABLE();
      /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin         = HAL_USART_DEBUG_GPIO_PIN_TX;
    GPIO_InitStruct.Mode        = HAL_USART_DEBUG_GPIO_MODE_TX;
    GPIO_InitStruct.Pull        = HAL_USART_DEBUG_GPIO_PULL_TX;
    GPIO_InitStruct.Speed       = HAL_USART_DEBUG_GPIO_SPEED_TX;
    GPIO_InitStruct.Alternate   = HAL_USART_DEBUG_GPIO_ALT_TX;
    HAL_GPIO_Init(HAL_USART_DEBUG_GPIO_PORT_TX, &GPIO_InitStruct);

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin         = HAL_USART_DEBUG_GPIO_PIN_RX;
    GPIO_InitStruct.Mode        = HAL_USART_DEBUG_GPIO_MODE_RX;
    GPIO_InitStruct.Pull        = HAL_USART_DEBUG_GPIO_PULL_RX;
    GPIO_InitStruct.Speed       = HAL_USART_DEBUG_GPIO_SPEED_RX;
    GPIO_InitStruct.Alternate   = HAL_USART_DEBUG_GPIO_ALT_RX;
    HAL_GPIO_Init(HAL_USART_DEBUG_GPIO_PORT_RX, &GPIO_InitStruct);

    /* Enable DMA2 clock */
    HAL_USART_DEBUG_DMA_CLK_ENABLE();
      /*##-3- Configure the DMA streams ##########################################*/
    /* Configure the DMA handler for Transmission process */
    dma_tx_hd.Instance                 = HAL_USART_DEBUG_DMA_STREAM_TX;
    dma_tx_hd.Init.Channel             = HAL_USART_DEBUG_DMA_CHAN_TX;
    dma_tx_hd.Init.Direction           = HAL_USART_DEBUG_DMA_DIRECTION_TX;
    dma_tx_hd.Init.PeriphInc           = HAL_USART_DEBUG_DMA_PERIPH_INC_TX;
    dma_tx_hd.Init.MemInc              = HAL_USART_DEBUG_DMA_MEMORY_INC_TX;
    dma_tx_hd.Init.PeriphDataAlignment = HAL_USART_DEBUG_DMA_PERIPH_DATA_ALIGN_TX;
    dma_tx_hd.Init.MemDataAlignment    = HAL_USART_DEBUG_DMA_MEMORY_DATA_ALIGN_TX;
    dma_tx_hd.Init.Mode                = HAL_USART_DEBUG_DMA_MODE_TX;
    dma_tx_hd.Init.Priority            = HAL_PRIO_DMA_USART_DEBUG_TX;
    dma_tx_hd.Init.FIFOMode            = HAL_USART_DEBUG_DMA_FIFO_MODE_TX;
    dma_tx_hd.Init.FIFOThreshold       = HAL_USART_DEBUG_DMA_FIFO_THRS_TX;
    dma_tx_hd.Init.MemBurst            = HAL_USART_DEBUG_DMA_MEMORY_BURST_TX;
    dma_tx_hd.Init.PeriphBurst         = HAL_USART_DEBUG_DMA_PERIPH_BURST_TX;
    HAL_DMA_Init(&dma_tx_hd);

    /* Associate the initialized DMA handle to the UART handle */
    __HAL_LINKDMA(&uart_hd, hdmatx, dma_tx_hd);

    /* Configure the DMA handler for reception process */
    dma_rx_hd.Instance                 = HAL_USART_DEBUG_DMA_STREAM_RX;
    dma_rx_hd.Init.Channel             = HAL_USART_DEBUG_DMA_CHAN_RX;
    dma_rx_hd.Init.Direction           = HAL_USART_DEBUG_DMA_DIRECTION_RX;
    dma_rx_hd.Init.PeriphInc           = HAL_USART_DEBUG_DMA_PERIPH_INC_RX;
    dma_rx_hd.Init.MemInc              = HAL_USART_DEBUG_DMA_MEMORY_INC_RX;
    dma_rx_hd.Init.PeriphDataAlignment = HAL_USART_DEBUG_DMA_PERIPH_DATA_ALIGN_RX;
    dma_rx_hd.Init.MemDataAlignment    = HAL_USART_DEBUG_DMA_MEMORY_DATA_ALIGN_RX;
    dma_rx_hd.Init.Mode                = HAL_USART_DEBUG_DMA_MODE_RX;
    dma_rx_hd.Init.Priority            = HAL_PRIO_DMA_USART_DEBUG_RX;
    dma_rx_hd.Init.FIFOMode            = HAL_USART_DEBUG_DMA_FIFO_MODE_RX;
    dma_rx_hd.Init.FIFOThreshold       = HAL_USART_DEBUG_DMA_FIFO_THRS_RX;
    dma_rx_hd.Init.MemBurst            = HAL_USART_DEBUG_DMA_MEMORY_BURST_RX;
    dma_rx_hd.Init.PeriphBurst         = HAL_USART_DEBUG_DMA_PERIPH_BURST_RX;
    HAL_DMA_Init(&dma_rx_hd);

    /* Associate the initialized DMA handle to the the UART handle */
    __HAL_LINKDMA(&uart_hd, hdmarx, dma_rx_hd);

    /*##-3- Configure the NVIC for IRQ #########################################*/
    /* NVIC configuration for interrupt (USARTx) */
    HAL_NVIC_SetPriority(HAL_USART_DEBUG_IRQ, HAL_PRIO_IRQ_USART_DEBUG, 0);
    HAL_NVIC_EnableIRQ(HAL_USART_DEBUG_IRQ);

    /*##-4- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt (USARTx_TX) */
    HAL_NVIC_SetPriority(HAL_USART_DEBUG_DMA_IRQ_TX, HAL_PRIO_IRQ_DMA_USART_DEBUG_TX, 0);
    HAL_NVIC_EnableIRQ(HAL_USART_DEBUG_DMA_IRQ_TX);

    /* NVIC configuration for DMA transfer complete interrupt (USARTx_RX) */
    HAL_NVIC_SetPriority(HAL_USART_DEBUG_DMA_IRQ_RX, HAL_PRIO_IRQ_DMA_USART_DEBUG_RX, 0);
    HAL_NVIC_EnableIRQ(HAL_USART_DEBUG_DMA_IRQ_RX);
    return s;
}

/******************************************************************************/
Status USART6_DeInit(void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_UART_DeInit(&uart_hd)) { return s = S_HARDWARE_ERROR; }
    /*##-1- Reset peripherals ##################################################*/
    HAL_USART_DEBUG_FORCE_RESET();
    HAL_USART_DEBUG_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks #################################*/
    /* Configure UART Tx as alternate function  */
    HAL_GPIO_DeInit(HAL_USART_DEBUG_GPIO_PORT_TX, HAL_USART_DEBUG_GPIO_PIN_TX);
    /* Configure UART Rx as alternate function  */
    HAL_GPIO_DeInit(HAL_USART_DEBUG_GPIO_PORT_RX, HAL_USART_DEBUG_GPIO_PIN_RX);

    /*##-3- Disable the DMA Streams ############################################*/
    /* De-Initialize the DMA Stream associate to transmission process */
    HAL_DMA_DeInit(&dma_tx_hd);
    /* De-Initialize the DMA Stream associate to reception process */
    HAL_DMA_DeInit(&dma_rx_hd);

    /*##-4- Disable the NVIC for IRQ ###########################################*/
    HAL_NVIC_DisableIRQ(HAL_USART_DEBUG_IRQ);
    /*##-5- Disable the NVIC for DMA ###########################################*/
    HAL_NVIC_DisableIRQ(HAL_USART_DEBUG_DMA_IRQ_TX);
    HAL_NVIC_DisableIRQ(HAL_USART_DEBUG_DMA_IRQ_RX);
    return s;
}

/******************************************************************************/
Status USART6_Open(void* args_p)
{
Status s = S_OK;
    //TODO(A. Filyanov)
    return s;
}

/******************************************************************************/
Status USART6_Close(void* args_p)
{
Status s = S_OK;
    //TODO(A. Filyanov)
    return s;
}

/******************************************************************************/
Status USART6_Read(void* data_in_p, Size size, void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_UART_Receive(&uart_hd, data_in_p, size, HAL_TIMEOUT_DRIVER)) { s = S_HARDWARE_ERROR; }
    return s;
}

/******************************************************************************/
Status USART6_Write(void* data_out_p, Size size, void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_UART_Transmit(&uart_hd, data_out_p, size, HAL_TIMEOUT_DRIVER)) { s = S_HARDWARE_ERROR; }
    return s;
}

/******************************************************************************/
Status USART6_DMA_Read(void* data_in_p, Size size, void* args_p)
{
Status s = S_OK;
    /*##-4- Wait for the end of the transfer ###################################*/
    while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_hd)) {};
    /*##-3- Put UART peripheral in reception process ###########################*/
    /* Any data received will be stored in "RxBuffer" buffer : the number max of
     data received is 10 */
    if (HAL_OK != HAL_UART_Receive_DMA(&uart_hd, data_in_p, size)) {
        /* Transfer error in reception process */
        s = S_HARDWARE_ERROR;
    }
    return s;
}

/******************************************************************************/
Status USART6_DMA_Write(void* data_out_p, Size size, void* args_p)
{
Status s = S_OK;
    /*##-4- Wait for the end of the transfer ###################################*/
    while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_hd)) {};
    /*##-5- Send the received Buffer ###########################################*/
    if (HAL_OK != HAL_UART_Transmit_DMA(&uart_hd, data_out_p, size)) {
        /* Transfer error in transmission process */
        s = S_HARDWARE_ERROR;
    }
    return s;
}

/******************************************************************************/
Status USART6_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
    switch (request_id) {
        case DRV_REQ_STD_SYNC:
            while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_hd)) {};
            break;
        case DRV_REQ_STD_POWER_SET:
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    s = S_OK;
                    break;
                case PWR_OFF:
                case PWR_SLEEP:
                case PWR_STOP:
                case PWR_STANDBY:
                case PWR_HIBERNATE:
                case PWR_SHUTDOWN:
                    while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_hd)) {};
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

// IRQ handlers-----------------------------------------------------------------
/******************************************************************************/
/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream
  *         used for USART data transmission
  */
void HAL_USART_DEBUG_IRQ_HANDLER(void);
void HAL_USART_DEBUG_IRQ_HANDLER(void)
{
    HAL_UART_IRQHandler(&uart_hd);

    extern OS_QueueHd stdin_qhd;
    const OS_SignalData sig_data = (U16)(HAL_USART_DEBUG_ITF->DR & (U16)0x01FF);
    const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_USART6, OS_SIG_STDIN, sig_data);
    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(stdin_qhd, signal, OS_MSG_PRIO_NORMAL));
}

/******************************************************************************/
/**
  * @brief  This function handles DMA interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream
  *         used for USART data transmission
  */
void HAL_USART_DEBUG_DMA_IRQ_HANDLER_RX(void);
void HAL_USART_DEBUG_DMA_IRQ_HANDLER_RX(void)
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
void HAL_USART_DEBUG_DMA_IRQ_HANDLER_TX(void);
void HAL_USART_DEBUG_DMA_IRQ_HANDLER_TX(void)
{
    HAL_NVIC_ClearPendingIRQ(HAL_USART_DEBUG_DMA_IRQ_TX);
    HAL_DMA_IRQHandler(uart_hd.hdmatx);
}