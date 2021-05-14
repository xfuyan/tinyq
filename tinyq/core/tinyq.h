/****************************************************************************
  tinyq.h
  Copyright (c) 2020 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef TQ_NETWORK_H
#define TQ_NETWORK_H

#define TQ_DSP_HIGH                   (0x00)
#define TQ_DSP_NORMAL                 (0x80)
#define TQ_DSP_MASK                   (0x80)

#define TQ_SIG_TYPE_NTF               (0x00)
#define TQ_SIG_TYPE_CMD               (0x20)
#define TQ_SIG_TYPE_RSP               (0x40)
#define TQ_SIG_TYPE_LCL               (0x60)
#define TQ_SIG_TYPE_MASK              (0x60)

#define TQ_SIG_MAKE_NTF(DSP, IDX)     ((DSP) | (IDX) | TQ_SIG_TYPE_NTF)
#define TQ_SIG_MAKE_CMD(DSP, IDX)     ((DSP) | (IDX) | TQ_SIG_TYPE_CMD)
#define TQ_SIG_MAKE_RSP(DSP, IDX)     ((DSP) | (IDX) | TQ_SIG_TYPE_RSP)
#define TQ_SIG_MAKE_LCL(IDX)          (TQ_DSP_NORMAL | (IDX) | TQ_SIG_TYPE_LCL)

#define TQ_SIG_TYPE(SIG)              ((SIG) & TQ_SIG_TYPE_MASK)
#define TQ_SIG_DISPATCHER(SIG)        ((SIG) & TQ_DSP_MASK)

#define QTI_BROADCAST                 (0xff)

struct  TQ_QTI
{
  uint8 self;
  void (*signal_entry)(const struct TQ_QTI *self, uint8 from, uint8 sig, const uint8 *param, uint8 param_size);
};


extern void tinyq_run(void);
extern void tinyq_send_signal(uint8 from, uint8 to, uint8 sig, const void *param, uint8 param_size);

#endif
