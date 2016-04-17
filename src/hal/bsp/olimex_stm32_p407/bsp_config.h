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

#define HAL_GPIO_DEBUG_1_PORT               GPIOA
#define HAL_GPIO_DEBUG_1_PIN                GPIO_PIN_4
#define HAL_GPIO_DEBUG_1_MODE               GPIO_MODE_OUTPUT_PP
#define HAL_GPIO_DEBUG_1_PULL               GPIO_NOPULL
#define HAL_GPIO_DEBUG_1_SPEED              GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_GPIO_DEBUG_1_ALT                0
#define HAL_GPIO_DEBUG_1_UP()               HAL_GPIO_WritePin(HAL_GPIO_DEBUG_1_PORT, HAL_GPIO_DEBUG_1_PIN, GPIO_PIN_SET)
#define HAL_GPIO_DEBUG_1_DOWN()             HAL_GPIO_WritePin(HAL_GPIO_DEBUG_1_PORT, HAL_GPIO_DEBUG_1_PIN, GPIO_PIN_RESET)
#define HAL_GPIO_DEBUG_1_TOGGLE()           HAL_GPIO_TogglePin(HAL_GPIO_DEBUG_1_PORT, HAL_GPIO_DEBUG_1_PIN)

#define HAL_GPIO_DEBUG_2_PORT               GPIOA
#define HAL_GPIO_DEBUG_2_PIN                GPIO_PIN_6
#define HAL_GPIO_DEBUG_2_MODE               GPIO_MODE_OUTPUT_PP
#define HAL_GPIO_DEBUG_2_PULL               GPIO_NOPULL
#define HAL_GPIO_DEBUG_2_SPEED              GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_GPIO_DEBUG_2_ALT                0
#define HAL_GPIO_DEBUG_2_UP()               HAL_GPIO_WritePin(HAL_GPIO_DEBUG_2_PORT, HAL_GPIO_DEBUG_2_PIN, GPIO_PIN_SET)
#define HAL_GPIO_DEBUG_2_DOWN()             HAL_GPIO_WritePin(HAL_GPIO_DEBUG_2_PORT, HAL_GPIO_DEBUG_2_PIN, GPIO_PIN_RESET)
#define HAL_GPIO_DEBUG_2_TOGGLE()           HAL_GPIO_TogglePin(HAL_GPIO_DEBUG_2_PORT, HAL_GPIO_DEBUG_2_PIN)

#define HAL_GPIO_ASSERT_PORT                GPIOF
#define HAL_GPIO_ASSERT_PIN                 GPIO_PIN_8
#define HAL_GPIO_ASSERT_MODE                GPIO_MODE_OUTPUT_PP
#define HAL_GPIO_ASSERT_PULL                GPIO_NOPULL
#define HAL_GPIO_ASSERT_SPEED               GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_GPIO_ASSERT_ALT                 0
#define HAL_GPIO_ASSERT_UP()                HAL_GPIO_WritePin(HAL_GPIO_ASSERT_PORT, HAL_GPIO_ASSERT_PIN, GPIO_PIN_SET)
#define HAL_GPIO_ASSERT_DOWN()              HAL_GPIO_WritePin(HAL_GPIO_ASSERT_PORT, HAL_GPIO_ASSERT_PIN, GPIO_PIN_RESET)
#define HAL_GPIO_ASSERT_TOGGLE()            HAL_GPIO_TogglePin(HAL_GPIO_ASSERT_PORT, HAL_GPIO_ASSERT_PIN)

#define HAL_GPIO_LED_PULSE_PORT             GPIOF
#define HAL_GPIO_LED_PULSE_PIN              GPIO_PIN_6

#define HAL_GPIO_LED_FS_PORT                GPIOF
#define HAL_GPIO_LED_FS_PIN                 GPIO_PIN_7

#define HAL_GPIO_LED_ASSERT_PORT            GPIOF
#define HAL_GPIO_LED_ASSERT_PIN             GPIO_PIN_8

#define HAL_GPIO_LED_USER_PORT              GPIOF
#define HAL_GPIO_LED_USER_PIN               GPIO_PIN_9

#define HAL_GPIO_BUTTON_WAKEUP_PORT         GPIOA
#define HAL_GPIO_BUTTON_WAKEUP_PIN          GPIO_PIN_0
#define HAL_GPIO_BUTTON_WAKEUP_MODE         (GPIO_MODE_IT_RISING_FALLING | GPIO_MODE_EVT_RISING)
#define HAL_GPIO_BUTTON_WAKEUP_PULL         GPIO_NOPULL
#define HAL_GPIO_BUTTON_WAKEUP_SPEED        GPIO_SPEED_FREQ_LOW
#define HAL_GPIO_BUTTON_WAKEUP_ALT          0
#define HAL_GPIO_BUTTON_WAKEUP_EXTI_IRQ     EXTI0_IRQn

#define HAL_GPIO_BUTTON_TAMPER_PORT         GPIOC
#define HAL_GPIO_BUTTON_TAMPER_PIN          GPIO_PIN_13
#define HAL_GPIO_BUTTON_TAMPER_MODE         (GPIO_MODE_IT_RISING_FALLING)
#define HAL_GPIO_BUTTON_TAMPER_PULL         GPIO_NOPULL
#define HAL_GPIO_BUTTON_TAMPER_SPEED        GPIO_SPEED_FREQ_LOW
#define HAL_GPIO_BUTTON_TAMPER_ALT          GPIO_AF0_TAMPER
#define HAL_GPIO_BUTTON_TAMPER_EXTI_IRQ     TAMP_STAMP_IRQn

#define HAL_USART_DEBUG_ITF                 USART6
#define HAL_USART_DEBUG_CLK_ENABLE()        __USART6_CLK_ENABLE()
#define HAL_USART_DEBUG_DMA_CLK_ENABLE()    __DMA2_CLK_ENABLE()
#define HAL_USART_DEBUG_FORCE_RESET()       __USART6_FORCE_RESET()
#define HAL_USART_DEBUG_RELEASE_RESET()     __USART6_RELEASE_RESET()
#define HAL_USART_DEBUG_IRQ                 USART6_IRQn
#define HAL_USART_DEBUG_IRQ_HANDLER         USART6_IRQHandler
#define HAL_USART_DEBUG_GPIO_CLK_RX_ENABLE() __GPIOG_CLK_ENABLE()
#define HAL_USART_DEBUG_GPIO_AF             GPIO_AF8_USART6
#define HAL_USART_DEBUG_GPIO_AF_RX          HAL_USART_DEBUG_GPIO_AF
#define HAL_USART_DEBUG_GPIO_PORT_RX        GPIOG
#define HAL_USART_DEBUG_GPIO_PIN_RX         GPIO_PIN_9
#define HAL_USART_DEBUG_GPIO_MODE_RX        GPIO_MODE_AF_PP
#define HAL_USART_DEBUG_GPIO_PULL_RX        GPIO_PULLUP
#define HAL_USART_DEBUG_GPIO_SPEED_RX       GPIO_SPEED_FREQ_HIGH
#define HAL_USART_DEBUG_GPIO_ALT_RX         HAL_USART_DEBUG_GPIO_AF_RX
#define HAL_USART_DEBUG_GPIO_CLK_TX_ENABLE() __GPIOC_CLK_ENABLE()
#define HAL_USART_DEBUG_GPIO_AF_TX          HAL_USART_DEBUG_GPIO_AF
#define HAL_USART_DEBUG_GPIO_PORT_TX        GPIOC
#define HAL_USART_DEBUG_GPIO_PIN_TX         GPIO_PIN_6
#define HAL_USART_DEBUG_GPIO_MODE_TX        GPIO_MODE_AF_PP
#define HAL_USART_DEBUG_GPIO_PULL_TX        GPIO_PULLUP
#define HAL_USART_DEBUG_GPIO_SPEED_TX       GPIO_SPEED_FREQ_HIGH
#define HAL_USART_DEBUG_GPIO_ALT_TX         HAL_USART_DEBUG_GPIO_AF_TX

#define HAL_ADC_TRIMMER_GPIO_CLK_ENABLE()   __GPIOC_CLK_ENABLE()
#define HAL_ADC_TRIMMER_GPIO_PORT           GPIOC
#define HAL_ADC_TRIMMER_GPIO_PIN            GPIO_PIN_0
#define HAL_ADC_TRIMMER_GPIO_MODE           GPIO_MODE_ANALOG
#define HAL_ADC_TRIMMER_GPIO_PULL           GPIO_NOPULL
#define HAL_ADC_TRIMMER_GPIO_SPEED          GPIO_SPEED_FREQ_LOW
#define HAL_ADC_TRIMMER_GPIO_ALT            0

/**SDIO GPIO Configuration
PC8     ------> SDIO_D0
PC9     ------> SDIO_D1
PC10    ------> SDIO_D2
PC11    ------> SDIO_D3
PC12    ------> SDIO_CK
PD2     ------> SDIO_CMD
*/
#define HAL_SD_ITF                          SDIO
#define HAL_SD_CLK_ENABLE()                 __SDIO_CLK_ENABLE()
#define HAL_SD_CLK_DISABLE()                __SDIO_CLK_DISABLE()
#define HAL_SD_IRQ                          SDIO_IRQn
#define HAL_SD_IRQ_HANDLER                  SDIO_IRQHandler
#define HAL_SD_GPIO_CLK_ENABLE()            __GPIOC_CLK_ENABLE(); __GPIOD_CLK_ENABLE()
#define HAL_SD_GPIO_CLK_DISABLE()           __GPIOC_CLK_DISABLE(); __GPIOD_CLK_DISABLE()
#define HAL_SD_GPIO_AF                      GPIO_AF12_SDIO
#define HAL_SD_GPIO1_PORT                   GPIOC
#define HAL_SD_GPIO1_PIN                    (GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12)
#define HAL_SD_GPIO1_MODE                   GPIO_MODE_AF_PP
#define HAL_SD_GPIO1_PULL                   GPIO_PULLUP
#define HAL_SD_GPIO1_SPEED                  GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_SD_GPIO1_ALT                    HAL_SD_GPIO_AF
#define HAL_SD_GPIO2_PORT                   GPIOD
#define HAL_SD_GPIO2_PIN                    GPIO_PIN_2
#define HAL_SD_GPIO2_MODE                   GPIO_MODE_AF_PP
#define HAL_SD_GPIO2_PULL                   GPIO_PULLUP
#define HAL_SD_GPIO2_SPEED                  GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_SD_GPIO2_ALT                    HAL_SD_GPIO_AF

// USB OTG FS
#define HAL_USB_OTG_FS_ITF                  USB_OTG_FS
#define HAL_USB_OTG_HS_ITF                  USB_OTG_HS
#define HAL_USB_OTG_FS_CLK_ENABLE()         __USB_OTG_FS_CLK_ENABLE()
#define HAL_USB_OTG_FS_CLK_DISABLE()        __USB_OTG_FS_CLK_DISABLE()
#define HAL_USB_OTG_HS_CLK_ENABLE()         __USB_OTG_HS_CLK_ENABLE()
#define HAL_USB_OTG_HS_CLK_DISABLE()        __USB_OTG_HS_CLK_DISABLE()
#define HAL_USB_OTG_FS_IRQ                  OTG_FS_IRQn
#define HAL_USB_OTG_HS_IRQ                  OTG_HS_IRQn

#define HAL_USB_OTG_FS_GPIO_CLK_ENABLE()    __GPIOA_CLK_ENABLE(); __GPIOB_CLK_ENABLE(); __GPIOC_CLK_ENABLE()
#define HAL_USB_OTG_FS_GPIO_AF              GPIO_AF10_OTG_FS

#define HAL_USB_OTG_FS_GPIO1_PORT           GPIOA
#define HAL_USB_OTG_FS_GPIO1_PIN            (GPIO_PIN_9)
#define HAL_USB_OTG_FS_GPIO1_MODE           GPIO_MODE_INPUT
#define HAL_USB_OTG_FS_GPIO1_PULL           GPIO_NOPULL
#define HAL_USB_OTG_FS_GPIO1_SPEED          GPIO_SPEED_FREQ_LOW
#define HAL_USB_OTG_FS_GPIO1_ALT            0

#define HAL_USB_OTG_FS_GPIO2_PORT           GPIOA
#define HAL_USB_OTG_FS_GPIO2_PIN            (GPIO_PIN_11 | GPIO_PIN_12)
#define HAL_USB_OTG_FS_GPIO2_MODE           GPIO_MODE_AF_PP
#define HAL_USB_OTG_FS_GPIO2_PULL           GPIO_NOPULL
#define HAL_USB_OTG_FS_GPIO2_SPEED          GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_USB_OTG_FS_GPIO2_ALT            HAL_USB_OTG_FS_GPIO_AF

#define HAL_USB_OTG_FS_GPIO3_PORT           GPIOC
#define HAL_USB_OTG_FS_GPIO3_PIN            (GPIO_PIN_2)
#define HAL_USB_OTG_FS_GPIO3_MODE           GPIO_MODE_OUTPUT_PP
#define HAL_USB_OTG_FS_GPIO3_PULL           GPIO_NOPULL
#define HAL_USB_OTG_FS_GPIO3_SPEED          GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_USB_OTG_FS_GPIO3_ALT            0

#define HAL_USB_OTG_FS_GPIO4_PORT           GPIOB
#define HAL_USB_OTG_FS_GPIO4_PIN            (GPIO_PIN_10)
#define HAL_USB_OTG_FS_GPIO4_MODE           GPIO_MODE_INPUT
#define HAL_USB_OTG_FS_GPIO4_PULL           GPIO_NOPULL
#define HAL_USB_OTG_FS_GPIO4_SPEED          GPIO_SPEED_FREQ_LOW
#define HAL_USB_OTG_FS_GPIO4_ALT            0

// USB OTG HS
#define HAL_USB_OTG_HS_GPIO_CLK_ENABLE()    __GPIOB_CLK_ENABLE(); __GPIOD_CLK_ENABLE(); __GPIOE_CLK_ENABLE()
#define HAL_USB_OTG_HS_GPIO_AF              GPIO_AF12_OTG_HS_FS

#define HAL_USB_OTG_HS_GPIO1_PORT           GPIOB
#define HAL_USB_OTG_HS_GPIO1_PIN            (GPIO_PIN_14 | GPIO_PIN_15)
#define HAL_USB_OTG_HS_GPIO1_MODE           GPIO_MODE_AF_PP
#define HAL_USB_OTG_HS_GPIO1_PULL           GPIO_NOPULL
#define HAL_USB_OTG_HS_GPIO1_SPEED          GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_USB_OTG_HS_GPIO1_ALT            HAL_USB_OTG_HS_GPIO_AF

#define HAL_USB_OTG_HS_GPIO2_PORT           GPIOB
#define HAL_USB_OTG_HS_GPIO2_PIN            (GPIO_PIN_12)
#define HAL_USB_OTG_HS_GPIO2_MODE           GPIO_MODE_AF_OD
#define HAL_USB_OTG_HS_GPIO2_PULL           GPIO_PULLUP
#define HAL_USB_OTG_HS_GPIO2_SPEED          GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_USB_OTG_HS_GPIO2_ALT            HAL_USB_OTG_HS_GPIO_AF

#define HAL_USB_OTG_HS_GPIO3_PORT           GPIOE
#define HAL_USB_OTG_HS_GPIO3_PIN            (GPIO_PIN_3)
#define HAL_USB_OTG_HS_GPIO3_MODE           GPIO_MODE_OUTPUT_PP
#define HAL_USB_OTG_HS_GPIO3_PULL           GPIO_NOPULL
#define HAL_USB_OTG_HS_GPIO3_SPEED          GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_USB_OTG_HS_GPIO3_ALT            0

#define HAL_USB_OTG_HS_GPIO4_PORT           GPIOB
#define HAL_USB_OTG_HS_GPIO4_PIN            (GPIO_PIN_13)
#define HAL_USB_OTG_HS_GPIO4_MODE           GPIO_MODE_INPUT
#define HAL_USB_OTG_HS_GPIO4_PULL           GPIO_NOPULL
#define HAL_USB_OTG_HS_GPIO4_SPEED          GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_USB_OTG_HS_GPIO4_ALT            0

// ETH
#define HAL_ETH_ITF                         ETH
#define HAL_ETH_CLK_ENABLE()                __ETH_CLK_ENABLE()
#define HAL_ETH_CLK_DISABLE()               __ETH_CLK_DISABLE()
#define HAL_ETH_MAC_TX_CLK_ENABLE()         __ETHMACTX_CLK_ENABLE()
#define HAL_ETH_MAC_TX_CLK_DISABLE()        __ETHMACTX_CLK_DISABLE()
#define HAL_ETH_MAC_RX_CLK_ENABLE()         __ETHMACRX_CLK_ENABLE()
#define HAL_ETH_MAC_RX_CLK_DISABLE()        __ETHMACRX_CLK_DISABLE()
#define HAL_ETH_IRQ                         ETH_IRQn
#define HAL_ETH_IRQ_HANDLER                 ETH_IRQHandler

#define HAL_ETH_GPIO_CLK_ENABLE()           __GPIOA_CLK_ENABLE(); __GPIOB_CLK_ENABLE(); __GPIOC_CLK_ENABLE(); __GPIOG_CLK_ENABLE()
#define HAL_ETH_GPIO_AF                     GPIO_AF11_ETH

#define HAL_ETH_GPIO1_PORT                  GPIOC
#define HAL_ETH_GPIO1_PIN                   (GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5)
#define HAL_ETH_GPIO1_MODE                  GPIO_MODE_AF_PP
#define HAL_ETH_GPIO1_PULL                  GPIO_NOPULL
#define HAL_ETH_GPIO1_SPEED                 GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_ETH_GPIO1_ALT                   HAL_ETH_GPIO_AF

#define HAL_ETH_GPIO2_PORT                  GPIOA
#define HAL_ETH_GPIO2_PIN                   (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7)
#define HAL_ETH_GPIO2_MODE                  GPIO_MODE_AF_PP
#define HAL_ETH_GPIO2_PULL                  GPIO_NOPULL
#define HAL_ETH_GPIO2_SPEED                 GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_ETH_GPIO2_ALT                   HAL_ETH_GPIO_AF

#define HAL_ETH_GPIO4_PORT                  GPIOB
#define HAL_ETH_GPIO4_PIN                   (GPIO_PIN_11)
#define HAL_ETH_GPIO4_MODE                  GPIO_MODE_AF_PP
#define HAL_ETH_GPIO4_PULL                  GPIO_NOPULL
#define HAL_ETH_GPIO4_SPEED                 GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_ETH_GPIO4_ALT                   HAL_ETH_GPIO_AF

#define HAL_ETH_GPIO5_PORT                  GPIOG
#define HAL_ETH_GPIO5_PIN                   (GPIO_PIN_13 | GPIO_PIN_14)
#define HAL_ETH_GPIO5_MODE                  GPIO_MODE_AF_PP
#define HAL_ETH_GPIO5_PULL                  GPIO_NOPULL
#define HAL_ETH_GPIO5_SPEED                 GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_ETH_GPIO5_ALT                   GPIO_AF11_ETH

#define HAL_ETH_GPIO_MDINT_PORT             GPIOA
#define HAL_ETH_GPIO_MDINT_PIN              GPIO_PIN_3
#define HAL_ETH_GPIO_MDINT_MODE             GPIO_MODE_IT_FALLING
#define HAL_ETH_GPIO_MDINT_PULL             GPIO_NOPULL
#define HAL_ETH_GPIO_MDINT_SPEED            GPIO_SPEED_FREQ_VERY_HIGH
#define HAL_ETH_GPIO_MDINT_ALT              0
#define HAL_ETH_GPIO_MDINT_EXTI_IRQ         EXTI3_IRQn

#endif //_BSP_CONFIG_H_
