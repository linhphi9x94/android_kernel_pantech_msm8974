/*
 *
 * File name: drivers/media/tdmb/mtv31/mtv319_ficdec.h
 *
 * Description : RAONTECH FIC Decoder API header file.
 *
 * Copyright (C) (2012, RAONTECH)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MTV319_FICDEC_H__
#define __MTV319_FICDEC_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "../mtv319/mtv319.h"
//#include "tdmb.h"

#define RTV_MAX_NUM_SUB_CH		64
#define RTV_MAX_ENSEMBLE_LABEL_SIZE	16
#define RTV_MAX_SERVICE_LABEL_SIZE	16

#if 1
struct sub_ch_info_type {
	/* Sub Channel Information */
	unsigned char sub_ch_id; /* 6 bits */
	unsigned short bit_rate; /* Service Component BitRate(kbit/s)  */
	unsigned short start_addr; /* 10 bits */

	/* FIG 0/2	*/
	unsigned char tmid; /* 2 bits */
	unsigned char svc_type; /* 6 bits */
	unsigned long svc_id; /* 16/32 bits */
	unsigned char svc_label[RTV_MAX_SERVICE_LABEL_SIZE+1]; /* 16*8 bits */
} ;

struct ensemble_info_type {
	unsigned long ensem_freq;	/* 4 bytes */
	unsigned char tot_sub_ch;	/* MAX: 64 */

	unsigned short ensem_id;
	unsigned char ensem_label[RTV_MAX_ENSEMBLE_LABEL_SIZE+1];
	struct sub_ch_info_type sub_ch[RTV_MAX_NUM_SUB_CH];
};
#endif

enum E_RTV_FIC_DEC_RET_TYPE {
	RTV_FIC_RET_GOING = 0,
	RTV_FIC_RET_DONE,
	RTV_FIC_RET_CRC_ERR
};

void rtvFICDEC_GetEnsembleInfo(struct ensemble_info_type *ensble,
				ULONG freq_khz);
enum E_RTV_FIC_DEC_RET_TYPE rtvFICDEC_Decode(U8 *fic_buf, UINT fic_size);
void rtvFICDEC_Init(void);



#ifdef __cplusplus
}
#endif

#endif /* __MTV319_FICDEC_H__ */

