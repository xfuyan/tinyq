/****************************************************************************
  qti_sample.c
  Copyright (c) 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include "tq_types.h"
#include "tinyq.h"
#include "hw_debug.h"
#include "state_machine.h"
#include "qties.h"
#include "qti_system.h"
#include "qti_button.h"
#include "qti_indication.h"
#include "qti_sample.h"

#define TIMER_POWER_UP_DELAY              (1)
#define POWER_UP_DELAY                    (500)

#define SM_MAKE_MESSAGE(FROM, SIG)        ((((uint16)(FROM)) << 8) | ((uint16)(SIG)))
#define MSG_TIMER                         SM_MAKE_MESSAGE(QTI_SYSTEM, SYSTEM_RSP_TIMER)
#define MSG_BUTTON_UP                     SM_MAKE_MESSAGE(QTI_BUTTON, BUTTON_NTF_UP)
#define MSG_BUTTON_DOWN                   SM_MAKE_MESSAGE(QTI_BUTTON, BUTTON_NTF_DOWN)
#define MSG_BUZZER_STOPPED                SM_MAKE_MESSAGE(QTI_INDICATION, INDICATION_NTF_BUZZER_STOPPED)

static void state_entry_init(struct STATE_MACHINE *sm, uint32 msg, const uint8 *p, uint8 size);
static void state_entry_stopped(struct STATE_MACHINE *sm, uint32 msg, const uint8 *p, uint8 size);
static void state_entry_playing(struct STATE_MACHINE *sm, uint32 msg, const uint8 *p, uint8 size);

enum STATES
{
  STATE_INIT = 0,
  STATE_STOPPED,
  STATE_PLAYING,
  TOTAL_STATE_COUNT
};

static const STATE_MACHINE_ENTRY _state_table[] =
{
  state_entry_init,
  state_entry_stopped,
  state_entry_playing,
};

static uint8 _self;
static struct STATE_MACHINE _sm;


/* qti signal entry */
void qti_sample_signal_entry(const struct TQ_QTI *self, uint8 from, uint8 sig, const uint8 *p, uint8 size)
{
  if(from == QTI_SYSTEM)
  {
    if(sig == SYSTEM_NTF_START)
    {
      _self = self->self;
      qti_button_set_listener(_self);
      state_machine_init(&_sm, _state_table, TOTAL_STATE_COUNT);
      return;
    }
  }

  state_machine_signal_entry(&_sm, SM_MAKE_MESSAGE(from, sig), p, size);
}

static void state_entry_init(struct STATE_MACHINE *sm, uint32 msg, const uint8 *p, uint8 size)
{
  switch(msg)
  {
    case STATE_MACHINE_MSG_ENTER:
      qti_indication_play_led(_self, INDICATION_LED_MELODY_POWER_UP);
      qti_indication_play_buzzer(_self, INDICATION_BUZZER_MELODY_POWER_UP);
      qti_system_start_timer(_self, TIMER_POWER_UP_DELAY, POWER_UP_DELAY);
      break;
    
    case MSG_TIMER:
      if(*((uint16*)p) == TIMER_POWER_UP_DELAY)
        state_machine_goto_state(sm, STATE_STOPPED, 0);
      break;
  }
}

static void state_entry_stopped(struct STATE_MACHINE *sm, uint32 msg, const uint8 *p, uint8 size)
{
  static uint8 melody = 1;
  
  switch(msg)
  {
    case STATE_MACHINE_MSG_ENTER:
    case STATE_MACHINE_MSG_LEAVE:
      break;

    case MSG_BUTTON_UP:
      qti_indication_play_led(_self, melody);
      qti_indication_play_buzzer(_self, melody);
      state_machine_goto_state(sm, STATE_PLAYING, 0);
    
      melody++;
      if(melody > 5)
        melody = 1;
      break;
  }
}

static void state_entry_playing(struct STATE_MACHINE *sm, uint32 msg, const uint8 *p, uint8 size)
{
  switch(msg)
  {
    case STATE_MACHINE_MSG_ENTER:
      TQ_DEBUG_PIN_SET(DEBUG_PIN_SAMPLE, TRUE);
      break;
    
    case STATE_MACHINE_MSG_LEAVE:
      TQ_DEBUG_PIN_SET(DEBUG_PIN_SAMPLE, FALSE);
      break;

    case MSG_BUTTON_UP:
      qti_indication_stop_led(_self);
      qti_indication_stop_buzzer(_self);
      break;
    
    case MSG_BUZZER_STOPPED:
      state_machine_goto_state(sm, STATE_STOPPED, 0);
      break;
  }
}
