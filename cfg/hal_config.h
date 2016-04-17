/**************************************************************************//**
* @file    hal_config.h
* @brief   Config header file for BSP.
* @author  A. Filyanov
******************************************************************************/
#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

#include "hal\bsp\olimex_stm32_p407\bsp_config.h"
#include "hal_config_prio.h"
#include "hal_config.S"

// HAL ------------------------------------------------------------------------

// DEBUG
#ifdef NDEBUG
#   define HAL_LOG_LEVEL                            L_WARNING
#else
#   define HAL_LOG_LEVEL                            L_DEBUG_1
#endif //NDEBUG
#define HAL_STDIO_TERM_HEIGHT                       24
#define HAL_STDIO_TERM_WIDTH                        80
#define HAL_STDIO_BUFF_LEN                          1024
#define drv_stdio_p                                 drv_usart_v[DRV_ID_USART6]
#define drv_stderr_p

//Tests
#define HAL_TEST_ENABLED                            0

// Common timeouts
#define HAL_TIMEOUT_DRIVER                          1000U

// PWR
#define HAL_PWR_LEVEL                               PWR_PVDLevel_6
#define HAL_PWR_REGULATOR_VOLTAGE_SCALE             PWR_REGULATOR_VOLTAGE_SCALE1
#define HAL_PWR_FLASH_LATENCY                       FLASH_LATENCY_5

#define HAL_PWR_RCC_OSCILLATOR_TYPE                 (RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE)
#define HAL_PWR_RCC_HSE_STATE                       RCC_HSE_ON
#define HAL_PWR_RCC_PLL_STATE                       RCC_PLL_ON
#define HAL_PWR_RCC_PLL_SOURCE                      RCC_PLLSOURCE_HSE
#define HAL_PWR_RCC_PLL_M                           25
#define HAL_PWR_RCC_PLL_N                           336
#define HAL_PWR_RCC_PLL_P                           RCC_PLLP_DIV2
#define HAL_PWR_RCC_PLL_Q                           7

#define HAL_PWR_RCC_CLOCK_TYPE                      (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2)
#define HAL_PWR_RCC_CLOCK_SYS_SOURCE                RCC_SYSCLKSOURCE_PLLCLK
#define HAL_PWR_RCC_CLOCK_DIV_AHB                   RCC_SYSCLK_DIV1
#define HAL_PWR_RCC_CLOCK_DIV_APB1                  RCC_HCLK_DIV4
#define HAL_PWR_RCC_CLOCK_DIV_APB2                  RCC_HCLK_DIV2

// RTC_CLOCK_SOURCE
//#define                                             RTC_CLOCK_SOURCE_LSI
#define                                             RTC_CLOCK_SOURCE_LSE
#define HAL_RTC_BACKUP_REGS_MAX                     20
#define HAL_RTC_YEAR_BASE                           2000U

// DMA
//#define HAL_DMA_STREAM_MEM2MEM8                     DMA2_Stream7
//#define HAL_DMA_CHANNEL_MEM2MEM8                    DMA_Channel_6
//#define HAL_DMA_IT_TCIF_MEM2MEM8                    DMA_IT_TCIF7
//#define HAL_DMA_IT_TEIF_MEM2MEM8                    DMA_IT_TEIF7
//
//#define HAL_DMA_STREAM_MEM2MEM32                    DMA2_Stream1
//#define HAL_DMA_CHANNEL_MEM2MEM32                   DMA_Channel_0
//#define HAL_DMA_IT_TCIF_MEM2MEM32                   DMA_IT_TCIF1
//#define HAL_DMA_IT_TEIF_MEM2MEM32                   DMA_IT_TEIF1

// CRC
#define HAL_CRC_ENABLED                             1

// Timers
// IWDG
// IWDG counter clock Frequency = LsiFreq / 32
// Counter Reload Value = 250ms / IWDG counter clock period
//                      = 0.25s / (32/LsiFreq)
//                      = LsiFreq / (32 * 4)
//                      = LsiFreq / 128 */
#define HAL_TIMER_IWDG_ENABLED                      0
#define HAL_TIMER_IWDG_ITF                          IWDG
#define HAL_TIMER_IWDG_PRESCALER                    IWDG_PRESCALER_32
#define HAL_TIMER_IWDG_TIMEOUT                      3000

#define HAL_TIMER_STOPWATCH_ITF                     TIM5
#define HAL_TIMER_STOPWATCH_CLK_ENABLE()            __TIM5_CLK_ENABLE()
#define HAL_TIMER_STOPWATCH_IRQ                     TIM5_IRQn
#define HAL_TIMER_STOPWATCH_IRQ_HANDLER             TIM5_IRQHandler
#define HAL_TIMER_STOPWATCH_PERIOD                  (~0)
#define HAL_TIMER_STOPWATCH_PRESCALER               (0)
#define HAL_TIMER_STOPWATCH_CLOCK_DIV               TIM_CLOCKDIVISION_DIV1
#define HAL_TIMER_STOPWATCH_COUNT_MODE              TIM_COUNTERMODE_UP

#define HAL_TIMER_TIMESTAMP_ITF                     TIM10
#define HAL_TIMER_TIMESTAMP_CLK_ENABLE()            __TIM10_CLK_ENABLE()
#define HAL_TIMER_TIMESTAMP_IRQ                     TIM1_UP_TIM10_IRQn
#define HAL_TIMER_TIMESTAMP_IRQ_HANDLER             TIM1_UP_TIM10_IRQHandler
#define HAL_TIMER_TIMESTAMP_TIMEOUT                 (20 * KHZ) //ms
#define HAL_TIMER_TIMESTAMP_PERIOD                  (HAL_TIMER_TIMESTAMP_TIMEOUT - 1)
#define HAL_TIMER_TIMESTAMP_PRESCALER               (((SystemCoreClock / 2) / HAL_TIMER_TIMESTAMP_TIMEOUT) - 1)
#define HAL_TIMER_TIMESTAMP_CLOCK_DIV               TIM_CLOCKDIVISION_DIV1
#define HAL_TIMER_TIMESTAMP_COUNT_MODE              TIM_COUNTERMODE_UP

#define HAL_TIMER_TRIMMER_ITF                       TIM8
#define HAL_TIMER_TRIMMER_CLK_ENABLE()              __TIM8_CLK_ENABLE()
#define HAL_TIMER_TRIMMER_FORCE_RESET()             __TIM8_FORCE_RESET()
#define HAL_TIMER_TRIMMER_RELEASE_RESET()           __TIM8_RELEASE_RESET()
#define HAL_TIMER_TRIMMER_IRQ                       TIM8_IRQn
#define HAL_TIMER_TRIMMER_IRQ_HANDLER               TIM8_IRQHandler
#define HAL_TIMER_TRIMMER_TIMEOUT                   (1000)
#define HAL_TIMER_TRIMMER_PERIOD                    (HAL_TIMER_TRIMMER_TIMEOUT - 1)
#define HAL_TIMER_TRIMMER_PRESCALER                 (((SystemCoreClock / 2) / HAL_TIMER_TRIMMER_TIMEOUT) - 1)
#define HAL_TIMER_TRIMMER_CLOCK_DIV                 TIM_CLOCKDIVISION_DIV1
#define HAL_TIMER_TRIMMER_COUNT_MODE                TIM_COUNTERMODE_UP
#define HAL_TIMER_TRIMMER_MASTER_OUT_TRIGGER        TIM_TRGO_UPDATE
#define HAL_TIMER_TRIMMER_MASTER_SLAVE_MODE         TIM_MASTERSLAVEMODE_DISABLE

// ADC
#define HAL_ADC_TRIMMER_ITF                         ADC3
#define HAL_ADC_TRIMMER_CLK_ENABLE()                __ADC3_CLK_ENABLE()
#define HAL_ADC_TRIMMER_FORCE_RESET()               __ADC_FORCE_RESET()
#define HAL_ADC_TRIMMER_RELEASE_RESET()             __ADC_RELEASE_RESET()
#define HAL_ADC_TRIMMER_IRQ                         ADC_IRQn
#define HAL_ADC_TRIMMER_CLOCK_PRESCALER             ADC_CLOCKPRESCALER_PCLK_DIV2
#define HAL_ADC_TRIMMER_RESOLUTION                  ADC_RESOLUTION12b
#define HAL_ADC_TRIMMER_SCAN_CONV_MODE              DISABLE
#define HAL_ADC_TRIMMER_CONT_CONV_MODE              ENABLE
#define HAL_ADC_TRIMMER_DISCONT_CONV_MODE           DISABLE
#define HAL_ADC_TRIMMER_DISCONT_CONV_NUM            0
#define HAL_ADC_TRIMMER_EXT_TRIG_CONV_EDGE          ADC_EXTERNALTRIGCONVEDGE_RISING
#define HAL_ADC_TRIMMER_EXT_TRIG_CONV               ADC_EXTERNALTRIGCONV_T8_TRGO
#define HAL_ADC_TRIMMER_DATA_ALIGN                  ADC_DATAALIGN_LEFT
#define HAL_ADC_TRIMMER_CONV_NUM                    1
#define HAL_ADC_TRIMMER_DMA_CONT_REQ                ENABLE
#define HAL_ADC_TRIMMER_EOC_SELECTION               ENABLE
#define HAL_ADC_TRIMMER_CHANNEL_REG                 ADC_CHANNEL_10
#define HAL_ADC_TRIMMER_CHANNEL_RANK                1
#define HAL_ADC_TRIMMER_CHANNEL_SAMPLING_TIME       ADC_SAMPLETIME_15CYCLES
#define HAL_ADC_TRIMMER_CHANNEL_OFFSET              0

// U(S)ART
#define HAL_USART_DEBUG_DMA_CHAN_RX                 DMA_CHANNEL_5
#define HAL_USART_DEBUG_DMA_STREAM_RX               DMA2_Stream2
#define HAL_USART_DEBUG_DMA_IRQ_RX                  DMA2_Stream2_IRQn
#define HAL_USART_DEBUG_DMA_IRQ_HANDLER_RX          DMA2_Stream2_IRQHandler
#define HAL_USART_DEBUG_DMA_DIRECTION_RX            DMA_PERIPH_TO_MEMORY
#define HAL_USART_DEBUG_DMA_PERIPH_INC_RX           DMA_PINC_DISABLE
#define HAL_USART_DEBUG_DMA_MEMORY_INC_RX           DMA_MINC_ENABLE
#define HAL_USART_DEBUG_DMA_PERIPH_DATA_ALIGN_RX    DMA_PDATAALIGN_BYTE
#define HAL_USART_DEBUG_DMA_MEMORY_DATA_ALIGN_RX    DMA_MDATAALIGN_BYTE
#define HAL_USART_DEBUG_DMA_MODE_RX                 DMA_NORMAL
#define HAL_USART_DEBUG_DMA_FIFO_MODE_RX            DMA_FIFOMODE_DISABLE
#define HAL_USART_DEBUG_DMA_FIFO_THRS_RX            DMA_FIFO_THRESHOLD_FULL
#define HAL_USART_DEBUG_DMA_MEMORY_BURST_RX         DMA_MBURST_INC4
#define HAL_USART_DEBUG_DMA_PERIPH_BURST_RX         DMA_PBURST_INC4

#define HAL_USART_DEBUG_DMA_CHAN_TX                 DMA_CHANNEL_5
#define HAL_USART_DEBUG_DMA_STREAM_TX               DMA2_Stream7
#define HAL_USART_DEBUG_DMA_IRQ_TX                  DMA2_Stream7_IRQn
#define HAL_USART_DEBUG_DMA_IRQ_HANDLER_TX          DMA2_Stream7_IRQHandler
#define HAL_USART_DEBUG_DMA_DIRECTION_TX            DMA_MEMORY_TO_PERIPH
#define HAL_USART_DEBUG_DMA_PERIPH_INC_TX           DMA_PINC_DISABLE
#define HAL_USART_DEBUG_DMA_MEMORY_INC_TX           DMA_MINC_ENABLE
#define HAL_USART_DEBUG_DMA_PERIPH_DATA_ALIGN_TX    DMA_PDATAALIGN_BYTE
#define HAL_USART_DEBUG_DMA_MEMORY_DATA_ALIGN_TX    DMA_MDATAALIGN_BYTE
#define HAL_USART_DEBUG_DMA_MODE_TX                 DMA_NORMAL
#define HAL_USART_DEBUG_DMA_PRIO_TX                 DMA_PRIORITY_LOW
#define HAL_USART_DEBUG_DMA_FIFO_MODE_TX            DMA_FIFOMODE_DISABLE
#define HAL_USART_DEBUG_DMA_FIFO_THRS_TX            DMA_FIFO_THRESHOLD_FULL
#define HAL_USART_DEBUG_DMA_MEMORY_BURST_TX         DMA_MBURST_INC4
#define HAL_USART_DEBUG_DMA_PERIPH_BURST_TX         DMA_PBURST_INC4

#define HAL_USART_DEBUG_BAUD_RATE                   115200
#define HAL_USART_DEBUG_WORD_LENGTH                 UART_WORDLENGTH_8B
#define HAL_USART_DEBUG_STOP_BITS                   UART_STOPBITS_1
#define HAL_USART_DEBUG_PARITY                      UART_PARITY_NONE
#define HAL_USART_DEBUG_HW_FLOW_CONTROL             UART_HWCONTROL_NONE
#define HAL_USART_DEBUG_MODE                        UART_MODE_TX_RX

// SDIO SD
#define HAL_SDIO_SD_ENABLED                         1
#define HAL_SD_DMA_CLK()                            __DMA2_CLK_ENABLE()
#define HAL_SD_DMA_CHAN_RX                          DMA_CHANNEL_4
#define HAL_SD_DMA_STREAM_RX                        DMA2_Stream3
#define HAL_SD_DMA_IRQ_RX                           DMA2_Stream3_IRQn
#define HAL_SD_DMA_IRQ_HANDLER_RX                   DMA2_Stream3_IRQHandler
#define HAL_SD_DMA_DIRECTION_RX                     DMA_PERIPH_TO_MEMORY
#define HAL_SD_DMA_PERIPH_INC_RX                    DMA_PINC_DISABLE
#define HAL_SD_DMA_MEMORY_INC_RX                    DMA_MINC_ENABLE
#define HAL_SD_DMA_PERIPH_DATA_ALIGN_RX             DMA_PDATAALIGN_WORD
#define HAL_SD_DMA_MEMORY_DATA_ALIGN_RX_BYTE        DMA_MDATAALIGN_BYTE
#define HAL_SD_DMA_MEMORY_DATA_ALIGN_RX_WORD        DMA_MDATAALIGN_WORD
#define HAL_SD_DMA_MODE_RX                          DMA_PFCTRL
#define HAL_SD_DMA_FIFO_MODE_RX                     DMA_FIFOMODE_ENABLE
#define HAL_SD_DMA_FIFO_THRS_RX                     DMA_FIFO_THRESHOLD_FULL
#define HAL_SD_DMA_MEMORY_BURST_RX_BYTE             DMA_MBURST_SINGLE
#define HAL_SD_DMA_MEMORY_BURST_RX_WORD             DMA_MBURST_INC4
#define HAL_SD_DMA_PERIPH_BURST_RX                  DMA_PBURST_INC4

#define HAL_SD_DMA_CHAN_TX                          DMA_CHANNEL_4
#define HAL_SD_DMA_STREAM_TX                        DMA2_Stream6
#define HAL_SD_DMA_IRQ_TX                           DMA2_Stream6_IRQn
#define HAL_SD_DMA_IRQ_HANDLER_TX                   DMA2_Stream6_IRQHandler
#define HAL_SD_DMA_DIRECTION_TX                     DMA_MEMORY_TO_PERIPH
#define HAL_SD_DMA_PERIPH_INC_TX                    DMA_PINC_DISABLE
#define HAL_SD_DMA_MEMORY_INC_TX                    DMA_MINC_ENABLE
#define HAL_SD_DMA_PERIPH_DATA_ALIGN_TX             DMA_PDATAALIGN_WORD
#define HAL_SD_DMA_MEMORY_DATA_ALIGN_TX_BYTE        DMA_MDATAALIGN_BYTE
#define HAL_SD_DMA_MEMORY_DATA_ALIGN_TX_WORD        DMA_MDATAALIGN_WORD
#define HAL_SD_DMA_MODE_TX                          DMA_PFCTRL
#define HAL_SD_DMA_FIFO_MODE_TX                     DMA_FIFOMODE_ENABLE
#define HAL_SD_DMA_FIFO_THRS_TX                     DMA_FIFO_THRESHOLD_FULL
#define HAL_SD_DMA_MEMORY_BURST_TX_BYTE             DMA_MBURST_SINGLE
#define HAL_SD_DMA_MEMORY_BURST_TX_WORD             DMA_MBURST_INC4
#define HAL_SD_DMA_PERIPH_BURST_TX                  DMA_PBURST_INC4

#define HAL_SDIO_SD_CLOCK_EDGE                      SDIO_CLOCK_EDGE_RISING
#define HAL_SDIO_SD_CLOCK_BYPASS                    SDIO_CLOCK_BYPASS_DISABLE
#define HAL_SDIO_SD_CLOCK_POWER_SAVE                SDIO_CLOCK_POWER_SAVE_DISABLE
#define HAL_SDIO_SD_CLOCK_DIV                       SDIO_TRANSFER_CLK_DIV
#define HAL_SDIO_SD_BUS_WIDE                        SDIO_BUS_WIDE_1B
//Do _NOT_ use HardwareFlowControl due 2.9.1 SDIO HW flow control errata.
#define HAL_SDIO_SD_HW_FLOW_CONTROL                 SDIO_HARDWARE_FLOW_CONTROL_DISABLE

#define HAL_SD_CARD_BLOCK_SIZE                      0x200
#define HAL_SD_CARD_SECTOR_SIZE                     HAL_SD_CARD_BLOCK_SIZE
#define HAL_SD_TIMEOUT_TRANSFER                     ((U32)100000000)

// USB OTG

// USB Host
#define HAL_USBH_ENABLED                            0
#define HAL_USBH_FS_ENABLED                         1
#define HAL_USBH_HS_ENABLED                         0

#define HAL_USBH_OTG_FS_HOST_CHANNELS               8
#define HAL_USBH_OTG_FS_DMA_ENABLE                  DISABLE
#define HAL_USBH_OTG_FS_LOW_POWER_ENABLE            DISABLE
#define HAL_USBH_OTG_FS_PHY_ITF                     HCD_PHY_EMBEDDED
#define HAL_USBH_OTG_FS_SOF_ENABLE                  DISABLE
#define HAL_USBH_OTG_FS_SPEED                       HCD_SPEED_FULL
#define HAL_USBH_OTG_FS_VBUS_SENS_ENABLE            ENABLE
#define HAL_USBH_OTG_FS_VBUS_EXT_ENABLE             ENABLE

#define HAL_USBH_OTG_HS_HOST_CHANNELS               11
#define HAL_USBH_OTG_HS_DMA_ENABLE                  ENABLE
#define HAL_USBH_OTG_HS_LOW_POWER_ENABLE            DISABLE
#define HAL_USBH_OTG_HS_PHY_ITF                     HCD_PHY_EMBEDDED
#define HAL_USBH_OTG_HS_SOF_ENABLE                  DISABLE
#define HAL_USBH_OTG_HS_SPEED                       HCD_SPEED_FULL
#define HAL_USBH_OTG_HS_VBUS_SENS_ENABLE            ENABLE
#define HAL_USBH_OTG_HS_VBUS_EXT_ENABLE             ENABLE

// USB Host Classes
#define HAL_USBH_HID_ENABLED                        0
#define HAL_USBH_MSC_ENABLED                        1
#define HAL_USBH_MSC_BLOCK_SIZE                     0x200
#define HAL_USBH_MSC_SECTOR_SIZE                    USBH_MSC_BLOCK_SIZE

// USB Device
#define HAL_USBD_ENABLED                            1
#define HAL_USBD_FS_ENABLED                         0
#define HAL_USBD_HS_ENABLED                         1

#define HAL_USBD_OTG_FS_DEV_ENDPOINTS               4
#define HAL_USBD_OTG_FS_USE_DED_EP1                 DISABLE
#define HAL_USBD_OTG_FS_EP0_MPS                     0x40
#define HAL_USBD_OTG_FS_DMA_ENABLE                  DISABLE
#define HAL_USBD_OTG_FS_LOW_POWER_ENABLE            DISABLE
#define HAL_USBD_OTG_FS_PHY_ITF                     PCD_PHY_EMBEDDED
#define HAL_USBD_OTG_FS_SOF_ENABLE                  DISABLE
#define HAL_USBD_OTG_FS_SPEED                       PCD_SPEED_FULL
#define HAL_USBD_OTG_FS_VBUS_SENS_ENABLE            ENABLE
#define HAL_USBD_OTG_FS_FIFO_SIZE_RX                0x80
#define HAL_USBD_OTG_FS_FIFO_SIZE_TX_0              0x40
#define HAL_USBD_OTG_FS_FIFO_SIZE_TX_1              0x80

#define HAL_USBD_OTG_HS_DEV_ENDPOINTS               6
#define HAL_USBD_OTG_HS_USE_DED_EP1                 DISABLE
#define HAL_USBD_OTG_HS_EP0_MPS                     0x40
/* Be aware that enabling DMA mode will result in data being sent only by
 multiple of 4 packet sizes. This is due to the fact that USB DMA does
 not allow sending data from non word-aligned addresses.
 For this specific application, it is advised to not enable this option
 unless required. */
#define HAL_USBD_OTG_HS_DMA_ENABLE                  ENABLE
#define HAL_USBD_OTG_HS_LOW_POWER_ENABLE            DISABLE
#define HAL_USBD_OTG_HS_PHY_ITF                     PCD_PHY_EMBEDDED
#define HAL_USBD_OTG_HS_SOF_ENABLE                  DISABLE
#define HAL_USBD_OTG_HS_SPEED                       PCD_SPEED_FULL
#define HAL_USBD_OTG_HS_VBUS_SENS_ENABLE            ENABLE
#define HAL_USBD_OTG_HS_FIFO_SIZE_RX                0x200
#define HAL_USBD_OTG_HS_FIFO_SIZE_TX_0              0x80
#define HAL_USBD_OTG_HS_FIFO_SIZE_TX_1              0x174

// USB Device Classes
#define HAL_USBD_AUDIO_ENABLED                      0
#define HAL_USBD_HID_ENABLED                        0
#define HAL_USBD_MSC_ENABLED                        1

// Ethernet
#define HAL_ETH_ENABLED                             1
#define HAL_ETH_AUTO_NEGOTIATION                    ETH_AUTONEGOTIATION_ENABLE
#define HAL_ETH_SPEED                               ETH_SPEED_100M
#define HAL_ETH_ADDR_PHYSICAL                       ETH0_PHY_ADDR
#define HAL_ETH_ADDR_MAC                            (U8*)(init_args_p->mac_addr_p)
#define HAL_ETH_MODE_RX                             ETH_RXINTERRUPT_MODE
#define HAL_ETH_MODE_DUPLEX                         ETH_MODE_FULLDUPLEX
#define HAL_ETH_MODE_CHECKSUM                       ETH_CHECKSUM_BY_HARDWARE
#define HAL_ETH_ITF_MEDIA                           ETH_MEDIA_INTERFACE_RMII
#define HAL_ETH_MEM_TYPE                            OS_MEM_RAM_INT_SRAM
#define HAL_ETH_MAC_ADDR_SIZE                       6
#define HAL_ETH_MAC_ADDR0                           0x00
#define HAL_ETH_MAC_ADDR1                           0x80
#define HAL_ETH_MAC_ADDR2                           0xE1
#define HAL_ETH_MAC_ADDR3                           0x00
#define HAL_ETH_MAC_ADDR4                           0x00
#define HAL_ETH_MAC_ADDR5                           0x00
#define HAL_ETH_MTU_SIZE                            1500

// Buttons
#define HAL_GPIO_BUTTON_TAMPER_FILTER               RTC_TAMPERFILTER_DISABLE
#define HAL_GPIO_BUTTON_TAMPER_PIN_SELECTION        RTC_TAMPERPIN_PC13
#define HAL_GPIO_BUTTON_TAMPER_TAMPER               RTC_TAMPER_1
#define HAL_GPIO_BUTTON_TAMPER_TRIGGER              RTC_TAMPERTRIGGER_RISINGEDGE
#define HAL_GPIO_BUTTON_TAMPER_SAMPLING_FREQ        RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV256
#define HAL_GPIO_BUTTON_TAMPER_PRECHARGE_DURATION   RTC_TAMPERPRECHARGEDURATION_1RTCCLK
#define HAL_GPIO_BUTTON_TAMPER_PULL_UP              RTC_TAMPER_PULLUP_DISABLE
#define HAL_GPIO_BUTTON_TAMPER_TIME_STAMP_DETECT    RTC_TIMESTAMPONTAMPERDETECTION_DISABLE

// LEDs
#define HAL_GPIO_LED_MODE                           GPIO_MODE_OUTPUT_PP
#define HAL_GPIO_LED_MODE_PWM                       GPIO_MODE_AF_PP
#define HAL_GPIO_LED_PULL                           GPIO_NOPULL
#define HAL_GPIO_LED_SPEED                          GPIO_SPEED_FREQ_LOW
#define HAL_GPIO_LED_ALT                            0

#define FOREACH_GPIO(GPIO)       \
        GPIO(GPIO_DEBUG_1)       \
        GPIO(GPIO_DEBUG_2)       \
        GPIO(GPIO_ASSERT)        \
        GPIO(GPIO_LED_PULSE)     \
        GPIO(GPIO_LED_FS)        \
        GPIO(GPIO_LED_ASSERT)    \
        GPIO(GPIO_LED_USER)      \
        GPIO(GPIO_BUTTON_WAKEUP) \
        GPIO(GPIO_BUTTON_TAMPER) \
        GPIO(GPIO_ETH_MDINT)     \
        GPIO(GPIO_LAST)          \

#define GENERATE_ENUM(ENUM)                         ENUM,
#define GENERATE_STRING(STRING)                     #STRING,

typedef enum {
    FOREACH_GPIO(GENERATE_ENUM)
} Gpio;

#ifdef _DRV_GPIO_C_
/// @note Define timers handlers for GPIO.
static TIM_HandleTypeDef gpio_led_user_tim;

ALIGN_BEGIN static const HAL_GPIO_InitStruct gpio_v[] ALIGN_END = {
    [GPIO_DEBUG_1] = {
        .port               = HAL_GPIO_DEBUG_1_PORT,
        .init = {
            .Pin            = HAL_GPIO_DEBUG_1_PIN,
            .Mode           = HAL_GPIO_DEBUG_1_MODE,
            .Pull           = HAL_GPIO_DEBUG_1_PULL,
            .Speed          = HAL_GPIO_DEBUG_1_SPEED,
            .Alternate      = HAL_GPIO_DEBUG_1_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_DEBUG_2] = {
        .port               = HAL_GPIO_DEBUG_2_PORT,
        .init = {
            .Pin            = HAL_GPIO_DEBUG_2_PIN,
            .Mode           = HAL_GPIO_DEBUG_2_MODE,
            .Pull           = HAL_GPIO_DEBUG_2_PULL,
            .Speed          = HAL_GPIO_DEBUG_2_SPEED,
            .Alternate      = HAL_GPIO_DEBUG_2_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_ASSERT] = {
        .port               = HAL_GPIO_ASSERT_PORT,
        .init = {
            .Pin            = HAL_GPIO_ASSERT_PIN,
            .Mode           = HAL_GPIO_ASSERT_MODE,
            .Pull           = HAL_GPIO_ASSERT_PULL,
            .Speed          = HAL_GPIO_ASSERT_SPEED,
            .Alternate      = HAL_GPIO_ASSERT_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_PULSE] = {
        .port               = HAL_GPIO_LED_PULSE_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_PULSE_PIN,
            .Mode           = HAL_GPIO_LED_MODE,
            .Pull           = HAL_GPIO_LED_PULL,
            .Speed          = HAL_GPIO_LED_SPEED,
            .Alternate      = HAL_GPIO_LED_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_FS] = {
        .port               = HAL_GPIO_LED_FS_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_FS_PIN,
            .Mode           = HAL_GPIO_LED_MODE,
            .Pull           = HAL_GPIO_LED_PULL,
            .Speed          = HAL_GPIO_LED_SPEED,
            .Alternate      = HAL_GPIO_LED_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_ASSERT] = {
        .port               = HAL_GPIO_LED_ASSERT_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_ASSERT_PIN,
            .Mode           = HAL_GPIO_LED_MODE,
            .Pull           = HAL_GPIO_LED_PULL,
            .Speed          = HAL_GPIO_LED_SPEED,
            .Alternate      = HAL_GPIO_LED_ALT
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_LED_USER] = {
        .port               = HAL_GPIO_LED_USER_PORT,
        .init = {
            .Pin            = HAL_GPIO_LED_USER_PIN,
            .Mode           = HAL_GPIO_LED_MODE_PWM,
            .Pull           = HAL_GPIO_LED_PULL,
            .Speed          = HAL_GPIO_LED_SPEED,
            .Alternate      = GPIO_AF9_TIM14
        },
        .timer_itf          = TIM14,
        .timer_channel      = TIM_CHANNEL_1,
        .timer_init = {
            .Period         = U16_MAX,
            .Prescaler      = 4,
            .ClockDivision  = TIM_CLOCKDIVISION_DIV4,
            .CounterMode    = TIM_COUNTERMODE_UP
        },
        .timer_oc = {
            .OCMode         = TIM_OCMODE_PWM1,
            .Pulse          = 0,
            .OCPolarity     = TIM_OCPOLARITY_HIGH,
            .OCNPolarity    = TIM_OCNPOLARITY_HIGH,
            .OCFastMode     = TIM_OCFAST_DISABLE,
            .OCIdleState    = TIM_OCIDLESTATE_RESET,
            .OCNIdleState   = TIM_OCNIDLESTATE_RESET
        },
        .timer_hd_p         = &gpio_led_user_tim,
        .is_inverted        = HAL_FALSE
    },
    [GPIO_BUTTON_WAKEUP] = {
        .port               = HAL_GPIO_BUTTON_WAKEUP_PORT,
        .init = {
            .Pin            = HAL_GPIO_BUTTON_WAKEUP_PIN,
            .Mode           = HAL_GPIO_BUTTON_WAKEUP_MODE,
            .Pull           = HAL_GPIO_BUTTON_WAKEUP_PULL,
            .Speed          = HAL_GPIO_BUTTON_WAKEUP_SPEED,
            .Alternate      = HAL_GPIO_BUTTON_WAKEUP_ALT
        },
        .exti = {
            .irq            = HAL_GPIO_BUTTON_WAKEUP_EXTI_IRQ,
            .nvic_prio_pre  = HAL_PRIO_IRQ_BUTTON_WAKEUP,
            .nvic_prio_sub  = 0
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_BUTTON_TAMPER] = {
        .port               = HAL_GPIO_BUTTON_TAMPER_PORT,
        .init = {
            .Pin            = HAL_GPIO_BUTTON_TAMPER_PIN,
            .Mode           = HAL_GPIO_BUTTON_TAMPER_MODE,
            .Pull           = HAL_GPIO_BUTTON_TAMPER_PULL,
            .Speed          = HAL_GPIO_BUTTON_TAMPER_SPEED,
            .Alternate      = HAL_GPIO_BUTTON_TAMPER_ALT
        },
        .exti = {
            .irq            = HAL_GPIO_BUTTON_TAMPER_EXTI_IRQ,
            .nvic_prio_pre  = HAL_PRIO_IRQ_BUTTON_TAMPER,
            .nvic_prio_sub  = 0
        },
        .is_inverted        = HAL_FALSE
    },
    [GPIO_ETH_MDINT] = {
        .port               = HAL_ETH_GPIO_MDINT_PORT,
        .init = {
            .Pin            = HAL_ETH_GPIO_MDINT_PIN,
            .Mode           = HAL_ETH_GPIO_MDINT_MODE,
            .Pull           = HAL_ETH_GPIO_MDINT_PULL,
            .Speed          = HAL_ETH_GPIO_MDINT_SPEED,
            .Alternate      = HAL_ETH_GPIO_MDINT_ALT
        },
        .exti = {
            .irq            = HAL_ETH_GPIO_MDINT_EXTI_IRQ,
            .nvic_prio_pre  = HAL_PRIO_IRQ_ETH0_MDINT,
            .nvic_prio_sub  = 0
        },
        .is_inverted        = HAL_FALSE
    },
};
#endif //_DRV_GPIO_C_

// Memory
#define DATA_IN_ExtSRAM
#define HAL_MEM_EXT_ENABLED                         1
#define HAL_MEM_BLOCK_SIZE_MIN                      8
#define HAL_MEM_INT_CCM_BASE_ADDRESS                0x10000000
#define HAL_MEM_INT_CCM_SIZE                        0x10000
#define HAL_MEM_EXT_SRAM_BASE_ADDRESS               0x60000000
#define HAL_MEM_EXT_SRAM_SIZE                       0x80000

// cstdlib redefenitions
// Memory
#define HAL_MemSet                                  memset
#define HAL_MemCmp                                  memcmp
#define HAL_MemCpy                                  memcpy
#define HAL_MemMov                                  memmove
// String
#define HAL_AtoI                                    atoi
#define HAL_StrLen                                  strlen
#define HAL_StrChr                                  strchr
#define HAL_StrCmp                                  strcmp
#define HAL_StrCpy                                  strcpy
#define HAL_StrCat                                  strcat
#define HAL_StrToK                                  strtok
#define HAL_StrToL                                  strtol
#define HAL_StrToUL                                 strtoul
#define HAL_StrNCpy                                 strncpy
#define HAL_SPrintF                                 sprintf
#define HAL_SNPrintF                                snprintf
#define HAL_SScanF                                  sscanf

// Locale
#define HAL_LOCALE_STRING_EN                        "en"
#define HAL_LOCALE_STRING_RU                        "ru"
#define HAL_LOCALE_DEFAULT                          HAL_LOCALE_STRING_EN

#endif // _HAL_CONFIG_H_