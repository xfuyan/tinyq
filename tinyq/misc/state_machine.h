/****************************************************************************
  state_machine.h
  Copyright (c) 2017 - 2020, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#define STATE_MACHINE_MSG_ENTER         (0xfffffffe)
#define STATE_MACHINE_MSG_LEAVE         (0xffffffff)

#define STATE_MACHINE_STATE_FIRST       (0)
#define STATE_MACHINE_STATE_INVALID     (-1)

struct  STATE_MACHINE;
typedef void (*STATE_MACHINE_ENTRY)(struct STATE_MACHINE *self, uint32 msg, const uint8 *p, uint8 size);

struct  STATE_MACHINE
{
	int8 	                     state;
	int8 	                     last_state;
  int8                       state_count;
  const STATE_MACHINE_ENTRY  *state_table;
};


extern void   state_machine_init(struct STATE_MACHINE *self, const STATE_MACHINE_ENTRY *state_table, int8 state_count);
extern void   state_machine_signal_entry(struct STATE_MACHINE *self, uint32 msg, const uint8 *p, uint8 size);
extern void   state_machine_goto_state(struct STATE_MACHINE *self, int8 new_state, uint32 param);
extern int8  state_machine_get_state(struct STATE_MACHINE *self);
extern int8  state_machine_get_last_state(struct STATE_MACHINE *self);

#endif
