/****************************************************************************
  hw_debug.h
  Copyright (c) 2017 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef HW_DEBUG_H
#define HW_DEBUG_H

#define TQ_DEBUG_PORT_NONE  (0)
#define TQ_DEBUG_PORTA      (1)
#define TQ_DEBUG_PORTB      (2)
#define TQ_DEBUG_PORTC      (3)
#define TQ_DEBUG_PORTD      (4)
#define TQ_DEBUG_PORTE      (5)
#define TQ_DEBUG_PORTF      (6)

#define TQ_DEBUG_MAKE_PIN(PORT, PIN)        ((((PORT) << 4) & 0xf0) | ((PIN) & 0x0f))
#define TQ_DEBUG_PIN_NONE                   TQ_DEBUG_MAKE_PIN(TQ_DEBUG_PORT_NONE, 0)
#define TQ_DEBUG_PIN_1                      TQ_DEBUG_MAKE_PIN(TQ_DEBUG_PORTC, 10)
#define TQ_DEBUG_PIN_2                      TQ_DEBUG_MAKE_PIN(TQ_DEBUG_PORTC, 11)
#define TQ_DEBUG_PIN_3                      TQ_DEBUG_MAKE_PIN(TQ_DEBUG_PORTC, 12)

#ifdef TQ_DEBUG
  extern void TQ_DEBUG_INIT(void);
  extern void TQ_ASSERT(boolean cond);
  extern void TQ_DEBUG_PIN_SET(uint8 pin, boolean assert);
#else
  #define TQ_DEBUG_INIT()
  #define TQ_ASSERT(C)
  #define TQ_DEBUG_PIN_SET(PIN, ASSERT)
  #define TQ_DEBUG_PIN_NUM(PIN, NUM)
#endif

  
#define DEBUG_PIN_TINYQ_NORMAL_EVT                  TQ_DEBUG_PIN_NONE
#define DEBUG_PIN_TINYQ_HIGH_EVT                    TQ_DEBUG_PIN_NONE
#define DEBUG_PIN_TINYQ_SLEEP                       TQ_DEBUG_PIN_NONE

#define DEBUG_PIN_SAMPLE                            TQ_DEBUG_PIN_1


#endif
