/****************************************************************************
  tinyq.c
  Copyright (c) 2020 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include "tq_types.h"
#include "tinyq.h"
#include "hw_debug.h"
#include "ring_buffer.h"
#include "qti_system.h"
#include "tq_port.h"


/* signal ring buffer */
#define INTERFACE_BUFFER_SIZE               (256 * 2)
#define LOGIC_BUFFER_SIZE                   (256 * 2)

extern void   qti_system_start(void);
extern void   qti_system_sleep(void);


/* temporary signal parameter buffer */
static uint8 _logic_parameter_buffer[256];
static uint8 _interface_parameter_buffer[256];

static uint8 _interface_buffer[INTERFACE_BUFFER_SIZE];
static struct RING_BUFFER _interface_ring_buffer = {0, 0, sizeof(_interface_buffer), _interface_buffer};

static uint8 _logic_buffer[LOGIC_BUFFER_SIZE];
static struct RING_BUFFER _logic_ring_buffer = {0, 0, sizeof(_logic_buffer), _logic_buffer};


/* signal processing */
extern const uint8 tq_qti_count;
extern const struct TQ_QTI tq_qti_table[];

static void process_signal(uint8 from, uint8 to, uint8 sig, const uint8 *param, uint8 size)
{
  if(to == QTI_BROADCAST)
  {
    for(to = 0; to < tq_qti_count; to++)
      tq_qti_table[to].signal_entry(&tq_qti_table[to], from, sig, param, size);
  }
  else if(to < tq_qti_count)
    tq_qti_table[to].signal_entry(&tq_qti_table[to], from, sig, param, size);
}

/* main dispatch loop */
static void normal_priority_dispatch_loop(void)
{
  uint8 buffer[4];
  
  /*the main loop*/
  while(1)
  {
    qti_system_lock();
    if(ring_buffer_size(&_logic_ring_buffer))
    {
      ring_buffer_pop_front(&_logic_ring_buffer, buffer, 4);
      if(buffer[3] != 0)
        ring_buffer_pop_front(&_logic_ring_buffer, &_logic_parameter_buffer[0], buffer[3]);

      qti_system_unlock();
      
      TQ_DEBUG_PIN_SET(DEBUG_PIN_TINYQ_NORMAL_EVT, TRUE);
      process_signal(buffer[0], buffer[1], buffer[2], _logic_parameter_buffer, buffer[3]);
      TQ_DEBUG_PIN_SET(DEBUG_PIN_TINYQ_NORMAL_EVT, FALSE);
    }
    else
    {
      /* SLEEP */
      TQ_DEBUG_PIN_SET(DEBUG_PIN_TINYQ_SLEEP, TRUE);
      qti_system_sleep();
      TQ_DEBUG_PIN_SET(DEBUG_PIN_TINYQ_SLEEP, FALSE);
    }
  }
}

void _tq_high_priority_dispatch(void)
{
  boolean signal;
  uint8 buffer[4];
  
  do
  {
    qti_system_lock();
    if(ring_buffer_size(&_interface_ring_buffer))
    {
      ring_buffer_pop_front(&_interface_ring_buffer, buffer, 4);
      if(buffer[3] != 0)
        ring_buffer_pop_front(&_interface_ring_buffer, &_interface_parameter_buffer[0], buffer[3]);
      signal = TRUE;
    }
    else
    {
      signal = FALSE;
    }
    qti_system_unlock();

    if(signal)
    {
      TQ_DEBUG_PIN_SET(DEBUG_PIN_TINYQ_HIGH_EVT, TRUE);
      process_signal(buffer[0], buffer[1], buffer[2], _interface_parameter_buffer, buffer[3]);
      TQ_DEBUG_PIN_SET(DEBUG_PIN_TINYQ_HIGH_EVT, TRUE);
    }
  } while(signal);
}

/* public interface */
void tinyq_run(void)
{
  TQ_DEBUG_INIT();
  
  qti_system_start();
  normal_priority_dispatch_loop();
}

void tinyq_send_signal(uint8 from, uint8 to, uint8 sig, const void *param, uint8 size)
{
  uint8 buffer[4];
  
  if(!to)
    return;
  
  buffer[0] = from;
  buffer[1] = to;
  buffer[2] = sig;
  buffer[3] = size;
  
  qti_system_lock();
  if(TQ_SIG_DISPATCHER(sig) == TQ_DSP_HIGH)
  {
    ring_buffer_push_back(&_interface_ring_buffer, buffer, 4);
    if(size)
      ring_buffer_push_back(&_interface_ring_buffer, param, size);
    tq_port_trigger_high_priority_dispatch();
  }
  else
  {
    ring_buffer_push_back(&_logic_ring_buffer, buffer, 4);
    if(size)
      ring_buffer_push_back(&_logic_ring_buffer, param, size);
  }
  qti_system_unlock();
}
