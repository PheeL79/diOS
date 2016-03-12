/**************************************************************************//**
* @file    ring_buffer.c
* @brief   Ring buffer.
* @author  http://embedjournal.com/author/siddharth/
******************************************************************************/
#include "common.h"
#include "ring_buffer.h"

/*****************************************************************************/
Status RingBufferU8Push(RingBuffer* p, U8 data)
{
    HAL_ASSERT_DEBUG(OS_NULL != p);
    Size next = p->head + 1;
    if (next >= p->size_mask) {
        next = 0;
    }
    // Cicular buffer is full
    if (next == p->tail) {
        return S_OVERFLOW;  // quit with an error
    }
    p->buffer_p[p->head] = data;
    p->head = next;
    return S_OK;
}

/*****************************************************************************/
Status RingBufferU8Pop(RingBuffer* p, U8* data_p)
{
    HAL_ASSERT_DEBUG(OS_NULL != p);
    HAL_ASSERT_DEBUG(OS_NULL != data_p);
    // if the head isn't ahead of the tail, we don't have any characters
    if (p->head == p->tail) {
        return S_IS_EMPTY;  // quit with an error
    }
    *data_p = p->buffer_p[p->tail];

    Size next = p->tail + 1;
    if (next >= p->size_mask) {
        next = 0;
    }
    p->tail = next;
    return S_OK;
}

/*****************************************************************************/
Status RingBuffer2U8Push(RingBuffer* p, U8 data)
{
    HAL_ASSERT_DEBUG(OS_NULL != p);
    const Size next = ((p->head + 1) & p->size_mask);
    if (next == p->tail) {
        return S_OVERFLOW;  // quit with an error
    }
    p->buffer_p[p->head] = data;
    p->head = next;
    return S_OK;
}

/*****************************************************************************/
Status RingBuffer2U8Pop(RingBuffer* p, U8* data_p)
{
    HAL_ASSERT_DEBUG(OS_NULL != p);
    HAL_ASSERT_DEBUG(OS_NULL != data_p);
    if (p->head == p->tail) {
        return S_IS_EMPTY;  // quit with an error
    }
    *data_p = p->buffer_p[p->tail];
    p->tail = ((p->tail + 1) & p->size_mask);
    return S_OK;
}

/*****************************************************************************/
void RingBuffer2U8Read(RingBuffer* p, U8** data_pp, Size* size_p)
{
    HAL_ASSERT_DEBUG(OS_NULL != p);
    HAL_ASSERT_DEBUG(IS_POWER_OF_2(p->size_max));
    HAL_ASSERT_DEBUG(OS_NULL != data_pp);
    HAL_ASSERT_DEBUG(OS_NULL != size_p);
    const Size size = (p->tail > p->head) ? (p->size_max - p->tail) : (p->head - p->tail);
    *data_pp = &p->buffer_p[p->tail];
    *size_p  = size;
    p->tail = ((p->tail + size) & p->size_mask);
}

/*****************************************************************************/
//Status RingBuffer2Write(RingBuffer* p, U8* data_p, Size size)
//{
//    HAL_ASSERT_DEBUG(OS_NULL != p);
//    HAL_ASSERT_DEBUG(IS_POWER_OF_2(p->size_max);
//    HAL_ASSERT_DEBUG(OS_NULL != data_p);
//    if ((p->head - p->tail) < size) {
//        return S_OVERFLOW;
//    }
//    return S_OK;
//}
