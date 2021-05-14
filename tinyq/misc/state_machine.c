/****************************************************************************
  state_machine.c
  Copyright (c) 2017 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include "tq_types.h"
#include "hw_debug.h"
#include "state_machine.h"


void state_machine_init(struct STATE_MACHINE *self, const STATE_MACHINE_ENTRY *state_table, int8 state_count)
{
  self->last_state = self->state = -1;
  self->state_table = state_table;
  self->state_count = state_count;
  state_machine_goto_state(self, 0, 0);
}

void state_machine_signal_entry(struct STATE_MACHINE *self, uint32 msg, const uint8 *p, uint8 size)
{
  TQ_ASSERT(self->state >= 0 && self->state <= self->state_count);
  self->state_table[self->state](self, msg, p, size);
}

void state_machine_goto_state(struct STATE_MACHINE *self, int8 new_state, uint32 param)
{
  if(self->state >= 0 && self->state < self->state_count)
    self->state_table[self->state](self, STATE_MACHINE_MSG_LEAVE, 0, 0);
  
  if(new_state >= 0 && new_state < self->state_count)
  {
    self->last_state = self->state;
    self->state = new_state;
    self->state_table[self->state](self, STATE_MACHINE_MSG_ENTER, (uint8*)(&param), 4);
  }
}

int8 state_machine_get_state(struct STATE_MACHINE *self)
{
  return self->state;
}

int8 state_machine_get_last_state(struct STATE_MACHINE *self)
{
  return self->last_state;
}
