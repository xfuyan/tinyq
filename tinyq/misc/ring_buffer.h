/****************************************************************************
  ring_buffer.h
  Copyright (c) 2017 - 2018, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

/*ring buffer*/
struct RING_BUFFER
{
  int16 front;
  int16 back;
  int16 size;
  void* buffer;
#ifdef TQ_DEBUG
  int16 high_water_mark;
#endif
};

extern void  ring_buffer_init(struct RING_BUFFER *ring, void *buffer, int16 size);
extern void  ring_buffer_clear(struct RING_BUFFER *ring);
extern int16 ring_buffer_space(struct RING_BUFFER *ring);
extern int16 ring_buffer_size(struct RING_BUFFER *ring);
extern void  ring_buffer_push_byte(struct RING_BUFFER *ring, uint8 b);
extern uint8 ring_buffer_pop_byte(struct RING_BUFFER *ring);
extern void  ring_buffer_pop_front(struct RING_BUFFER *ring, void *buffer, int16 size);
extern void  ring_buffer_push_back(struct RING_BUFFER *ring, const void *data, int16 size);

extern void ring_buffer_read(struct RING_BUFFER *ring, void *buffer, int16 offset, int16 size);
extern void ring_buffer_write(struct RING_BUFFER *ring, const void *data, int16 offset, int16 size);
#endif
