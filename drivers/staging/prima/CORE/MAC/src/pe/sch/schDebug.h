/*
<<<<<<< HEAD
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
=======
 * Copyright (c) 2012-2013 The Linux Foundation. All rights reserved.
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
<<<<<<< HEAD
/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
=======

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
 */

/*
 *
<<<<<<< HEAD
 * Airgo Networks, Inc proprietary. All rights reserved.
=======
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
 * This file schDebug.h contains some debug macros.
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#ifndef __SCH_DEBUG_H__
#define __SCH_DEBUG_H__

#include "utilsApi.h"
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halCommonApi.h"
#endif
#include "sirDebug.h"

<<<<<<< HEAD


void schLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...) ;

// -------------------------------------------------------------
/**
 *
 */

#ifdef SCH_DEBUG_STATS
inline void schClass::schTrace(tSchTrace event, tANI_U32 arg)
{
    if (gSchFreezeDump) return;
    if ((tANI_U32)event >= traceLevel) return;

    traceBuf[curTrace].event = event;
    traceBuf[curTrace].arg = arg;
    traceBuf[curTrace].timestamp = halGetTsfLow(pMac);
    curTrace = (curTrace+1)%SCH_TRACE_BUF_SIZE;
}
#endif

=======
#if !defined(__printf)
#define __printf(a,b)
#endif

void __printf(3,4) schLog(tpAniSirGlobal pMac, tANI_U32 loglevel,
                          const char *pString, ...);

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#endif
