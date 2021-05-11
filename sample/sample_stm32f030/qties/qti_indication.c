/****************************************************************************
  qti_indication.c
  Copyright (c) 2017 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include <string.h>
#include "tq_types.h"
#include "hw_debug.h"
#include "interpolator.h"
#include "tinyq.h"
#include "qties.h"
#include "qti_system.h"
#include "qti_indication.h"
#include "hw_gpio.h"
#include "stm32f0xx_rcc.h"
#include "melodies.h"


#define MELODY_TIMER_RANGE        (256)
#define BUZZER_TIMER_BASE         (16)
#define LED_TIMER_BASE            (BUZZER_TIMER_BASE + MELODY_TIMER_RANGE)


static void init_led(void);
static void stop_led(void);
static void play_led(uint8 id);
static void stop_buzzer(void);
static void play_buzzer(uint8 id);
static void init_buzzer(void);
static void timer_expire_handler(uint16 id);


static uint8 _self;
static uint8 _led_listener;
static uint8 _buzzer_listener;

void qti_indication_play_led(uint8 from, uint8 melody)
{
  stop_led();
  _led_listener = from;
  play_led(melody);
}

void qti_indication_stop_led(uint8 from)
{
  stop_led();
}

void qti_indication_play_buzzer(uint8 from, uint8 melody)
{
  stop_buzzer();
  _buzzer_listener = from;
  play_buzzer(melody);
}

void qti_indication_stop_buzzer(uint8 from)
{
  stop_buzzer();
}

void qti_indication_signal_entry(const struct TQ_QTI *self, uint8 from, uint8 sig, const uint8 *p, uint8 size)
{
  if(from == QTI_SYSTEM)
  {
    if(sig == SYSTEM_NTF_START)
    {
      _self = self->self;
      init_led();
      init_buzzer();
    }
    else if(sig == SYSTEM_RSP_TIMER)
      timer_expire_handler(*((uint16*)(p)));
  }
}

/* MELODY_PLAY */
typedef void (*HW_PLAY)(const struct S_NOTE*, struct S_INTERPOLATOR*, boolean descending);
typedef void (*HW_STOP)(void);
typedef void (*PLAY_FINISHED)(boolean finished);

struct S_MELODY_PLAY
{
  HW_PLAY hw_play;
  HW_STOP hw_stop;
  PLAY_FINISHED play_finished;
  
  const struct S_NOTE *melody;
  uint16 note_index;
  uint16 timer_id;
  uint16 timer_base;
  uint16 factor_x;
  uint8  repeat;
  struct S_INTERPOLATOR interpolator;
};

static void melody_play_note(struct S_MELODY_PLAY *play);

static void play_melody(struct S_MELODY_PLAY *play, const struct S_NOTE *melody, uint16 factor_x)
{
  play->hw_stop();
  
  play->melody = melody;
  play->factor_x = factor_x;
  play->note_index = 0;
  play->repeat = 255;
  melody_play_note(play);
}

static void stop_melody(struct S_MELODY_PLAY *play)
{
  if(play->melody)
  {
    play->hw_stop();
    qti_system_stop_timer(_self, play->timer_id);
    play->melody = 0;
    play->play_finished(FALSE);
  }
}

static void melody_play_note(struct S_MELODY_PLAY *play)
{
  const struct S_NOTE *note;
  int16  base;
  uint8  func;
  uint16 period;
 
  note = &(play->melody[play->note_index]);
  func = TEMPO_GET_FUNC(note->tempo); 
  period = TEMPO_GET_PERIOD(note->tempo);
  if(func == CTRL_STOP)
  {
    play->melody = 0;
    play->hw_stop();
    play->play_finished(TRUE);
    return;
  }
  
  if(func == CTRL_REPT)
  {
    if(period && play->repeat > period)
      play->repeat = period;
    
    if(period && play->repeat)
      play->repeat--;
    
    if(play->repeat)
      play->note_index = 0;
    else
    {
      play->melody = 0;
      play->hw_stop();
      play->play_finished(TRUE);
      return;
    }
  }
  
  note = &(play->melody[play->note_index]);
  func = TEMPO_GET_FUNC(note->tempo); 
  period = TEMPO_GET_PERIOD(note->tempo);
  if(func == WAVE_SLNT)
  {
    play->hw_stop();
    play->timer_id++;
    if(play->timer_id >= play->timer_base + MELODY_TIMER_RANGE)
      play->timer_id = play->timer_base;
    qti_system_start_timer(_self, play->timer_id, period * 10);
  }
  else
  {
    base = note[1].tone - note[0].tone;
    interpolator_init(&(play->interpolator), period * 10 * play->factor_x, base < 0 ? -base : base,
            _wave_table[func], WAVE_TABLE_SIZE);
    play->hw_play(note, &(play->interpolator), base < 0);
  }
}

static void melody_play_next_note(struct S_MELODY_PLAY *play)
{
  play->note_index++;
  melody_play_note(play);
}

static void melody_timer_expired(struct S_MELODY_PLAY *play, uint16 id)
{
  if(play->melody && id == play->timer_id)
    melody_play_next_note(play);
}

/* LED indication */
#define LED_PWM_FREQ        (100)
#define LED_LEVEL_RANGE     (100)

#define LED_PWM_PERIOD_MS   (1000 / LED_PWM_FREQ)

#define LED_CLK_FREQ        (8 * 1000 * 1000)
#define LED_TIM             (TIM3)
#define LED_TIM_AAR         (LED_LEVEL_RANGE - 1)
#define LED_TIM_PSC         (LED_CLK_FREQ / LED_LEVEL_RANGE / LED_PWM_FREQ - 1)

static void hw_led_stop(void);
static void hw_led_play(const struct S_NOTE *note, struct S_INTERPOLATOR *interpolator, boolean descending);
static void play_led_finished(boolean finished);

static uint8    _led_melody_id;
static boolean  _led_interpolate_descending;
static struct S_INTERPOLATOR *_led_interpolator;
static const struct S_NOTE   *_led_note;
static struct S_MELODY_PLAY   _led_play;

static void init_led(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  memset(&_led_play, 0, sizeof(_led_play));
  _led_play.hw_play = hw_led_play;
  _led_play.hw_stop = hw_led_stop;
  _led_play.play_finished = play_led_finished;
  _led_play.timer_base = LED_TIMER_BASE;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  
  //PB05
  GPIO_AF(GPIOB, 5, 1);
  GPIO_MODE(GPIOB, 5, GPIO_MODE_AF);

  LED_TIM->SR &= (~TIM_SR_UIF);
  LED_TIM->CR1 |= TIM_CR1_ARPE;
  LED_TIM->CCMR1 |= (TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2);
  LED_TIM->DIER |= TIM_DIER_UIE;
  LED_TIM->PSC = LED_TIM_PSC;
  LED_TIM->ARR = LED_TIM_AAR;

  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

static void play_led(uint8 id)
{
  if(id >= sizeof(_led_melody_table) / sizeof(_led_melody_table[0]))
    return;
  
  _led_melody_id = id + 1;
  play_melody(&_led_play, _led_melody_table[id], 1);
}

static void stop_led(void)
{
  stop_melody(&_led_play);
}

static void play_led_finished(boolean finished)
{
  TQ_ASSERT(_led_melody_id);
  _led_melody_id--;
  tinyq_send_signal(_self, _led_listener, INDICATION_NTF_LED_STOPPED, &_led_melody_id, 1);
  _led_melody_id = 0;
}

static void hw_led_play(const struct S_NOTE *note, struct S_INTERPOLATOR *interpolator, boolean descending)
{
  int16 v;

  qti_system_lock();
  if(!_led_interpolator)
  {
    qti_system_request_wait(_self);
  }
  _led_interpolate_descending = descending;
  _led_note = note;
  _led_interpolator = interpolator;
  qti_system_unlock();
  
  v = _led_note->tone;
  LED_TIM->CCR2 = v;
  LED_TIM->EGR |= TIM_EGR_UG; //trigger update event
  LED_TIM->CR1 |= (TIM_CR1_CEN);
  LED_TIM->CCER |= (TIM_CCER_CC2E);
}

static void hw_led_stop(void)
{
  LED_TIM->CCER &= (~TIM_CCER_CC2E);
  LED_TIM->CR1 &= (~TIM_CR1_CEN);
  LED_TIM->SR = 0;
  NVIC_ClearPendingIRQ(TIM3_IRQn);
  
  qti_system_lock();
  if(_led_interpolator)
  {
    _led_interpolator = 0;
    qti_system_release_wait(_self);
  }
  qti_system_unlock();
}

static boolean led_next_value(int16 step, int16 *value)
{
  static uint16   last_counter;

  if(_led_interpolator && interpolator_step(_led_interpolator, LED_PWM_PERIOD_MS))
  {
    last_counter = _led_note->tone;
    if(_led_interpolate_descending)
      last_counter -= interpolator_get_value(_led_interpolator);
    else
      last_counter += interpolator_get_value(_led_interpolator);
    
    *value = last_counter;
    return TRUE;
  }
  else
  {
    *value = last_counter;
    return FALSE;
  }
}

void TIM3_IRQHandler(void)
{
  int16 v;
  boolean next;
  
  if(LED_TIM->SR & TIM_SR_UIF)
  { // update
    LED_TIM->SR &= ~(TIM_SR_UIF);
    next = led_next_value(LED_PWM_PERIOD_MS, &v);
    LED_TIM->CCR2 = v;
    if(!next)
    {
      melody_play_next_note(&_led_play);
    }
  }
}


/* BUZZER indication */
/*
  TIM PSC = 1 to make pwm clock to be 0.25us period.
  256 * pwm counter = 256 * 0.25us = 64us
  set interpolator x unit in 64us
*/

#define BUZZER_X_FACTOR         (1000 / 64)           
#define BUZZER_CLK_FREQ         (4 * 1000 * 1000UL)
#define BUZZER_TIM              (TIM17)
#define BUZZER_DMA              (DMA1_Channel1)
#define BUZZER_DMA_SIZE         (20)

static void hw_buzzer_stop(void);
static void hw_buzzer_play(const struct S_NOTE*, struct S_INTERPOLATOR*, boolean descending);
static void play_buzzer_finished(boolean finished);
static uint16 buzzer_fill_buffer(uint16 *addr, uint16 n);

static uint8    _buzzer_melody_id;
static boolean  _buzzer_interpolate_descending;
static struct S_INTERPOLATOR  *_buzzer_interpolator;
static const struct S_NOTE    *_buzzer_note;

static boolean  _buzzer_hw_running = FALSE;
static uint16   _buzzer_dma_buffer[BUZZER_DMA_SIZE * 3];
static uint16   _buzzer_dma_count;
static uint32   _buzzer_cx;
static uint32   _buzzer_last_cx;
static int16    _buzzer_last_step;

struct S_MELODY_PLAY _buzzer_play;

static void init_buzzer(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
 
  memset(&_buzzer_play, 0, sizeof(_buzzer_play));
  _buzzer_play.hw_play = hw_buzzer_play;
  _buzzer_play.hw_stop = hw_buzzer_stop;
  _buzzer_play.play_finished = play_buzzer_finished;
  _buzzer_play.timer_base = BUZZER_TIMER_BASE;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
 
  DMA_DeInit(DMA1_Channel1);
  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&TIM17->DMAR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_buzzer_dma_buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = BUZZER_DMA_SIZE * 3;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//DMA_Mode_Normal;//
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  DMA_ITConfig(DMA1_Channel1, DMA_IT_HT | DMA_IT_TC, ENABLE);

  //PB07
  GPIO_AF(GPIOB, 7, 2);
  GPIO_MODE(GPIOB, 7, GPIO_MODE_AF);
  
  //PB09
  GPIO_AF(GPIOB, 9, 2);
  GPIO_MODE(GPIOB, 9, GPIO_MODE_AF);
  
  BUZZER_TIM->CR1 |= TIM_CR1_ARPE;
  BUZZER_TIM->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC1NE);
  BUZZER_TIM->CCMR1 |= (TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);
  BUZZER_TIM->DCR |= (TIM_DMABurstLength_3Transfers | TIM_DMABase_ARR);    // [DMA BURST:3] [DMA BASE ADDR:TIMX_ARR(0x2C)]
  BUZZER_TIM->DIER |= TIM_DIER_UDE;
  BUZZER_TIM->BDTR = 0x50;      // [DEAD TIME:80 * 125ns = 10us]
  
   // BUZZER_DMA Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

static void play_buzzer(uint8 id)
{
  if(id >= sizeof(_buzzer_melody_table) / sizeof(_buzzer_melody_table[0]))
    return;
  
  _buzzer_melody_id = id + 1;
  play_melody(&_buzzer_play, _buzzer_melody_table[id], BUZZER_X_FACTOR);
}

static void stop_buzzer(void)
{
  stop_melody(&_buzzer_play);
}

static void play_buzzer_finished(boolean finished)
{
  TQ_ASSERT(_buzzer_melody_id);
  _buzzer_melody_id--;
  tinyq_send_signal(_self, _buzzer_listener, INDICATION_NTF_BUZZER_STOPPED, &_buzzer_melody_id, 1);
  _buzzer_melody_id = 0;
}

static void hw_buzzer_play(const struct S_NOTE* note, struct S_INTERPOLATOR* interpolator, boolean descending)
{
  qti_system_lock();
  if(!_buzzer_interpolator)
  {
    qti_system_request_wait(_self);
  }
  _buzzer_interpolate_descending = descending;
  _buzzer_note = note;
  _buzzer_cx = _buzzer_last_cx = 0;
  _buzzer_interpolator = interpolator;
  qti_system_unlock();
  
  if(!_buzzer_hw_running)
  {
    _buzzer_dma_count = buzzer_fill_buffer(_buzzer_dma_buffer, BUZZER_DMA_SIZE);
    BUZZER_DMA->CNDTR = 3 * BUZZER_DMA_SIZE;
    BUZZER_DMA->CCR |= DMA_CCR_EN;

    BUZZER_TIM->PSC = 1;
    BUZZER_TIM->ARR = 1;
    BUZZER_TIM->RCR = 0;
    BUZZER_TIM->CCR1 = 0;
    BUZZER_TIM->EGR |= TIM_EGR_UG;
    
    BUZZER_TIM->CR1 |= (TIM_CR1_CEN);
    BUZZER_TIM->BDTR |= TIM_BDTR_MOE;
    
    _buzzer_hw_running = TRUE;
  }
}

static void hw_buzzer_stop(void)
{
  BUZZER_DMA->CCR &= (~DMA_CCR_EN);
  BUZZER_TIM->CR1 &= (~TIM_CR1_CEN);
  BUZZER_TIM->BDTR &= (~TIM_BDTR_MOE);
  
  BUZZER_TIM->SR = 0;
  NVIC_ClearPendingIRQ(DMA1_Channel1_IRQn);
  _buzzer_hw_running = FALSE;

  qti_system_lock();
  if(_buzzer_interpolator)
  {
    _buzzer_interpolator = 0;
    qti_system_release_wait(_self);
  }
  qti_system_unlock();
}

static uint16 buzzer_fill_buffer(uint16 *addr, uint16 n)
{
  static uint16   last_counter;
  uint32 freq;
  uint16 count = 0;
  
  while(n--)
  {
    if(_buzzer_interpolator && interpolator_step(_buzzer_interpolator, _buzzer_last_step))
    {
      freq = _buzzer_note->tone;
      if(_buzzer_interpolate_descending)
        freq -= interpolator_get_value(_buzzer_interpolator);
      else
        freq += interpolator_get_value(_buzzer_interpolator);
      
      TQ_ASSERT(freq > 0);
      
      last_counter = BUZZER_CLK_FREQ / freq ;
      _buzzer_cx += last_counter;
      _buzzer_last_step = (_buzzer_cx - _buzzer_last_cx) >> 8;
      if(_buzzer_last_step)
        _buzzer_last_cx = _buzzer_cx;
      count++;
    }
    
    *addr++ = last_counter;
    *addr++ = 0;
    *addr++ = (last_counter >> 1);   
  }
  return count;
}

void DMA1_Channel1_IRQHandler(void)
{
  if(_buzzer_dma_count == 0 && _buzzer_interpolator)
  {
    melody_play_next_note(&_buzzer_play);
  }
  if(DMA_GetITStatus(DMA1_IT_HT1))
  {
    _buzzer_dma_count = buzzer_fill_buffer(&_buzzer_dma_buffer[0], BUZZER_DMA_SIZE / 2);
    DMA_ClearITPendingBit(DMA1_IT_HT1);
  }
  if(DMA_GetITStatus(DMA1_IT_TC1))
  {
    _buzzer_dma_count = buzzer_fill_buffer(&_buzzer_dma_buffer[BUZZER_DMA_SIZE * 3 / 2], BUZZER_DMA_SIZE / 2);
    DMA_ClearITPendingBit(DMA1_IT_TC1);
  }
}

static void timer_expire_handler(uint16 id)
{
  melody_timer_expired(&_led_play, id);
  melody_timer_expired(&_buzzer_play, id);
}
