/**************************************************************************//**
* @file    hal_config_irq_prio.h
* @brief   Config IRQ priorities header file for BSP.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_CONFIG_IRQ_PRIO_H_
#define _HAL_CONFIG_IRQ_PRIO_H_

#define IRQ_PRIO_ETH                    (OS_PRIORITY_INT_MIN)
#define IRQ_PRIO_ETH0_MDINT             (OS_PRIORITY_INT_MIN)

#define IRQ_PRIO_OTG_FS                 (OS_PRIORITY_INT_MIN)
#define IRQ_PRIO_OTG_HS                 (OS_PRIORITY_INT_MIN)

#define IRQ_PRIO_SDIO                   (OS_PRIORITY_INT_MIN - 1)
#define IRQ_PRIO_SDIO_DMA               (OS_PRIORITY_INT_MIN)

#define IRQ_PRIO_USART6                 (OS_PRIORITY_INT_MIN)
#define IRQ_PRIO_USART6_DMA_RX          (OS_PRIORITY_INT_MIN)
#define IRQ_PRIO_USART6_DMA_TX          (OS_PRIORITY_INT_MIN)

#define IRQ_PRIO_SRAM_DMA               (OS_PRIORITY_INT_MIN)
#define IRQ_PRIO_I2S_DMA_CS4344         (OS_PRIORITY_INT_MAX + 1)

#define IRQ_PRIO_ADC3                   (OS_PRIORITY_INT_MIN)

#define IRQ_PRIO_EXTI0                  (OS_PRIORITY_INT_MIN)
#define IRQ_PRIO_TAMP_STAMP             (OS_PRIORITY_INT_MIN)

#define IRQ_PRIO_TIMER_TIMESTAMP        (OS_PRIORITY_INT_MIN)
#define IRQ_PRIO_TIMER_STOPWATCH        (OS_PRIORITY_INT_MIN)

#endif //_HAL_CONFIG_IRQ_PRIO_H_