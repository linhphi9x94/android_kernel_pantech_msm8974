/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include "msm_sd.h"
#include "msm_cci.h"
#include "msm_eeprom.h"

#undef CDBG
#ifdef MSM_EEPROM_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) pr_debug(fmt, ##args)
#endif

#ifdef CONFIG_PANTECH_CAMERA_EEPROM_CHECKSUM
uint16_t crc16table[] = 
{0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040};
#endif

DEFINE_MSM_MUTEX(msm_eeprom_mutex);

int32_t msm_eeprom_config(struct msm_eeprom_ctrl_t *e_ctrl,
	void __user *argp)
{
	struct msm_eeprom_cfg_data *cdata =
		(struct msm_eeprom_cfg_data *)argp;
	int32_t rc = 0;

	CDBG("%s E\n", __func__);
	switch (cdata->cfgtype) {
	case CFG_EEPROM_GET_INFO:
	CDBG("%s E CFG_EEPROM_GET_INFO\n", __func__);
		cdata->is_supported = e_ctrl->is_supported;
		memcpy(cdata->cfg.eeprom_name,
			e_ctrl->eboard_info->eeprom_name,
			sizeof(cdata->cfg.eeprom_name));
		break;
	case CFG_EEPROM_GET_CAL_DATA:
		CDBG("%s E CFG_EEPROM_GET_CAL_DATA\n", __func__);
		cdata->cfg.get_data.num_bytes =
			e_ctrl->num_bytes;
		break;
	case CFG_EEPROM_READ_CAL_DATA:
		if (cdata->cfg.read_data.num_bytes <= e_ctrl->num_bytes) {
		CDBG("%s E CFG_EEPROM_READ_CAL_DATA\n", __func__);
		rc = copy_to_user(cdata->cfg.read_data.dbuffer,
			e_ctrl->memory_data,
			cdata->cfg.read_data.num_bytes);
		}
		break;
	default:
		break;
	}

	CDBG("%s X\n", __func__);
	return rc;
}
static int32_t msm_eeprom_get_subdev_id(
	struct msm_eeprom_ctrl_t *e_ctrl, void *arg)
{
	uint32_t *subdev_id = (uint32_t *)arg;
	CDBG("%s E\n", __func__);
	if (!subdev_id) {
		pr_err("%s failed\n", __func__);
		return -EINVAL;
	}
	*subdev_id = e_ctrl->subdev_id;
	CDBG("subdev_id %d\n", *subdev_id);
	CDBG("%s X\n", __func__);
	return 0;
}

static long msm_eeprom_subdev_ioctl(struct v4l2_subdev *sd,
		unsigned int cmd, void *arg)
{
	struct msm_eeprom_ctrl_t *e_ctrl = v4l2_get_subdevdata(sd);
	void __user *argp = (void __user *)arg;
	CDBG("%s E\n", __func__);
	CDBG("%s:%d a_ctrl %p argp %p\n", __func__, __LINE__, e_ctrl, argp);
	switch (cmd) {
	case VIDIOC_MSM_SENSOR_GET_SUBDEV_ID:
		return msm_eeprom_get_subdev_id(e_ctrl, argp);
	case VIDIOC_MSM_EEPROM_CFG:
		return msm_eeprom_config(e_ctrl, argp);
	default:
		return -ENOIOCTLCMD;
	}

	CDBG("%s X\n", __func__);
}

static struct msm_camera_i2c_fn_t msm_eeprom_cci_func_tbl = {
	.i2c_read = msm_camera_cci_i2c_read,
	.i2c_read_seq = msm_camera_cci_i2c_read_seq,
	.i2c_write = msm_camera_cci_i2c_write,
	.i2c_write_seq = msm_camera_cci_i2c_write_seq,
	.i2c_write_table = msm_camera_cci_i2c_write_table,
	.i2c_write_seq_table = msm_camera_cci_i2c_write_seq_table,
	.i2c_write_table_w_microdelay =
	msm_camera_cci_i2c_write_table_w_microdelay,
	.i2c_util = msm_sensor_cci_i2c_util,
	.i2c_poll = msm_camera_cci_i2c_poll,
};

static struct msm_camera_i2c_fn_t msm_eeprom_qup_func_tbl = {
	.i2c_read = msm_camera_qup_i2c_read,
	.i2c_read_seq = msm_camera_qup_i2c_read_seq,
	.i2c_write = msm_camera_qup_i2c_write,
	.i2c_write_table = msm_camera_qup_i2c_write_table,
	.i2c_write_seq_table = msm_camera_qup_i2c_write_seq_table,
	.i2c_write_table_w_microdelay =
	msm_camera_qup_i2c_write_table_w_microdelay,
};

static struct msm_camera_i2c_fn_t msm_eeprom_spi_func_tbl = {
	.i2c_read = msm_camera_spi_read,
	.i2c_read_seq = msm_camera_spi_read_seq,
};

static int msm_eeprom_open(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh) {
	int rc = 0;
	struct msm_eeprom_ctrl_t *e_ctrl =  v4l2_get_subdevdata(sd);
	CDBG("%s E\n", __func__);
	if (!e_ctrl) {
		pr_err("%s failed e_ctrl is NULL\n", __func__);
		return -EINVAL;
	}
	CDBG("%s X\n", __func__);
	return rc;
}

static int msm_eeprom_close(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh) {
	int rc = 0;
	struct msm_eeprom_ctrl_t *e_ctrl =  v4l2_get_subdevdata(sd);
	CDBG("%s E\n", __func__);
	if (!e_ctrl) {
		pr_err("%s failed e_ctrl is NULL\n", __func__);
		return -EINVAL;
	}
	CDBG("%s X\n", __func__);
	return rc;
}

static const struct v4l2_subdev_internal_ops msm_eeprom_internal_ops = {
	.open = msm_eeprom_open,
	.close = msm_eeprom_close,
};
#ifdef CONFIG_PANTECH_CAMERA_IMX214
int32_t rumba_command_resigster_read(struct msm_eeprom_ctrl_t *e_ctrl, uint8_t * readdata, uint32_t readsize, uint32_t addr_offset)
{
	int rc = 0;
	uint8_t SendData[2];
	uint16_t RcvData[1];
	uint8_t* ReadData = NULL;

	SendData[0] = readsize; /* Data Read Size 256byte set */
       CDBG("%s: Data Read Size = 0x%x", __func__, SendData[0]);

	e_ctrl->i2c_client.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
		&(e_ctrl->i2c_client), 0x0F,
		SendData[0], MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s:%d rumba comand write failed\n", __func__, __LINE__);
		return rc;
	}

       CDBG("%s: addr_offset = 0x%x", __func__, addr_offset );
	if(addr_offset < 0x7400 || addr_offset > 0x7BFF){
		pr_err("%s:%d addr_offset out of range\n", __func__, __LINE__);
		return -1;
	}

	addr_offset = addr_offset - 0x7400;

       SendData[0] = (addr_offset & 0x00FF); 
       SendData[1] = (addr_offset & 0xFF00) >> 8; /* Data Read Start Address offset 0 set (0x7400-0x74FF) */

       CDBG("%s: SendData[0] = 0x%x", __func__, SendData[0] );
       CDBG("%s: SendData[1] = 0x%x", __func__, SendData[1] );

	e_ctrl->i2c_client.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_seq(
		&(e_ctrl->i2c_client), 0x10,
		&SendData[0], 2);
	if (rc < 0) {
		pr_err("%s:%d rumba comand write failed\n", __func__, __LINE__);
		return rc;
	}

       SendData[0] = 0x04; /* READ Command Set */

	e_ctrl->i2c_client.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
		&(e_ctrl->i2c_client), 0x0E,
		SendData[0], MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s:%d rumba comand write failed\n", __func__, __LINE__);
		return rc;
	}
       CDBG("%s: SendData[0] = 0x%x", __func__, SendData[0]);

	e_ctrl->i2c_client.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
		&(e_ctrl->i2c_client), 0x0E,
		&RcvData[0], MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s:%d rumba comand read failed\n", __func__, __LINE__);
		return rc;
	}

       CDBG("%s: RcvData[0] = 0x%x", __func__, RcvData[0]);
	if (RcvData[0] == 0x14){
		ReadData = readdata;
       	e_ctrl->i2c_client.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
       	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read_seq(
       		&(e_ctrl->i2c_client), 0x0100,
       		ReadData, readsize);
       	if (rc < 0) {
       		pr_err("%s:%d rumba comand read failed\n", __func__, __LINE__);
       		return rc;
       	}
 	}
 	return rc;

}
#endif

#ifdef CONFIG_PANTECH_CAMERA_EEPROM_CHECKSUM
uint16_t get_nvm_checksum(const uint8_t *a_data_ptr, signed long a_size)
{
       uint16_t crc = 0;
       int i;
       
       for (i = 0; i < a_size; i++)
       {
            uint8_t index = crc ^ a_data_ptr[i];
            crc = (crc >> 8) ^ crc16table[index];
       }
       
       return crc;
}

int32_t read_eeprom_memory_checksum(struct msm_eeprom_ctrl_t *e_ctrl)
{
       int rc = 0;
       int i, j;
#ifdef CONFIG_PANTECH_CAMERA_IMX214
	int loop, count = 0;
#endif
       uint8_t *memptr = NULL;
       uint32_t checksum_addr = 0;
       uint32_t checksum_read_data = 0;
       uint32_t checksum_data_byte = 0;
       uint32_t num_bytes = 0;
       uint32_t num_block = 0;
       uint32_t checksum = 0;
       uint16_t valid_size = 0;
       uint16_t data_ptr_index = 0;
#ifdef CONFIG_PANTECH_CAMERA_READ_EEPROM
       uint16_t main_offset = 0;
#endif

       struct msm_eeprom_board_info *eb_info = NULL;
       struct eeprom_memory_map_t *emap = NULL;
       
       if (!e_ctrl) {
              pr_err("%s e_ctrl is NULL", __func__);
              rc = -1;
              return rc;
       }
       eb_info = e_ctrl->eboard_info;
       emap = eb_info->eeprom_map;
       num_bytes = e_ctrl->num_bytes;
#if defined(CONFIG_PANTECH_CAMERA_IMX135)
       num_block = eb_info->num_blocks -1; //just check awb,lsc within second block reading time
#elif defined(CONFIG_PANTECH_CAMERA_IMX214)
       num_block = eb_info->num_blocks - 2; //just check awb,lsc within first block reading time
#endif
       checksum_data_byte = EEPROM_READ_CHECKSUM_BYTE;
       memptr = kzalloc(checksum_data_byte, GFP_KERNEL);
#ifdef CONFIG_PANTECH_CAMERA_READ_EEPROM
       main_offset = eb_info->i2c_slaveaddr >> 1;
#endif
	   	
       for (i = 0; i < num_block; i++) { // exeception of af checksum 
              if (i == 0) {
                     checksum_addr = EEPROM_AWB_CHECKSUM_ADDR; 
              } else if (i == 1) {
                     checksum_addr = EEPROM_LSC_CHECKSUM_ADDR; 
              } else {
                     checksum_addr = EEPROM_AF_CHECKSUM_ADDR; 
              }
#ifdef CONFIG_PANTECH_CAMERA_READ_EEPROM
              if (e_ctrl->is_increase_slave_address > 0){
                     e_ctrl->i2c_client.cci_client->sid = main_offset + checksum_addr/e_ctrl->set_block_bytes;
              }
#endif
              CDBG("%s: e_ctrl->i2c_client->cci_client->sid = 0x%x", __func__, e_ctrl->i2c_client.cci_client->sid);
#if defined(CONFIG_PANTECH_CAMERA_IMX135)
              e_ctrl->i2c_client.addr_type = emap[i].mem.addr_t;
              rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read_seq(
                     &(e_ctrl->i2c_client), checksum_addr,
                     memptr, checksum_data_byte);
              if (rc < 0) {
                     pr_err("%s: read failed\n", __func__);
                     goto out;
              }
              
              for (j=0; j < checksum_data_byte; j++){
                     checksum_read_data |= memptr[j] << (8*(checksum_data_byte -j-1)) ;
                     CDBG("%s: memptr[%d] = 0x%x", __func__, j, memptr[j]);
              }
              CDBG("%s: checksum_read_data  = %d", __func__, checksum_read_data );
 			  
              valid_size = emap[i].mem.valid_size;
              CDBG("%s: valid_size = %d, data_ptr_index = %d ", __func__, valid_size, data_ptr_index);
              checksum = get_nvm_checksum(&e_ctrl->memory_data[data_ptr_index], valid_size);
              data_ptr_index += valid_size;
              
              CDBG("%s: checksum = %d ", __func__, checksum);
              if (checksum_read_data == checksum){
                     if (i == 0)
                            pr_err("%s: AWB(0x%x) checksum success\n", __func__, checksum_addr);
                     else
                            pr_err("%s: LSC(0x%x) checksum success\n", __func__, checksum_addr);
              } else {
                     if (i == 0)              
                            pr_err("%s:  AWB(0x%x) checksum failed\n", __func__, checksum_addr);
                     else 
                            pr_err("%s: LSC(0x%x) checksum failed\n", __func__, checksum_addr);
                     goto out;
              }
              checksum = 0;
              checksum_read_data = 0;
			  
#elif defined(CONFIG_PANTECH_CAMERA_IMX214)
              // rumba-sa has to read as 4byte block size;
              rc = rumba_command_resigster_read(e_ctrl, memptr, checksum_data_byte, checksum_addr);
              if (rc < 0) {
                     pr_err("%s: read failed\n", __func__);
                     goto out;
              }
              
              for (loop=0; loop < 2; loop++){
                     for (j=0; j < 2; j++){
                            checksum_read_data |= memptr[count] << (8*(1-j)) ;
                            CDBG("%s: memptr[%d] = 0x%x", __func__, count, memptr[count]);
                            count++;
                     }
                     CDBG("%s: checksum_read_data  = %d", __func__, checksum_read_data);
                 
                     if (loop == 0){
                            valid_size = EEPROM_AWB_BLOCK_SIZE;
                     } else {
                            valid_size = emap[i].mem.valid_size - EEPROM_AWB_BLOCK_SIZE;
                     }
                     CDBG("%s: valid_size = %d, data_ptr_index = %d ", __func__, valid_size, data_ptr_index);
                     checksum = get_nvm_checksum(&e_ctrl->memory_data[data_ptr_index], valid_size);
                     data_ptr_index += valid_size;
                     
                     CDBG("%s: checksum = %d ", __func__, checksum);
                     if (checksum_read_data == checksum){
                            if (loop == 0)
                                   pr_err("%s: AWB(0x%x) checksum success\n", __func__, checksum_addr);
                            else
                                   pr_err("%s: LSC(0x%x) checksum success\n", __func__, EEPROM_LSC_CHECKSUM_ADDR);
                     } else {
                            if (loop == 0)                     
                                   pr_err("%s: AWB(0x%x) checksum failed\n", __func__, checksum_addr);
                            else
                                   pr_err("%s: LSC(0x%x) checksum failed\n", __func__, EEPROM_LSC_CHECKSUM_ADDR);
                            goto out;
                     }
                     checksum = 0;		 
                     checksum_read_data = 0;		 
              }
#endif
       }
       
       kfree(memptr);
       return rc;
out:
	kfree(memptr);
	return rc;
}
#endif

int32_t read_eeprom_memory(struct msm_eeprom_ctrl_t *e_ctrl)
{
	int rc = 0;
	int j;
	uint8_t *memptr = NULL;

#ifdef CONFIG_PANTECH_CAMERA_READ_EEPROM
       int loop = 0;
       uint16_t main_offset = 0;
       uint32_t set_block_bytes = 0;
       uint16_t valid_addr = 0;
       uint16_t valid_size = 0;
       uint16_t init_addr = 0;
       uint32_t block_bytes = 0;
       uint32_t read_bytes = 0;
       uint32_t remain_bytes = 0;
#endif

	struct msm_eeprom_board_info *eb_info = NULL;
	struct eeprom_memory_map_t *emap = NULL;
	if (!e_ctrl) {
		pr_err("%s e_ctrl is NULL", __func__);
		rc = -1;
		return rc;
	}
	memptr = e_ctrl->memory_data;
	eb_info = e_ctrl->eboard_info;
	emap = eb_info->eeprom_map;

	for (j = 0; j < eb_info->num_blocks; j++) {
		if (emap[j].page.valid_size) {
			e_ctrl->i2c_client.addr_type = emap[j].page.addr_t;
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
				&(e_ctrl->i2c_client), emap[j].page.addr,
				emap[j].page.data, emap[j].page.data_t);
				msleep(emap[j].page.delay);
			if (rc < 0) {
				pr_err("%s: page write failed\n", __func__);
				return rc;
			}
		}
#ifndef CONFIG_PANTECH_CAMERA		
		if (emap[j].pageen.valid_size) {
			e_ctrl->i2c_client.addr_type = emap[j].pageen.addr_t;
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
				&(e_ctrl->i2c_client), emap[j].pageen.addr,
				emap[j].pageen.data, emap[j].pageen.data_t);
				msleep(emap[j].pageen.delay);
			if (rc < 0) {
				pr_err("%s: page enable failed\n", __func__);
				return rc;
			}
		}
#endif		
		if (emap[j].poll.valid_size) {
			e_ctrl->i2c_client.addr_type = emap[j].poll.addr_t;
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_poll(
				&(e_ctrl->i2c_client), emap[j].poll.addr,
				emap[j].poll.data, emap[j].poll.data_t);
				msleep(emap[j].poll.delay);
			if (rc < 0) {
				pr_err("%s: poll failed\n", __func__);
				return rc;
			}
		}
#ifdef CONFIG_PANTECH_CAMERA_READ_EEPROM
              if (emap[j].mem.valid_size) {
                     main_offset = eb_info->i2c_slaveaddr >> 1;
                     set_block_bytes = e_ctrl->set_block_bytes;
                     valid_addr = emap[j].mem.valid_size + emap[j].mem.addr;
#if defined(CONFIG_PANTECH_CAMERA_IMX135)
                     valid_size = valid_addr;
#elif defined(CONFIG_PANTECH_CAMERA_IMX214)
                     valid_size = emap[j].mem.valid_size;
#endif 
                     init_addr = emap[0].mem.addr;

                     CDBG("%s: j = %d, main_offset = 0x%x, mem.addr = 0x%x, mem.addr_t = %d", __func__, j, main_offset, emap[j].mem.addr, emap[j].mem.addr_t);
                     CDBG("%s: emap[%d].mem.valid_size = %d, set_block_bytes = %d", __func__, j, emap[j].mem.valid_size, set_block_bytes);
                     if (emap[j].mem.valid_size > CCI_READ_MAX && set_block_bytes > 0) {
                            for (loop=0; valid_addr-emap[j].mem.addr > 0;loop++) {
                                   if (e_ctrl->is_increase_slave_address){
                                       e_ctrl->i2c_client.cci_client->sid = main_offset + emap[j].mem.addr/set_block_bytes;
                                   }
                                   CDBG("%s: loop = %d, e_ctrl->i2c_client->cci_client->sid = 0x%x", __func__, loop, e_ctrl->i2c_client.cci_client->sid);
                                   
                                   if (valid_size-(emap[j].mem.addr-init_addr) > set_block_bytes)
                                          block_bytes = set_block_bytes - ((emap[j].mem.addr-init_addr)%set_block_bytes);
                                   else 
                                          block_bytes = valid_size -(emap[j].mem.addr-init_addr) ;
                                   
                                   CDBG("%s: block_bytes = %d", __func__, block_bytes);
                                   if (block_bytes > CCI_READ_MAX)
                                          read_bytes = block_bytes - (block_bytes%CCI_READ_MAX);
                                   else
                                          read_bytes = block_bytes;

                                   if (read_bytes > 0){
                                        CDBG("%s: read_bytes = %d", __func__, read_bytes);
#if defined(CONFIG_PANTECH_CAMERA_IMX135)
                                        e_ctrl->i2c_client.addr_type = emap[j].mem.addr_t;
                                        rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read_seq(
                                               &(e_ctrl->i2c_client), emap[j].mem.addr,
                                               memptr, read_bytes);
#elif defined(CONFIG_PANTECH_CAMERA_IMX214)
                                        rc = rumba_command_resigster_read(e_ctrl, memptr, read_bytes, emap[j].mem.addr);
#endif
                                        if (rc < 0) {
                                               pr_err("%s: read failed\n", __func__);
                                               return rc;
                                        }
                                        emap[j].mem.addr += read_bytes;
                                        memptr += read_bytes;
                                   }

                                   if (block_bytes > CCI_READ_MAX)
                                          remain_bytes = block_bytes%CCI_READ_MAX;
                                   else
                                          remain_bytes = 0;

                                   if (remain_bytes > 0){
                                        CDBG("%s: remain_bytes = %d", __func__, remain_bytes);
#if defined(CONFIG_PANTECH_CAMERA_IMX135)
                                        e_ctrl->i2c_client.addr_type = emap[j].mem.addr_t;
                                        rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read_seq(
                                               &(e_ctrl->i2c_client), emap[j].mem.addr,
                                               memptr, remain_bytes);
#elif defined(CONFIG_PANTECH_CAMERA_IMX214)
                                        rc = rumba_command_resigster_read(e_ctrl, memptr, remain_bytes, emap[j].mem.addr);
#endif
                                        if (rc < 0) {
                                               pr_err("%s: read failed\n", __func__);
                                               return rc;
                                        }
                                        emap[j].mem.addr += remain_bytes;
                                        memptr += remain_bytes;
                                  }
                            }
                     } else {
                            if (e_ctrl->is_increase_slave_address){
                                   e_ctrl->i2c_client.cci_client->sid = main_offset + emap[j].mem.addr/set_block_bytes;
                            }
							
                            CDBG("%s: loop = %d, e_ctrl->i2c_client->cci_client->sid = 0x%x", __func__, loop, e_ctrl->i2c_client.cci_client->sid);
#if defined(CONFIG_PANTECH_CAMERA_IMX135)
                            e_ctrl->i2c_client.addr_type = emap[j].mem.addr_t;
                            rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read_seq(
                                   &(e_ctrl->i2c_client), emap[j].mem.addr,
                                   memptr, emap[j].mem.valid_size);
#elif defined(CONFIG_PANTECH_CAMERA_IMX214)
                            rc = rumba_command_resigster_read(e_ctrl, memptr, emap[j].mem.valid_size, emap[j].mem.addr);
#endif
                            if (rc < 0) {
                                   pr_err("%s: read failed\n", __func__);
                                   return rc;
                            }
                            memptr += emap[j].mem.valid_size;
                     }
              }
 #else
		if (emap[j].mem.valid_size) {
			e_ctrl->i2c_client.addr_type = emap[j].mem.addr_t;
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read_seq(
				&(e_ctrl->i2c_client), emap[j].mem.addr,
				memptr, emap[j].mem.valid_size);
			if (rc < 0) {
				pr_err("%s: read failed\n", __func__);
				return rc;
			}
			memptr += emap[j].mem.valid_size;
		}
#ifndef CONFIG_PANTECH_CAMERA
		if (emap[j].pageen.valid_size) {
			e_ctrl->i2c_client.addr_type = emap[j].pageen.addr_t;
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
				&(e_ctrl->i2c_client), emap[j].pageen.addr,
				0, emap[j].pageen.data_t);
			if (rc < 0) {
				pr_err("%s: page disable failed\n", __func__);
				return rc;
			}
		}
#endif	
#endif	
	}
	return rc;
}

static int msm_eeprom_get_dt_data(struct msm_eeprom_ctrl_t *e_ctrl)
{
	int rc = 0, i = 0;
	struct msm_eeprom_board_info *eb_info;
	struct msm_camera_power_ctrl_t *power_info =
		&e_ctrl->eboard_info->power_info;
	struct device_node *of_node = NULL;
	struct msm_camera_gpio_conf *gconf = NULL;
	uint16_t gpio_array_size = 0;
	uint16_t *gpio_array = NULL;

	eb_info = e_ctrl->eboard_info;
	if (e_ctrl->eeprom_device_type == MSM_CAMERA_SPI_DEVICE)
		of_node = e_ctrl->i2c_client.
			spi_client->spi_master->dev.of_node;
	else if (e_ctrl->eeprom_device_type == MSM_CAMERA_PLATFORM_DEVICE)
		of_node = e_ctrl->pdev->dev.of_node;
	else if (e_ctrl->eeprom_device_type == MSM_CAMERA_I2C_DEVICE)
		of_node = e_ctrl->i2c_client.client->dev.of_node;

	rc = msm_camera_get_dt_vreg_data(of_node, &power_info->cam_vreg,
					     &power_info->num_vreg);
	if (rc < 0)
		return rc;

	rc = msm_camera_get_dt_power_setting_data(of_node,
		power_info->cam_vreg, power_info->num_vreg,
		&power_info->power_setting, &power_info->power_setting_size);
	if (rc < 0)
		goto error1;

	power_info->gpio_conf = kzalloc(sizeof(struct msm_camera_gpio_conf),
					GFP_KERNEL);
	if (!power_info->gpio_conf) {
		rc = -ENOMEM;
		goto error2;
	}
	gconf = power_info->gpio_conf;
	gpio_array_size = of_gpio_count(of_node);
	CDBG("%s gpio count %d\n", __func__, gpio_array_size);

	if (gpio_array_size) {
		gpio_array = kzalloc(sizeof(uint16_t) * gpio_array_size,
			GFP_KERNEL);
		if (!gpio_array) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto error3;
		}
		for (i = 0; i < gpio_array_size; i++) {
			gpio_array[i] = of_get_gpio(of_node, i);
			CDBG("%s gpio_array[%d] = %d\n", __func__, i,
				gpio_array[i]);
		}

		rc = msm_camera_get_dt_gpio_req_tbl(of_node, gconf,
			gpio_array, gpio_array_size);
		if (rc < 0) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto error4;
		}

		rc = msm_camera_init_gpio_pin_tbl(of_node, gconf,
			gpio_array, gpio_array_size);
		if (rc < 0) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto error4;
		}
		kfree(gpio_array);
	}

	return rc;
error4:
	kfree(gpio_array);
error3:
	kfree(power_info->gpio_conf);
error2:
	kfree(power_info->cam_vreg);
error1:
	kfree(power_info->power_setting);
	return rc;
}

static int msm_eeprom_alloc_memory_map(struct msm_eeprom_ctrl_t *e_ctrl,
				       struct device_node *of)
{
	int i, rc = 0;
	char property[14];
	uint32_t count = 6;
	struct msm_eeprom_board_info *eb = e_ctrl->eboard_info;

	rc = of_property_read_u32(of, "qcom,num-blocks", &eb->num_blocks);
	CDBG("%s: qcom,num_blocks %d\n", __func__, eb->num_blocks);
	if (rc < 0) {
		pr_err("%s failed rc %d\n", __func__, rc);
		return rc;
	}

	eb->eeprom_map = kzalloc((sizeof(struct eeprom_memory_map_t)
				 * eb->num_blocks), GFP_KERNEL);

	if (!eb->eeprom_map) {
		pr_err("%s failed line %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	for (i = 0; i < eb->num_blocks; i++) {
		snprintf(property, 12, "qcom,page%d", i);
		rc = of_property_read_u32_array(of, property,
			(uint32_t *) &eb->eeprom_map[i].page, count);
		if (rc < 0) {
			pr_err("%s: failed %d\n", __func__, __LINE__);
			goto out;
		}
#ifndef CONFIG_PANTECH_CAMERA
		snprintf(property, 14, "qcom,pageen%d", i);
		rc = of_property_read_u32_array(of, property,
			(uint32_t *) &eb->eeprom_map[i].pageen, count);
		if (rc < 0)
			pr_err("%s: pageen not needed\n", __func__);
#endif
		snprintf(property, 12, "qcom,poll%d", i);
		rc = of_property_read_u32_array(of, property,
			(uint32_t *) &eb->eeprom_map[i].poll, count);
		if (rc < 0) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto out;
		}

		snprintf(property, 12, "qcom,mem%d", i);
		rc = of_property_read_u32_array(of, property,
			(uint32_t *) &eb->eeprom_map[i].mem, count);
		if (rc < 0) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto out;
		}
		e_ctrl->num_bytes += eb->eeprom_map[i].mem.valid_size;
	}

	CDBG("%s num_bytes %d\n", __func__, e_ctrl->num_bytes);

	e_ctrl->memory_data = kzalloc(e_ctrl->num_bytes, GFP_KERNEL);
	if (!e_ctrl->memory_data) {
		pr_err("%s failed line %d\n", __func__, __LINE__);
		rc = -ENOMEM;
		goto out;
	}
	return rc;

out:
	kfree(eb->eeprom_map);
	return rc;
}

static struct msm_cam_clk_info cam_8960_clk_info[] = {
	[SENSOR_CAM_MCLK] = {"cam_clk", 24000000},
};

static struct msm_cam_clk_info cam_8974_clk_info[] = {
	[SENSOR_CAM_MCLK] = {"cam_src_clk", 19200000},
	[SENSOR_CAM_CLK] = {"cam_clk", 0},
};

static struct v4l2_subdev_core_ops msm_eeprom_subdev_core_ops = {
	.ioctl = msm_eeprom_subdev_ioctl,
};

static struct v4l2_subdev_ops msm_eeprom_subdev_ops = {
	.core = &msm_eeprom_subdev_core_ops,
};

int32_t msm_eeprom_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id) {
	int rc = 0;
	int32_t j = 0;
	uint32_t temp = 0;
	struct msm_eeprom_ctrl_t *e_ctrl = NULL;
	struct msm_camera_power_ctrl_t *power_info = NULL;
	struct device_node *of_node = client->dev.of_node;
	CDBG("%s E\n", __func__);


	if (!of_node) {
		pr_err("%s of_node NULL\n", __func__);
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s i2c_check_functionality failed\n", __func__);
		goto probe_failure;
	}

	e_ctrl = kzalloc(sizeof(struct msm_eeprom_ctrl_t), GFP_KERNEL);
	if (!e_ctrl) {
		pr_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		return -ENOMEM;
	}
	e_ctrl->eeprom_v4l2_subdev_ops = &msm_eeprom_subdev_ops;
	e_ctrl->eeprom_mutex = &msm_eeprom_mutex;
	CDBG("%s client = %x\n", __func__, (unsigned int)client);
	e_ctrl->eboard_info = kzalloc(sizeof(
		struct msm_eeprom_board_info), GFP_KERNEL);
	if (!e_ctrl->eboard_info) {
		pr_err("%s:%d board info NULL\n", __func__, __LINE__);
		return -EINVAL;
	}

	rc = of_property_read_u32(of_node, "qcom,slave-addr", &temp);
	if (rc < 0) {
		pr_err("%s failed rc %d\n", __func__, rc);
		return rc;
	}

	power_info = &e_ctrl->eboard_info->power_info;
	e_ctrl->eboard_info->i2c_slaveaddr = temp;
	e_ctrl->i2c_client.client = client;
	e_ctrl->is_supported = 0;

	/* Set device type as I2C */
	e_ctrl->eeprom_device_type = MSM_CAMERA_I2C_DEVICE;
	e_ctrl->i2c_client.i2c_func_tbl = &msm_eeprom_qup_func_tbl;

	if (e_ctrl->eboard_info->i2c_slaveaddr != 0)
		e_ctrl->i2c_client.client->addr =
					e_ctrl->eboard_info->i2c_slaveaddr;
	power_info->clk_info = cam_8960_clk_info;
	power_info->clk_info_size = ARRAY_SIZE(cam_8960_clk_info);
	power_info->dev = &client->dev;

	rc = of_property_read_string(of_node, "qcom,eeprom-name",
		&e_ctrl->eboard_info->eeprom_name);
	CDBG("%s qcom,eeprom-name %s, rc %d\n", __func__,
		e_ctrl->eboard_info->eeprom_name, rc);
	if (rc < 0) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		goto board_free;
	}

	rc = msm_eeprom_get_dt_data(e_ctrl);
	if (rc)
		goto board_free;

	rc = msm_eeprom_alloc_memory_map(e_ctrl, of_node);
	if (rc)
		goto board_free;

	rc = msm_camera_power_up(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("%s failed power up %d\n", __func__, __LINE__);
		goto memdata_free;
	}
	rc = read_eeprom_memory(e_ctrl);
	if (rc < 0) {
		pr_err("%s read_eeprom_memory failed\n", __func__);
		goto power_down;
	}

	for (j = 0; j < e_ctrl->num_bytes; j++)
		CDBG("memory_data[%d] = 0x%X\n", j, e_ctrl->memory_data[j]);

	rc = msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		goto power_down;
	}

	/*IMPLEMENT READING PART*/
	/* Initialize sub device */
	v4l2_i2c_subdev_init(&e_ctrl->msm_sd.sd,
		e_ctrl->i2c_client.client,
		e_ctrl->eeprom_v4l2_subdev_ops);
	v4l2_set_subdevdata(&e_ctrl->msm_sd.sd, e_ctrl);
	e_ctrl->msm_sd.sd.internal_ops = &msm_eeprom_internal_ops;
	e_ctrl->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	media_entity_init(&e_ctrl->msm_sd.sd.entity, 0, NULL, 0);
	e_ctrl->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	e_ctrl->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_EEPROM;
	msm_sd_register(&e_ctrl->msm_sd);
	e_ctrl->is_supported = 1;
	CDBG("%s success result=%d X\n", __func__, rc);
	return rc;

power_down:
	msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
memdata_free:
	kfree(e_ctrl->memory_data);
	kfree(e_ctrl->eboard_info->eeprom_map);
board_free:
	kfree(e_ctrl->eboard_info);
probe_failure:
	pr_err("%s failed! rc = %d\n", __func__, rc);
	return rc;
}

static int32_t msm_eeprom_i2c_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct msm_eeprom_ctrl_t  *e_ctrl;
	if (!sd) {
		pr_err("%s: Subdevice is NULL\n", __func__);
		return 0;
	}

	e_ctrl = (struct msm_eeprom_ctrl_t *)v4l2_get_subdevdata(sd);
	if (!e_ctrl) {
		pr_err("%s: eeprom device is NULL\n", __func__);
		return 0;
	}

	kfree(e_ctrl->memory_data);
	if (e_ctrl->eboard_info) {
		kfree(e_ctrl->eboard_info->power_info.gpio_conf);
		kfree(e_ctrl->eboard_info->eeprom_map);
	}
	kfree(e_ctrl->eboard_info);
	kfree(e_ctrl);
	return 0;
}

#define msm_eeprom_spi_parse_cmd(spic, str, name, out, size)		\
	{								\
		if (of_property_read_u32_array(				\
			spic->spi_master->dev.of_node,			\
			str, out, size)) {				\
			return -EFAULT;					\
		} else {						\
			spic->cmd_tbl.name.opcode = out[0];		\
			spic->cmd_tbl.name.addr_len = out[1];		\
			spic->cmd_tbl.name.dummy_len = out[2];		\
		}							\
	}

static int msm_eeprom_spi_parse_of(struct msm_camera_spi_client *spic)
{
	int rc = -EFAULT;
	uint32_t tmp[3];
	msm_eeprom_spi_parse_cmd(spic, "qcom,spiop,read", read, tmp, 3);
	msm_eeprom_spi_parse_cmd(spic, "qcom,spiop,readseq", read_seq, tmp, 3);
	msm_eeprom_spi_parse_cmd(spic, "qcom,spiop,queryid", query_id, tmp, 3);

	rc = of_property_read_u32_array(spic->spi_master->dev.of_node,
					"qcom,eeprom-id", tmp, 2);
	if (rc) {
		pr_err("%s: Failed to get eeprom id\n", __func__);
		return rc;
	}
	spic->mfr_id = tmp[0];
	spic->device_id = tmp[1];

	return 0;
}

static int msm_eeprom_check_id(struct msm_eeprom_ctrl_t *e_ctrl)
{
	int rc;
	struct msm_camera_i2c_client *client = &e_ctrl->i2c_client;
	uint8_t id[2];

	rc = msm_camera_spi_query_id(client, 0, &id[0], 2);
	if (rc)
		return rc;
	if (id[0] != client->spi_client->mfr_id
		    || id[1] != client->spi_client->device_id) {
		CDBG("%s: read 0x%x 0x%x, check 0x%x 0x%x\n", __func__, id[0],
		     id[1], client->spi_client->mfr_id,
		     client->spi_client->device_id);
		return -ENODEV;
	}

	return 0;
}

static int msm_eeprom_spi_setup(struct spi_device *spi)
{
	struct msm_eeprom_ctrl_t *e_ctrl = NULL;
	struct msm_camera_i2c_client *client = NULL;
	struct msm_camera_spi_client *spi_client;
	struct msm_eeprom_board_info *eb_info;
	struct msm_camera_power_ctrl_t *power_info = NULL;
	int rc = 0;

	e_ctrl = kzalloc(sizeof(struct msm_eeprom_ctrl_t), GFP_KERNEL);
	if (!e_ctrl) {
		pr_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		return -ENOMEM;
	}
	e_ctrl->eeprom_v4l2_subdev_ops = &msm_eeprom_subdev_ops;
	e_ctrl->eeprom_mutex = &msm_eeprom_mutex;
	client = &e_ctrl->i2c_client;
	e_ctrl->is_supported = 0;

	spi_client = kzalloc(sizeof(spi_client), GFP_KERNEL);
	if (!spi_client) {
		pr_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		kfree(e_ctrl);
		return -ENOMEM;
	}

	rc = of_property_read_u32(spi->dev.of_node, "cell-index",
				  &e_ctrl->subdev_id);
	CDBG("cell-index %d, rc %d\n", e_ctrl->subdev_id, rc);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		return rc;
	}

	e_ctrl->eeprom_device_type = MSM_CAMERA_SPI_DEVICE;
	client->spi_client = spi_client;
	spi_client->spi_master = spi;
	client->i2c_func_tbl = &msm_eeprom_spi_func_tbl;
	client->addr_type = MSM_CAMERA_I2C_3B_ADDR;

	eb_info = kzalloc(sizeof(eb_info), GFP_KERNEL);
	if (!eb_info)
		goto spi_free;
	e_ctrl->eboard_info = eb_info;
	rc = of_property_read_string(spi->dev.of_node, "qcom,eeprom-name",
		&eb_info->eeprom_name);
	CDBG("%s qcom,eeprom-name %s, rc %d\n", __func__,
		eb_info->eeprom_name, rc);
	if (rc < 0) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		goto board_free;
	}
	power_info = &eb_info->power_info;

	power_info->clk_info = cam_8974_clk_info;
	power_info->clk_info_size = ARRAY_SIZE(cam_8974_clk_info);
	power_info->dev = &spi->dev;

	rc = msm_eeprom_get_dt_data(e_ctrl);
	if (rc)
		goto board_free;

	/* set spi instruction info */
	spi_client->retry_delay = 1;
	spi_client->retries = 0;

	if (msm_eeprom_spi_parse_of(spi_client)) {
		dev_err(&spi->dev,
			"%s: Error parsing device properties\n", __func__);
		goto board_free;
	}

	rc = msm_eeprom_alloc_memory_map(e_ctrl, spi->dev.of_node);
	if (rc)
		goto board_free;

	rc = msm_camera_power_up(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		goto memmap_free;
	}

	/* check eeprom id */
	rc = msm_eeprom_check_id(e_ctrl);
	if (rc) {
		CDBG("%s: eeprom not matching %d\n", __func__, rc);
		goto power_down;
	}
	/* read eeprom */
	rc = read_eeprom_memory(e_ctrl);
	if (rc) {
		dev_err(&spi->dev, "%s: read eeprom memory failed\n", __func__);
		goto power_down;
	}

	rc = msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		goto memmap_free;
	}

	/* initiazlie subdev */
	v4l2_spi_subdev_init(&e_ctrl->msm_sd.sd,
		e_ctrl->i2c_client.spi_client->spi_master,
		e_ctrl->eeprom_v4l2_subdev_ops);
	v4l2_set_subdevdata(&e_ctrl->msm_sd.sd, e_ctrl);
	e_ctrl->msm_sd.sd.internal_ops = &msm_eeprom_internal_ops;
	e_ctrl->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	media_entity_init(&e_ctrl->msm_sd.sd.entity, 0, NULL, 0);
	e_ctrl->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	e_ctrl->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_EEPROM;
	msm_sd_register(&e_ctrl->msm_sd);
	e_ctrl->is_supported = 1;
	CDBG("%s success result=%d X\n", __func__, rc);

	return 0;

power_down:
	msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
memmap_free:
	kfree(e_ctrl->eboard_info->eeprom_map);
	kfree(e_ctrl->memory_data);
board_free:
	kfree(e_ctrl->eboard_info);
spi_free:
	kfree(spi_client);
	return rc;
}

static int msm_eeprom_spi_probe(struct spi_device *spi)
{
	int irq, cs, cpha, cpol, cs_high;

	CDBG("%s\n", __func__);
	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	spi_setup(spi);

	irq = spi->irq;
	cs = spi->chip_select;
	cpha = (spi->mode & SPI_CPHA) ? 1 : 0;
	cpol = (spi->mode & SPI_CPOL) ? 1 : 0;
	cs_high = (spi->mode & SPI_CS_HIGH) ? 1 : 0;
	dev_info(&spi->dev, "irq[%d] cs[%x] CPHA[%x] CPOL[%x] CS_HIGH[%x]\n",
			irq, cs, cpha, cpol, cs_high);
	dev_info(&spi->dev, "max_speed[%u]\n", spi->max_speed_hz);

	return msm_eeprom_spi_setup(spi);
}

static int32_t msm_eeprom_spi_remove(struct spi_device *sdev)
{
	struct v4l2_subdev *sd = spi_get_drvdata(sdev);
	struct msm_eeprom_ctrl_t  *e_ctrl;
	if (!sd) {
		pr_err("%s: Subdevice is NULL\n", __func__);
		return 0;
	}

	e_ctrl = (struct msm_eeprom_ctrl_t *)v4l2_get_subdevdata(sd);
	if (!e_ctrl) {
		pr_err("%s: eeprom device is NULL\n", __func__);
		return 0;
	}

	kfree(e_ctrl->i2c_client.spi_client);
	kfree(e_ctrl->memory_data);
	if (e_ctrl->eboard_info) {
		kfree(e_ctrl->eboard_info->power_info.gpio_conf);
		kfree(e_ctrl->eboard_info->eeprom_map);
	}
	kfree(e_ctrl->eboard_info);
	kfree(e_ctrl);
	return 0;
}

static int32_t msm_eeprom_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	int32_t j = 0;
	uint32_t temp;

	struct msm_camera_cci_client *cci_client = NULL;
	struct msm_eeprom_ctrl_t *e_ctrl = NULL;
	struct msm_eeprom_board_info *eb_info = NULL;
	struct device_node *of_node = pdev->dev.of_node;
	struct msm_camera_power_ctrl_t *power_info = NULL;

	CDBG("%s E\n", __func__);

	e_ctrl = kzalloc(sizeof(struct msm_eeprom_ctrl_t), GFP_KERNEL);
	if (!e_ctrl) {
		pr_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		return -ENOMEM;
	}
	e_ctrl->eeprom_v4l2_subdev_ops = &msm_eeprom_subdev_ops;
	e_ctrl->eeprom_mutex = &msm_eeprom_mutex;

	e_ctrl->is_supported = 0;
	if (!of_node) {
		pr_err("%s dev.of_node NULL\n", __func__);
		kfree(e_ctrl);
		return -EINVAL;
	}

	rc = of_property_read_u32(of_node, "cell-index",
		&pdev->id);
	CDBG("cell-index %d, rc %d\n", pdev->id, rc);
	if (rc < 0) {
		pr_err("failed rc %d\n", rc);
		kfree(e_ctrl);
		return rc;
	}
	e_ctrl->subdev_id = pdev->id;

	rc = of_property_read_u32(of_node, "qcom,cci-master",
		&e_ctrl->cci_master);
	CDBG("qcom,cci-master %d, rc %d\n", e_ctrl->cci_master, rc);
	if (rc < 0) {
		pr_err("%s failed rc %d\n", __func__, rc);
		kfree(e_ctrl);
		return rc;
	}
	rc = of_property_read_u32(of_node, "qcom,slave-addr",
		&temp);
	if (rc < 0) {
		pr_err("%s failed rc %d\n", __func__, rc);
		kfree(e_ctrl);
		return rc;
	}

	/* Set platform device handle */
	e_ctrl->pdev = pdev;
	/* Set device type as platform device */
	e_ctrl->eeprom_device_type = MSM_CAMERA_PLATFORM_DEVICE;
	e_ctrl->i2c_client.i2c_func_tbl = &msm_eeprom_cci_func_tbl;
	e_ctrl->i2c_client.cci_client = kzalloc(sizeof(
		struct msm_camera_cci_client), GFP_KERNEL);
	if (!e_ctrl->i2c_client.cci_client) {
		pr_err("%s failed no memory\n", __func__);
		kfree(e_ctrl);
		return -ENOMEM;
	}

	e_ctrl->eboard_info = kzalloc(sizeof(
		struct msm_eeprom_board_info), GFP_KERNEL);
	if (!e_ctrl->eboard_info) {
		pr_err("%s failed line %d\n", __func__, __LINE__);
		rc = -ENOMEM;
		goto cciclient_free;
	}
#ifdef CONFIG_PANTECH_CAMERA_READ_EEPROM
       e_ctrl->set_block_bytes = EEPROM_READ_BLOCK;
#if defined(CONFIG_PANTECH_CAMERA_IMX135)
       e_ctrl->is_increase_slave_address = true;
#else
       e_ctrl->is_increase_slave_address = false;
#endif
#endif 
	
	eb_info = e_ctrl->eboard_info;
	power_info = &eb_info->power_info;
	eb_info->i2c_slaveaddr = temp;

	power_info->clk_info = cam_8974_clk_info;
	power_info->clk_info_size = ARRAY_SIZE(cam_8974_clk_info);
	power_info->dev = &pdev->dev;

	CDBG("qcom,slave-addr = 0x%X\n", eb_info->i2c_slaveaddr);
	cci_client = e_ctrl->i2c_client.cci_client;
	cci_client->cci_subdev = msm_cci_get_subdev();
	cci_client->cci_i2c_master = e_ctrl->cci_master;
	cci_client->sid = eb_info->i2c_slaveaddr >> 1;
	cci_client->retries = 3;
	cci_client->id_map = 0;

	rc = of_property_read_string(of_node, "qcom,eeprom-name",
		&eb_info->eeprom_name);
	CDBG("%s qcom,eeprom-name %s, rc %d\n", __func__,
		eb_info->eeprom_name, rc);
	if (rc < 0) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		goto board_free;
	}

	rc = msm_eeprom_get_dt_data(e_ctrl);
	if (rc)
		goto board_free;

	rc = msm_eeprom_alloc_memory_map(e_ctrl, of_node);
	if (rc)
		goto board_free;

	rc = msm_camera_power_up(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		goto memdata_free;
	}

 	rc = read_eeprom_memory(e_ctrl);
	if (rc < 0) {
		pr_err("%s read_eeprom_memory failed\n", __func__);
		goto power_down;
	}
#ifdef CONFIG_PANTECH_CAMERA_EEPROM_CHECKSUM
	rc = read_eeprom_memory_checksum(e_ctrl);
	if (rc < 0) {
		pr_err("%s read_eeprom_memory checksum failed\n", __func__);
		goto power_down;
	}
#endif	
 		pr_err("%s line %d\n", __func__, __LINE__);
	for (j = 0; j < e_ctrl->num_bytes; j++)
		CDBG("memory_data[%d] = 0x%X\n", j, e_ctrl->memory_data[j]);

	rc = msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		goto memdata_free;
	}
	v4l2_subdev_init(&e_ctrl->msm_sd.sd,
		e_ctrl->eeprom_v4l2_subdev_ops);
	v4l2_set_subdevdata(&e_ctrl->msm_sd.sd, e_ctrl);
	platform_set_drvdata(pdev, &e_ctrl->msm_sd.sd);
	e_ctrl->msm_sd.sd.internal_ops = &msm_eeprom_internal_ops;
	e_ctrl->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	snprintf(e_ctrl->msm_sd.sd.name,
		ARRAY_SIZE(e_ctrl->msm_sd.sd.name), "msm_eeprom");
	media_entity_init(&e_ctrl->msm_sd.sd.entity, 0, NULL, 0);
	e_ctrl->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	e_ctrl->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_EEPROM;
	msm_sd_register(&e_ctrl->msm_sd);


	e_ctrl->is_supported = 1;
	CDBG("%s X\n", __func__);
	return rc;

power_down:
	msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
memdata_free:
	kfree(e_ctrl->memory_data);
	kfree(eb_info->eeprom_map);
board_free:
	kfree(e_ctrl->eboard_info);
cciclient_free:
	kfree(e_ctrl->i2c_client.cci_client);
	kfree(e_ctrl);
	return rc;
}

static int32_t msm_eeprom_platform_remove(struct platform_device *pdev)
{
	struct v4l2_subdev *sd = platform_get_drvdata(pdev);
	struct msm_eeprom_ctrl_t  *e_ctrl;
	if (!sd) {
		pr_err("%s: Subdevice is NULL\n", __func__);
		return 0;
	}

	e_ctrl = (struct msm_eeprom_ctrl_t *)v4l2_get_subdevdata(sd);
	if (!e_ctrl) {
		pr_err("%s: eeprom device is NULL\n", __func__);
		return 0;
	}

	kfree(e_ctrl->i2c_client.cci_client);
	kfree(e_ctrl->memory_data);
	if (e_ctrl->eboard_info) {
		kfree(e_ctrl->eboard_info->power_info.gpio_conf);
		kfree(e_ctrl->eboard_info->eeprom_map);
	}
	kfree(e_ctrl->eboard_info);
	kfree(e_ctrl);
	return 0;
}

static const struct of_device_id msm_eeprom_dt_match[] = {
	{ .compatible = "qcom,eeprom" },
	{ }
};

MODULE_DEVICE_TABLE(of, msm_eeprom_dt_match);

static struct platform_driver msm_eeprom_platform_driver = {
	.driver = {
		.name = "qcom,eeprom",
		.owner = THIS_MODULE,
		.of_match_table = msm_eeprom_dt_match,
	},
	.remove = __devexit_p(msm_eeprom_platform_remove),
};

static const struct i2c_device_id msm_eeprom_i2c_id[] = {
	{ "msm_eeprom", (kernel_ulong_t)NULL},
	{ }
};

static struct i2c_driver msm_eeprom_i2c_driver = {
	.id_table = msm_eeprom_i2c_id,
	.probe  = msm_eeprom_i2c_probe,
	.remove = __devexit_p(msm_eeprom_i2c_remove),
	.driver = {
		.name = "msm_eeprom",
	},
};

static struct spi_driver msm_eeprom_spi_driver = {
	.driver = {
		.name = "qcom_eeprom",
		.owner = THIS_MODULE,
		.of_match_table = msm_eeprom_dt_match,
	},
	.probe = msm_eeprom_spi_probe,
	.remove = __devexit_p(msm_eeprom_spi_remove),
};

static int __init msm_eeprom_init_module(void)
{
	int32_t rc = 0;
	CDBG("%s E\n", __func__);
	rc = platform_driver_probe(&msm_eeprom_platform_driver,
		msm_eeprom_platform_probe);
	CDBG("%s:%d platform rc %d\n", __func__, __LINE__, rc);
#ifdef CONFIG_PANTECH_CAMERA//eeprom
	return rc;
#else
	rc = spi_register_driver(&msm_eeprom_spi_driver);
	CDBG("%s:%d spi rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&msm_eeprom_i2c_driver);
#endif
}

static void __exit msm_eeprom_exit_module(void)
{
	platform_driver_unregister(&msm_eeprom_platform_driver);
	spi_unregister_driver(&msm_eeprom_spi_driver);
	i2c_del_driver(&msm_eeprom_i2c_driver);
}

module_init(msm_eeprom_init_module);
module_exit(msm_eeprom_exit_module);
MODULE_DESCRIPTION("MSM EEPROM driver");
MODULE_LICENSE("GPL v2");
