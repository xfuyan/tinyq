/****************************************************************************
  qti_button.h
  Copyright (c) 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef QTI_BUTTON_H
#define QTI_BUTTON_H

#define BUTTON_NTF_DOWN             TQ_SIG_MAKE_NTF(TQ_DSP_NORMAL, 1)
#define BUTTON_NTF_UP               TQ_SIG_MAKE_NTF(TQ_DSP_NORMAL, 2)

extern void qti_button_set_listener(uint8 qti);
extern void qti_button_signal_entry(const struct TQ_QTI *self, uint8 from, uint8 sig, const uint8 *p, uint8 size);

#endif
