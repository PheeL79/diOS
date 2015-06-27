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
#define HAL_MB_OLIMEX_STM32_P407
#define HAL_MB_NAME                         "Olimex STM32 P407"
#define HAL_MB_REV_MAJ                      1
#define HAL_MB_REV_MIN                      0

#define HAL_DEBUG_PIN1_CLK_ENABLE()         __GPIOA_CLK_ENABLE()
#define HAL_DEBUG_PIN1_PIN                  GPIO_PIN_4
#define HAL_DEBUG_PIN1_GPIO                 GPIOA
#define HAL_DEBUG_PIN1_UP()                 HAL_GPIO_WritePin(HAL_DEBUG_PIN1_GPIO, HAL_DEBUG_PIN1_PIN, GPIO_PIN_SET)
#define HAL_DEBUG_PIN1_DOWN()               HAL_GPIO_WritePin(HAL_DEBUG_PIN1_GPIO, HAL_DEBUG_PIN1_PIN, GPIO_PIN_RESET)
#define HAL_DEBUG_PIN1_TOGGLE()             HAL_GPIO_TogglePin(HAL_DEBUG_PIN1_GPIO, HAL_DEBUG_PIN1_PIN)

#define HAL_DEBUG_PIN2_CLK_ENABLE()         __GPIOA_CLK_ENABLE()
#define HAL_DEBUG_PIN2_PIN                  GPIO_PIN_6
#define HAL_DEBUG_PIN2_GPIO                 GPIOA
#define HAL_DEBUG_PIN2_UP()                 HAL_GPIO_WritePin(HAL_DEBUG_PIN2_GPIO, HAL_DEBUG_PIN2_PIN, GPIO_PIN_SET)
#define HAL_DEBUG_PIN2_DOWN()               HAL_GPIO_WritePin(HAL_DEBUG_PIN2_GPIO, HAL_DEBUG_PIN2_PIN, GPIO_PIN_RESET)
#define HAL_DEBUG_PIN2_TOGGLE()             HAL_GPIO_TogglePin(HAL_DEBUG_PIN2_GPIO, HAL_DEBUG_PIN2_PIN)

#define HAL_ASSERT_PIN_CLK_ENABLE()         __GPIOF_CLK_ENABLE()
#define HAL_ASSERT_PIN_PIN                  GPIO_PIN_8
#define HAL_ASSERT_PIN_GPIO                 GPIOF
#define HAL_ASSERT_PIN_UP()                 HAL_GPIO_WritePin(HAL_ASSERT_PIN_GPIO, HAL_ASSERT_PIN_PIN, GPIO_PIN_SET)
#define HAL_ASSERT_PIN_DOWN()               HAL_GPIO_WritePin(HAL_ASSERT_PIN_GPIO, HAL_ASSERT_PIN_PIN, GPIO_PIN_RESET)
#define HAL_ASSERT_PIN_TOGGLE()             HAL_GPIO_TogglePin(HAL_ASSERT_PIN_GPIO, HAL_ASSERT_PIN_PIN)

// Buttons
#define HAL_BUTTON_WAKEUP_PIN               GPIO_PIN_0

//#define HAL_DMA_STREAM_MEM2MEM8             DMA2_Stream7
//#define HAL_DMA_CHANNEL_MEM2MEM8            DMA_Channel_6
//#define HAL_DMA_IT_TCIF_MEM2MEM8            DMA_IT_TCIF7
//#define HAL_DMA_IT_TEIF_MEM2MEM8            DMA_IT_TEIF7
//
//#define HAL_DMA_STREAM_MEM2MEM32            DMA2_Stream1
//#define HAL_DMA_CHANNEL_MEM2MEM32           DMA_Channel_0
//#define HAL_DMA_IT_TCIF_MEM2MEM32           DMA_IT_TCIF1
//#define HAL_DMA_IT_TEIF_MEM2MEM32           DMA_IT_TEIF1

#define HAL_USART_DEBUG_ITF                 USART6
#define HAL_USART_DEBUG_CLK_ENABLE()        __USART6_CLK_ENABLE()
#define HAL_USART_DEBUG_DMA_CLK_ENABLE()    __DMA2_CLK_ENABLE()
#define HAL_USART_DEBUG_FORCE_RESET()       __USART6_FORCE_RESET()
#define HAL_USART_DEBUG_RELEASE_RESET()     __USART6_RELEASE_RESET()
#define HAL_USART_DEBUG_GPIO_CLK_RX_ENABLE() __GPIOG_CLK_ENABLE()
#define HAL_USART_DEBUG_GPIO_RX             GPIOG
#define HAL_USART_DEBUG_GPIO_PIN_RX         GPIO_PIN_9
#define HAL_USART_DEBUG_GPIO_AF_RX          GPIO_AF8_USART6
#define HAL_USART_DEBUG_GPIO_CLK_TX_ENABLE() __GPIOC_CLK_ENABLE()
#define HAL_USART_DEBUG_GPIO_TX             GPIOC
#define HAL_USART_DEBUG_GPIO_PIN_TX         GPIO_PIN_6
#define HAL_USART_DEBUG_GPIO_AF_TX          GPIO_AF8_USART6

#define HAL_USART_DEBUG_IRQ                 USART6_IRQn
#define HAL_USART_DEBUG_IRQ_HANDLER         USART6_IRQHandler
#define HAL_USART_DEBUG_DMA_CHAN_RX         DMA_CHANNEL_5
#define HAL_USART_DEBUG_DMA_STREAM_RX       DMA2_Stream2
#define HAL_USART_DEBUG_DMA_IRQ_RX          DMA2_Stream2_IRQn
#define HAL_USART_DEBUG_DMA_IRQ_HANDLER_RX  DMA2_Stream2_IRQHandler
#define HAL_USART_DEBUG_DMA_CHAN_TX         DMA_CHANNEL_5
#define HAL_USART_DEBUG_DMA_STREAM_TX       DMA2_Stream7
#define HAL_USART_DEBUG_DMA_IRQ_TX          DMA2_Stream7_IRQn
#define HAL_USART_DEBUG_DMA_IRQ_HANDLER_TX  DMA2_Stream7_IRQHandler

#define HAL_ADC_TRIMMER_ITF                 ADC3
#define HAL_ADC_TRIMMER_CLK_ENABLE()        __ADC3_CLK_ENABLE()
#define HAL_ADC_TRIMMER_GPIO_CLK_ENABLE()   __GPIOC_CLK_ENABLE()
#define HAL_ADC_TRIMMER_GPIO                GPIOC
#define HAL_ADC_TRIMMER_GPIO_PIN            GPIO_PIN_0
#define HAL_ADC_TRIMMER_CHANNEL             ADC_CHANNEL_10
#define HAL_ADC_TRIMMER_FORCE_RESET()       __ADC_FORCE_RESET()
#define HAL_ADC_TRIMMER_RELEASE_RESET()     __ADC_RELEASE_RESET()
#define HAL_ADC_TRIMMER_IRQ                 ADC_IRQn

#define HAL_SD_CLK_ENABLE()                 __SDIO_CLK_ENABLE()
#define HAL_SD_CLK_DISABLE()                __SDIO_CLK_DISABLE()
#define HAL_SD_GPIO_CLK_ENABLE()            __GPIOC_CLK_ENABLE(); __GPIOD_CLK_ENABLE();
#define HAL_SD_GPIO_CLK_DISABLE()           __GPIOC_CLK_DISABLE(); __GPIOD_CLK_DISABLE();
#define HAL_SD_GPIO1                        GPIOC
#define HAL_SD_GPIO1_PIN                    (GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12)
#define HAL_SD_GPIO2                        GPIOD
#define HAL_SD_GPIO2_PIN                    GPIO_PIN_2
#define HAL_SD_GPIO_AF                      GPIO_AF12_SDIO
#define HAL_SD_IRQ                          SDIO_IRQn
#define HAL_SD_IRQ_HANDLER                  SDIO_IRQHandler
#define HAL_SD_DMA_CLK()                    __DMA2_CLK_ENABLE()
#define HAL_SD_DMA_CHAN_RX                  DMA_CHANNEL_4
#define HAL_SD_DMA_STREAM_RX                DMA2_Stream3
#define HAL_SD_DMA_IRQ_RX                   DMA2_Stream3_IRQn
#define HAL_SD_DMA_IRQ_HANDLER_RX           DMA2_Stream3_IRQHandler
#define HAL_SD_DMA_CHAN_TX                  DMA_CHANNEL_4
#define HAL_SD_DMA_STREAM_TX                DMA2_Stream6
#define HAL_SD_DMA_IRQ_TX                   DMA2_Stream6_IRQn
#define HAL_SD_DMA_IRQ_HANDLER_TX           DMA2_Stream6_IRQHandler

#define HAL_ETH_CLK_ENABLE()                __ETH_CLK_ENABLE()
#define HAL_ETH_CLK_DISABLE()               __ETH_CLK_DISABLE()
#define HAL_ETH_MAC_TX_CLK_ENABLE()         __ETHMACTX_CLK_ENABLE()
#define HAL_ETH_MAC_TX_CLK_DISABLE()        __ETHMACTX_CLK_DISABLE()
#define HAL_ETH_MAC_RX_CLK_ENABLE()         __ETHMACRX_CLK_ENABLE()
#define HAL_ETH_MAC_RX_CLK_DISABLE()        __ETHMACRX_CLK_DISABLE()
/**ETH GPIO Configuration
PC1     ------> ETH_MDC
PA1     ------> ETH_REF_CLK
PA2     ------> ETH_MDIO
PA3     ------> ETH_MDINT
PA7     ------> ETH_CRS_DV
PC4     ------> ETH_RXD0
PC5     ------> ETH_RXD1
PB11    ------> ETH_TX_EN
PG13    ------> ETH_TXD0
PG14    ------> ETH_TXD1
*/
#define HAL_ETH_GPIO_CLK_ENABLE()           __GPIOA_CLK_ENABLE(); __GPIOB_CLK_ENABLE(); __GPIOC_CLK_ENABLE(); __GPIOG_CLK_ENABLE();
#define HAL_ETH_GPIO1                       GPIOC
#define HAL_ETH_GPIO1_PIN                   (GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5)
#define HAL_ETH_GPIO2                       GPIOA
#define HAL_ETH_GPIO2_PIN                   (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7)
#define HAL_ETH_GPIO3                       GPIOA
#define HAL_ETH_GPIO3_PIN                   (GPIO_PIN_3)
#define HAL_ETH_GPIO4                       GPIOB
#define HAL_ETH_GPIO4_PIN                   (GPIO_PIN_11)
#define HAL_ETH_GPIO5                       GPIOG
#define HAL_ETH_GPIO5_PIN                   (GPIO_PIN_13 | GPIO_PIN_14)
#define HAL_ETH_GPIO_AF                     GPIO_AF11_ETH
#define HAL_ETH_IRQ                         ETH_IRQn
#define HAL_ETH_IRQ_HANDLER                 ETH_IRQHandler
#define HAL_ETH_MDINT_IRQ                   EXTI3_IRQn

#define HAL_TIMER_TIMESTAMP_IRQ             TIM1_UP_TIM10_IRQn
#define HAL_TIMER_TIMESTAMP_IRQ_HANDLER     TIM1_UP_TIM10_IRQHandler

#endif //_BSP_CONFIG_H_
