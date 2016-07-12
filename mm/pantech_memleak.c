/*
 * mm/pantech_memleak.c
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/export.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/thread_info.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/workqueue.h>
#include <mach/pantech_memlog.h>


/*
 * leak info configuration and common defines.
 * 
 * STANDARD_OLD_AGE_SEC : 12 HR(43200 sec)
 * SCAN_DELAY_SEC
 *   it should be set to value 
 *   that makes the remainder zero when STANDARD_OLD_AGE_SEC is divided by it.
 */
#define MAX_TRACE				1024
#define SECS_FIRST_SCAN			30
#define SCAN_DELAY_SEC			90	
#define STANDARD_OLD_AGE_SEC	43200
#define STANDARD_OLD_AGE		(STANDARD_OLD_AGE_SEC/STANDARD_OLD_AGE_SEC)
#define STANDARD_FEW_PAGES		(2*4096)	// 2 pages

atomic_t memleak_trace_index = ATOMIC_INIT(-1);
static atomic_t memleak_trace_overflowed = ATOMIC_INIT(0);
static struct workqueue_struct* memscan_wq;
static struct delayed_work scan_work_struct;
static int scan_enable = 0;
unsigned int scan_delay = SCAN_DELAY_SEC;  // just to make symbol(for ramdump)

static void memory_scan_work(struct work_struct *work); 


static const struct file_operations memleak_fops = {
	.owner		= THIS_MODULE,
	.open		= NULL,
	.read		= NULL,
	.write		= NULL,
	.llseek		= NULL,
	.release	= NULL,
};

static void increase_dtask_age(void)
{
	struct dead_task_struct *t;
	unsigned long flag;

	spin_lock_irqsave(&dtask_list_lock, flag);
	for_each_dtask(t) {
		t->age++;
	}
	spin_unlock_irqrestore(&dtask_list_lock, flag);
	mb();
}

static void gather_task_meminfo(unsigned int index)
{
	struct task_struct *tsk = current;
	struct task_struct *group_leader;
	unsigned long flags;

	read_lock_irqsave(&tasklist_lock, flags);

	do_each_thread(group_leader, tsk) {
		tsk->leak_detector.alloc_trace[index]=tsk->leak_detector.total_alloc_size;
	}while_each_thread(group_leader, tsk);

	read_unlock_irqrestore(&tasklist_lock, flags);
	mb();
}

static int dtask_is_old(struct dead_task_struct* t)
{
	return ((t->age-1) >= min(STANDARD_OLD_AGE, atomic_read(&memleak_trace_index)));
}

static int dtask_for_few_pages(struct dead_task_struct* t)
{
	return (t->mem_info.total_alloc_size <= STANDARD_FEW_PAGES);
}

/* 
   clean dtasks which are old enough and in charge of only few pages(less than 2 Pages) 
 */
static void do_clean_dtask_list(void)
{
	struct dead_task_struct* t;
	unsigned long flag;

	spin_lock_irqsave(&dtask_list_lock, flag);
	for_each_dtask_rev(t) {
		if( dtask_is_old(t) ) {
			if ( dtask_for_few_pages(t) )
				free_dtask(t);
			
			continue;
		}
		break;
	}

	spin_unlock_irqrestore(&dtask_list_lock, flag);
}


static void pantech_memleak_detector(void)
{
	unsigned int index;

	index = atomic_inc_return(&memleak_trace_index) & (MAX_TRACE - 1);
	if (index == (MAX_TRACE - 1))
		atomic_set(&memleak_trace_overflowed, 1);

	printk("[PANTECH_MEMLEAK] SCANNING.... index=%d, overflowed[%d]\n",index, atomic_read(&memleak_trace_overflowed));
	
	gather_task_meminfo(index);
	increase_dtask_age();

	if( need2clean_dtask_list() )
	{
		do_clean_dtask_list();
		set_clean_dtask_list_done();
	}
}


static void memory_scan_work(struct work_struct* work)
{
	struct delayed_work *delayed_work;
	unsigned long delay_time;

	if( !scan_enable )
		return;

	pantech_memleak_detector();

	if( scan_enable )
	{
		delayed_work = to_delayed_work(work);
		delay_time = msecs_to_jiffies(SCAN_DELAY_SEC * 1000);
		queue_delayed_work(memscan_wq, delayed_work, delay_time);
	}
}

static int __devinit pantech_memleak_probe(void)
{
	unsigned long first_scan_delay;

	memscan_wq = alloc_workqueue("mem_scan_wq", 0, 0); 

	if (!memscan_wq) {
		pr_err("Failed to allocate memory scan workqueue\n");
		return -EIO;
	}

	first_scan_delay = msecs_to_jiffies(SECS_FIRST_SCAN*1000);
	INIT_DELAYED_WORK(&scan_work_struct, memory_scan_work);
	queue_delayed_work(memscan_wq, &scan_work_struct, first_scan_delay);

	scan_enable = 1;
	mb();

	return 0;
}

static int __init pantech_memleak_init(void)
{
	struct dentry *dentry;
	
	dentry = debugfs_create_file("pantech_memleak", S_IRUGO, NULL, NULL, &memleak_fops);
	
	if (!dentry)
		printk("Failed to create the debugfs pantech_memleak file\n");

	// probe function forced
	if( pantech_memleak_probe() )
		pr_err("Failed to initialize pantech memory leak_detector\n");

	else
		printk("Pantech memory leak detector initialized\n");




	return 0;
}
late_initcall(pantech_memleak_init);
