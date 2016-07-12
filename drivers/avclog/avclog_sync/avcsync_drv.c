#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/spinlock.h>              
#include <linux/interrupt.h> /* 2.6 taiou */
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/avcsync.h>
#include <linux/avclog.h>

#include "avcsync_ring_queue.h"
#include "avcsync_minilib.h"

struct avcsync_info {
    int  avcsync_nodes;    
    int  avcsync_datasize; 
};

#define AVCSYNC_DRV_NAME   "avcsync"

#define AVCSYNC_NORMAL            0
#define AVCSYNC_READ_THREAD_LEFT  1

#define AVCSYNC_QUEUE_CREATE  1000
#define AVCSYNC_QUEUE_GET_ID  1001  


static int __init   avcsync_init(void);
static int          avcsync_open(struct inode*, struct file*);
static int          avcsync_close(struct inode*, struct file*);
static long         avcsync_ioctl(struct file*, uint, ulong);
static ssize_t      avcsync_read(struct file*, char*, size_t, loff_t*);
static ssize_t      avcsync_write(struct file*, const char* ,size_t, loff_t*);
static int          avcsync_flush(struct file*, fl_owner_t id);
static unsigned int avcsync_poll(struct file*, poll_table*);
int avcsync_write_queue( int,                     
                     const char*,              
                     size_t );               


static struct file_operations  avcsync_fops = {
    llseek:   no_llseek,
    open:     avcsync_open,
    release:  avcsync_close,
    unlocked_ioctl:    avcsync_ioctl,
    read:     avcsync_read,
    write:    avcsync_write,
    flush:    avcsync_flush,
    poll:     avcsync_poll,
};


spinlock_t   avcsync_lock;                       
unsigned int avcsync_op_count;                  

static struct miscdevice avcsync_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = AVCSYNC_DRV_NAME,
    .fops = &avcsync_fops
};
static void print_avcsync_function(const char *fmt)
{
    AVC_DBG_CONFIRM(AVC_CONFIRM, printk("SUNGHWAN AVCSYNC[%s]\n", fmt));
}
//-----------------------------------------------------------------------------
static int __init avcsync_init(void)
{
    int  err;
    print_avcsync_function(__func__);

    if((err = misc_register(&avcsync_miscdev)) < 0) {
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] misc_register error.\n", __func__));
        return err;
    }
    AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] initialized.\n", __func__));
    spin_lock_init( &avcsync_lock );
    avcsync_op_count = 0;                       
    avc_rq_id_init();                      
    
    return 0;
}


//-----------------------------------------------------------------------------
static int avcsync_open(struct inode* ino, struct file* fp)
{
    avcsync_fpd*  fpd;
    int       ret;                           
    unsigned long flags;                    

     print_avcsync_function(__func__);

    if ((fp->private_data = kmalloc(sizeof(avcsync_fpd), GFP_USER)) == NULL) {
	    AVC_DBG_FATAL(AVC_FATAL, printk("[%s] cannot allocate avcsync_ring_queue.\n", __func__));
	    return -ENOMEM;
    }

    avcsync_op_count += AVC_ADD_OP_COUNT;              

    fpd = (avcsync_fpd*)fp->private_data;
    spin_lock_irqsave( &avcsync_lock, flags );      
    ret = avc_rq_id_take( fpd );                    
    spin_unlock_irqrestore( &avcsync_lock, flags );
    if( ret == -1 )                           
    {                                        
        kfree( fpd );                         
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] cannot get id.\n", __func__));           
        return -ENOMEM;                       
    }                                        
    fpd->allocated = FALSE;
    fpd->status    = AVCSYNC_NORMAL;
    fpd->tgid    = current->tgid;

    init_waitqueue_head(&fpd->waitq);
    
    return 0;
}


//-----------------------------------------------------------------------------
static int avcsync_close(struct inode* ino, struct file* fp)
{
    avcsync_fpd*  fpd;
    unsigned long flags;                      

    print_avcsync_function(__func__);

    fpd = fp->private_data;
    if (fpd == NULL)
    	return 0;
                     
    spin_lock_irqsave( &avcsync_lock, flags );   
    spin_unlock_irqrestore( &avcsync_lock, flags ); 
    if (fpd->allocated) {
	    fpd->allocated = FALSE;
	    avc_rq_final(&fpd->ringq);
    }
    kfree(fpd);
    
    return 0;
}


//-----------------------------------------------------------------------------
static long avcsync_ioctl(struct file* fp, uint cmd, ulong arg)
{
    struct avcsync_info*  info;
    avcsync_fpd*          fpd;
    int               ctfu_ret;              
    struct avcsync_info   work_info;
                                       
    info = &work_info;                     
    
    fpd  = (avcsync_fpd*)fp->private_data;

    print_avcsync_function(__func__);

    switch (cmd) {
    case AVCSYNC_QUEUE_CREATE:
        ctfu_ret = copy_from_user(          
                ( void * )info,             
                ( void * )arg,         
                sizeof( struct avcsync_info ));    
        if( ctfu_ret != 0 )                
        {                              
            AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ioctl:copy from user ERR\n", __func__));
            return -EFAULT;             
        }                            
        if (info == NULL) {
            AVC_DBG_FATAL(AVC_FATAL, printk("[%s] info is null.\n", __func__));
            return -EFAULT;
        }
        if (info->avcsync_nodes < 1 || AVC_RQ_COUNT_MAX < info->avcsync_nodes) {
            AVC_DBG_FATAL(AVC_FATAL, printk("[%s] avcsync_nodes(%d) mistake.\n", __func__, info->avcsync_nodes));
            return -EINVAL;
        }
        if (info->avcsync_datasize < 0 || AVC_RQ_DATASIZE_MAX < info->avcsync_datasize) {
            AVC_DBG_FATAL(AVC_FATAL, printk("[%s] avcsync_datasize(%d) mistake.\n", __func__, info->avcsync_datasize ));
            return -EINVAL;
        }
        if (fpd->allocated) {
            AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] fpd allocated.\n", __func__));
            return -EEXIST;
        }
        if (!avc_rq_init(&fpd->ringq, info->avcsync_datasize, info->avcsync_nodes)) {
            AVC_DBG_FATAL(AVC_FATAL, printk("[%s] rq init fail.\n", __func__));
            return -ENOMEM;
        }
        AVC_DBG_CONFIRM(AVC_CONFIRM, printk("[%s] QUEUE_CREATE fpd->id (%d)\n", __func__, fpd->id));
        fpd->allocated = TRUE;
    break;

    case AVCSYNC_QUEUE_GET_ID:                      
        AVC_DBG_CONFIRM(AVC_CONFIRM, printk("[%s] QUEUE_GET_ID fpd->id (%d)\n", __func__, fpd->id));
        ctfu_ret = copy_to_user(              
                ( void * )arg,                  
                ( void * )&fpd->id,
                sizeof( int ));                
        if( ctfu_ret !=0 )                       
        {                                       
            AVC_DBG_FATAL(AVC_FATAL, printk("[%s] ioctl:copy to user ERR\n", __func__)); 
            return -EFAULT;                 
        }                                   
        break;                      

    default:
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] unknown command.\n", __func__));
    return -EINVAL;
    }

    return 0;
}


//-----------------------------------------------------------------------------
static ssize_t avcsync_read(struct file* fp, char* buf, size_t cnt, loff_t* pos)
{
    int       n;
    avcsync_fpd*  fpd;
    char      bucket[AVC_RQ_BUCKET_MAX];
    unsigned long flags;                
    
    DECLARE_WAITQUEUE(wait, current);
    
    print_avcsync_function(__func__);

    fpd = (avcsync_fpd*)fp->private_data;
    if (!fpd->allocated) {
	    AVC_DBG_FATAL(AVC_FATAL, printk("[%s] fpd not allocated.\n", __func__));
	    return -ENOSPC;
    }
    
    if (cnt < 0 || AVC_RQ_BUCKET_MAX < cnt) {
	    AVC_DBG_FATAL(AVC_FATAL, printk("[%s] cnt out of range.\n", __func__));
	    return -EINVAL;
    }
    
    if (buf == NULL) {
	    AVC_DBG_FATAL(AVC_FATAL, printk("[%s] buf is NULL\n", __func__));
	    return -EINVAL;
    }

    do {
    spin_lock_irqsave( &avcsync_lock, flags );       
    if (avc_rq_is_empty(&fpd->ringq)) {
        add_wait_queue(&fpd->waitq, &wait);
        current->state = TASK_INTERRUPTIBLE;

        spin_unlock_irqrestore( &avcsync_lock, flags );  
        schedule();

        spin_lock_irqsave( &avcsync_lock, flags );      
        AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] current->state = %ld (RUNNING = %d, INTER = %d)\n",
            __func__, current->state, TASK_RUNNING, TASK_INTERRUPTIBLE));
        current->state = TASK_RUNNING;
        remove_wait_queue(&fpd->waitq, &wait);
        if (fpd->status == AVCSYNC_READ_THREAD_LEFT) {
	        AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] read thread was left.\n", __func__));

	        spin_unlock_irqrestore( &avcsync_lock, flags );  
	        return -EBADF;
        }
        if (signal_pending(current)) {
	        AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] woken up, but no data in ringqueue. (1)\n", __func__));

	        spin_unlock_irqrestore( &avcsync_lock, flags );  
	        return -EINTR;
        }
    }
    n = avc_rq_deque(&fpd->ringq, bucket, cnt);

    spin_unlock_irqrestore( &avcsync_lock, flags );  
    } while (n == -1);
    
    AVCLOG_ASSERT(0 <= n || n <= AVC_RQ_BUCKET_MAX);
    AVCLOG_ASSERT(n <= cnt);
    
    if (0 < n) {
    if (copy_to_user((void*)buf, (void*)bucket, n))
        return -EFAULT;
    }
    
    return n;
}


//-----------------------------------------------------------------------------
static ssize_t avcsync_write(struct file* fp, const char* buf, size_t cnt, loff_t* ppos)
{
    int       n;
    avcsync_fpd*  fpd;
    char      bucket[AVC_RQ_BUCKET_MAX];
    unsigned long flags;                    

    print_avcsync_function(__func__);

    fpd = (avcsync_fpd*)fp->private_data;
    if (!fpd->allocated) {
    AVC_DBG_FATAL(AVC_FATAL, printk("[%s] fpd not allocated.\n", __func__));
    return -ENOSPC;
    }

    if( fpd->ringq.data == NULL )                
    {                                            
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] fpd data null.\n", __func__));       
        return -ENOSPC;                          
    }                                            
    
    if (cnt < 0 || AVC_RQ_BUCKET_MAX < cnt) {
    AVC_DBG_FATAL(AVC_FATAL, printk("[%s] cnt out of range.\n", __func__));
    return -EINVAL;
    }
    
    if (buf == NULL) {
    AVC_DBG_FATAL(AVC_FATAL, printk("[%s] buf is null.\n", __func__));
    return -EINVAL;
    }
    
    if (0 < cnt) {
    if (copy_from_user((void*)bucket, (void*)buf, cnt))
        return -EFAULT;
    }

    spin_lock_irqsave( &avcsync_lock, flags );       
    if (avc_rq_is_full(&fpd->ringq, cnt)) {
   	 	spin_unlock_irqrestore( &avcsync_lock, flags ); 
        return -EAGAIN;                     
    }

    n = avc_rq_enque(&fpd->ringq, bucket, cnt);
    if (n == -1) {
	    AVC_DBG_FATAL(AVC_FATAL, printk("[%s] enque error.\n", __func__));
	    spin_unlock_irqrestore( &avcsync_lock, flags );  
        return -EAGAIN;                      
    }

    if (&fpd->waitq.task_list != fpd->waitq.task_list.next) {
    wake_up_interruptible(&fpd->waitq);
    }
    spin_unlock_irqrestore( &avcsync_lock, flags ); 
    
    return n;
}

//-----------------------------------------------------------------------------
static int avcsync_flush(struct file* fp, fl_owner_t id)
{
    avcsync_fpd*  fpd;
    
    print_avcsync_function(__func__);
    
    fpd = fp->private_data;
    if (fpd == NULL)
    return 0;

    if( current->tgid == fpd->tgid )
    {
	    if (&fpd->waitq.task_list != fpd->waitq.task_list.next) {
		    fpd->status = AVCSYNC_READ_THREAD_LEFT;
		    wake_up_interruptible(&fpd->waitq);
		    AVC_DBG_NORMAL(AVC_NORMAL, printk("[%s] close before waking up process.\n", __func__));
   		}
    }
    return 0;
}


//-----------------------------------------------------------------------------
static unsigned int avcsync_poll(struct file* fp, poll_table* wait)
{
    unsigned int  mask;
    avcsync_fpd*      fpd;

    print_avcsync_function(__func__);
    
    fpd = (avcsync_fpd*)fp->private_data;
    poll_wait(fp, &fpd->waitq, wait);
    
    mask = 0;
    mask |= (POLLOUT | POLLWRNORM);
    
    if (!avc_rq_is_empty(&fpd->ringq))
    mask |= (POLLIN | POLLRDNORM);
    
    return mask;
}

int avcsync_write_queue( int id,                     
                     const char* buf,            
                     size_t cnt )                
{                                               
    int           ret;                           
    avcsync_fpd*      fpd;                           
/*    spinlock_t    avcsync_lock;*/                  
    unsigned long flags;                        

    print_avcsync_function(__func__);

    fpd = NULL;                                 

    spin_lock_irqsave( &avcsync_lock, flags );      
    ret = avc_rq_id_get( id, &fpd );                
    spin_unlock_irqrestore( &avcsync_lock, flags );  

    if( ret == -1 )                              
    {                                            
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] id is not found.\n", __func__));         
        return -ENOSPC;                          
    }                                            

    if( fpd->allocated == FALSE )                
    {                                            
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] fpd not allocated.\n", __func__));       
        return -ENOSPC;                          
    }                                            

    if( fpd->ringq.data == NULL )                
    {                                            
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] fpd data null.\n", __func__));       
        return -ENOSPC;                          
    }                                            
    
    if(( cnt < 0 ) || ( AVC_RQ_BUCKET_MAX < cnt ))  
    {                                            
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] cnt out of range.\n", __func__));        
        return -EINVAL;                          
    }                                            
    
    if( buf == NULL )                           
    {                                            
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] buf is null.\n", __func__));             
        return -EINVAL;                          
    }                                            

    spin_lock_irqsave( &avcsync_lock, flags );       
    ret = avc_rq_is_full( &fpd->ringq, cnt );        
                                                 
    if( ret == TRUE )                            
    {                                            
        spin_unlock_irqrestore( &avcsync_lock,       
                                flags );         
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] queue is full.\n", __func__));           
        return -EAGAIN;                          
    }                                            

    ret = avc_rq_enque( &fpd->ringq, buf, cnt );     
                                                 
    if( ret == -1 )                              
    {                                            
        spin_unlock_irqrestore( &avcsync_lock,       
                                flags );         
        AVC_DBG_FATAL(AVC_FATAL, printk("[%s] enque error.\n", __func__));                               
        return -EAGAIN;                    
    }                                      

    if( &fpd->waitq.task_list              
        != fpd->waitq.task_list.next )     
    {                                      
        wake_up_interruptible( &fpd->waitq ); 
    }                                      
    spin_unlock_irqrestore( &avcsync_lock, flags );
    
    return ret;                            
                                           
}                                              

EXPORT_SYMBOL(avcsync_write_queue); /* 2.6 taiou */


//module_init(avcsync_init);
subsys_initcall(avcsync_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);

