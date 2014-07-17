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

//------------------------------------------------------------------------------
static Status   USART6_Init(void);
static Status   USART6_DeInit(void);
static void     USART6_GPIO_Init(void);
static void     USART6_NVIC_Init(void);
static Status   USART6_Open(void* args_p);
static Status   USART6_Close(void);
static Status   USART6_Read(U8* data_in_p, U32 size, void* args_p);
static Status   USART6_Write(U8* data_out_p, U32 size, void* args_p);
static Status   USART6_IoCtl(const U32 request_id, void* args_p);

//------------------------------------------------------------------------------
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
USART_ClockInitTypeDef USART_ClockInitStruct;
USART_InitTypeDef USART_InitStructure;

    //D_LOG(D_INFO, "Init: ");
    OS_CriticalSectionEnter(); {
        USART6_GPIO_Init();
        USART_ClockStructInit(&USART_ClockInitStruct);
        USART_StructInit(&USART_InitStructure);

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6,  ENABLE);

        // Deinitialize USART6 peripheral
        USART_DeInit(USART6);

        // Configure USART6
        USART_InitStructure.USART_BaudRate              = 115200;
        USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits              = USART_StopBits_1;
        USART_InitStructure.USART_Parity                = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;
        USART_Init(USART6, &USART_InitStructure);

    //    USART_DMA_Init();
        USART6_NVIC_Init();

        USART_Cmd(USART6, ENABLE);
    } OS_CriticalSectionExit();
    return S_OK;
}

/******************************************************************************/
void USART6_GPIO_Init(void)
{
GPIO_InitTypeDef GPIO_InitStructure;

    //D_LOG(D_INFO, "GPIO Init: ");
    GPIO_StructInit(&GPIO_InitStructure);
    /* Enable the GPIOC & GPIOG clocks */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOG, ENABLE);

    // Connect PC6 to USART6_TX
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
    // Connect PG9 to USART6_RX
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource9, GPIO_AF_USART6);

    // Configure USART6 pins as alternate function
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    //D_TRACE(D_INFO, S_STRING_GET(S_OK));
}

/******************************************************************************/
void USART6_NVIC_Init(void)
{
NVIC_InitTypeDef NVIC_InitStructure;
    //D_LOG(D_INFO, "NVIC Init: ");
    NVIC_StructInit(&NVIC_InitStructure);
    // USART6 IRQ channel configuration
    NVIC_InitStructure.NVIC_IRQChannel                  = USART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= OS_PRIORITY_INT_MIN;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // Configure error interrupts for USART6.
    USART_ITConfig(USART6, USART_IT_ERR, ENABLE);
    USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);
    //D_TRACE(D_INFO, S_STRING_GET(S_OK));
}

/******************************************************************************/
Status USART6_DeInit(void)
{
    //TODO(A. Filyanov)
    USART_ITConfig(USART6, USART_IT_ERR, DISABLE);
    USART_ITConfig(USART6, USART_IT_RXNE, DISABLE);
    USART_Cmd(USART6, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, DISABLE);
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
    while (size--) {
        if (BIT_TEST(USART6->SR, USART_FLAG_FE | USART_FLAG_PE | USART_FLAG_ORE)) {
           s = S_HARDWARE_FAULT;
           break;
        }
        *data_in_p++ = (U8)(USART_ReceiveData(USART6) & 0xFF);
        //OS_ContextSwitchForce();
    }
    return s;
}

static BL is_sync = OS_TRUE;
/******************************************************************************/
Status USART6_Write(U8* data_out_p, U32 size, void* args_p)
{
Status s = S_OK;
//TODO(A. Filyanov) DMA transfer.
    is_sync = OS_FALSE;
    while (size--) {
        while (!BIT_TEST(USART6->SR, USART_FLAG_TXE)) {};
        USART_SendData(USART6, *data_out_p++);
        //OS_ContextSwitchForce();
    }
    while (!BIT_TEST(USART6->SR, USART_FLAG_TC)) {
        //OS_ContextSwitchForce();
    }
    is_sync = OS_TRUE;
    return s;
}

/******************************************************************************/
Status USART6_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_OK;
    switch (request_id) {
        case DRV_REQ_STD_SYNC:
            while (OS_TRUE != is_sync) {};
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

// USART IRQ handlers----------------------------------------------------------
/******************************************************************************/
void USART6_IRQHandler(void);
void USART6_IRQHandler(void)
{
extern OS_QueueHd stdio_qhd;

    if (RESET != USART_GetITStatus(USART6, USART_IT_RXNE)) {
        const U32 signal = OS_ISR_SIGNAL_CREATE(OS_SIG_STDIN, USART_ReceiveData(USART6));
        if (1 == OS_ISR_SIGNAL_EMIT(stdio_qhd, signal, OS_MSG_PRIO_NORMAL)) {
            OS_ContextSwitchForce();
        }
    } else {
        D_LOG(D_WARNING, "USART6: Error!");
        //D_ASSERT(OS_FALSE);
        USART_ClearFlag(USART6, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE | USART_FLAG_ORE);
    }
}
