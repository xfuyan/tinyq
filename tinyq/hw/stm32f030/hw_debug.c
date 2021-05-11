/****************************************************************************
  hw_debug.c
  Copyright (c) 2017 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include <string.h>
#include "tq_types.h"
#include "hw_debug.h"
#include "hw_gpio.h"
#include "ring_buffer.h"
#include "tinyq.h"
#include "qti_system.h"
#include "stm32f0xx_dbgmcu.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"

#ifdef TQ_DEBUG
static void init_debug_pins(void);
static void init_debug_pin(uint8 pin);


void TQ_ASSERT(boolean cond)
{
  if(cond == FALSE)
  {
    qti_system_lock();
    while(1)
      ;
  }
}

static GPIO_TypeDef* _gpio_ports[] =
{
  0, GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF
};


void TQ_DEBUG_INIT(void)
{
  init_debug_pins();
}

static void init_debug_pins(void)
{
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  init_debug_pin(TQ_DEBUG_PIN_1);
  init_debug_pin(TQ_DEBUG_PIN_2);
  init_debug_pin(TQ_DEBUG_PIN_3);
}

static void init_debug_pin(uint8 pin)
{
  GPIO_TypeDef* gpio_port = _gpio_ports[(pin >> 4) & 0x0f];
  
  if(!gpio_port)
    return;
  GPIO_MODE(gpio_port, pin &0x0f, GPIO_MODE_OUTPUT);
}

void TQ_DEBUG_PIN_SET(uint8 pin, boolean assert)
{
  GPIO_TypeDef* gpio_port = _gpio_ports[(pin >> 4) & 0x0f];
  
  if(!gpio_port)
    return;
  
  if(assert)
    GPIO_SET_BITS(gpio_port, GPIO_BIT(pin & 0x0f));
  else
    GPIO_RESET_BITS(gpio_port, GPIO_BIT(pin & 0x0f));
}

#endif
