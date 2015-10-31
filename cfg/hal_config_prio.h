/**************************************************************************//**
* @file    hal_config_irq_prio.h
* @brief   Config IRQ priorities header file for BSP.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_CONFIG_IRQ_PRIO_H_
#define _HAL_CONFIG_IRQ_PRIO_H_

#define HAL_PRIO_IRQ_ETH                    (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_ETH0_MDINT             (OS_PRIORITY_INT_MIN)

#define HAL_PRIO_IRQ_USB_OTG_FS             (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_USB_OTG_HS             (OS_PRIORITY_INT_MIN)

#define HAL_PRIO_IRQ_SDIO                   (OS_PRIORITY_INT_MIN - 1)
#define HAL_PRIO_IRQ_DMA_SDIO_RX            (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_DMA_SDIO_TX            (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_DMA_SDIO_RX                (DMA_PRIORITY_HIGH)
#define HAL_PRIO_DMA_SDIO_TX                (DMA_PRIORITY_HIGH)

#define HAL_PRIO_IRQ_USART_DEBUG            (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_DMA_USART_DEBUG_RX     (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_DMA_USART_DEBUG_TX     (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_DMA_USART_DEBUG_RX         (DMA_PRIORITY_LOW)
#define HAL_PRIO_DMA_USART_DEBUG_TX         (DMA_PRIORITY_LOW)

#define HAL_PRIO_IRQ_DMA_SRAM               (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_DMA_I2S_CS4344         (OS_PRIORITY_INT_MAX + 1)

#define HAL_PRIO_IRQ_ADC_TRIMMER            (OS_PRIORITY_INT_MIN)

#define HAL_PRIO_IRQ_BUTTON_WAKEUP          (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_BUTTON_TAMPER          (OS_PRIORITY_INT_MIN)

#define HAL_PRIO_IRQ_TIMER_TIMESTAMP        (OS_PRIORITY_INT_MIN)
#define HAL_PRIO_IRQ_TIMER_STOPWATCH        (OS_PRIORITY_INT_MIN)

#endif //_HAL_CONFIG_IRQ_PRIO_H_