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

#if (HAL_ETH_ENABLED)
//-----------------------------------------------------------------------------
#define MDL_NAME    "drv_eth"

#define ETH0_OS_MEM_TYPE                OS_MEM_RAM_INT_SRAM

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
static ETH_DMADescTypeDef*  dma_rx_desc_tab_p;          /* Ethernet Rx MA Descriptor */
static ETH_DMADescTypeDef*  dma_tx_desc_tab_p;          /* Ethernet Tx DMA Descriptor */
static U8*                  rx_buff_p;                  /* Ethernet Receive Buffer */
static U8*                  tx_buff_p;                  /* Ethernet Transmit Buffer */
ETH_HandleTypeDef           eth0_hd;
OS_QueueHd                  netd_stdin_qhd;
HAL_DriverItf*              drv_eth_v[DRV_ID_ETH_LAST];

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
Status s = S_OK;
    OS_MemSet(drv_eth_v, 0x0, sizeof(drv_eth_v));
    drv_eth_v[DRV_ID_ETH0] = &drv_eth0;
    return s;
}

/******************************************************************************/
Status ETH__Init(void* args_p)
{
const OS_NetworkItfInitArgs* init_args_p = (OS_NetworkItfInitArgs*)args_p;
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "Init: ");
    dma_rx_desc_tab_p   = OS_MallocEx(sizeof(ETH_DMADescTypeDef) * ETH_RXBUFNB, ETH0_OS_MEM_TYPE);
    dma_tx_desc_tab_p   = OS_MallocEx(sizeof(ETH_DMADescTypeDef) * ETH_TXBUFNB, ETH0_OS_MEM_TYPE);
    rx_buff_p           = OS_MallocEx(sizeof(U8) * ETH_RXBUFNB * ETH_RX_BUF_SIZE, ETH0_OS_MEM_TYPE);
    tx_buff_p           = OS_MallocEx(sizeof(U8) * ETH_TXBUFNB * ETH_TX_BUF_SIZE, ETH0_OS_MEM_TYPE);
    if ((OS_NULL == dma_rx_desc_tab_p) || (OS_NULL == dma_tx_desc_tab_p) ||
        (OS_NULL == rx_buff_p) || (OS_NULL == tx_buff_p)) {
        return s = S_OUT_OF_MEMORY;
    }
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
                    extern void ETH_MACDMAConfig(ETH_HandleTypeDef *heth, uint32_t err);
                    /* Config MAC and DMA */
                    ETH_MACDMAConfig(&eth0_hd, ETH_SUCCESS);
                    /* Initialize Tx Descriptors list: Chain Mode */
                    OS_ASSERT(HAL_OK == HAL_ETH_DMATxDescListInit(&eth0_hd, dma_tx_desc_tab_p, tx_buff_p, ETH_TXBUFNB));
                    /* Initialize Rx Descriptors list: Chain Mode  */
                    OS_ASSERT(HAL_OK == HAL_ETH_DMARxDescListInit(&eth0_hd, dma_rx_desc_tab_p, rx_buff_p, ETH_RXBUFNB));
                }
            }
        } else { s = S_HARDWARE_ERROR; }
    }
    HAL_TRACE_S(D_INFO, s);
    return s;
}

/******************************************************************************/
Status ETH__DeInit(void* args_p)
{
Status s = S_UNDEF;
    HAL_LOG(D_INFO, "DeInit: ");
    IF_OK(s = ETH_LL_DeInit(OS_NULL)) {
        OS_FreeEx(tx_buff_p, ETH0_OS_MEM_TYPE);
        OS_FreeEx(rx_buff_p, ETH0_OS_MEM_TYPE);
        OS_FreeEx(dma_tx_desc_tab_p, ETH0_OS_MEM_TYPE);
        OS_FreeEx(dma_rx_desc_tab_p, ETH0_OS_MEM_TYPE);
    }
    HAL_TRACE_S(D_INFO, s);
    return s;
}

/******************************************************************************/
Status ETH_LL_Init(void* args_p)
{
GPIO_InitTypeDef GPIO_InitStruct;
Status s = S_OK;
    /* Peripheral clock enable */
    HAL_ETH_CLK_ENABLE();
    HAL_ETH_MAC_TX_CLK_ENABLE();
    HAL_ETH_MAC_RX_CLK_ENABLE();
    /* Enable GPIOs clocks */
    HAL_ETH_GPIO_CLK_ENABLE();
    /* ETH GPIO Configuration */
    GPIO_InitStruct.Pin = HAL_ETH_GPIO1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = HAL_ETH_GPIO_AF;
    HAL_GPIO_Init(HAL_ETH_GPIO1, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = HAL_ETH_GPIO2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = HAL_ETH_GPIO_AF;
    HAL_GPIO_Init(HAL_ETH_GPIO2, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = HAL_ETH_GPIO3_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HAL_ETH_GPIO3, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = HAL_ETH_GPIO4_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = HAL_ETH_GPIO_AF;
    HAL_GPIO_Init(HAL_ETH_GPIO4, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = HAL_ETH_GPIO5_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = HAL_ETH_GPIO_AF;
    HAL_GPIO_Init(HAL_ETH_GPIO5, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(HAL_ETH_IRQ, HAL_IRQ_PRIO_ETH, 0);
    HAL_NVIC_SetPriority(HAL_ETH_MDINT_IRQ, HAL_IRQ_PRIO_ETH0_MDINT, 0);
    HAL_NVIC_EnableIRQ(HAL_ETH_IRQ);
    HAL_NVIC_EnableIRQ(HAL_ETH_MDINT_IRQ);
    return s;
}

/******************************************************************************/
Status ETH_LL_DeInit(void* args_p)
{
Status s = S_UNDEF;
    IF_OK(s = DRV_ETH0_PHY.DeInit(OS_NULL)) {
        if (HAL_OK == HAL_ETH_Stop(&eth0_hd)) {
            s = S_OK;
        } else {
            s = S_HARDWARE_ERROR;
        }
        /* Peripheral clock disable */
        HAL_ETH_MAC_RX_CLK_DISABLE();
        HAL_ETH_MAC_TX_CLK_DISABLE();
        HAL_ETH_CLK_DISABLE();
        /* ETH GPIO Configuration */
        HAL_GPIO_DeInit(HAL_ETH_GPIO1, HAL_ETH_GPIO1_PIN);
        HAL_GPIO_DeInit(HAL_ETH_GPIO2, HAL_ETH_GPIO2_PIN);
        HAL_GPIO_DeInit(HAL_ETH_GPIO3, HAL_ETH_GPIO3_PIN);
        HAL_GPIO_DeInit(HAL_ETH_GPIO4, HAL_ETH_GPIO4_PIN);
        /* Peripheral interrupt Deinit */
        HAL_NVIC_DisableIRQ(HAL_ETH_IRQ);
        HAL_NVIC_DisableIRQ(HAL_ETH_MDINT_IRQ);
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
        if (HAL_OK != HAL_ETH_Start(&eth0_hd)) {
            s = S_HARDWARE_ERROR;
        }
    }
    return s;
}

/******************************************************************************/
Status ETH_Close(void* args_p)
{
Status s = S_UNDEF;
    IF_OK(s = DRV_ETH0_PHY.Close(OS_NULL)) {
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
HAL_IO ETH_DMADescTypeDef *rx_dma_desc;
U32 buffer_offset = 0;
U32 payload_offset = 0;
U32 bytes_left_to_copy = 0;
U32 i = 0;
U16 len = 0;
Status s = S_OK;
    /* get received frame */
    if (HAL_OK != HAL_ETH_GetReceivedFrame_IT(&eth0_hd)) {
        return (s = S_HARDWARE_ERROR);
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
HAL_IO ETH_DMADescTypeDef *tx_dma_desc;
U32 frame_length = 0;
U32 buffer_offset = 0;
U32 bytes_left_to_copy = 0;
U32 payload_offset = 0;
Status s = S_UNDEF;

    tx_dma_desc = eth0_hd.TxDesc;
    buffer_offset = 0;
    /* copy frame from pbufs to driver buffers */
    for (q = data_out_p; q != NULL; q = q->next) {
        /* Is this buffer available? If not, goto error */
        if ((U32)RESET != (tx_dma_desc->Status & ETH_DMATXDESC_OWN)) {
            s = S_BUSY;
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
                s = S_BUSY;
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
    if (HAL_OK == HAL_ETH_TransmitFrame(&eth0_hd, frame_length)) {
        s = S_OK;
    } else {
        s = S_HARDWARE_ERROR;
    }
error:
    /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
    if ((U32)RESET != (eth0_hd.Instance->DMASR & ETH_DMASR_TUS)) {
        /* Clear TUS ETHERNET DMA flag */
        eth0_hd.Instance->DMASR = ETH_DMASR_TUS;
        /* Resume DMA transmission*/
        eth0_hd.Instance->DMATPDR = 0;
    }
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
            if (HAL_OK != hal_status) { s = S_HARDWARE_ERROR; }
            }
            break;
        case DRV_REQ_ETH_LINK_INT_CLEAR:
            s = DRV_ETH0_PHY.IoCtl(DRV_REQ_KS8721BL_LINK_INT_CLEAR, OS_NULL);
            break;
        case DRV_REQ_ETH_LINK_STATE_GET:
            {
            Bool* link_state_p = (Bool*)args_p;
                //TODO(A. Filyanov) Interface driver ID from the input args.
                IF_OK(s = DRV_ETH0_PHY.IoCtl(DRV_REQ_KS8721BL_LINK_STATUS_GET, link_state_p)) {
                }
            }
            break;
        case DRV_REQ_ETH_SETUP:
            {
            const Bool link_state = *(Bool*)args_p;
                if (link_state) {
                    /* Restart the auto-negotiation */
                    IF_OK(s = DRV_ETH0_PHY.IoCtl(DRV_REQ_KS8721BL_AUTO_NEG_RESTART, OS_NULL)) {
                        /* ETHERNET MAC Re-Configuration */
                        HAL_ETH_ConfigMAC(&eth0_hd, (ETH_MACInitTypeDef*)NULL);
                        /* Restart MAC interface */
                        if (HAL_OK == HAL_ETH_Start(&eth0_hd)) {
                        } else {
                            s = S_HARDWARE_ERROR;
                        }
                    } else {
                        /* Stop MAC interface */
                        if (HAL_OK == HAL_ETH_Stop(&eth0_hd)) {
                        } else {
                            s = S_HARDWARE_ERROR;
                        }
                    }
                }
            }
            break;
        case DRV_REQ_ETH_PHY_REG_GET:
            {
            ETH_DrvArgsPhyRegGetSet* reg_val_p = (ETH_DrvArgsPhyRegGetSet*)args_p;
            if (HAL_OK == HAL_ETH_ReadPHYRegister(&eth0_hd, reg_val_p->reg, &(reg_val_p->val))) {
                s = S_OK;
            } else {
                s = S_TIMEOUT;
            }
            }
            break;
        case DRV_REQ_ETH_PHY_REG_SET:
            {
            ETH_DrvArgsPhyRegGetSet* reg_val_p = (ETH_DrvArgsPhyRegGetSet*)args_p;
            if (HAL_OK == HAL_ETH_WritePHYRegister(&eth0_hd, reg_val_p->reg, reg_val_p->val)) {
                s = S_OK;
            } else {
                s = S_TIMEOUT;
            }
            }
            break;
        default:
            s = S_INVALID_REQ_ID;
            break;
    }
    return s;
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

// IRQ handlers-----------------------------------------------------------------
/******************************************************************************/
/**
* @brief This function handles Ethernet global interrupt.
*/
void HAL_ETH_IRQ_HANDLER(void);
void HAL_ETH_IRQ_HANDLER(void)
{
    HAL_NVIC_ClearPendingIRQ(ETH_IRQn);
    HAL_ETH_IRQHandler(&eth0_hd);
}

#endif //(HAL_ETH_ENABLED)