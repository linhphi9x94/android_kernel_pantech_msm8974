//=============================================================================
// File       : fc8150_baseband.h
//
// Description: 
//
// Revision History:
//
// Version         Date           Author        Description of Changes
//-----------------------------------------------------------------------------
//  1.0.0       2013/05/23     zeros(P11274)    Create
//=============================================================================

#ifndef _FC8150_BB_H_
#define _FC8150_BB_H_

#include "../../dmb_type.h"
#include "../isdbt_bb.h"


/* ========== Message ID for SHARP ========== */

#define ISDBT_MSG_FC8150_BB(fmt, arg...) \
  DMB_KERN_MSG_ALERT(fmt, ##arg)



/*====================================================================
FUNCTION       isdbt_bb_fc8150_init
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
boolean isdbt_bb_fc8150_init(isdbt_bb_function_table_type *);

int fc8150_i2c_write_data(unsigned char c, unsigned char *data, unsigned long data_width);
int fc8150_i2c_read_data(unsigned char c, unsigned char *data, unsigned long data_width);

/*====================================================================
FUNCTION       fc8150_power_on
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_power_on(void);

/*====================================================================
FUNCTION       fc8150_power_off
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_power_off(void);

/*====================================================================
FUNCTION       fc8150_bb_init
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8150_bb_init(void);

/*====================================================================
FUNCTION       fc8150_bb_set_frequency
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8150_bb_set_frequency(int freq_idx);

/*====================================================================
FUNCTION       fc8150_bb_start_ts
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8150_bb_start_ts(int enable);

/*====================================================================
FUNCTION       fc8150_bb_get_status
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_bb_get_status(tIsdbtSigInfo *sig_info);

/*====================================================================
FUNCTION       fc8150_bb_get_tuner_info
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_bb_get_tuner_info(tIsdbtTunerInfo *tuner_info);

/*====================================================================
FUNCTION       fc8150_test
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_test(int index);

/*====================================================================
FUNCTION       fc8150_bb_read_int
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_bb_read_int(void);

/*====================================================================
FUNCTION       fc8150_bb_read_ts
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8150_bb_read_ts(byte *);

/*====================================================================
FUNCTION       fc8150_bb_set_scan_mode
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_bb_set_scan_mode(int);

#endif /* _FC8150_BB_H_ */

