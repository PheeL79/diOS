/**************************************************************************//**
* @file    bsp_config.h
* @brief   BSP configuration.
* @author  A. Filyanov
******************************************************************************/
#ifndef _BSP_CONFIG_H_
#define _BSP_CONFIG_H_

/*
 * BSP identifier.
 * MB = Motherboard
 */
#define OLIMEX_STM32_P407
#define HAL_MB_NAME                     "Olimex STM32 P407"
#define HAL_MB_REV_MAJ                  1
#define HAL_MB_REV_MIN                  0

#define HAL_DEBUG_PIN1_CLK_ENABLE()     __GPIOA_CLK_ENABLE()
#define HAL_DEBUG_PIN1                  GPIO_PIN_4
#define HAL_DEBUG_PIN1_PORT             GPIOA
#define HAL_DEBUG_PIN1_UP()             HAL_GPIO_WritePin(HAL_DEBUG_PIN1_PORT, HAL_DEBUG_PIN1, GPIO_PIN_SET)
#define HAL_DEBUG_PIN1_DOWN()           HAL_GPIO_WritePin(HAL_DEBUG_PIN1_PORT, HAL_DEBUG_PIN1, GPIO_PIN_RESET)
#define HAL_DEBUG_PIN1_TOGGLE()         HAL_GPIO_TogglePin(HAL_DEBUG_PIN1_PORT, HAL_DEBUG_PIN1)

#define HAL_DEBUG_PIN2_CLK_ENABLE()     __GPIOA_CLK_ENABLE()
#define HAL_DEBUG_PIN2                  GPIO_PIN_6
#define HAL_DEBUG_PIN2_PORT             GPIOA
#define HAL_DEBUG_PIN2_UP()             HAL_GPIO_WritePin(HAL_DEBUG_PIN2_PORT, HAL_DEBUG_PIN2, GPIO_PIN_SET)
#define HAL_DEBUG_PIN2_DOWN()           HAL_GPIO_WritePin(HAL_DEBUG_PIN2_PORT, HAL_DEBUG_PIN2, GPIO_PIN_RESET)
#define HAL_DEBUG_PIN2_TOGGLE()         HAL_GPIO_TogglePin(HAL_DEBUG_PIN2_PORT, HAL_DEBUG_PIN2)

#define HAL_ASSERT_PIN_CLK_ENABLE()     __GPIOF_CLK_ENABLE()
#define HAL_ASSERT_PIN                  GPIO_PIN_8
#define HAL_ASSERT_PIN_PORT             GPIOF
#define HAL_ASSERT_PIN_UP()             HAL_GPIO_WritePin(HAL_ASSERT_PIN_PORT, HAL_ASSERT_PIN, GPIO_PIN_SET)
#define HAL_ASSERT_PIN_DOWN()           HAL_GPIO_WritePin(HAL_ASSERT_PIN_PORT, HAL_ASSERT_PIN, GPIO_PIN_RESET)
#define HAL_ASSERT_PIN_TOGGLE()         HAL_GPIO_TogglePin(HAL_ASSERT_PIN_PORT, HAL_ASSERT_PIN)

// Buttons
#define HAL_BUTTON_WAKEUP_PIN           GPIO_PIN_0

//#define DMA_STREAM_MEM2MEM8             DMA2_Stream7
//#define DMA_CHANNEL_MEM2MEM8            DMA_Channel_6
//#define DMA_IT_TCIF_MEM2MEM8            DMA_IT_TCIF7
//#define DMA_IT_TEIF_MEM2MEM8            DMA_IT_TEIF7
//
//#define DMA_STREAM_MEM2MEM32            DMA2_Stream1
//#define DMA_CHANNEL_MEM2MEM32           DMA_Channel_0
//#define DMA_IT_TCIF_MEM2MEM32           DMA_IT_TCIF1
//#define DMA_IT_TEIF_MEM2MEM32           DMA_IT_TEIF1

#define USART_DEBUG_ITF                 USART6
#define USART_DEBUG_CLK                 __USART6_CLK_ENABLE()
#define USART_DEBUG_DMA_CLK             __DMA2_CLK_ENABLE()
#define USART_DEBUG_GPIO_RX             GPIOG
#define USART_DEBUG_GPIO_TX             GPIOC
#define USART_DEBUG_GPIO_CLK_RX         __GPIOG_CLK_ENABLE()
#define USART_DEBUG_GPIO_CLK_TX         __GPIOC_CLK_ENABLE()
#define USART_DEBUG_GPIO_PIN_RX         GPIO_Pin_10
#define USART_DEBUG_GPIO_PIN_TX         GPIO_PIN_6
#define USART_DEBUG_GPIO_AF_RX          GPIO_AF8_USART6
#define USART_DEBUG_GPIO_AF_TX          GPIO_AF8_USART6

#define USART_DEBUG_DMA_CHAN_RX         DMA_CHANNEL_5
#define USART_DEBUG_DMA_STREAM_RX       DMA2_Stream2
#define USART_DEBUG_DMA_FLAG_RX         DMA1_FLAG_TC5
#define USART_DEBUG_DMA_CHAN_TX         DMA_CHANNEL_5
#define USART_DEBUG_DMA_STREAM_TX       DMA2_Stream7
#define USART_DEBUG_DMA_FLAG_TX         DMA1_FLAG_TC4
#define USART_DEBUG_IRQ                 USART6_IRQn
#define USART_DEBUG_IRQ_HANDLER         USART6_IRQHandler
#define USART_DEBUG_DMA_IRQ_CHAN_RX     DMA2_Stream2_IRQn
#define USART_DEBUG_DMA_IRQ_CHAN_TX     DMA2_Stream7_IRQn
#define USART_DEBUG_DMA_IRQ_HANDLER_RX  DMA2_Stream2_IRQHandler
#define USART_DEBUG_DMA_IRQ_HANDLER_TX  DMA2_Stream7_IRQHandler

#endif //_BSP_CONFIG_H_
