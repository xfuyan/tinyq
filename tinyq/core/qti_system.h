/****************************************************************************
  qti_system.h
  Copyright (c) 2017 - 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#ifndef QTI_SYSTEM_H
#define QTI_SYSTEM_H


#define SYSTEM_NTF_START                    TQ_SIG_MAKE_NTF(TQ_DSP_NORMAL, 0)
#define SYSTEM_NTF_TIME_CHANGE              TQ_SIG_MAKE_NTF(TQ_DSP_NORMAL, 1)

#define SYSTEM_RSP_TIMER                    TQ_SIG_MAKE_RSP(TQ_DSP_NORMAL, 0)

struct TQ_QTI;

extern void qti_system_us_delay(uint32 us);
extern void qti_system_reset(void); 
extern void qti_system_lock(void);
extern void qti_system_unlock(void);
extern void qti_system_request_wait(uint8 qti);
extern void qti_system_release_wait(uint8 qti);
extern void qti_system_start_timer(uint8 qti, uint16 id, uint32 period);
extern void qti_system_stop_timer(uint8 qti, uint16 id);

extern void qti_system_signal_entry(const struct TQ_QTI *self, uint8 from, uint8 sig, const uint8 *p, uint8 size);


#endif
