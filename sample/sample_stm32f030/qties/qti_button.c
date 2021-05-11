/****************************************************************************
  qti_button.c
  Copyright (c) 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include <string.h>
#include "tq_types.h"
#include "tinyq.h"
#include "qties.h"
#include "hw_debug.h"
#include "qti_system.h"
#include "qti_button.h"
#include "hw_exti.h"
#include "hw_gpio.h"
#include "stm32f0xx_rcc.h"

#define TIMER_DEBOUNCE                      (0x01)
#define DEBOUNCE_PERIOD                     (100)

#define GPIO_PIN_BUTTON                     (13)
#define GPIO_CLK_BUTTON                     RCC_AHBPeriph_GPIOC
#define EXTI_PORT_BUTTON                    EXTI_PortSourceGPIOC
#define EXTI_PIN_BUTTON                     EXTI_PinSource13

static void io_init(void);
static void button_state_exti_irq(void);
static void button_debounce_timeout(void);

static uint8 _self;
static uint8 _listener = 0;
static boolean _debouncing = FALSE;


void qti_button_set_listener(uint8 qti)
{
  _listener = qti;
}

void qti_button_signal_entry(const struct TQ_QTI *self, uint8 from, uint8 sig, const uint8 *p, uint8 size)
{
  uint16 timer_id;
  
  if(from == QTI_SYSTEM)
  {
    if(sig == SYSTEM_NTF_START)
    {
      _self = self->self;
      io_init();
    }
    else if(sig == SYSTEM_RSP_TIMER)
    {
      timer_id = *((uint16*)(p));
      if(timer_id == TIMER_DEBOUNCE)
        button_debounce_timeout();
    }
  }
}

static void io_init(void)
{
  /* Enable clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOF, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /*BUTTON*/
  GPIO_MODE(GPIOC, GPIO_PIN_BUTTON, GPIO_MODE_INPUT);
  GPIO_PUPD(GPIOC, GPIO_PIN_BUTTON, GPIO_PULL_DOWN);
  
  hw_exti_set(EXTI_PORT_BUTTON, EXTI_PIN_BUTTON, EXTI_Trigger_Rising_Falling, button_state_exti_irq);
}

static void button_state_exti_irq(void)
{
  if(!_debouncing)
  {
    _debouncing = TRUE;
    qti_system_start_timer(_self, TIMER_DEBOUNCE, DEBOUNCE_PERIOD);
  }
}

static void button_debounce_timeout(void)
{
  uint8 evt = (GPIO_GET_BITS(GPIOC, GPIO_BIT(GPIO_PIN_BUTTON)) == 0) ? BUTTON_NTF_DOWN : BUTTON_NTF_UP;

  _debouncing = FALSE;
  tinyq_send_signal(_self, _listener, evt, 0, 0);
}
