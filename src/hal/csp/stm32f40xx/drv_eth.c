/**************************************************************************//**
* @file    drv_eth.c
* @brief   ETH driver.
* @author  A. Filyanov
******************************************************************************/
#include "hal.h"
#include "os_debug.h"
#include "os_supervise.h"
#include "os_semaphore.h"
#include "os_signal.h"
#include "os_time.h"
#include "os_network.h"

#if (ETH_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_eth"

#define ETH0_MDINT_IRQn                 EXTI3_IRQn

//-----------------------------------------------------------------------------
/// @brief   Init ETH.
/// @return  #Status.
Status ETH_Init_(void);

static Status   ETH__Init(void* args_p);
static Status   ETH__DeInit(void* args_p);
static Status   ETH_LL_Init(void* args_p);
static Status   ETH_LL_DeInit(void* args_p);
static Status   ETH_Open(void* args_p);
static Status   ETH_Close(void* args_p);
static Status   ETH_DMA_Read(void* data_in_p, Size size, void* args_p);
static Status   ETH_DMA_Write(void* data_out_p, Size size, void* args_p);
static Status   ETH_IoCtl(const U32 request_id, void* args_p);

//-----------------------------------------------------------------------------
ETH_HandleTypeDef       eth0_hd;
OS_QueueHd              netd_stdin_qhd;
HAL_DriverItf*          drv_eth_v[DRV_ID_ETH_LAST];

#if defined(__ICCARM__) /*!< IAR Compiler */
#   pragma data_alignment=4
#endif
ALIGN_BEGIN ETH_DMADescTypeDef dma_rx_desc_tab[ETH_RXBUFNB] ALIGN_END; /* Ethernet Rx MA Descriptor */

#if defined(__ICCARM__) /*!< IAR Compiler */
#   pragma data_alignment=4
#endif
ALIGN_BEGIN ETH_DMADescTypeDef dma_tx_desc_tab[ETH_TXBUFNB] ALIGN_END; /* Ethernet Tx DMA Descriptor */

#if defined(__ICCARM__) /*!< IAR Compiler */
#   pragma data_alignment=4
#endif
ALIGN_BEGIN U8 rx_buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] ALIGN_END; /* Ethernet Receive Buffer */

#if defined(__ICCARM__) /*!< IAR Compiler */
#   pragma data_alignment=4
#endif
ALIGN_BEGIN U8 tx_buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] ALIGN_END; /* Ethernet Transmit Buffer */

//-----------------------------------------------------------------------------
static HAL_DriverItf drv_eth0 = {
    .Init   = ETH__Init,
    .DeInit = ETH__DeInit,
    .Open   = ETH_Open,
    .Close  = ETH_Close,
    .Read   = ETH_DMA_Read,
    .Write  = ETH_DMA_Write,
    .IoCtl  = ETH_IoCtl
};

/*****************************************************************************/
Status ETH_Init_(void)
{
Status s = S_UNDEF;
    OS_MemSet(drv_eth_v, 0x0, sizeof(drv_eth_v));
    drv_eth_v[DRV_ID_ETH0] = &drv_eth0;
s = S_OK;
    return s;
}

/******************************************************************************/
Status ETH__Init(void* args_p)
{
const OS_NetworkItfInitArgs* init_args_p = (OS_NetworkItfInitArgs*)args_p;
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "Init: ");
    IF_OK(s = ETH_LL_Init(OS_NULL)) {
        /* Init ETH */
        eth0_hd.Instance             = ETH;
        eth0_hd.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
        eth0_hd.Init.Speed           = ETH_SPEED_100M;
        eth0_hd.Init.DuplexMode      = ETH_MODE_FULLDUPLEX;
        eth0_hd.Init.PhyAddress      = ETH0_PHY_ADDR;
        eth0_hd.Init.MACAddr         = (U8*)(init_args_p->mac_addr_p);
        eth0_hd.Init.RxMode          = ETH_RXINTERRUPT_MODE;
        eth0_hd.Init.ChecksumMode    = ETH_CHECKSUM_BY_HARDWARE;
        eth0_hd.Init.MediaInterface  = ETH_MEDIA_INTERFACE_RMII;
        if (HAL_OK == HAL_ETH_Init(&eth0_hd)) {
            IF_OK(s = DRV_ETH0_PHY.IoCtl(DRV_REQ_KS8721BL_PHY_ID_TEST, OS_NULL)) {
                IF_OK(s = DRV_ETH0_PHY.Init(OS_NULL)) {
                    /* Initialize Tx Descriptors list: Chain Mode */
                    OS_ASSERT(HAL_OK == HAL_ETH_DMATxDescListInit(&eth0_hd, dma_tx_desc_tab, &tx_buff[0][0], ETH_TXBUFNB));
                    /* Initialize Rx Descriptors list: Chain Mode  */
                    OS_ASSERT(HAL_OK == HAL_ETH_DMARxDescListInit(&eth0_hd, dma_rx_desc_tab, &rx_buff[0][0], ETH_RXBUFNB));
                }
            }
        } else { s = S_HARDWARE_FAULT; }
    }
    HAL_TRACE_S(D_INFO, s);
    return s;
}

/******************************************************************************/
Status ETH__DeInit(void* args_p)
{
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "DeInit: ");
    IF_OK(s = ETH_LL_DeInit(OS_NULL)) {}
    HAL_TRACE_S(D_INFO, s);
    return s;
}

/******************************************************************************/
Status ETH_LL_Init(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStruct;
Status s = S_UNDEF;
    s = S_OK;
    /* Peripheral clock enable */
    __ETH_CLK_ENABLE();
    __ETHMACTX_CLK_ENABLE();
    __ETHMACRX_CLK_ENABLE();
    /* Enable GPIOs clocks */
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();
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
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(ETH_IRQn, OS_PRIORITY_INT_MIN, 0);
    HAL_NVIC_SetPriority(ETH0_MDINT_IRQn, OS_PRIORITY_INT_MIN, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
    HAL_NVIC_EnableIRQ(ETH0_MDINT_IRQn);
    return s;
}

/******************************************************************************/
Status ETH_LL_DeInit(void* args_p)
{
Status s = S_UNDEF;
    IF_OK(s = DRV_ETH0_PHY.DeInit(OS_NULL)) {
        /* Peripheral clock disable */
        __ETHMACRX_CLK_DISABLE();
        __ETHMACTX_CLK_DISABLE();
        __ETH_CLK_DISABLE();
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
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_7);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);
        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_13|GPIO_PIN_14);
        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(ETH_IRQn);
        HAL_NVIC_DisableIRQ(ETH0_MDINT_IRQn);
    }
    return s;
}

/******************************************************************************/
Status ETH_Open(void* args_p)
{
const OS_NetworkItfOpenArgs* open_args_p = (OS_NetworkItfOpenArgs*)args_p;
Status s = S_UNDEF;
    netd_stdin_qhd = open_args_p->netd_stdin_qhd;
    IF_OK(s = DRV_ETH0_PHY.Open(OS_NULL)) {
        /* Enable MAC and DMA transmission and reception */
        if (HAL_OK == HAL_ETH_Start(&eth0_hd)) {
            IF_OK(s = DRV_ETH0_PHY.IoCtl(DRV_REQ_KS8721BL_LINK_INT_SETUP, OS_NULL)) {}
        } else { s = S_HARDWARE_FAULT; }
    }
    return s;
}

/******************************************************************************/
Status ETH_Close(void* args_p)
{
Status s = S_UNDEF;
    IF_OK(s = DRV_ETH0_PHY.Close(OS_NULL)) {
        if (HAL_OK == HAL_ETH_Stop(&eth0_hd)) {
            s = S_OK;
        } else {
            s = S_HARDWARE_FAULT;
        }
        netd_stdin_qhd = OS_NULL;
    }
    return s;
}

/******************************************************************************/
Status ETH_DMA_Read(void* data_in_p, Size size, void* args_p)
{
OS_NetworkBuf* p = OS_NULL;
OS_NetworkBuf* q = OS_NULL;
U8* buffer_p;
__IO ETH_DMADescTypeDef *rx_dma_desc;
U32 buffer_offset = 0;
U32 payload_offset = 0;
U32 bytes_left_to_copy = 0;
U32 i = 0;
U16 len = 0;
Status s = S_UNDEF;
    s = S_OK;
    /* get received frame */
    if (HAL_OK != HAL_ETH_GetReceivedFrame_IT(&eth0_hd)) {
        return (s = S_HARDWARE_FAULT);
    }
    /* Obtain the size of the packet and put it into the "len" variable. */
    len = eth0_hd.RxFrameInfos.length;
    buffer_p = (U8*)eth0_hd.RxFrameInfos.buffer;
    if (len > 0) {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }

    if (p) {
        rx_dma_desc = eth0_hd.RxFrameInfos.FSRxDesc;
        buffer_offset = 0;

        for (q = p; q != OS_NULL; q = q->next) {
            bytes_left_to_copy = q->len;
            payload_offset = 0;
            /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size*/
            while ((bytes_left_to_copy + buffer_offset) > ETH_RX_BUF_SIZE) {
                /* Copy data to pbuf*/
                OS_MemCpy((U8*)((U8*)q->payload + payload_offset), (U8*)((U8*)buffer_p + buffer_offset), (ETH_RX_BUF_SIZE - buffer_offset));
                /* Point to next descriptor */
                rx_dma_desc = (ETH_DMADescTypeDef*)(rx_dma_desc->Buffer2NextDescAddr);
                buffer_p = (U8*)(rx_dma_desc->Buffer1Addr);

                bytes_left_to_copy = bytes_left_to_copy - (ETH_RX_BUF_SIZE - buffer_offset);
                payload_offset = payload_offset + (ETH_RX_BUF_SIZE - buffer_offset);
                buffer_offset = 0;
            }
            /* Copy remaining data in pbuf */
            OS_MemCpy((U8*)((U8*)q->payload + payload_offset), (U8*)((U8*)buffer_p + buffer_offset), bytes_left_to_copy);
            buffer_offset = buffer_offset + bytes_left_to_copy;
        }
        /* Release descriptors to DMA */
        rx_dma_desc = eth0_hd.RxFrameInfos.FSRxDesc;
        /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
        for (i = 0; i < eth0_hd.RxFrameInfos.SegCount; ++i) {
            rx_dma_desc->Status |= ETH_DMARXDESC_OWN;
            rx_dma_desc = (ETH_DMADescTypeDef*)(rx_dma_desc->Buffer2NextDescAddr);
        }
        /* Clear Segment_Count */
        eth0_hd.RxFrameInfos.SegCount = 0;
    }
    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((U32)RESET != (eth0_hd.Instance->DMASR & ETH_DMASR_RBUS)) {
        /* Clear RBUS ETHERNET DMA flag */
        eth0_hd.Instance->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        eth0_hd.Instance->DMARPDR = 0;
    }
    *(OS_NetworkBuf**)args_p = p;
    return s;
}

/******************************************************************************/
Status ETH_DMA_Write(void* data_out_p, Size size, void* args_p)
{
OS_NetworkBuf* q;
U8* buffer_p = (U8*)(eth0_hd.TxDesc->Buffer1Addr);
__IO ETH_DMADescTypeDef *tx_dma_desc;
U32 frame_length = 0;
U32 buffer_offset = 0;
U32 bytes_left_to_copy = 0;
U32 payload_offset = 0;
err_t err_val;
Status s = S_UNDEF;

    tx_dma_desc = eth0_hd.TxDesc;
    buffer_offset = 0;
    /* copy frame from pbufs to driver buffers */
    for (q = data_out_p; q != NULL; q = q->next) {
        /* Is this buffer available? If not, goto error */
        if ((U32)RESET != (tx_dma_desc->Status & ETH_DMATXDESC_OWN)) {
            err_val = ERR_USE;
            goto error;
        }
        /* Get bytes in current lwIP buffer */
        bytes_left_to_copy = q->len;
        payload_offset = 0;
        /* Check if the length of data to copy is bigger than Tx buffer size*/
        while ((bytes_left_to_copy + buffer_offset) > ETH_TX_BUF_SIZE) {
            /* Copy data to Tx buffer*/
            OS_MemCpy((U8*)((U8*)buffer_p + buffer_offset), (U8*)((U8*)q->payload + payload_offset), (ETH_TX_BUF_SIZE - buffer_offset));
            /* Point to next descriptor */
            tx_dma_desc = (ETH_DMADescTypeDef*)(tx_dma_desc->Buffer2NextDescAddr);
            /* Check if the buffer is available */
            if ((U32)RESET != (tx_dma_desc->Status & ETH_DMATXDESC_OWN)) {
                err_val = ERR_USE;
                goto error;
            }

            buffer_p = (U8*)(tx_dma_desc->Buffer1Addr);

            bytes_left_to_copy = bytes_left_to_copy - (ETH_TX_BUF_SIZE - buffer_offset);
            payload_offset = payload_offset + (ETH_TX_BUF_SIZE - buffer_offset);
            frame_length = frame_length + (ETH_TX_BUF_SIZE - buffer_offset);
            buffer_offset = 0;
        }
        /* Copy the remaining bytes */
        OS_MemCpy((U8*)((U8*)buffer_p + buffer_offset), (U8*)((U8*)q->payload + payload_offset), bytes_left_to_copy);
        buffer_offset = buffer_offset + bytes_left_to_copy;
        frame_length = frame_length + bytes_left_to_copy;
    }
    /* Prepare transmit descriptors to give to DMA */
    HAL_ETH_TransmitFrame(&eth0_hd, frame_length);
    err_val = ERR_OK;
error:
    /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
    if ((U32)RESET != (eth0_hd.Instance->DMASR & ETH_DMASR_TUS)) {
        /* Clear TUS ETHERNET DMA flag */
        eth0_hd.Instance->DMASR = ETH_DMASR_TUS;
        /* Resume DMA transmission*/
        eth0_hd.Instance->DMATPDR = 0;
    }
    s = (ERR_OK == err_val) ? S_OK : S_HARDWARE_FAULT;
    return s;
}

/******************************************************************************/
Status ETH_IoCtl(const U32 request_id, void* args_p)
{
Status s = S_UNDEF;
    switch (request_id) {
        case DRV_REQ_STD_POWER_SET: {
            HAL_StatusTypeDef hal_status = HAL_OK;
            switch (*(OS_PowerState*)args_p) {
                case PWR_ON:
                    s = S_OK;
                    break;
                case PWR_OFF:
                    s = S_OK;
                    break;
                default:
                    break;
            }
            if (HAL_OK != hal_status) { s = S_HARDWARE_FAULT; }
            }
            break;
        case DRV_REQ_ETH_LINK_STATUS_GET:
            {
            Bool* link_status_p = (Bool*)args_p;
                //TODO(A. Filyanov) Interface driver ID from the input args.
                IF_OK(s = DRV_ETH0_PHY.IoCtl(DRV_REQ_KS8721BL_LINK_STATUS_GET, link_status_p)) {
                }
            }
            break;
        case DRV_REQ_ETH_SETUP:
            {
            OS_NetworkItf* net_itf_p = (OS_NetworkItf*)args_p;
            if (netif_is_link_up(net_itf_p)) {
                /* Restart the auto-negotiation */
                IF_OK(s = DRV_ETH0_PHY.IoCtl(DRV_REQ_KS8721BL_AUTO_NEG_RESTART, OS_NULL)) {
                    /* ETHERNET MAC Re-Configuration */
                    HAL_ETH_ConfigMAC(&eth0_hd, (ETH_MACInitTypeDef*)NULL);
                    /* Restart MAC interface */
                    if (HAL_OK == HAL_ETH_Start(&eth0_hd)) {
                        const OS_Signal signal = OS_SignalCreateEx(DRV_ID_ETH0, OS_SIG_ETH_CONN_STATE_CHANGED, 0);
                        s = OS_SignalSend(netd_stdin_qhd, signal, OS_MSG_PRIO_NORMAL);
                    }
                }
            } else {
                /* Stop MAC interface */
                HAL_ETH_Stop(&eth0_hd);
            }
            }
            break;
        default:
            s = S_UNDEF_REQ_ID;
            break;
    }
    return s;
}

/******************************************************************************/
U32 sys_jiffies(void);
U32 sys_jiffies(void)
{
    return HAL_GetTick();
}

/******************************************************************************/
U32 sys_now(void);
U32 sys_now(void)
{
    return OS_TICKS_TO_MS(sys_jiffies());
}

/******************************************************************************/
/**
  * @brief  Ethernet Rx Transfer completed callback
  * @param  heth: ETH handle
  * @retval None
  */
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *eth0_hd_p)
{
const OS_Signal signal = OS_ISR_SignalCreate(DRV_ID_ETH0, OS_SIG_ETH_RX, 0);
    OS_ISR_ContextSwitchForce(OS_ISR_SignalSend(netd_stdin_qhd, signal, OS_MSG_PRIO_HIGH));
}

#endif //(ETH_ENABLED)