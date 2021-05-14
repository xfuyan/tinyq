/****************************************************************************
  qti_indication.h
  Copyright (c) 2017 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef QTI_INDICATION_H
#define QTI_INDICATION_H

#define INDICATION_NTF_LED_STOPPED          TQ_SIG_MAKE_NTF(TQ_DSP_NORMAL, 0)
#define INDICATION_NTF_BUZZER_STOPPED       TQ_SIG_MAKE_NTF(TQ_DSP_NORMAL, 1)


#define INDICATION_BUZZER_MELODY_POWER_UP             (0)
#define INDICATION_BUZZER_MELODY_1                    (1)
#define INDICATION_BUZZER_MELODY_2                    (2)
#define INDICATION_BUZZER_MELODY_3                    (3)
#define INDICATION_BUZZER_MELODY_4                    (4)
#define INDICATION_BUZZER_MELODY_5                    (5)


#define INDICATION_LED_MELODY_POWER_UP                (0)
#define INDICATION_LED_MELODY_1                       (1)
#define INDICATION_LED_MELODY_2                       (2)
#define INDICATION_LED_MELODY_3                       (3)
#define INDICATION_LED_MELODY_4                       (4)
#define INDICATION_LED_MELODY_5                       (5)


extern void qti_indication_play_led(uint8 from, uint8 melody);
extern void qti_indication_stop_led(uint8 from);
extern void qti_indication_play_buzzer(uint8 from, uint8 melody);
extern void qti_indication_stop_buzzer(uint8 from);

extern void qti_indication_signal_entry(const struct TQ_QTI *self, uint8 from, uint8 sig, const uint8 *p, uint8 size);


#endif
