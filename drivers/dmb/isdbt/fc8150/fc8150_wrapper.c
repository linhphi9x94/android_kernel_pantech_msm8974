//=============================================================================
// File       : fc8150_wrapper.c
// Description: 
//
// Revision History:
// Version         Date           Author        Description of Changes
//-----------------------------------------------------------------------------
//  1.0.0       2013/05/23       zeros(P11274)  Create
//=============================================================================

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include "../../dmb_hw.h"
#include "../../dmb_interface.h"
#include "../../dmb_test.h"

#include "../isdbt_comdef.h"
#include "../isdbt_dev.h"
#include "../isdbt_test.h"

#include "bbm.h"
#include "fc8150_bb.h"
#include "fc8150_regs.h"
#include "fci_tun.h"
#include "fc8150_wrapper.h"


/*================================================================== */
/*=================       FC8150 BB Definition      ================= */
/*================================================================== */

boolean isdbt_chip_power_on = FALSE;
uint8 scan_mode = 0;

#define FC8150_TS_BUFFERS          16
#define FC8150_TS_BUFFER_SIZE      TS_BUF_SIZE

typedef struct _INT_TYPE
{
    uint8   valid;
    uint32  address;
    uint32  length;
} INT_TYPE;
INT_TYPE ts_type[FC8150_TS_BUFFERS];
int gInputBuffer = 0;
int gOutputBuffer = 0;
u8 gTSData[FC8150_TS_BUFFER_SIZE * FC8150_TS_BUFFERS];


/*================================================================== */
/*=================       FC8150 BB Function       ================== */
/*================================================================== */

static boolean fc8150_function_register(isdbt_bb_function_table_type *ftable_ptr)
{
    ISDBT_MSG_FC8150_BB("[%s] !!!\n", __func__);

    ftable_ptr->isdbt_bb_power_on           = fc8150_power_on;
    ftable_ptr->isdbt_bb_power_off          = fc8150_power_off;
    ftable_ptr->isdbt_bb_init               = fc8150_bb_init;
    ftable_ptr->isdbt_bb_set_freq           = fc8150_bb_set_frequency;
    ftable_ptr->isdbt_bb_start_ts           = fc8150_bb_start_ts;
    ftable_ptr->isdbt_bb_get_status         = fc8150_bb_get_status;
    ftable_ptr->isdbt_bb_get_tuner_info     = fc8150_bb_get_tuner_info;
    ftable_ptr->isdbt_bb_test               = fc8150_test;
    ftable_ptr->isdbt_bb_read_int           = fc8150_bb_read_int;
    ftable_ptr->isdbt_bb_read_ts            = fc8150_bb_read_ts;
    ftable_ptr->isdbt_bb_set_scan_mode      = fc8150_bb_set_scan_mode;

    return TRUE;
}

int ISDBT_DATA_CALLBACK(u32 hDevice, u8 *data, int len)
{
    //ISDBT_MSG_FC8150_BB("[%s] data_callback\n", __func__);

    if (((gInputBuffer + 1) % FC8150_TS_BUFFERS) == gOutputBuffer) {
        ISDBT_MSG_FC8150_BB("[%s] BUFFER ALLOCATION ERRROR\n", __func__);
        return 0;
    }

    memcpy((void*)&gTSData[gInputBuffer * FC8150_TS_BUFFER_SIZE], data, len);

    ts_type[gInputBuffer].length = len;
    ts_type[gInputBuffer].address = (uint32)&gTSData[gInputBuffer * FC8150_TS_BUFFER_SIZE];
    
    gInputBuffer = (gInputBuffer + 1) % FC8150_TS_BUFFERS;

#ifdef FEATURE_TEST_READ_DATA_ON_BOOT
    ISDBT_MSG_FC8150_BB("[%s] data (0)0x%X (1)0x%X (2)0x%X (3)0x%X (len)%d\n", __func__, *(data), *(data+1), *(data+2), *(data+3), len);
#ifdef FEATURE_DMB_DUMP_FILE
    dmb_data_dump(data, len, FILENAME_BEFORE_PARSING);
#endif /* FEATURE_DMB_DUMP_FILE */
#endif /* FEATURE_TEST_READ_DATA_ON_BOOT */

    return TRUE;
}

/*====================================================================
FUNCTION       isdbt_bb_fc8150_init  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
boolean isdbt_bb_fc8150_init(isdbt_bb_function_table_type *function_table_ptr)
{
    boolean bb_initialized;

    bb_initialized = fc8150_function_register(function_table_ptr);

    return bb_initialized;
}

/*====================================================================
FUNCTION        fc8150_power_on
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_power_on(void)
{
// 1. DMB_PWR_EN : HIGH
// 2. DMB_RESET : HIGH
// 3. DMB_RESET : LOW
// 4. DMB_RESET : HIGH

    ISDBT_MSG_FC8150_BB("[%s] start!!!\n", __func__);

    dmb_power_on();

	dmb_power_on_chip();

	dmb_set_gpio(DMB_RESET, 1);
	mdelay(5);
	dmb_set_gpio(DMB_RESET, 0);
	mdelay(1);
	dmb_set_gpio(DMB_RESET, 1);

    isdbt_chip_power_on = TRUE;

    //ISDBT_MSG_FC8150_BB("[%s] end!!!\n", __func__);
}


/*====================================================================
FUNCTION       fc8150_power_off
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_power_off(void)
{
// 1. DMB_PWR_EN : LOW

    ISDBT_MSG_FC8150_BB("[%s] start!!![%d]\n", __func__, isdbt_chip_power_on);

    if (!isdbt_chip_power_on) {
        ISDBT_MSG_FC8150_BB("[%s] aleady power off\n", __func__);
        return;
    }

    dmb_power_off();

    dmb_set_gpio(DMB_RESET, 0);

    isdbt_chip_power_on = FALSE;

    //ISDBT_MSG_FC8150_BB("[%s] end!!!\n", __func__);
}

/*====================================================================
FUNCTION       fc8150_bb_init
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8150_bb_init(void)
{
    int ret = BBM_NOK;

    ISDBT_MSG_FC8150_BB("[%s] start (BBM_XTAL_FREQ)%d\n", __func__, BBM_XTAL_FREQ);

    ret = BBM_HOSTIF_SELECT(NULL, BBM_SPI);
    if (ret) {
        ISDBT_MSG_FC8150_BB("[%s] hostif select fail!!! \n", __func__);
        return ret;
    }

    ret = BBM_I2C_INIT(NULL, FCI_I2C_TYPE);
    ret |= BBM_PROBE(NULL);
    if (ret) {
        ISDBT_MSG_FC8150_BB("[%s] FC8150 Initialize Fail (ret)%d\n", __func__, ret);
        return ret;
    }

    ret = BBM_INIT(NULL);
    ret |= BBM_TUNER_SELECT(NULL, FC8150_TUNER, 0);
    if (ret) {
        ISDBT_MSG_FC8150_BB("[%s] fail to BBM_INIT (ret)%d\n", __func__, ret);
        return ret;
    }

    BBM_TS_CALLBACK_REGISTER(0, ISDBT_DATA_CALLBACK);

    scan_mode = 0;

    //ISDBT_MSG_FC8150_BB("[%s] end \n", __func__);
    return BBM_OK;
}

/*====================================================================
FUNCTION       fc8150_bb_set_scan_mode
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_bb_set_scan_mode(int scanmode)
{
	ISDBT_MSG_FC8150_BB("[%s] start (scanmode)%d\n", __func__, scanmode);
	
	if (scanmode) {
		if (!scan_mode) {
			BBM_WRITE(0, 0x3040, 0x00);
			BBM_WRITE(0, 0x3004, 0x02);
			BBM_WRITE(0, 0x3006, 0x02);
			BBM_WRITE(0, 0x2020, 0x18);
			BBM_WRITE(0, 0x2021, 0x14);
			BBM_WRITE(0, 0x2022, 0xea);
			BBM_WRITE(0, 0x2082, 0x70);
			BBM_WRITE(0, 0x2083, 0x70);
			BBM_WRITE(0, 0x2084, 0x70);
			BBM_WRITE(0, 0x2085, 0x60);
			
			scan_mode = 1;
			ISDBT_MSG_FC8150_BB("[%s] SCAN MODE ON\n", __func__);
		}
	} else {
		if (scan_mode) {
			BBM_WRITE(0, 0x3004, 0x04);
			BBM_WRITE(0, 0x3006, 0x04);
			BBM_WRITE(0, 0x2020, 0x10);
			BBM_WRITE(0, 0x2021, 0x0e);
			BBM_WRITE(0, 0x2022, 0x4a);
			BBM_WRITE(0, 0x2082, 0x45);
			BBM_WRITE(0, 0x2083, 0x5f);
			BBM_WRITE(0, 0x2084, 0x37);
			BBM_WRITE(0, 0x2085, 0x30);
		
			scan_mode = 0;
			ISDBT_MSG_FC8150_BB("[%s] SCAN MODE OFF\n", __func__);
		}
	}

	//ISDBT_MSG_FC8150_BB("[%s] end \n", __func__);
}

/*====================================================================
FUNCTION       fc8150_bb_set_frequency
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8150_bb_set_frequency(int freq_idx)
{
    int ret = 0;
    u32 f_rf = 0;

    //ISDBT_MSG_FC8150_BB("[%s] start \n", __func__);

    f_rf = (freq_idx - 13) * 6000 + 473143;
    ISDBT_MSG_FC8150_BB("[%s] set FREQ = %d \n", __func__, f_rf);
    ret = BBM_TUNER_SET_FREQ(NULL, f_rf);
    if (ret != BBM_OK) {
        ISDBT_MSG_FC8150_BB("[%s] fail to BBM_TUNER_SET_FREQ (f_rf)%d (ret)%d\n", __func__, f_rf, ret);
        return -1;
    }

#if 1 // for test
    ret = BBM_SCAN_STATUS(NULL);
    if (ret)
        ISDBT_MSG_FC8150_BB("[%s] LOCK FAIL!\n", __func__);
    else
    	ISDBT_MSG_FC8150_BB("[%s] LOCK OK! \n", __func__);
#endif // 0

    BBM_WRITE(0, BBM_BUF_INT, 0x01);

	//ISDBT_MSG_FC8150_BB("[%s] end \n", __func__);
	return 0;
}

/*====================================================================
FUNCTION       fc8150_bb_start_ts
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8150_bb_start_ts(int enable)
{
    int ret = 0;

    ISDBT_MSG_FC8150_BB("[%s] enable [%d]\n", __func__, enable);

    return ret;
}

/*====================================================================
FUNCTION       fc8150_bb_get_status
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_bb_get_status(tIsdbtSigInfo *sig_info)
{
	struct dm_st {
		u8	start;
		s8	rssi;
		u8	wscn;
		u8	reserved;
		u16 main_rxd_rsps;
		u16 main_err_rsps;
		u32 main_err_bits;
		u32 dmp_rxd_bits;
		u32 dmp_err_bits;
		u16 inter_rxd_rsps;
		u16 inter_err_rsps;
		u32 inter_err_bits;
		u8	lna_code;
		u8	rfvga;
		u8	k;
		u8	csf_gain;
		u8	pga_gain;
		u8	extlna;
		u8	high_current_mode_gain;
		u8	extlna_gain;
	} dm;

    /** read info */
	BBM_WRITE(NULL, BBM_REQ_BER, 0x0e);
	BBM_BULK_READ(NULL, BBM_DM_DATA, (u8*) &dm + 1, sizeof(dm) - 1);

    /** calc BER */
	if (dm.dmp_rxd_bits)
		sig_info->ber = ((u32)dm.dmp_err_bits * 10000 / (u32)dm.dmp_rxd_bits);
	else
		sig_info->ber = 10000;

    /** calc PER */
	if (dm.inter_rxd_rsps)
		sig_info->per = ((u32)dm.inter_err_rsps * 10000 / (u32)dm.inter_rxd_rsps);
	else
		sig_info->per = 10000;

    /** read CN */
    sig_info->cninfo = dm.wscn;
}

/*====================================================================
FUNCTION       fc8150_bb_get_tuner_info
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_bb_get_tuner_info(tIsdbtTunerInfo* tuner_info)
{
    u8 mode;
	u8 buf_lock;
	s32 i32RSSI;
	u32 ui32BER;
	u32 ui32PER;
	s32 CN;
    static u32 prelvl = 0;
	u32 ui32Antlvl;
    u16 AGC;
    u8 GI;
    u8 TM;
	struct dm_st {
		u8	start;
		s8	rssi;
		u8	wscn;
		u8	reserved;
		u16 main_rxd_rsps;
		u16 main_err_rsps;
		u32 main_err_bits;
		u32 dmp_rxd_bits;
		u32 dmp_err_bits;
		u16 inter_rxd_rsps;
		u16 inter_err_rsps;
		u32 inter_err_bits;
		u8	lna_code;
		u8	rfvga;
		u8	k;
		u8	csf_gain;
		u8	pga_gain;
		u8	extlna;
		u8	high_current_mode_gain;
		u8	extlna_gain;
	} dm;

	BBM_READ(NULL, 0x5053, &buf_lock);
	ISDBT_MSG_FC8150_BB("[%s] Lock = %d\n", __func__, buf_lock);
	if (!buf_lock)
        ui32Antlvl = prelvl = 0;
    tuner_info->rssi = buf_lock;

	BBM_WRITE(NULL, BBM_REQ_BER, 0x0e);
	BBM_BULK_READ(NULL, BBM_DM_DATA, (u8*) &dm + 1, sizeof(dm) - 1);

	ISDBT_MSG_FC8150_BB("[%s] main_rxd_rsps: %d, dmp_rxd_bits: %d, inter_rxd_rsps: %d\n", __func__, dm.main_rxd_rsps, dm.dmp_rxd_bits, dm.inter_rxd_rsps);
		
	if (dm.inter_rxd_rsps)
		ui32PER = ((u32)dm.inter_err_rsps * 10000 / (u32)dm.inter_rxd_rsps);
	else
		ui32PER = 10000;

	if (dm.dmp_rxd_bits)
		ui32BER = ((u32)dm.dmp_err_bits * 10000 / (u32)dm.dmp_rxd_bits);
	else
		ui32BER = 10000;
	
	i32RSSI = dm.rssi;
	CN = dm.wscn;

    BBM_READ(NULL, 0x302a, &mode);
    GI = (mode & 0x70) >> 4;    //	1 : 1/32, 2 : 1/16, 3 : 1/8, 4 : 1/4
    TM = mode & 0x03;           //	1 : mode1, 2 : mode2, 3 : mode3

	BBM_WORD_READ(0, 0x100e, &AGC);

	switch (prelvl) {
    case 0:
		if (ui32BER < 650)
			ui32Antlvl = prelvl = 1;
		else
			ui32Antlvl = prelvl;
	    break;

	case 1:
		if ((ui32BER > 700) || ((ui32BER > 500) && (CN <= 3)))
			ui32Antlvl = prelvl = 0;
		else if ((ui32BER < 300) && (CN > 6))
			ui32Antlvl = prelvl = 2;
		else
			ui32Antlvl = prelvl;
		break;

	case 2:
		if ((ui32BER > 500) || ((ui32BER > 300) && (CN <= 5)))
			ui32Antlvl = prelvl = 1;
		else if ((ui32BER < 100) && (CN >= 9))
			ui32Antlvl = prelvl = 3;
		else
			ui32Antlvl = prelvl;
		break;

	case 3:
		if ((ui32BER > 200) || ((ui32BER > 100) && (CN <= 9)))
			ui32Antlvl = prelvl = 2;
		else if ((ui32BER < 50) && (CN >= 12))
			ui32Antlvl = prelvl = 4;
		else
			ui32Antlvl = prelvl;
		break;

	case 4:
		if ((ui32BER > 100) || (CN <= 14))
			ui32Antlvl = prelvl = 3;
		else
			ui32Antlvl = prelvl;
		break;

	default :
		ui32Antlvl = prelvl = 0;
		break;
	}

    ISDBT_MSG_FC8150_BB("[%s] (PER)%d (BER)%d (RSSI)%d (CN)%d (AGC)%d (Ant)%d\n", __func__, ui32PER, ui32BER, i32RSSI, CN, AGC, ui32Antlvl);
}

/*====================================================================
FUNCTION       fc8150_test
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_test(int index)
{
    //int i = 0, lock_ok_ch = 0;
    int ret = 0;

    ISDBT_MSG_FC8150_BB("[%s] start!!!\n", __func__);

    fc8150_bb_init();

// test scan all channel
#if 0
    // scan
    fc8150_bb_set_scanmode(1);
	for(i = 13; i <= 63; i++) {
		fc8150_bb_set_frequency(i);
		ISDBT_MSG_FC8150_BB("[%s] Channel number : %d \n", __func__, i);
		ret = BBM_SCAN_STATUS(NULL);
		if (ret) {
			ISDBT_MSG_FC8150_BB("[%s] LOCK FAIL!\n", __func__);
        } else {
            lock_ok_ch = i;
			ISDBT_MSG_FC8150_BB("[%s] LOCK OK! \n", __func__);
		}
	}
	if (i == 64) 
	    index = 27; // set freq_index forced
	else 
	    index = lock_ok_ch;
#else
    // scan
    fc8150_bb_set_scan_mode(1);
    fc8150_bb_set_frequency(index);
    ISDBT_MSG_FC8150_BB("[%s] Channel number : %d \n", __func__, index);
    ret = BBM_SCAN_STATUS(NULL);
    if (ret)
        ISDBT_MSG_FC8150_BB("[%s] LOCK FAIL!\n", __func__);
    else
    	ISDBT_MSG_FC8150_BB("[%s] LOCK OK! \n", __func__);
#endif // 0

    // set channel
    ISDBT_MSG_FC8150_BB("[%s] freq index[%d]\n", __func__, index);
    fc8150_bb_set_scan_mode(0);
    fc8150_bb_set_frequency(index);
    ret = BBM_SCAN_STATUS(NULL);
    if (ret) {
        ISDBT_MSG_FC8150_BB("[%s] LOCK FAIL! (ret)%d\n", __func__, ret);
        return;
    }

    fc8150_bb_start_ts(1);

    //ISDBT_MSG_FC8150_BB("[%s] end!!!\n", __func__);
}

/*====================================================================
FUNCTION       fc8150_bb_read_int
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8150_bb_read_int(void)
{
    //ISDBT_MSG_FC8150_BB("[%s] start!!!\n", __func__);

    BBM_ISR(NULL);

    //ISDBT_MSG_FC8150_BB("[%s] end\n", __func__);
}

/*====================================================================
FUNCTION       fc8150_bb_read_ts
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8150_bb_read_ts(byte* ts_buf)
{
    int ts_size = 0;
    
    ts_size = ts_type[gOutputBuffer].length;
    if (!ts_type[gOutputBuffer].address || !ts_size) {
        ISDBT_MSG_FC8150_BB("[%s] invalid buffer\n", __func__);
        return 0;
    }
    
    memcpy(&ts_buf[0], (void*)&gTSData[gOutputBuffer * FC8150_TS_BUFFER_SIZE], ts_size);

    ts_type[gOutputBuffer].address = 0;
    ts_type[gOutputBuffer].length = 0;

    gOutputBuffer = (gOutputBuffer + 1) % FC8150_TS_BUFFERS;

    return ts_size;
}
