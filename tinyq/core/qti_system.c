/****************************************************************************
  qti_system.c
  Copyright (c) 2020 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include "tq_types.h"
#include "tinyq.h"
#include "qti_system.h"
#include "hw_debug.h"
#include "tq_port.h"


struct S_TIMER
{
  uint32 id;
  uint32 period;
};

static uint8 start_timer(uint32 id, uint32 period, uint32 *timeup_table);
static void  cancel_timer(uint32 id);
static void  notify_timer_clients(uint32 *table, uint8 count);


static int32  _system_wait_counter = 0;
static int32  _system_lock_counter = 0;

static uint32 _timer_map;
static struct S_TIMER _timer_table[32];

#ifdef TQ_DEBUG
uint8 debug_system_wait_table[256];
#endif


void qti_system_signal_entry(const struct TQ_QTI *self, uint8 from, uint8 sig, const uint8 *p, uint8 size)
{
}

void qti_system_us_delay(uint32 us)
{
  tq_port_us_delay(us);
}

void qti_system_reset(void)
{
  tq_port_system_reset();
}

void qti_system_lock(void)
{
  tq_port_disable_irq();
  _system_lock_counter++;
}

void qti_system_unlock(void)
{
  tq_port_disable_irq();
  _system_lock_counter = _system_lock_counter > 0 ? _system_lock_counter - 1 : 0;
  if(!_system_lock_counter)
    tq_port_enable_irq();
}

void qti_system_request_wait(uint8 qti)
{
  qti_system_lock();
#ifdef TQ_DEBUG
  debug_system_wait_table[qti]++;
#endif
  _system_wait_counter++;
  qti_system_unlock();
}

void qti_system_release_wait(uint8 qti)
{
  qti_system_lock();
#ifdef TQ_DEBUG
  debug_system_wait_table[qti]--;
#endif
  _system_wait_counter--;
  qti_system_unlock();
}

void qti_system_sleep(void)
{
  TQ_ASSERT(_system_lock_counter == 1);
  
  /*balancing the system lock in dispatch_loop()*/
  _system_lock_counter--; 

  tq_port_sleep(_system_wait_counter ? FALSE : TRUE);
  tq_port_enable_irq();
}

void qti_system_start_timer(uint8 qti, uint16 id, uint32 period)
{
  uint32 timeup_table[32];
  uint8 timeups;
  
  uint32 timer_id = qti;
  timer_id = (timer_id << 16) | id;
  
  qti_system_lock();
  timeups = start_timer(timer_id, period, timeup_table);
  qti_system_unlock();
  
  notify_timer_clients(timeup_table, timeups);
}

void qti_system_stop_timer(uint8 qti, uint16 id)
{
  uint32 timer_id = qti;
  timer_id = (timer_id << 16) | id;
  
  qti_system_lock();
  cancel_timer(timer_id);
  qti_system_unlock();
}

void qti_system_start(void)
{
  tq_port_init();
  
  tinyq_send_signal(0, QTI_BROADCAST, SYSTEM_NTF_START, 0, 0);
}

static void notify_timer_clients(uint32 *table, uint8 count)
{
  uint8 i, qti;
  uint16 timer_id;
  
  for(i = 0; i < count; i++)
  {
    timer_id = table[i] & 0xffff;
    qti = (table[i] >> 16) & 0xff;
    
    tinyq_send_signal(0, qti, SYSTEM_RSP_TIMER, &timer_id, sizeof(timer_id));
  }
}

static uint8 start_timer(uint32 id, uint32 period, uint32 *timeup_table)
{
  uint8 i, timeups = 0;
  uint32 ticks_elapsed;
  uint32 next_alarm = 0x7fffffff;
  uint32 mask = 1;
  uint32 timer_map = _timer_map;
 
  TQ_ASSERT(_timer_map != 0xffffffff);   // you are starting too many timers.
  TQ_ASSERT(period  < (0x7fffffff / _PT_SLEEP_TIMER_TICK_PER_MS));

  period *= _PT_SLEEP_TIMER_TICK_PER_MS;
  
  ticks_elapsed = tq_port_sleep_timer_get_time_elapsed(TRUE);
  tq_port_sleep_timer_stop();

  for(i = 0; i < 32; i++)
  {
    if(timer_map & 1)
    {
      if(_timer_table[i].id == id)
      {
        _timer_map ^= mask;
        timer_map ^= 1;
      }
      else
      {
        if(_timer_table[i].period <= ticks_elapsed)
        {
          _timer_map ^= mask;
          timer_map ^= 1;
          timeup_table[timeups++] = _timer_table[i].id; 
        }
        else
        {
          _timer_table[i].period -= ticks_elapsed;
          next_alarm = (next_alarm < _timer_table[i].period) ? next_alarm : _timer_table[i].period;
        }
      }
    }

    if(period && !(timer_map & 1))
    {
      _timer_table[i].id = id;
      _timer_table[i].period = period;
      _timer_map |= mask;
      next_alarm = (next_alarm < period) ?  next_alarm : period;
      period = 0;
    }
    
    timer_map >>= 1;
    mask <<= 1;
    
    if(!timer_map && !period)
      break;
  }
  
  TQ_ASSERT(_timer_map);
  tq_port_sleep_timer_start(next_alarm);
  return timeups;
}

static void cancel_timer(uint32 id)
{
  uint8 i;
  uint32 mask = 1;
  uint32 timer_map = _timer_map;
  
  for(i = 0; i < 32; i++)
  {
    if(!timer_map)
      break;
    
    if((timer_map & 1) && _timer_table[i].id == id)
      _timer_map ^= mask;

    timer_map >>= 1;
    mask <<= 1;
  }
}

static uint32 timer_timeup(uint32 *timeup_table)
{
  uint8 i, timeups = 0;
  uint32 ticks_elapsed;
  uint32 mask = 1;
  uint32 timer_map = _timer_map;
  uint32 next_alarm = 0x7fffffff;
  
  ticks_elapsed = tq_port_sleep_timer_get_time_elapsed(FALSE);
  
  for(i = 0; i < 32; i++)
  {
    if(!timer_map)
      break;
    
    if((timer_map & 1))
    {
      if(_timer_table[i].period <= ticks_elapsed)
      {
        _timer_map ^= mask;
        timeup_table[timeups++] = _timer_table[i].id; 
      }
      else
      {
        _timer_table[i].period -= ticks_elapsed;
        next_alarm = next_alarm > _timer_table[i].period ? _timer_table[i].period : next_alarm;
      }
    }
    timer_map >>= 1;
    mask <<= 1;
  }
  
  tq_port_sleep_timer_start(next_alarm);
  return timeups;
}

void _system_sleep_timer_handler(void)
{
  uint32 timeup_table[32];
  uint8 timeups;
  
  qti_system_lock();
  timeups = timer_timeup(timeup_table);
  qti_system_unlock();
  
  notify_timer_clients(timeup_table, timeups);
}
