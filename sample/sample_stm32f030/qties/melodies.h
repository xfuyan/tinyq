/****************************************************************************
  melodies.h
  Copyright (c) 2017 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef MELODIES_H
#define MELODIES_H


#define WAVE_TABLE_SIZE               (16)

#define WAVE_STEP                     (0x00)
#define WAVE_LINR                     (0x01)
#define WAVE_SQRE                     (0x02)
#define WAVE_SINE                     (0x03)
#define WAVE_COSN                     (0x04)
#define WAVE_SLNT                     (0x0d)
#define CTRL_REPT                     (0x0e) /*value : 0 -> infinite loop*/
#define CTRL_STOP                     (0x0f)

#define TEMPO_GET_PERIOD(TEMPO)       ((TEMPO) & 0x0fff)
#define TEMPO_GET_FUNC(TEMPO)         (((TEMPO) >> 12) & 0x000f)
#define TEMPO_MAKE(FUNC, PERIOD)      ((((uint16)(FUNC)) << 12) | ((PERIOD) & 0x0fff))

#define NOTE(FUNC, TONE, PERIOD)      {(TONE), TEMPO_MAKE(FUNC, PERIOD)}

struct S_NOTE
{
  int16 tone;
  uint16 tempo;
};

/*Wave forms*/
static const uint8 _wave_table_zero[WAVE_TABLE_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const uint8 _wave_table_linear[WAVE_TABLE_SIZE] = {0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240};
static const uint8 _wave_table_sqr[WAVE_TABLE_SIZE] = {0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 169, 196, 225};
static const uint8 _wave_table_sin[WAVE_TABLE_SIZE] = {0, 25, 50, 74, 98, 121, 142, 162, 181, 198, 213, 226, 237, 245, 251, 255};
static const uint8 _wave_table_cos[WAVE_TABLE_SIZE] = {0, 1, 5, 11, 19, 30, 43, 58, 75, 94, 114, 135, 158, 182, 206, 231};

static const uint8* _wave_table[] =
{
  _wave_table_zero,
  _wave_table_linear,
  _wave_table_sqr,
  _wave_table_sin,
  _wave_table_cos,
};


/*LED MELODIES*/
static const struct S_NOTE _led_melody_power_up[] =
{
  NOTE(WAVE_SQRE, 0,    10),
  NOTE(WAVE_SINE, 100,  20),
  NOTE(CTRL_STOP, 0,    0),
};

static const struct S_NOTE _led_melody_1[] =
{
  NOTE(WAVE_SINE, 20,  20),
  NOTE(WAVE_COSN, 100, 40),
  NOTE(CTRL_REPT, 20,  0),
};
 
static const struct S_NOTE _led_melody_2[] =
{
  NOTE(WAVE_SQRE, 0,    20),
  NOTE(WAVE_SQRE, 100,  20),
  NOTE(WAVE_SLNT, 0,    20),
  NOTE(CTRL_REPT, 0,    0),
};
 
static const struct S_NOTE _led_melody_3[] =
{
  NOTE(WAVE_LINR, 20,   50),
  NOTE(WAVE_LINR, 50,   50),
  NOTE(CTRL_REPT, 100,  0),
};

static const struct S_NOTE _led_melody_4[] =
{
  NOTE(WAVE_LINR, 20,   50),
  NOTE(WAVE_LINR, 50,   50),
  NOTE(CTRL_REPT, 100,  0),
};

 static const struct S_NOTE _led_melody_5[] = 
{
  NOTE(WAVE_SINE, 0,   10),
  NOTE(WAVE_COSN, 100, 10),
  NOTE(CTRL_REPT, 0,  0),
};

/*BUZZER MELODIES*/
static const struct S_NOTE _buzzer_melody_power_up[] =
{
  NOTE(WAVE_LINR, 1000,   5),
  NOTE(WAVE_LINR, 2000,   2),
  NOTE(WAVE_LINR, 1000,   2),
  NOTE(CTRL_STOP, 2000,   0),
};
 
static const struct S_NOTE _buzzer_melody_1[] =
{
  NOTE(WAVE_LINR, 1700,  13),
  NOTE(WAVE_LINR, 3800,  12),
  NOTE(CTRL_REPT, 1700,  0),
};

static const struct S_NOTE _buzzer_melody_2[] =
{
  NOTE(WAVE_LINR, 2250,  24),
  NOTE(WAVE_LINR, 1025,  1),
  NOTE(CTRL_REPT, 2250,  0),
};

static const struct S_NOTE _buzzer_melody_3[] =
{
  NOTE(WAVE_STEP, 1790,  63),
  NOTE(WAVE_STEP, 1250,  57),
  NOTE(CTRL_REPT, 1790,  0),
};

static const struct S_NOTE _buzzer_melody_4[] =
{
  NOTE(WAVE_LINR, 626,   99),
  NOTE(WAVE_SLNT, 1640,  10),
  NOTE(CTRL_REPT, 626,  0),
};

static const struct S_NOTE _buzzer_melody_5[] =
{
  NOTE(WAVE_STEP, 2270,  25),
  NOTE(WAVE_SLNT, 2270,  25),
  NOTE(CTRL_REPT, 2270,  0),
};

/*MELODY TABLES*/
static const struct S_NOTE* const _led_melody_table[] =
{
  _led_melody_power_up,
  _led_melody_1,
  _led_melody_2,
  _led_melody_3,
  _led_melody_4,
  _led_melody_5
};

static const struct S_NOTE* const _buzzer_melody_table[] =
{
  _buzzer_melody_power_up,
  _buzzer_melody_1,
  _buzzer_melody_2,
  _buzzer_melody_3,
  _buzzer_melody_4,
  _buzzer_melody_5
};

#endif
