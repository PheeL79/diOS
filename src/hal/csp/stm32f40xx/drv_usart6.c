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
static Status   USART6_Init(void);
static Status   USART6_DeInit(void);
static void     USART6_GPIO_Init(void);
static void     USART6_NVIC_Init(void);
static void     USART6_DMA_Init(void);
static Status   USART6_Open(void* args_p);
static Status   USART6_Close(void);
static Status   USART6_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USART6_Write(U8* data_out_p, U32 size, void* args_p);
//static Status   USART6_IT_Read(U8* data_in_p, U32 size, void* args_p);
//static Status   USART6_IT_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USART6_DMA_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USART6_DMA_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USART6_IoCtl(const U32 request_id, void* args_p);

//------------------------------------------------------------------------------
static UART_HandleTypeDef   uart_handle;
static DMA_HandleTypeDef    hdma_tx;
static DMA_HandleTypeDef    hdma_rx;

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
Status USART6_Init(void)
{
    //HAL_LOG(D_INFO, "Init: ");
    USART6_GPIO_Init();
    USART6_DMA_Init();
    USART6_NVIC_Init();
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
    uart_handle.Instance        = USARTx;
    uart_handle.Init.BaudRate   = 115200;
    uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
    uart_handle.Init.StopBits   = UART_STOPBITS_1;
    uart_handle.Init.Parity     = UART_PARITY_NONE;
    uart_handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    uart_handle.Init.Mode       = UART_MODE_TX_RX;

    if (HAL_OK != HAL_UART_Init(&uart_handle)) {
        /* Initialization Error */
        return S_HARDWARE_FAULT;
    }
    /* Enable the UART Data Register not empty Interrupt */
    __HAL_UART_ENABLE_IT(&uart_handle, UART_IT_RXNE);
    return S_OK;
}

/******************************************************************************/
void USART6_GPIO_Init(void)
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
}

/******************************************************************************/
void USART6_DMA_Init(void)
{
    /* Enable DMA2 clock */
    DMAx_CLK_ENABLE();
      /*##-3- Configure the DMA streams ##########################################*/
    /* Configure the DMA handler for Transmission process */
    hdma_tx.Instance                 = USARTx_TX_DMA_STREAM;

    hdma_tx.Init.Channel             = USARTx_TX_DMA_CHANNEL;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&hdma_tx);

    /* Associate the initialized DMA handle to the UART handle */
    __HAL_LINKDMA(&uart_handle, hdmatx, hdma_tx);

    /* Configure the DMA handler for reception process */
    hdma_rx.Instance                 = USARTx_RX_DMA_STREAM;

    hdma_rx.Init.Channel             = USARTx_RX_DMA_CHANNEL;
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&hdma_rx);

    /* Associate the initialized DMA handle to the the UART handle */
    __HAL_LINKDMA(&uart_handle, hdmarx, hdma_rx);
}

/******************************************************************************/
void USART6_NVIC_Init(void)
{
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
}

/******************************************************************************/
Status USART6_DeInit(void)
{
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
    HAL_DMA_DeInit(&hdma_tx);
    /* De-Initialize the DMA Stream associate to reception process */
    HAL_DMA_DeInit(&hdma_rx);

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
Status USART6_Close(void)
{
    //TODO(A. Filyanov)
    return S_OK;
}

/******************************************************************************/
Status USART6_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_UART_Receive(&uart_handle, data_in_p, size, HAL_TIMEOUT_DRIVER)) { s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USART6_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    if (HAL_OK != HAL_UART_Transmit(&uart_handle, data_out_p, size, HAL_TIMEOUT_DRIVER)) { s = S_HARDWARE_FAULT; }
    return s;
}

/******************************************************************************/
Status USART6_DMA_Read(U8* data_in_p, U32 size, void* args_p)
{
Status s = S_OK;
    /*##-4- Wait for the end of the transfer ###################################*/
    while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_handle)) {};
    /*##-3- Put UART peripheral in reception process ###########################*/
    /* Any data received will be stored in "RxBuffer" buffer : the number max of
     data received is 10 */
    if (HAL_OK != HAL_UART_Receive_DMA(&uart_handle, data_in_p, size)) {
        /* Transfer error in reception process */
        s = S_HARDWARE_FAULT;
    }
    return s;
}

/******************************************************************************/
Status USART6_DMA_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
    /*##-4- Wait for the end of the transfer ###################################*/
    while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_handle)) {};
    /*##-5- Send the received Buffer ###########################################*/
    if (HAL_OK != HAL_UART_Transmit_DMA(&uart_handle, data_out_p, size)) {
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
            while (HAL_UART_STATE_READY != HAL_UART_GetState(&uart_handle)) {};
            break;
        case DRV_REQ_STD_MODE_IO_SET: {
            const HAL_DriverModeIo mode_io = (*(HAL_DriverModeIo*)args_p);
//            CRITICAL_ENTER(); {
                switch (mode_io) {
                    case DRV_MODE_IO_POLL:
                        drv_usart6.Read = USART6_Read;
                        drv_usart6.Write= USART6_Write;
                        break;
                    case DRV_MODE_IO_IT:
//                        drv_usart6.Read = USART6_IT_Read;
//                        drv_usart6.Write= USART6_IT_Write;
                        break;
                    case DRV_MODE_IO_DMA:
                        drv_usart6.Read = USART6_DMA_Read;
                        drv_usart6.Write= USART6_DMA_Write;
                        break;
                    case DRV_MODE_IO_USER:
                        drv_usart6.Read = OS_NULL;
                        drv_usart6.Write= USART6_DMA_Write;
                        break;
                    default:
                        s = S_INVALID_VALUE;
                        break;
                }
//            } CRITICAL_EXIT();
            }
            break;
        case DRV_REQ_STD_POWER_SET:
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
    HAL_UART_IRQHandler(&uart_handle);

    extern OS_QueueHd stdio_qhd;
    const OS_SignalData sig_data = (U16)(USARTx->DR & (U16)0x01FF);
    const OS_Signal signal = OS_ISR_SIGNAL_CREATE(OS_SIG_STDIN, sig_data);
    if (1 == OS_ISR_SIGNAL_EMIT(stdio_qhd, signal, OS_MSG_PRIO_NORMAL)) {
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
    HAL_DMA_IRQHandler(uart_handle.hdmarx);
}

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
    HAL_DMA_IRQHandler(uart_handle.hdmatx);
}