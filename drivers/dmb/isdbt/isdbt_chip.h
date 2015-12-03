//=============================================================================
// File       : isdbt_chip.h
//
// Description: 
//
// Revision History:
//
// Version         Date           Author        Description of Changes
//-----------------------------------------------------------------------------
//  1.0.0       2011/09/29       yschoi         Create
//=============================================================================

#ifndef _ISDBT_CHIP_H_
#define _ISDBT_CHIP_H_

#include "isdbt_comdef.h"

#if defined (FEATURE_ISDBT_USE_FC8150)
#include "fc8150/fc8150_wrapper.h"
#elif defined (FEATURE_ISDBT_USE_SHARP)
#include "sharp/sharp_bb.h"
//#include "sharp/ntv/inc/nmiioctl.h"
//#elif defined()
#else
#error
#endif

#endif /* _ISDBT_CHIP_H_ */

