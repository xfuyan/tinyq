/****************************************************************************
  hw_gpio.h
  Copyright (c) 2017 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef HW_GPIO_H
#define HW_GPIO_H

#define GPIO_NO_PULL                        (0x00)
#define GPIO_PULL_UP                        (0x01)
#define GPIO_PULL_DOWN                      (0x02)
#define GPIO_PUPD_MASK(PIN)                 (~(0x03UL << ((PIN) * 2)))

#define GPIO_OTYPE_PP                       (0x00)
#define GPIO_OTYPE_OD                       (0x01)
#define GPIO_OTYPE_MASK(PIN)                (~(((uint16)(0x01)) << (PIN)))

#define GPIO_OSPEED_LOW                     (0x00)
#define GPIO_OSPEED_MEDIUM                  (0x01)
#define GPIO_OSPEED_HIGH                    (0x03)
#define GPIO_OSPEED_MASK(PIN)               (~(0x03UL << ((PIN) * 2)))

#define GPIO_MODE_INPUT                     (0x00)
#define GPIO_MODE_OUTPUT                    (0x01)
#define GPIO_MODE_AF                        (0x02)
#define GPIO_MODE_ANALOG                    (0x03)
#define GPIO_MODE_MASK(PIN)                 (~(0x03UL << ((PIN) * 2)))

#define GPIO_AF_MASK(PIN)                   (~(0x0fUL << (((PIN) & 7) * 4)))

#define GPIO_MODE(PORT, PIN, MODE)          ((PORT)->MODER = ((PORT)->MODER & GPIO_MODE_MASK(PIN)) | (((uint32)(MODE)) << ((PIN) * 2)))
#define GPIO_PUPD(PORT, PIN, PULL)          ((PORT)->PUPDR = ((PORT)->PUPDR & GPIO_PUPD_MASK(PIN)) | (((uint32)(PULL)) << ((PIN) * 2)))
#define GPIO_OSPEED(PORT, PIN, SPEED)       ((PORT)->OSPEEDR  = ((PORT)->OSPEEDR & GPIO_OSPEED_MASK(PIN)) | (((uint32)(SPEED)) << ((PIN) * 2)))
#define GPIO_OTYPE(PORT, PIN, TYPE)         ((PORT)->OTYPER   = ((PORT)->OTYPER  & GPIO_OTYPE_MASK(PIN))  | (((uint16)(TYPE)) << (PIN)))
#define GPIO_AF(PORT, PIN, AF)              ((PORT)->AFR[(PIN) / 8] = ((PORT)->AFR[(PIN) / 8] & GPIO_AF_MASK(PIN)) | (((uint32)(AF)) << (((PIN) & 7) * 4)))

#define GPIO_BIT(PIN)                       (((uint16)(1)) << (PIN))
#define GPIO_SET_BITS(PORT, BITS)           ((PORT)->BSRR = (BITS))
#define GPIO_RESET_BITS(PORT, BITS)         ((PORT)->BRR = (BITS))
#define GPIO_GET_BITS(PORT, BITS)           ((PORT)->IDR & (BITS))

#endif
