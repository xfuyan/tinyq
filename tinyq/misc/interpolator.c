/****************************************************************************
  interpolator.c
  Copyright (c) 2017 - 2018, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include <string.h>
#include "tq_types.h"
#include "hw_debug.h"
#include "interpolator.h"

#define WAVE_TABLE_BITS         (8)
#define WAVE_VALUE_MAX          (1UL << (WAVE_TABLE_BITS))


void interpolator_init(struct S_INTERPOLATOR *itp, uint16 range_x, uint16 range_y, const uint8 *function_table, uint8 table_size)
{
  memset(itp, 0, sizeof(*itp));
  itp->table = function_table;
  itp->table_size = table_size;
  itp->f_range_x = (((uint32)range_x) << 16);
  itp->f_slot_width = itp->f_range_x / table_size;
  
  TQ_ASSERT(itp->f_slot_width > 0);
  
  itp->f_wave_factor = (((uint32)range_y) << WAVE_TABLE_BITS);
  interpolator_set_pos(itp, 0);
}

void interpolator_set_pos(struct S_INTERPOLATOR *itp, uint16 pos)
{
  uint32 wav_y1, wav_y2;
  uint32 f_x = ((uint32)pos << 16);
  
  TQ_ASSERT(f_x <= itp->f_range_x);
  
  itp->slot_index = f_x / itp->f_slot_width;
  itp->f_slot_dx = f_x - itp->slot_index * itp->f_slot_width;
  
  if(itp->slot_index < itp->table_size)
  {
    wav_y1 = itp->table[itp->slot_index];
    wav_y2 = itp->slot_index == itp->table_size - 1 ? wav_y1 : itp->table[itp->slot_index + 1];
    itp->f_slot_y =  wav_y1 * itp->f_wave_factor;
    itp->f_slot_k = (wav_y2 * itp->f_wave_factor - itp->f_slot_y) / (itp->f_slot_width >> 16);
  }
}

boolean interpolator_step(struct S_INTERPOLATOR *itp, uint16 offset)
{
  uint32 wav_y1, wav_y2;
  
  if(itp->slot_index < itp->table_size)
  {  
    itp->f_slot_dx += ((uint32)offset << 16);
    while(itp->f_slot_dx >= itp->f_slot_width)
    {
      itp->f_slot_dx -= itp->f_slot_width;
      itp->slot_index++;
      if(itp->f_slot_dx < itp->f_slot_width && itp->slot_index < itp->table_size)
      {
        wav_y1 = itp->table[itp->slot_index];
        wav_y2 = itp->slot_index == itp->table_size - 1 ? wav_y1 : itp->table[itp->slot_index + 1];
        itp->f_slot_y =  wav_y1 * itp->f_wave_factor;
        itp->f_slot_k = (wav_y2 * itp->f_wave_factor - itp->f_slot_y) / (itp->f_slot_width >> 16);
      }
    }
  }
  return itp->slot_index < itp->table_size;
}

boolean interpolator_at_end(struct S_INTERPOLATOR *itp)
{
  return itp->slot_index >= itp->table_size;
}

uint16 interpolator_get_pos(struct S_INTERPOLATOR *itp)
{
  return (itp->slot_index * itp->f_slot_width + itp->f_slot_dx) >> 16;
}

uint16 interpolator_get_value(struct S_INTERPOLATOR *itp)
{
  uint32 y;
  uint32 kh, kl, dx, f_dy;
  
  kh = itp->f_slot_k >> 16;
  kl = itp->f_slot_k & 0xffff;
  
  dx = itp->f_slot_dx >> 16;
  f_dy = (dx * kh << 16) + dx *kl;
  y = (itp->f_slot_y + f_dy) >> 16;
  
  return y;
}
