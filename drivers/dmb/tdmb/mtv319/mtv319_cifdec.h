/******************************************************************************
* (c) COPYRIGHT 2012 RAONTECH, Inc. ALL RIGHTS RESERVED.
*
* This software is the property of RAONTECH and is furnished under license
* by RAONTECH.
* This software may be used only in accordance with the terms of said license.
* This copyright noitce may not be remoced, modified or obliterated without
* the prior written permission of RAONTECH, Inc.
*
* This software may not be copied, transmitted, provided to or otherwise
* made available to any other person, company, corporation or other entity
* except as specified in the terms of said license.
*
* No right, title, ownership or other interest in the software is hereby
* granted or transferred.
*
* The information contained herein is subject to change without notice
* and should not be construed as a commitment by RAONTECH, Inc.
*
* TITLE : MTV319 CIF decoder header file.
*
* FILENAME    : mtv319_cifdec.h
*
* DESCRIPTION :
*		This file contains types and declarations associated with the
*		CIF decoding Services.
*
******************************************************************************/
/******************************************************************************
* REVISION HISTORY
*
*    DATE         NAME          REMARKS
* ----------  -------------    ------------------------------------------------
* 07/12/2012  Ko, Kevin        Created.
******************************************************************************/

#ifndef __MTV319_CIFDEC_H__
#define __MTV319_CIFDEC_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "mtv319.h"

#define RTV_CIFDEC_INVALID_BUF_IDX	0xFF

struct RTV_CIF_DEC_INFO {
#ifdef RTV_FIC_CIFMODE_ENABLED
	unsigned int fic_size; /* Result size. */

	/* Source buffer address to be decoded. (Input/Output) */
	unsigned char *fic_buf_ptr;
#endif

	/* Decoded MSC sub channel size (Output) */
	unsigned int subch_size[RTV_MAX_NUM_USE_SUBCHANNEL];

	/* Decoded MSC sub channel ID.(Output) */
	unsigned int subch_id[RTV_MAX_NUM_USE_SUBCHANNEL];

	/* Source MSC buffer address to be decoded. (Input/Output) */
	unsigned char *subch_buf_ptr[RTV_MAX_NUM_USE_SUBCHANNEL];

	/* Source MSC buffer size. (Input) */
	unsigned int subch_buf_size[RTV_MAX_NUM_USE_SUBCHANNEL];
};

#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
	#define RTV_CIFDEC_HEADER_SYNC_BYTE		0xE2
#else
	#define RTV_CIFDEC_HEADER_SYNC_BYTE		0x47
#endif

int rtvCIFDEC_Decode(struct RTV_CIF_DEC_INFO *ptDecInfo,
			const U8 *pbTsBuf, UINT nTsLen);
UINT rtvCIFDEC_SetDiscardTS(int nFicMscType, U8 *pbTsBuf, UINT nTsLen);
UINT rtvCIFDEC_GetDecBufIndex(UINT nSubChID);
void rtvCIFDEC_DeleteSubChannelID(UINT nSubChID);
BOOL rtvCIFDEC_AddSubChannelID(UINT nSubChID,
				enum E_RTV_SERVICE_TYPE eServiceType);
void rtvCIFDEC_Deinit(void);
void rtvCIFDEC_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MTV319_CIFDEC_H__ */


