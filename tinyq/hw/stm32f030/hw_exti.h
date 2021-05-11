/****************************************************************************
  hw_exti.h
  Copyright (c) 2017 - 2018, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef EXTI_H
#define EXTI_H

typedef void (*STM32_EXTI_IRQ_HANDLER)(void);

extern void hw_exti_set(uint8 port, uint8 pin, uint8 trigger, STM32_EXTI_IRQ_HANDLER irq_handler);

#endif
