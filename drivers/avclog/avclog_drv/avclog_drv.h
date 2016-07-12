#ifndef __AVCLOG_DRV_H
#define __AVCLOG_DRV_H

#define AVCLOG_SKIP_BEFORE_READY
/* ------------------------------------ */
/*     SHARE with avclog_thread         */
/* ------------------------------------ */
#define AVCLOG_FLASH_HEADER_SIZE 	0x10
#define AVCLOG_BUFFER_HEADER_SIZE 0x10
#define AVCLOG_MEMORY_ALLOC_SIZE	1024
#define AVCLOG_BUFFER_SIZE			AVCLOG_MEMORY_ALLOC_SIZE - AVCLOG_BUFFER_HEADER_SIZE - AVCLOG_FLASH_HEADER_SIZE
#define AVCLOG_ITEM_MAGIC 			0xCDCDCDCD
#define AVCLOG_ITEM_HEADER_SIZE 	0x10

struct avclog_info
{
	unsigned int magic;
	unsigned int write_pos;
	unsigned int logcnt;
	unsigned int ram_pos;
};

struct avclog_buffer_info
{
	unsigned int buffer_pos;
};

struct avclog_item_info
{
	unsigned int magic;	
	unsigned int idx;
	unsigned int err;
	unsigned int len;
};

struct avcsync_ddinfo
{
	unsigned int pos;
	unsigned int size;
};

/* ------------------------------------ */

#define AVCLOG_IOCTL_SET_DATASYNC	0000
#define AVCLOG_IOCTL_READY			0001
#define AVCLOG_IOCTL_WRITE			0002
#define AVCLOG_IOCTL_DOWN_SEM		0003
#define AVCLOG_IOCTL_UP_SEM			0004


#define AVC_WORD_SIZE                   16
#define AVC_WORD_ALIGNMENT(x)           (((x)+(AVC_WORD_SIZE-1)) & ~(AVC_WORD_SIZE-1))

#endif
