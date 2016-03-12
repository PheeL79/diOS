/**************************************************************************//**
* @file    hal_config_irq_prio.h
* @brief   Config IRQ priorities header file for BSP.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_CONFIG_IRQ_PRIO_H_
#define _HAL_CONFIG_IRQ_PRIO_H_

/// @note These values are depends on OS!
#define HAL_PRIORITY_INT_MAX                0x01
#define HAL_PRIORITY_INT_MIN                0x0E

#define HAL_PRIO_IRQ_ETH                    (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_ETH0_MDINT             (HAL_PRIORITY_INT_MIN)

#define HAL_PRIO_IRQ_USB_OTG_FS             (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_USB_OTG_HS             (HAL_PRIORITY_INT_MIN)

#define HAL_PRIO_IRQ_SDIO                   (HAL_PRIORITY_INT_MIN - 1)
#define HAL_PRIO_IRQ_DMA_SDIO_RX            (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_DMA_SDIO_TX            (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_DMA_SDIO_RX                (DMA_PRIORITY_HIGH)
#define HAL_PRIO_DMA_SDIO_TX                (DMA_PRIORITY_HIGH)

#define HAL_PRIO_IRQ_USART_DEBUG            (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_DMA_USART_DEBUG_RX     (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_DMA_USART_DEBUG_TX     (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_DMA_USART_DEBUG_RX         (DMA_PRIORITY_LOW)
#define HAL_PRIO_DMA_USART_DEBUG_TX         (DMA_PRIORITY_LOW)

#define HAL_PRIO_IRQ_DMA_SRAM               (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_DMA_I2S_CS4344         (HAL_PRIORITY_INT_MAX + 1)

#define HAL_PRIO_IRQ_ADC_TRIMMER            (HAL_PRIORITY_INT_MIN)

#define HAL_PRIO_IRQ_BUTTON_WAKEUP          (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_BUTTON_TAMPER          (HAL_PRIORITY_INT_MIN)

#define HAL_PRIO_IRQ_TIMER_TIMESTAMP        (HAL_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_TIMER_STOPWATCH        (HAL_PRIORITY_INT_MIN)

#endif //_HAL_CONFIG_IRQ_PRIO_H_