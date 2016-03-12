/**************************************************************************//**
* @file    ring_buffer.h
* @brief   Ring buffer.
* @author  http://embedjournal.com/author/siddharth/
******************************************************************************/
#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

//------------------------------------------------------------------------------
#define RING_BUFFER_STATIC_CREATE(x, y)     ALIGN_BEGIN static U8 x##_v[y] ALIGN_END; RingBuffer x = { x##_v, 0, 0, y, (y - 1) }

//------------------------------------------------------------------------------
typedef struct {
    U8* const buffer_p;
    Size head;
    Size tail;
    const Size size_max;
    const Size size_mask;
} RingBuffer;

//------------------------------------------------------------------------------
Status  RingBufferU8Push(RingBuffer* ring_buf_p, U8 data);
Status  RingBufferU8Pop(RingBuffer* ring_buf_p, U8* data);

Status  RingBuffer2U8Push(RingBuffer* ring_buf_p, U8 data);
Status  RingBuffer2U8Pop(RingBuffer* ring_buf_p, U8* data);
void    RingBuffer2U8Read(RingBuffer* p, U8** data_pp, Size* size_p);

#endif // _RING_BUFFER_H_
