#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <mach/msm_iomap.h>
#include <asm/uaccess.h>
#include <linux/rtc.h>
#include <linux/list.h>
#include <linux/hardirq.h>
#include <linux/avclog.h>
#include <linux/avcsync.h>

#include "avclog_drv.h"

#define DRV_NAME		"avclog"

unsigned char *avclog_header;
unsigned char *avclog_data;

unsigned long avclog_phy_addr;

struct avclog_info *header_info;
struct avclog_buffer_info *buffer_info;

static int avcsync_id = -1;
int avclog_ready = 0;

#ifndef AVCLOG_SKIP_BEFORE_READY
  LIST_HEAD(to_save_previous_log);
struct previous_log_buf {
	char *log;
	unsigned int len;
	unsigned int err;
	struct list_head list;
};
#endif

static DEFINE_SEMAPHORE(avclog_sem);

int avclog_write(unsigned int err, const char *format, ...);
static int avclog_open(struct inode *inode, struct file *file);
static int avclog_close(struct inode *inode, struct file *file);
static long avclog_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t avclog_sync_write (struct file *file, const char __user *buf, size_t size, loff_t *offset); 
static ssize_t avclog_print(struct file *file, char __user *buf, size_t size, loff_t *offset);
static int avclog_mmap(struct file *file, struct vm_area_struct *vma);
static int __init avclog_init(void);
//static void __exit avclog_exit(void);

struct file_operations avclog_fops = {
    .owner =    THIS_MODULE,
    .open =     avclog_open,
    .release =  avclog_close,
    .unlocked_ioctl = avclog_ioctl,
	.write = avclog_sync_write,	
	.read = avclog_print,
	.mmap = 	avclog_mmap
};

static struct miscdevice avclog_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DRV_NAME,
    .fops = &avclog_fops,
};

static void print_avclog_function(const char *fmt)
{
    AVC_DBG_CONFIRM(AVC_CONFIRM, printk("SUNGHWAN AVCLOG[%s]\n", fmt));
}

inline int avc_check_overwrite_prev_log(unsigned int pos, unsigned int size, unsigned int* ram_pos)
{
	print_avclog_function(__func__);
	*ram_pos = header_info->ram_pos;

	return (pos < (*ram_pos)) && ((pos + size) > (*ram_pos));
}

void avc_overwrite_error_handler(unsigned int pos, unsigned int size, unsigned int ram_pos)
{
	print_avclog_function(__func__);
	 AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] can't write new log. Not enough buffer\n", __func__));
	 AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] address range for log to go (0x%x ~ 0x%x)\n", __func__, pos, pos+size));
	 AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] address for log being written to flash (0x%x)\n", __func__, ram_pos));
}

#ifndef AVCLOG_SKIP_BEFORE_READY
static int avc_pre_save_log(char *buffer, unsigned int len, unsigned int err)
{
	gfp_t gfp_flag;
	struct previous_log_buf *buf_list;

	print_avclog_function(__func__);

	if(in_atomic() || in_interrupt() || irqs_disabled())
		gfp_flag=GFP_ATOMIC;
	else
		gfp_flag=GFP_KERNEL;

	buf_list=kzalloc(sizeof(struct previous_log_buf), gfp_flag);
	if(buf_list==NULL)
		return -1;
	
	buf_list->log=buffer;
	buf_list->len=len;
	buf_list->err=err;

	list_add_tail(&buf_list->list, &to_save_previous_log);

	return 0;
}
#endif

static int avclog_setbuf_sync(char *buffer, unsigned int len, unsigned int err)
{
	unsigned int write_pos;
	int ret;
	struct avcsync_ddinfo ddinfo;
	unsigned char *dst;
	unsigned int ram_pos;
	struct avclog_item_info item_header_info;
	unsigned int seq_no;

	print_avclog_function(__func__);
	
	// Get pos to write & seq_no
	//down(&avclog_sem);
	if(down_trylock(&avclog_sem)){
		if(in_atomic() || in_interrupt() || irqs_disabled()){
			AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR : Fail to acquire semaphore in atomic context..!\n",__func__));
			return -1;
		}
		else
	      down(&avclog_sem);
	}	

	write_pos = buffer_info->buffer_pos;

	if ((write_pos + AVCLOG_ITEM_HEADER_SIZE + len) > AVCLOG_BUFFER_SIZE){
		write_pos = 0;
  	}

	if( avc_check_overwrite_prev_log(write_pos, len, &ram_pos))
	{
		// not ready to write log
		up(&avclog_sem);
		avc_overwrite_error_handler(write_pos, len, ram_pos);
		return -1;
	}
	// ready to write log
	buffer_info->buffer_pos = write_pos;
	buffer_info->buffer_pos += (len + AVCLOG_ITEM_HEADER_SIZE);
	seq_no = header_info->logcnt++;

	up(&avclog_sem);

	// STORE LOG to mem
	dst = avclog_data + write_pos;
	item_header_info.magic = AVCLOG_ITEM_MAGIC;
	item_header_info.idx = seq_no;
	item_header_info.err = err;
	item_header_info.len = len;

	AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] err=%d, size=%d, wpos=%d\n", __func__, err, len, write_pos));

	//update to the buffer (header + data)
	memcpy(dst, &item_header_info, sizeof(struct avclog_item_info));
	memcpy(dst+AVCLOG_ITEM_HEADER_SIZE, buffer, len);
	
 	AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] update write pos[%d]\n", __func__, buffer_info->buffer_pos));

	// SYNC
 	ddinfo.pos = write_pos;
	ddinfo.size = len + AVCLOG_ITEM_HEADER_SIZE;

	AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] err(%d), size(%d), pos(%d) set data sync..\n",__func__, err ,len, ddinfo.pos));
    ret = avcsync_write_queue(avcsync_id, (const char *)(&ddinfo), sizeof(struct avcsync_ddinfo));
    if ( ret < 0 )
    {
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR : write func was avcsync_write_queue err.\n", __func__));
        return -1;
    }  

	return 0;

}

int avclog_write(unsigned int err, const char *format, ...)
{
	unsigned int aligned_size;
	va_list vl;
	char *buffer;
	int ret=0;
	int n_size=0;	
	gfp_t gfp_flag;
	int temp_size=512;		//MAX String size..

	print_avclog_function(__func__);

#ifdef AVCLOG_SKIP_BEFORE_READY
	if (!avclog_ready)
    return ret;
#endif

	if(in_atomic() || in_interrupt() || irqs_disabled())
		gfp_flag=GFP_ATOMIC;
	else
		gfp_flag=GFP_KERNEL;

	buffer=kzalloc(temp_size, gfp_flag);
	if (buffer==NULL)
	{
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR : kmalloc fail...\n", __func__));
        return -1;
    }
	
	va_start(vl, format);
	n_size=vsnprintf(buffer, temp_size, format, vl);
	va_end(vl);

	n_size++; //to push null in case of aligned size.
	aligned_size = AVC_WORD_ALIGNMENT(n_size);

	AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] n_size=%d, aligned_size=%d buffer=%s\n", __func__, n_size, aligned_size, buffer ));
#ifndef AVCLOG_SKIP_BEFORE_READY
	if (!avclog_ready)
    {
    	ret = avc_pre_save_log(buffer, aligned_size, err);
    	AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] avclog was not ready.. avc_pre_save_log..[RET=%d]\n", __func__, ret));
        return ret;
    }
#endif

    if ( avcsync_id == -1 )
    {
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR : avcsync id error...\n", __func__));
        kfree(buffer);
        return -1;
    }

    ret = avclog_setbuf_sync(buffer, aligned_size, err);

    kfree(buffer);

	return ret;
}

static int avclog_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_RESERVED;
	vma->vm_flags |= VM_IO;

	print_avclog_function(__func__);

	if(remap_pfn_range(vma, vma->vm_start, avclog_phy_addr>>PAGE_SHIFT,
                      vma->vm_end - vma->vm_start, pgprot_noncached(vma->vm_page_prot)))
    {
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR \n", __func__));
        return -1;
    }

    AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] sucess\n", __func__));
    return 0;
}

static int avclog_open(struct inode *inode, struct file *file)
{
	print_avclog_function(__func__);

//   AVC_DBG_NORMAL(AVC_NORMAL, printk("avclogdrv open\n"));
    return 0;
}


static int avclog_close(struct inode *inode, struct file *file)
{
	print_avclog_function(__func__);

//   AVC_DBG_NORMAL(AVC_NORMAL, printk("nvdrv close\n"));
    return 0;
}

static ssize_t avclog_print(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	struct avcsync_ddinfo ddinfo;

	print_avclog_function(__func__);

	// SYNC
	ddinfo.pos = 0;
	ddinfo.size = 0;

	AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] size=%d, buf_addr=%p\n", __func__, size, buf));
	avcsync_write_queue(avcsync_id, (const char *)(&ddinfo), sizeof(struct avcsync_ddinfo));

	return 0;
}

static ssize_t avclog_sync_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	print_avclog_function(__func__);
//   AVC_DBG_NORMAL(AVC_NORMAL, printk("nvdrv sync_write\n"));
    return 0;
}

/******************************************************************************/
/* MODULE   : avclog_ioctl                                                    */
/******************************************************************************/
static long avclog_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret=0;
    
	print_avclog_function(__func__);

	switch(cmd)
	{	
		case AVCLOG_IOCTL_SET_DATASYNC:
	        ret = copy_from_user( &avcsync_id, (void *)arg, sizeof(int) );
	        AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] copy_from_user : ret = %ld\n", __func__, ret));
	        if ( ret != 0 )
	        {
	            AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR : data copy err\n", __func__));
	            ret = -1;
	            break;
	        }
			AVC_DBG_CONFIRM(AVC_CONFIRM, printk("[%s] NVDRV_IOCTL_SET_DATASYNC...avcsync_id=%d\n", __func__, avcsync_id));
		break;

		case AVCLOG_IOCTL_READY:
#ifndef AVCLOG_SKIP_BEFORE_READY
			if (!list_empty(&to_save_previous_log))
		    {
		    	struct previous_log_buf *curr_list;
		    	
		    	AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] Proceed pre save.....\n", __func__));
		    	list_for_each_entry(curr_list, &to_save_previous_log, list)
		    	{    		
					AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] avclog_setbuf_sync,, to save previous log..\n", __func__));
		    		avclog_setbuf_sync(curr_list->log, curr_list->len, curr_list->err);
		    		kfree(curr_list->log);  //free log buffer alloc
		    		kfree(curr_list);		//free list alloc
		    	}		    	
		    	list_del_init(&to_save_previous_log);
		    }
#endif
			avclog_ready=1;
			buffer_info->buffer_pos = 0;
			AVC_DBG_CONFIRM(AVC_CONFIRM, printk("[%s] avclog_header = %p, avclog_data=%p\n", __func__, avclog_header, avclog_data));
			AVC_DBG_CONFIRM(AVC_CONFIRM, printk("[%s] magic=0x%x, mmc_wpos=0x%x, logcnt=%d\n", __func__, header_info->magic, header_info->write_pos, header_info->logcnt));
			AVC_DBG_CONFIRM(AVC_CONFIRM, printk("[%s] AVCLOG Ready Done\n", __func__));
		break;
		
		case AVCLOG_IOCTL_DOWN_SEM:
		{
			down(&avclog_sem);
		}
		break;

		case AVCLOG_IOCTL_UP_SEM:
		{
			up(&avclog_sem);
		}
		break;
		
		default:
			AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR : cmd err : cmd=%d, arg=%ld\n", __func__, cmd, arg));
		break;    
    }

    return ret;
}

/******************************************************************************/
/*    MODULE   : avclog_init                                                  */
/******************************************************************************/
static int __init avclog_init(void)
{
    int ret;
    char *buffer_head_ptr;

	print_avclog_function(__func__);

    AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] \n", __func__));

    ret = misc_register(&avclog_miscdev);
    if ( ret != 0 )
    {
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR \n", __func__));
        return -EIO;
    }

	avclog_phy_addr=allocate_contiguous_ebi_nomap(AVCLOG_MEMORY_ALLOC_SIZE, SZ_1K);
	if (!avclog_phy_addr)
	{
		AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR : init failed...!\n", __func__));
		return -1;
	}
	
	avclog_header = ioremap_nocache(avclog_phy_addr, AVCLOG_MEMORY_ALLOC_SIZE);
	if (avclog_header == NULL) {
		AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ERROR : init failed!\n", __func__));
		return -1;
	}

	memset(avclog_header, 0x0, AVCLOG_MEMORY_ALLOC_SIZE);
	buffer_head_ptr = avclog_header + AVCLOG_FLASH_HEADER_SIZE;
	avclog_data = avclog_header+AVCLOG_FLASH_HEADER_SIZE+AVCLOG_BUFFER_HEADER_SIZE;
	header_info = (struct avclog_info*)avclog_header;
	buffer_info = (struct avclog_buffer_info*)buffer_head_ptr;

    AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] header_info=0x%x\n", __func__, (unsigned int)header_info));
    AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] buffer_info=0x%x\n", __func__, (unsigned int)buffer_info));

    return 0;
}
//module_init(avclog_init);
subsys_initcall(avclog_init);

EXPORT_SYMBOL(avclog_write);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
