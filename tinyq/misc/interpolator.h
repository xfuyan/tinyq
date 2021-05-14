/****************************************************************************
  interpolator.c
  Copyright (c) 2017 - 2018, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

struct S_INTERPOLATOR
{
  uint8 table_size;
  const uint8 *table;
  
  uint16  slot_index;
  uint32  f_slot_width;
  uint32  f_range_x;
  uint32  f_wave_factor;
  uint32  f_slot_dx;
  uint32  f_slot_y;
  uint32  f_slot_k;
};


extern void     interpolator_init(struct S_INTERPOLATOR *itp, uint16 range_x, uint16 range_y, const uint8 *function_table, uint8 table_size);
extern boolean  interpolator_step(struct S_INTERPOLATOR *itp, uint16 offset);
extern void     interpolator_set_pos(struct S_INTERPOLATOR *itp, uint16 pos);
extern boolean  interpolator_at_end(struct S_INTERPOLATOR *itp);
extern uint16   interpolator_get_pos(struct S_INTERPOLATOR *itp);
extern uint16   interpolator_get_value(struct S_INTERPOLATOR *itp);

#endif
