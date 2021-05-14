/****************************************************************************
  tq_port.c
  Copyright (c) 2020 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include "tq_types.h"
#include "tq_port.h"
#include "hw_debug.h"
#include "stm32f0xx.h"
#include "stm32f0xx_pwr.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_rtc.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"


#define RTC_PREDIV_A                          ((_PT_SLEEP_TIMER_FREQ / _PT_SLEEP_TIMER_TICK_PER_SECOND) - 1)
#define RTC_PREDIV_S                          (_PT_SLEEP_TIMER_TICK_PER_SECOND - 1)


static void rtc_init(void);
static void pendsv_init(void);

static int32 _last_rtc_tick = -1;


void tq_port_init(void)
{
  pendsv_init();
  rtc_init();
}

void tq_port_us_delay(uint32 us)
{
  uint32 i;
  
  for(i = 0; i < us; i++)
  {
    __nop();
    __nop();
    __nop();
    __nop();
  }
}

void tq_port_system_reset(void)
{
  NVIC_SystemReset();
}

static void pendsv_init(void)
{
  /* Set PendSV to lowest priority : 3 */
  NVIC_SetPriority(PendSV_IRQn , 0x03);
}

void PendSV_Handler(void)
{
  _tq_high_priority_dispatch();
}

void tq_port_trigger_high_priority_dispatch(void)
{
  /* Set PendSV pending status */
  SCB->ICSR = SCB->ICSR | (1UL << 28); 
}

void tq_port_sleep(boolean low_power)
{
#ifdef TQ_DEBUG
  PWR_EnterSleepMode(PWR_SLEEPEntry_WFI);
#else
  if(!low_power)
    PWR_EnterSleepMode(PWR_SLEEPEntry_WFI);
  else
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
#endif
}

/* timer with RTC*/
static void rtc_init(void)
{
  uint32 prediv;

  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  /* Allow access to Backup Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Clear Wakeup flag */
  PWR_ClearFlag(PWR_FLAG_WU);

  /* Reset RTC domain */
  RCC_BackupResetCmd(ENABLE);
  RCC_BackupResetCmd(DISABLE);
 
  /* Enable the LSI OSC */
  RCC_LSICmd(ENABLE);
  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
    ;
  
  {/* RTC Configuration*/        
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    RCC_RTCCLKCmd(ENABLE);
    
    RTC->WPR = 0xca;
    RTC->WPR = 0x53;
    RTC->ISR |= RTC_ISR_INIT;
    while(!(RTC->ISR & RTC_ISR_INITF))
      ;
    prediv = RTC_PREDIV_A;
    prediv <<= 16;
    prediv |= RTC_PREDIV_S;
    RTC->PRER = prediv;
    RTC->TR = 0;
    RTC->DR = 0x00002101;
    
    RTC->CR |= RTC_CR_BYPSHAD;
    RTC->ALRMAR = 0x80808080;
    RTC->ISR &= ~(RTC_ISR_INIT);
    RTC->WPR = 0xff;
  }

  {/* RTC interrupt init */
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;

    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    /* Enable the RTC Alarm Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  }
}

uint32 tq_port_sleep_timer_get_time_elapsed(boolean update)
{
  int32 current_rtc_tick;
  uint32 period;
  
  if(_last_rtc_tick < 0)
    return 0;
  
  current_rtc_tick = _PT_SLEEP_TIMER_TICK_PER_SECOND - RTC->SSR;
	
  if(current_rtc_tick < _last_rtc_tick)
    period = _PT_SLEEP_TIMER_TICK_PER_SECOND - _last_rtc_tick + current_rtc_tick;
  else
    period = current_rtc_tick - _last_rtc_tick;
  
  if(update)
    _last_rtc_tick = current_rtc_tick;

  return period;
}

void tq_port_sleep_timer_start(int32 ticks)
{
  TQ_ASSERT(ticks > 0);
  
  ticks = (ticks > _PT_SLEEP_TIMER_PERIOD_MAX) ? _PT_SLEEP_TIMER_PERIOD_MAX : ticks;
  ticks = (ticks < 4) ? 4 : ticks;  // The minimum vaule of ticks = 4
  
  _last_rtc_tick = _PT_SLEEP_TIMER_TICK_PER_SECOND - RTC->SSR;
	
  ticks += _last_rtc_tick;
  if(ticks > _PT_SLEEP_TIMER_TICK_PER_SECOND)
    ticks -= _PT_SLEEP_TIMER_TICK_PER_SECOND;

  RTC->WPR = 0xca;
  RTC->WPR = 0x53;
  RTC->CR &= ~(RTC_CR_ALRAE);
  while(!(RTC->ISR & RTC_ISR_ALRAWF))
    ;
  
  ticks = _PT_SLEEP_TIMER_TICK_PER_SECOND - ticks;
  ticks |= ((uint32)RTC_AlarmSubSecondMask_None << 24);
	RTC->ALRMASSR = ticks;

  RTC->CR |= RTC_CR_ALRAIE;
  RTC->ISR &= ~(RTC_ISR_ALRAF);
  RTC->CR |= RTC_CR_ALRAE;

  RTC->WPR = 0xff;
}

void tq_port_sleep_timer_stop(void)
{
  RTC->WPR = 0xca;
  RTC->WPR = 0x53;
  RTC->CR &= ~(RTC_CR_ALRAE);
  RTC->WPR = 0xff;

  _last_rtc_tick = -1;
}

void RTC_IRQHandler(void)
{
  if(RTC->ISR & RTC_ISR_ALRAF)
  { 
    RTC->ISR &= ~(RTC_ISR_ALRAF);
    _system_sleep_timer_handler();
  }
  EXTI_ClearITPendingBit(EXTI_Line17);
}
