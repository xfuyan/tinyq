/****************************************************************************
  qties.c
  Copyright (c) 2021, Xiaofu Yan.  All rights reserved.
****************************************************************************/
#include "tq_types.h"
#include "tinyq.h"
#include "qties.h"
#include "qti_system.h"
#include "qti_button.h"
#include "qti_indication.h"
#include "qti_sample.h"

const uint8 tq_qti_count = _QTI_COUNT_;


/* table of Qties */
const struct TQ_QTI tq_qti_table[_QTI_COUNT_] =
{
  {QTI_SYSTEM,        qti_system_signal_entry},
  {QTI_BUTTON,        qti_button_signal_entry},
  {QTI_INDICATION,    qti_indication_signal_entry},
  {QTI_SAMPLE,        qti_sample_signal_entry}
};

