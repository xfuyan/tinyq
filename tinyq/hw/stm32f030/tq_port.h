/****************************************************************************
  tq_port.h
  Copyright (c) 2020 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef TQ_PORT_H
#define TQ_PORT_H

#define _PT_SLEEP_TIMER_FREQ                            (40 * 1000)
#define _PT_SLEEP_TIMER_TICK_PER_SECOND                 (4 * 1000)
#define _PT_SLEEP_TIMER_TICK_PER_MS                     (_PT_SLEEP_TIMER_TICK_PER_SECOND / 1000)
#define _PT_SLEEP_TIMER_PERIOD_MAX                      (_PT_SLEEP_TIMER_TICK_PER_SECOND)

#define tq_port_disable_irq                             __disable_irq
#define tq_port_enable_irq                              __enable_irq

extern void tq_port_init(void);
extern void tq_port_system_reset(void);
extern void tq_port_us_delay(uint32 us);

extern void tq_port_trigger_high_priority_dispatch(void);
extern void tq_port_sleep(boolean low_power);

extern void tq_port_sleep_timer_stop(void);
extern void tq_port_sleep_timer_start(int32 ticks);
extern uint32 tq_port_sleep_timer_get_time_elapsed(boolean update);

extern void _system_sleep_timer_handler(void);
extern void _tq_high_priority_dispatch(void);

#endif
