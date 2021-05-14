/****************************************************************************
  hw_exti.c
  Copyright (c) 2017 - 2018, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include "tq_types.h"
#include "tinyq.h"
#include "qti_system.h"
#include "hw_debug.h"
#include "hw_exti.h"
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_syscfg.h"


struct S_STM32_EXTI_ENTRY
{
  uint8 port;
  uint8 mode;
  uint8 trigger;
  STM32_EXTI_IRQ_HANDLER handler;
};

static void config_exti(uint8 pin, struct S_STM32_EXTI_ENTRY *entry);
static void config_irq(uint8 priority, uint8 channel, boolean enable);
static void invoke_irqs(uint32 irq_flags);


static struct S_STM32_EXTI_ENTRY _exti_table[16] =
{
{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
};

static int8 _exti_0_1_active = 0;
static int8 _exti_2_3_active = 0;
static int8 _exti_4_15_active = 0;


void hw_exti_set(uint8 port, uint8 pin, uint8 trigger, STM32_EXTI_IRQ_HANDLER irq_handler)
{
  int8 i, last;
  TQ_ASSERT(pin < 16);
  
  qti_system_lock();
  
  if(_exti_table[pin].handler && !irq_handler)
    i = -1;
  else if(!_exti_table[pin].handler && irq_handler)
    i = 1;
  else
    i = 0;
  
  _exti_table[pin].mode = EXTI_Mode_Interrupt;
  _exti_table[pin].port = port;
  _exti_table[pin].trigger = trigger;
  _exti_table[pin].handler = irq_handler;
  
  config_exti(pin, &_exti_table[pin]);
  
  if(pin == 0 || pin == 1)
  {
    last = _exti_0_1_active;
    _exti_0_1_active += i;
    if(i && (!_exti_0_1_active || !last))
      config_irq(2, EXTI0_1_IRQn, _exti_0_1_active);
  }
  else if(pin >= 2 && pin <= 3)
  {
    last = _exti_2_3_active;
    _exti_2_3_active += i;
    if(i && (!_exti_2_3_active || !last))
      config_irq(2, EXTI2_3_IRQn, _exti_2_3_active);
  }
  else if(pin >= 4 && pin <= 15)
  {
    last = _exti_4_15_active;
    _exti_4_15_active += i;
    if(i && (!_exti_4_15_active || !last))
      config_irq(2, EXTI4_15_IRQn, _exti_4_15_active);
  }
  
  qti_system_unlock();
}

static void config_exti(uint8 pin, struct S_STM32_EXTI_ENTRY *entry)
{
  EXTI_InitTypeDef    EXTI_InitStructure;

  if(entry->handler)
    SYSCFG_EXTILineConfig(entry->port, pin);
  
  EXTI_InitStructure.EXTI_Line = (1UL) << pin;
  EXTI_InitStructure.EXTI_Mode = (EXTIMode_TypeDef)entry->mode;
  EXTI_InitStructure.EXTI_Trigger = (EXTITrigger_TypeDef)entry->trigger;
  EXTI_InitStructure.EXTI_LineCmd = entry->handler ? ENABLE : DISABLE;
  EXTI_Init(&EXTI_InitStructure);
}

static void config_irq(uint8 priority, uint8 channel, boolean enable)
{
  NVIC_InitTypeDef    NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannelPriority = priority;
  NVIC_InitStructure.NVIC_IRQChannelCmd = enable ? ENABLE : DISABLE;
  NVIC_InitStructure.NVIC_IRQChannel = channel;
  NVIC_Init(&NVIC_InitStructure);
}

void EXTI0_1_IRQHandler(void)
{
  invoke_irqs(EXTI->PR & 0x0003);
}

void EXTI2_3_IRQHandler(void)
{
  invoke_irqs(EXTI->PR & 0x000c);
}

void EXTI4_15_IRQHandler(void)
{
  invoke_irqs(EXTI->PR & 0xfff0);
}

static void invoke_irqs(uint32 irq_flags)
{
  uint8 i;
  uint32 mask = 1;
  
  for(i = 0; i < 16; i++)
  {
    if(!irq_flags)
      break;
    
    if((irq_flags & 1) && _exti_table[i].handler)
    {
      _exti_table[i].handler();
      EXTI_ClearITPendingBit(mask);
    }
    irq_flags >>= 1;
    mask <<= 1;
  }
}
