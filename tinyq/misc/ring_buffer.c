/****************************************************************************
  ring_buffer.c
  Copyright (c) 2017 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include <string.h>
#include "tq_types.h"
#include "hw_debug.h"
#include "ring_buffer.h"


void ring_buffer_init(struct RING_BUFFER *ring, void *buffer, int16 size)
{
  ring->buffer = buffer;
  ring->size = size;
  ring->back = ring->front = 0;
#ifdef TQ_DEBUG
  ring->high_water_mark = 0;
#endif
}

void ring_buffer_clear(struct RING_BUFFER *ring)
{
  ring->front = ring->back = 0;
}

int16 ring_buffer_size(struct RING_BUFFER *ring)
{
  int16 size;
  int16 front = ring->front, back = ring->back;
  
  size = (back < front) ? ring->size : 0;
  size += back - front;
  
  return size;
}

int16 ring_buffer_space(struct RING_BUFFER *ring)
{
  return ring->size - ring_buffer_size(ring) - 1;
}

void ring_buffer_push_back(struct RING_BUFFER *ring, const void *data, int16 size)
{
  int16 copy_size, back;
  
  if(!size)
    return;
  
  TQ_ASSERT(ring_buffer_space(ring) >= size);
  back = ring->back;
  copy_size = (back + size < ring->size) ? (size) : (ring->size - back);
  memcpy((uint8*)(ring->buffer) + back, data, copy_size);
  if(size > copy_size)
    memcpy(ring->buffer, ((const uint8*)data) + copy_size, size - copy_size);
  
  back += size;
  if(back >= ring->size)
    back -= ring->size;
  ring->back = back;
  
#ifdef TQ_DEBUG
  if(ring_buffer_size(ring) > ring->high_water_mark)
    ring->high_water_mark = ring_buffer_size(ring);
#endif
}

void ring_buffer_pop_front(struct RING_BUFFER *ring, void *buffer, int16 size)
{
  int16 copy_size, front;
  
  if(!size)
    return;
  
  TQ_ASSERT(ring_buffer_size(ring) >= size);

  front = ring->front;
  if(buffer)
  {
    copy_size = (front + size < ring->size) ? (size) : (ring->size - front);
    if(buffer)
      memcpy(buffer, (uint8*)(ring->buffer) + front, copy_size);
    if(buffer && size > copy_size)
      memcpy(((uint8*)buffer) + copy_size, ring->buffer, size - copy_size);
  }
  
  front += size;
  if(front >= ring->size)
    front -= ring->size;
  
  ring->front = front;
}

void ring_buffer_push_byte(struct RING_BUFFER *ring, uint8 b)
{
  TQ_ASSERT(ring_buffer_space(ring));
  
  *((uint8*)(ring->buffer) + ring->back) = b;
  ring->back = ring->back + 1 >= ring->size ? 0 : ring->back + 1;
}

uint8 ring_buffer_pop_byte(struct RING_BUFFER *ring)
{
  uint8 b;
  TQ_ASSERT(ring_buffer_size(ring));
  
  b = *((uint8*)(ring->buffer) + ring->front);
  ring->front = ring->front + 1 >= ring->size ? 0 : ring->front + 1;
  return b;
}

//test function
void ring_buffer_read(struct RING_BUFFER *ring, void *buffer, int16 offset, int16 size)
{
  int16 copy_size, front;
  
  if(!size)
    return;
  
  front = ring->front + offset;
  if(front >= ring->size)
    front -= ring->size;
  
  if(buffer)
  {
    copy_size = (front + size < ring->size) ? (size) : (ring->size - front);
    if(buffer)
      memcpy(buffer, (uint8*)(ring->buffer) + front, copy_size);
    if(buffer && size > copy_size)
      memcpy(((uint8*)buffer) + copy_size, ring->buffer, size - copy_size);
  }
}

void ring_buffer_write(struct RING_BUFFER *ring, const void *data, int16 offset, int16 size)
{
  int16 copy_size, front;
  
  if(!size)
    return;
  
  front = ring->front + offset;
  if(front >= ring->size)
    front -= ring->size;
  
  copy_size = (front + size < ring->size) ? (size) : (ring->size - front);
  memcpy((uint8*)(ring->buffer) + front, data, copy_size);
  if(size > copy_size)
    memcpy(ring->buffer, ((uint8*)data) + copy_size, size - copy_size);
}
