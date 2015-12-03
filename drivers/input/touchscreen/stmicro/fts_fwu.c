
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>

#include "fts_ts.h"

#define USE_HEADER_FOR_FW

#define WRITE_CHUNK_SIZE 64
#define FTS_DEFAULT_UMS_FW "/sdcard/stm.fw"
#define FTS64FILE_SIGNATURE 0xaaaa5555
extern int pan_debug_state;

enum {
	BUILT_IN = 0,
	UMS,
};

#ifdef USE_HEADER_FOR_FW
#include "fts_fwu.h"

struct fw_header {
	unsigned char released_ver[8];
	unsigned short firmware_ver;
	unsigned short config_ver;
};
#endif

struct fts64_header {
	unsigned int signature;
	unsigned short fw_ver;
	unsigned char fw_id;
	unsigned char reserved1;
	unsigned char internal_ver[8];
	unsigned char released_ver[8];
	unsigned int reserved2;
	unsigned int checksum;
};



int fts_fw_wait_for_flash_ready(struct fts_ts_info *info)
{
	unsigned char regAdd;
	unsigned char buf[3];
	int retry = 0;

	regAdd = FTS_CMD_READ_FLASH_STAT;
	retry = 0;
	while (info->fts_read_reg
		(info, &regAdd, 1, (unsigned char *)buf, 1)) {
		if ((buf[0] & 0x01) == 0)
			break;

		if (retry++ > 300)
		{
			dbg_cr("%s: Time Over\n", __func__);
			return -1;
		}
		mdelay(10);
	}
	return 0;
}

int fts_fw_burn(struct fts_ts_info *info, unsigned char *fw_data)
{
	unsigned char regAdd[WRITE_CHUNK_SIZE + 3];
	int section;

	// Check busy Flash
	if (fts_fw_wait_for_flash_ready(info)<0)
		return -1;

	// FTS_CMD_UNLOCK_FLASH
	dbg_cr("%s: Unlock Flash\n", __func__);
	regAdd[0] = FTS_CMD_UNLOCK_FLASH;
	regAdd[1] = 0x74;
	regAdd[2] = 0x45;
	info->fts_write_reg(info, &regAdd[0], 3);
	msleep(500);

	// Copy to PRAM
	dbg_cr("%s: Copy to PRAM\n", __func__);
	regAdd[0] = FTS_CMD_WRITE_PRAM;
	for (section = 0; section < (64 * 1024 / WRITE_CHUNK_SIZE); section++) {
		regAdd[1] = ((section * WRITE_CHUNK_SIZE) >> 8) & 0xff;
		regAdd[2] = (section * WRITE_CHUNK_SIZE) & 0xff;
#ifdef USE_HEADER_FOR_FW
		memcpy(&regAdd[3],
			&fw_data[section * WRITE_CHUNK_SIZE +
				 sizeof(struct fw_header)],
				WRITE_CHUNK_SIZE);
#else
		memcpy(&regAdd[3],
			&fw_data[section * WRITE_CHUNK_SIZE +
				 sizeof(struct fts64_header)],
			WRITE_CHUNK_SIZE);
#endif
		info->fts_write_reg(info, &regAdd[0], WRITE_CHUNK_SIZE + 3);
	} 
	msleep(100);

	// Erase Program Flash
	dbg_cr("%s: Erase Program Flash\n", __func__);
	info->fts_command(info, FTS_CMD_ERASE_PROG_FLASH);
	msleep(100);

	// Check busy Flash
	if (fts_fw_wait_for_flash_ready(info)<0)
		return -1;

	// Burn Program Flash
	dbg_cr("%s: Burn Program Flash\n", __func__);
	info->fts_command(info, FTS_CMD_BURN_PROG_FLASH);
	msleep(100);

	// Check busy Flash
	if (fts_fw_wait_for_flash_ready(info)<0)
		return -1;

	// Reset FTS
	dbg_cr("%s: Reset FTS\n", __func__);
	info->fts_systemreset(info);
	return 0;
}

int GetSystemStatus(struct fts_ts_info *info, unsigned char *val1, unsigned char *val2)
{
	bool rc = -1;
	unsigned char regAdd1[4] = { 0xb2, 0x07, 0xfb, 0x04 };
	unsigned char regAdd2[4] = { 0xb2, 0x17, 0xfb, 0x04 };
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;

	info->fts_write_reg(info, &regAdd1[0], 4);
	info->fts_write_reg(info, &regAdd2[0], 4);

	memset(data, 0x0, FTS_EVENT_SIZE);
	regAdd1[0] = READ_ONE_EVENT;

	while (info->fts_read_reg(info, &regAdd1[0], 1, (unsigned char *)data,
				   FTS_EVENT_SIZE)) {
		if ((data[0] == 0x12) && (data[1] == regAdd1[1])
			&& (data[2] == regAdd1[2])) {
			rc = 0;
			*val1 = data[3];
			dbg_cr("%s: System Status 1 : 0x%02x\n", __func__, data[3]);
		}
		else if ((data[0] == 0x12) && (data[1] == regAdd2[1])
			&& (data[2] == regAdd2[2])) {
			rc = 0;
			*val2 = data[3];
			dbg_cr("%s: System Status 2 : 0x%02x\n", __func__, data[3]);
			break;
		}

		if (retry++ > 30) {
			rc = -1;
			dbg_cr("Time Over - GetSystemStatus\n");
			break;
		}
	}
	return rc;
}

int fts_fw_wait_for_event(struct fts_ts_info *info, unsigned char eid)
{
	int rc;
	unsigned char regAdd;
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;

	memset(data, 0x0, FTS_EVENT_SIZE);

	regAdd = READ_ONE_EVENT;
	rc = -1;
	while (info->fts_read_reg
	       (info, &regAdd, 1, (unsigned char *)data, FTS_EVENT_SIZE)) {
		if ((data[0] == EVENTID_STATUS_EVENT) &&
			(data[1] == 0x0B) &&
			(data[2] == eid)) {
			rc = 0;
			break;
		}

		if (retry++ > 300) {
			rc = -1;
			dbg_cr("%s: Time Over\n", __func__);
			break;
		}
		mdelay(10);
	}

	return rc;
}

//++ p11309 - 2013.12.22
#ifdef TSP_FACTORY_TEST
int ts_specin_test(struct fts_ts_info *info)
{
	const int RxLen = info->SenseChannelLength;
	const int TxLen = info->ForceChannelLength;
	const int Cx2MaxGap = 2;
	const int RawMaxVal = RAW_MAX;
	unsigned char cmd = READ_ONE_EVENT;
	unsigned char *pRead = NULL;
	unsigned char regAdd[4];
	unsigned short addr;
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;
	int r,t, gap, rtn;

	// Read CX2 Value
	pRead = kzalloc(RxLen*TxLen, GFP_KERNEL);
	if (pRead == NULL) {
		dbg_cr("FTS pRead kzalloc failed\n");
		return -1;
	}

	info->fts_command(info, FLUSHBUFFER);

	for (t=0; t<TxLen; t++) {
		for (r=0; r<(RxLen/4)+1; r++) {
			addr = 0x1000+(24*t)+r*3;
			regAdd[0] = 0xB2;
			regAdd[1] = (addr >> 8) & 0xff;
			regAdd[2] = addr & 0xff;
			regAdd[3] = 3;
			info->fts_write_reg(info, &regAdd[0], 4);

			retry = 0;
			while (info->fts_read_reg
					(info, &cmd, 1, (unsigned char *)data, FTS_EVENT_SIZE)) {

				if (data[0]==0x12) {
					if ((data[1]==regAdd[1]) && (data[2]==regAdd[2])) {
						pRead[(RxLen*t)+r*4]   = data[3] & 0x3f;
						if ((r*4+1)<RxLen)
							pRead[(RxLen*t)+r*4+1] = ((data[3]>>6)&0x03) + ((data[4]<<2) & 0x3C);
						if ((r*4+2)<RxLen)
							pRead[(RxLen*t)+r*4+2] = ((data[4]>>4)&0x0f) + ((data[5]<<4) & 0x30);
						if ((r*4+3)<RxLen)
							pRead[(RxLen*t)+r*4+3] = ((data[5]>>2)&0x3f);
						break;
					}
				}
				if (retry++ > 30) {
					dbg_cr("%s: Time over - wait for result of read event\n",
						__func__);
					break;
				}
				mdelay(10);
			}
		}
	}

	/*for (t=0; t<TxLen; t++) {
		printk("TX%d ", t);
		for (r=0; r<RxLen; r++) {
			printk("%02x ", pRead[(RxLen*t)+r]);

		}
		printk("\n");
	}*/

	rtn=0;
	for (t=0; t<TxLen; t++) {
		for (r=0; r<RxLen; r++) {
			if (r<RxLen-1) {
				gap = abs(pRead[(RxLen*t)+r+1] - pRead[(RxLen*t)+r]);
				if (gap>Cx2MaxGap) {
					dbg_cr("Cx2 fail - Node [Tx:%d, Rx:%d] "
						"vs [Tx:%d, Rx:%d]\n", t, r, t, r+1);
					rtn = -2;
				}
			}

			if (t<TxLen-1) {
				gap = abs(pRead[RxLen*(t+1)+r] - pRead[(RxLen*t)+r]);
				if (gap>Cx2MaxGap) {
					dbg_cr("Cx2 fail - Node [Tx:%d, Rx:%d] "
						"vs [Tx:%d, Rx:%d]\n", t, r, t+1, r);
					rtn = -2;
				}
			}
		}
	}

	kfree(pRead);

	if (rtn!=0)
		return -rtn;

	// Read Raw Data
	info->fts_command(info, SENSEON);

	pRead = kzalloc(RxLen*TxLen*2, GFP_KERNEL);
	if (pRead == NULL) {
		dbg_cr("%s: FTS pRead kzalloc failed\n", __func__);
		return -3;
	}

	regAdd[0] = 0xD0;
	regAdd[1] = 00;
	regAdd[2] = 02;
	if (!info->fts_read_reg(info, &regAdd[0], 3, (unsigned char *)&addr, 2)) {
		dbg_cr("%s: FTS Read failed\n", __func__);
		kfree(pRead);
		return -4;
	}

	//Skip Noise Channel
	addr+=RxLen*2;

	for (t=0; t<TxLen; t++) {
		regAdd[0] = 0xD0;
		regAdd[1] = (addr>>8) & 0xff;
		regAdd[2] = addr & 0xff;
		info->fts_read_reg(info, &regAdd[0], 3, (unsigned char *)&pRead[t*RxLen*2], RxLen*2);

		addr+=RxLen*2;
	}

	/*for (t=0; t<TxLen; t++) {
		printk("TX%d ", t);
		for (r=0; r<RxLen; r++) {
			printk("%04d ", *(unsigned short *)&pRead[t*RxLen*2+r*2]);
		}
		printk("\n");
	}*/

	for (t=0; t<TxLen*RxLen; t++) {
		if (*(unsigned short *)&pRead[t*2]>RawMaxVal) {
			rtn=-10;
			dbg_cr("Raw Data fail - Node [Tx:%d, Rx:%d] Value:%d\n", 
					t/RxLen, t%RxLen, *(unsigned short *)&pRead[t*2]);
			break;
		}
	}

	kfree(pRead);

	return rtn;
}
#endif
//-- p11309

void fts_fw_init(struct fts_ts_info *info)
{
	int retval;
	int retry = 1;
#ifdef FTS_PANTECH_PRODUCT_LINE
	unsigned char regAdd[4];
#endif /* FTS_PANTECH_PRODUCT_LINE */

	while (1) {
		info->fts_command(info, SLEEPOUT);
		msleep(200);
		info->fts_command(info, CX_TUNNING);
		msleep(300);
		fts_fw_wait_for_event(info, 0x03);

		info->fts_command(info, SELF_AUTO_TUNE);
		msleep(300);
		fts_fw_wait_for_event(info, 0x07);

//++ p11309 - 2013.12.22
#ifdef TSP_FACTORY_TEST
		retval = ts_specin_test(info);
		if (retval == 0)
			break;
#endif
//-- p11309

		// Reset FTS
		info->fts_systemreset(info);
		info->fts_wait_for_ready(info);
		msleep(200);

		if (retry++ > 2)
		{
			dbg_cr("%s: Fail auto tune\n", __func__);

			info->fts_command(info, SLEEPOUT);
			msleep(200);

			retval = -1;
			break;
		}
		msleep(300);
	}

//++ p11309 - 2013.12.22
#ifdef TSP_FACTORY_TEST
	if (retval==0) {
#ifdef FTS_PANTECH_PRODUCT_LINE
		regAdd[0]=0xB0;
		regAdd[1]=0x07;
		regAdd[2]=0xE2;
		regAdd[3]=0x08;
		info->fts_write_reg(info, &regAdd[0], 4);
#endif /* FTS_PANTECH_PRODUCT_LINE */

		info->fts_command(info, FTS_CMD_SAVE_FWCONFIG);
		msleep(200);
		info->fts_command(info, FTS_CMD_SAVE_CX_TUNING);
		msleep(200);
	}
#endif
//-- p11309

	info->fts_command(info, SENSEON);
}

const int fts_fw_updater(struct fts_ts_info *info, unsigned char *fw_data)
{
#ifdef USE_HEADER_FOR_FW
	const struct fw_header *header;
#else
	const struct fts64_header *header;
#endif

	int retval;
	int retry;
	unsigned short fw_main_version;

	if (!fw_data) {
		dbg_cr("%s: Firmware data is NULL\n", __func__);
		return -ENODEV;
	}

#ifdef USE_HEADER_FOR_FW
	header = (struct fw_header *)fw_data;
#else
	header = (struct fts64_header *)fw_data;
#endif
	fw_main_version = (header->released_ver[0] << 8) +
							(header->released_ver[1]);

	dbg_cr("Starting firmware update : 0x%04X\n", fw_main_version);

	retry = 0;
	while (1) {
		retval = fts_fw_burn(info, fw_data);
		if (retval >= 0) {
			info->fts_wait_for_ready(info);
			info->fts_get_version_info(info);
			if (fw_main_version == info->fw_main_version_of_ic) {
				dbg_cr("%s: Success Firmware update\n", __func__);
				fts_fw_init(info);
				retval = 0;
				break;
			}
		}

		if (retry++ > 3)
		{
			dbg_cr("%s: Fail Firmware update\n", __func__);
			retval = -1;
			break;
		}
	}
	return retval;
}
EXPORT_SYMBOL(fts_fw_updater);

#ifdef USE_HEADER_FOR_FW
int fts_fw_update_on_probe(struct fts_ts_info *info)
{
	int retval;
	const struct fw_header *header;
	unsigned char SYS_STAT[2];

	dbg_func_in();
	
	header = (struct fw_header *)pFirmwareData;

	info->fw_version_of_bin = header->firmware_ver;
	info->fw_main_version_of_bin = (header->released_ver[0] << 8) +
								(header->released_ver[1]);
	info->config_version_of_bin = header->config_ver;

	dbg_cr("[Firmware Version] IC: 0x%04X, BIN: 0x%04X\n",
		info->fw_version_of_ic, info->fw_version_of_bin);
	dbg_cr("[Config Version] IC: 0x%04X, BIN: 0x%04X\n",		
		info->config_version_of_ic, info->config_version_of_bin);
	dbg_cr("[Main Version] IC: 0x%04X, BIN: 0x%04X\n",		
		info->fw_main_version_of_ic, info->fw_main_version_of_bin);

	if ((info->fw_main_version_of_ic != info->fw_main_version_of_bin)
		|| ((info->config_version_of_ic != info->config_version_of_bin)))
	{	
  	retval = fts_fw_updater(info, pFirmwareData);
	}
	else {
		retval = -2;
		goto done;
	}

	if (retval < 0) {
		if (GetSystemStatus(info, &SYS_STAT[0], &SYS_STAT[1]) >= 0) {
			if (SYS_STAT[0] != SYS_STAT[1])
				fts_fw_init(info);
		}
	}

done:
	dbg_func_out();
	return retval;
}
#else
int fts_fw_update_on_probe(struct fts_ts_info *info)
{
	int retval;
	const struct firmware *fw_entry = NULL;
	unsigned char *fw_data = NULL;
	char fw_path[FTS_MAX_FW_PATH];
	const struct fts64_header *header;
	unsigned char SYS_STAT[2];

	snprintf(fw_path, FTS_MAX_FW_PATH, "%s", info->board->firmware_name);
	
	dbg_cr("%s: Load firmware : %s\n", __func__, fw_path);

	retval = request_firmware(&fw_entry, fw_path, info->dev);
	if (retval) {
		dbg_cr("%s: Firmware image %s not available\n", __func__, fw_path);
		goto done;
	}
	fw_data = (unsigned char *)fw_entry->data;
	header = (struct fts64_header *)fw_data;

	info->fw_version_of_bin = (fw_data[5] << 8)+fw_data[4];
	info->fw_main_version_of_bin = (header->released_ver[0] << 8) +
								(header->released_ver[1]);
	info->config_version_of_bin = (fw_data[0xf822] << 8)+fw_data[0xf821];

	dbg_cr("Bin Firmware Version : 0x%04X "
		   "Bin Config Version : 0x%04X "
		   "Bin Main Version : 0x%04X\n",
				info->fw_version_of_bin,
				info->config_version_of_bin,
				info->fw_main_version_of_bin);

	if ((info->fw_main_version_of_ic < info->fw_main_version_of_bin)
		|| ((info->config_version_of_ic < info->config_version_of_bin)))
		retval = fts_fw_updater(info, fw_data);
	else
		retval = -2;
done:
	if (fw_entry)
		release_firmware(fw_entry);
	if (retval < 0) {
		if (GetSystemStatus(info, &SYS_STAT[0], &SYS_STAT[1]) >= 0) {
			if (SYS_STAT[0] != SYS_STAT[1])
				fts_fw_init(info);
		}
	}
	return retval;
}
#endif

EXPORT_SYMBOL(fts_fw_update_on_probe);

#ifdef USE_HEADER_FOR_FW
static int fts_load_fw_from_kernel(struct fts_ts_info *info)
{
	int retval;

	// Disable Interrupt
	if (info->irq)
		disable_irq(info->irq);
	else
		hrtimer_cancel(&info->timer);

	// Reset FTS
	info->fts_systemreset(info);
	info->fts_wait_for_ready(info);

	retval = fts_fw_updater(info, pFirmwareData);
	if (retval)
		dbg_cr("%s: failed update firmware\n", __func__);

	// Enable Interrupt
	if (info->irq)
		enable_irq(info->irq);
	else
		hrtimer_start(&info->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	return retval;

}
#else
static int fts_load_fw_from_kernel(struct fts_ts_info *info,
				 const char *fw_path)
{
	int retval;
	const struct firmware *fw_entry = NULL;
	unsigned char *fw_data = NULL;

	if (!fw_path) {
		dbg_cr("%s: Firmware name is not defined\n",__func__);
		return -EINVAL;
	}

	dbg_cr("%s: Load firmware : %s\n", __func__, fw_path);

	retval = request_firmware(&fw_entry, fw_path, info->dev);

	if (retval) {
		dbg_cr("%s: Firmware image %s not available\n", __func__, fw_path);
		goto done;
	}

	// Disable Interrupt
	if (info->irq)
		disable_irq(info->irq);
	else
		hrtimer_cancel(&info->timer);
	fw_data = (unsigned char *)fw_entry->data;

	// Reset FTS
	info->fts_systemreset(info);
	info->fts_wait_for_ready(info);

	retval = fts_fw_updater(info, fw_data);
	if (retval)
		dbg_cr("%s: failed update firmware\n",__func__);

	// Enable Interrupt
	if (info->irq)
		enable_irq(info->irq);
	else
		hrtimer_start(&info->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
 done:
	if (fw_entry)
		release_firmware(fw_entry);

	return retval;
}
#endif

static int fts_load_fw_from_ums(struct fts_ts_info *info)
{
	struct file *fp;
	mm_segment_t old_fs;
	unsigned int fw_size, nread;
	int error = 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(FTS_DEFAULT_UMS_FW, O_RDONLY, S_IRUSR);
	if (IS_ERR(fp)) {
		dbg_cr("%s: failed to open %s.\n", __func__, FTS_DEFAULT_UMS_FW);
		error = -ENOENT;
		goto open_err;
	}

	fw_size = fp->f_path.dentry->d_inode->i_size;

	if (0 < fw_size) {
		unsigned char *fw_data;
		const struct fts64_header *header;
		fw_data = kzalloc(fw_size, GFP_KERNEL);
		nread = vfs_read(fp, (char __user *)fw_data,
				 fw_size, &fp->f_pos);

		dbg_cr("%s: start, file path %s, size %u Bytes\n",
			 __func__, FTS_DEFAULT_UMS_FW, fw_size);

		if (nread != fw_size) {
			dbg_cr("%s: failed to read firmware file, nread %u Bytes\n",
				__func__, nread);
			error = -EIO;
		} else {
			header = (struct fts64_header *)fw_data;
			if (header->signature == FTS64FILE_SIGNATURE) {
				/* UMS case */
				// Disable Interrupt
				if (info->irq)
					disable_irq(info->irq);
				else
					hrtimer_cancel(&info->timer);

				// Reset FTS
				info->fts_systemreset(info);
				info->fts_wait_for_ready(info);

				dbg_cr("[UMS] Firmware Version : 0x%04X "
					   "[UMS] Main Version : 0x%04X\n",
						(fw_data[5] << 8)+fw_data[4],
						(header->released_ver[0] << 8) + 
							(header->released_ver[1]));
				error = fts_fw_updater(info, fw_data);

				// Enable Interrupt
				if (info->irq)
					enable_irq(info->irq);
				else
					hrtimer_start(&info->timer,
						  ktime_set(1, 0),
						  HRTIMER_MODE_REL);
			}

			else
				 {
				error = -1;
				dbg_cr("%s: File type is not match with FTS64 file. [%8x]\n",
					 __func__, header->signature);
				}
		}

		if (error < 0)
			dbg_cr("%s: failed update firmware\n", __func__);

		kfree(fw_data);
	}

	filp_close(fp, NULL);

 open_err:
	set_fs(old_fs);
	return error;
}

int fts_fw_update_on_hidden_menu(struct fts_ts_info *info, int update_type)
{
	int retval = 0;

	/* Factory cmd for firmware update
	 * argument represent what is source of firmware like below.
	 *
	 * 0 : [BUILT_IN] Getting firmware which is for user.
	 * 1 : [UMS] Getting firmware from sd card.
	 */
	switch (update_type) {
	case BUILT_IN:
#ifdef USE_HEADER_FOR_FW
		retval = fts_load_fw_from_kernel(info);
#else
		retval = fts_load_fw_from_kernel(info, info->board->firmware_name);
#endif
		break;

	case UMS:
		retval = fts_load_fw_from_ums(info);
		break;

	default:
		dbg_cr("%s: Not support command[%d]\n",	__func__, update_type);
		break;
	}

	return retval;
}
EXPORT_SYMBOL(fts_fw_update_on_hidden_menu);

