
/* arch/arm/mach-msm/apanic_pantech.c
 *
 * Copyright (C) 2011 PANTECH, Co. Ltd.
 * based on drivers/misc/apanic.c
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      See the
 * GNU General Public License for more details.
 *
 */
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/preempt.h>
#include <asm/cacheflush.h>
#include <asm/system.h>
#include <linux/fb.h>
#include <linux/time.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/nmi.h>
#include <mach/msm_iomap.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/console.h>
#include <linux/irq.h>
#include <linux/spinlock.h>
#include <linux/bitops.h>
#include <asm/processor.h>
#include <mach/pantech_memlog.h>


struct dead_task_struct init_dtask = 
{
	.dead_task_list = LIST_HEAD_INIT(init_dtask.dead_task_list),
};
struct dead_task_struct dtask_cache[MAX_DAED_TASKS];
DEFINE_SPINLOCK(dtask_list_lock);

static long dtask_bitmap[BITMAP_SIZE] = {0};
static DEFINE_SPINLOCK(dtask_bitmap_lock);
unsigned int task_head_offset;
unsigned int thread_group_offset;
unsigned int page_different = 0;

atomic_t clean_dtask_list = ATOMIC_INIT(0);
atomic_t second_ids = ATOMIC_INIT(1);

#define if_task_is_page_onwer(t, p)		\
			( (t)->pid == (p)->onwer_info.pid && \
			  (t)->lt_info.sid == (p)->onwer_info.sid )

#define if_dtask_is_page_onwer(dt, p)		\
			( (dt)->pid == (p)->onwer_info.pid && \
			  (dt)->sid == (p)->onwer_info.sid )

#define bitmap_not_available(bitmap)	((bitmap) == (-1))  // 0xffffffff
#define mark_bitmap(bitmap, bit)		((bitmap) |= (1UL << bit));
#define clear_bitmap(bitmap, bit)		((bitmap) &= ~(1UL << bit));

#define dtask_cache_being_used(dtask)	((dtask)->addtl_info != 0 )


#if 0
static struct rb_root dtask_root = RB_ROOT;

// rbtree operation
static void insert_dtask_to_tree(struct dead_task_struct* dtask)
{
	struct rb_node **p = &dtask_root.rb_node;
	struct rb_node *parent = NULL;
	struct dead_task_struct *tmp_dtask;

	while(*p) {

		parent = *p; 
		tmp_dtask = rb_entry(parent, struct dead_task_struct, rb_node);

			// smaller
		if (dtask->pid < tmp_dtask->pid)
			p = &(*p)->rb_left;
		else if(dtask->pid > tmp_dtask->pid) 
			p = &(*p)->rb_right;
		else
			panic("same pid on dead task tree\n");
	}

	rb_link_node(&dtask->rb_node, parent, p);
	rb_insert_color(&dtask->rb_node, &dtask_root);

}


static struct dead_task_struct* find_dtask_from_tree(pid_t pid)
{
	struct rb_node *n = dtask_root.rb_node;
	struct dead_task_struct *dtask;

	while(n) {
		dtask = rb_entry(n, struct dead_task_struct, rb_node);
		if (pid < dtask->pid)
			n = n->rb_left;
		else if (pid > dtask->pid)
			n = n->rb_right;
		else
			return dtask;
	}

	return NULL;
}

static inline void delete_dtask_from_tree(struct dead_task_struct* dtask)
{
	rb_erase(&dtask->rb_node, &dtask_root);
	RB_CLEAR_NODE(&dtask->rb_node);
}
#endif

static int get_available_dtask_cache_index(void)
{
	int index = -1;
	int i,j;
	unsigned long flags;

	spin_lock_irqsave(&dtask_bitmap_lock, flags);
	for(i = 0; i < BITMAP_SIZE ; i++)
	{
		if (bitmap_not_available(dtask_bitmap[i]))
				continue;

		for( j=0; j<BITS_PER_LONG ; j++)
		{
			if (!(dtask_bitmap[i] & BIT(j)))
			{
				mark_bitmap(dtask_bitmap[i], j);			
				index = (i * BITS_PER_LONG) + j;
				goto out;
			}

		}
	}
out:
	spin_unlock_irqrestore(&dtask_bitmap_lock, flags);

	return index;
}

struct dead_task_struct* alloc_dtask(void)
{
	struct dead_task_struct* dtask = NULL;
	int index;

	index = get_available_dtask_cache_index();
	if(index < 0)
	{
		printk(KERN_ERR "[alloc_dtask] failed to alloc dtask cache. it's full\n");
		goto out;
	}

	dtask = &dtask_cache[index];

	if (dtask_cache_being_used(dtask))
	{
		dtask = NULL;
		goto out;
	}
	memset(dtask, 0, sizeof(struct dead_task_struct));
	INIT_LIST_HEAD(&dtask->dead_task_list);
	rb_init_node(&dtask->rb_node);
	dtask->addtl_info = (index+1);
	rwlock_init(&dtask->mem_info.ld_lock);

out:
	return dtask;
}

static void free_dtask_cache(int cache_index)
{
	int index = cache_index >> LONG_BIT_ORDER;
	int bit = (cache_index & ((1UL << LONG_BIT_ORDER)-1));
	unsigned long flags;

	spin_lock_irqsave(&dtask_bitmap_lock, flags);

	/* just clear that bit on bitmap */
	clear_bitmap(dtask_bitmap[index], bit);

	spin_unlock_irqrestore(&dtask_bitmap_lock, flags);
}


void free_dtask(struct dead_task_struct* dtask)
{
	int cache_index = dtask->addtl_info - 1;

	remove_from_dtask_list(dtask);

	dtask->addtl_info = 0;
	free_dtask_cache(cache_index);
}

void backup_mem_info_if_need(struct task_struct* tsk)
{
	struct dead_task_struct* dtask;
	unsigned long flags;

	if( tsk->leak_detector.total_alloc_size == 0 )
		return ;

	dtask =	alloc_dtask();

	if( !dtask )
	{
		printk(KERN_ERR "[backup_mem_info_if_need] ERROR! no more dead task cache\n");
		return;
	}

	dtask->pid = tsk->pid;
	dtask->sid = tsk->lt_info.sid;
	dtask->asid = tsk->lt_info.backup_asid;
	memcpy(dtask->comm, tsk->comm, TASK_COMM_LEN);
	memcpy(dtask->leader, tsk->lt_info.backup_group_leader, TASK_COMM_LEN);
	dtask->mem_info.total_alloc_size = tsk->leak_detector.total_alloc_size;
	dtask->mem_info.total_highmem_alloc_size = tsk->leak_detector.total_highmem_alloc_size;
	dtask->mem_info.total_lowmem_alloc_size = tsk->leak_detector.total_lowmem_alloc_size;

	spin_lock_irqsave(&dtask_list_lock, flags);
//	insert_dtask_to_tree(dtask);
	add_to_dtask_list(dtask);
	spin_unlock_irqrestore(&dtask_list_lock, flags);

	// If more than 80% dtask cacheds are being used, then it need cleaning the dtask list
	if ( dtask->addtl_info >= TOO_MANY_DATSK_ALRAM_THR )
		atomic_set(&clean_dtask_list, 1);  

	mb();
}

static void __write_mem_usage_to_dtask(struct dead_task_struct *dtask, unsigned int nr_pages, int high)
{
	unsigned int size =  (nr_pages*PAGE_SIZE);

	mb();
	dtask->mem_info.total_alloc_size -= size;

	if( high )
		dtask->mem_info.total_highmem_alloc_size -= size;
	else
		dtask->mem_info.total_lowmem_alloc_size -= size;
	
	mb();
}

static void __write_mem_usage( struct leak_detector_struct *usage, unsigned int nr_pages, int high, int type)
{
	int size = (int)(nr_pages*PAGE_SIZE);
	unsigned long flags;

	mb();
	size = ((type == FREE) ? -size : size );

	write_lock_irqsave(&usage->ld_lock, flags);

	usage->total_alloc_size += size;
	if(high)
		usage->total_highmem_alloc_size += size;
	else
		usage->total_lowmem_alloc_size += size;

	if( usage->total_alloc_size >= 0xFFF00000 ||
			usage->total_highmem_alloc_size >= 0xFFF00000 ||
			usage->total_lowmem_alloc_size >= 0xFFF00000 )
	{
		printk( KERN_ERR "[%s] task: 0x%x, nr_pages(%d), high(%d), type(%d)\n", 
								__func__ , (unsigned int)current, nr_pages, high, type);
		BUG_ON(1);
	}

	// For Deallocation
	if(type == FREE)
	{
		usage->free_cnt++;
		usage->free_page_cnt += nr_pages;
	}

	// For Allocation
	else
	{
		usage->alloc_cnt++;
		usage->alloc_page_cnt += nr_pages;
		if( size > usage->max_alloc_size )
			usage->max_alloc_size = size;
	
		if( usage->hi_watermark <= usage->total_alloc_size )
			usage->hi_watermark = usage->total_alloc_size;
	}
	write_unlock_irqrestore(&usage->ld_lock, flags);

	mb();
}


inline void write_alloc_mem_usage( struct leak_detector_struct *usage, unsigned int nr_pages, int high)
{
	__write_mem_usage(usage, nr_pages, high, ALLOC);
}

inline void write_free_mem_usage( struct leak_detector_struct *usage, unsigned int nr_pages, int high)
{
	__write_mem_usage(usage, nr_pages, high, FREE);
}


static int search_task_and_mark(struct page* page, unsigned int nr_pages)
{
	struct task_struct* p;
	struct task_struct* t;
	pid_t glpid = page->onwer_info.group_leader_pid;
	int high = is_highmem(page_zone(page));
	unsigned long flags;
	int task_found = 0;

	read_lock_irqsave(&tasklist_lock, flags);
	for_each_process(p) {
		if(p->pid == glpid) {
			t = p;
			do {
				if( if_task_is_page_onwer(t, page ))
				{
					task_found = 1;
					write_free_mem_usage(&t->leak_detector, nr_pages, high);
					goto out_tasklist_loop;
				}
			} while_each_thread(p, t);
			break;
		}
	}
out_tasklist_loop:
	read_unlock_irqrestore(&tasklist_lock, flags);

	return task_found;
}

static void search_dead_task_and_mark(struct page* page, unsigned int nr_pages)
{
	struct dead_task_struct* t;
	int high = is_highmem(page_zone(page));
	unsigned long flags;

	spin_lock_irqsave(&dtask_list_lock, flags);

#if 0
	t = find_dtask_from_tree(pid);

	if( t )
	{
		__write_mem_usage_to_dtask(t, nr_pages, high);
		if (t->mem_info.total_alloc_size <= 0)
		{
			delete_dtask_from_tree(t);
			goto out;
		}
	}
#else
	for_each_dtask(t) {
		if( if_dtask_is_page_onwer (t, page)) {
			__write_mem_usage_to_dtask(t, nr_pages, high);
			if (t->mem_info.total_alloc_size == 0)
			{
				free_dtask(t);
			}
			break;
		}
	}
#endif
	spin_unlock_irqrestore(&dtask_list_lock, flags);

}


inline int need_to_backup_asid(struct task_struct* tsk)
{
	return (tsk->lt_info.backup_asid == 0);
}

inline void task_backup_asid(struct task_struct *tsk)
{
	tsk->lt_info.backup_asid = (tsk->mm ? tsk->mm->context.id : KERNEL_ASID);
}


void writeLog_alloc_info(unsigned int order, struct page *page)
{
	struct task_struct *tsk = current;
	unsigned int nr_pages = (1 << order);
	int high = is_highmem(page_zone(page));
	int i;

#ifdef CONFIG_CPU_HAS_ASID
	if (unlikely(need_to_backup_asid(tsk)))
		task_backup_asid(tsk);
#endif
	write_alloc_mem_usage( &tsk->leak_detector, nr_pages, high);

	for(i=0 ; i < nr_pages ; i++)
	{
		(page + i)->onwer_info.pid = tsk->pid;
		(page + i)->onwer_info.sid = tsk->lt_info.sid;  
		(page + i)->onwer_info.group_leader_pid = tsk->group_leader->pid;
		(page + i)->onwer_info.magic = PAGE_MAGIC;  
	}
}


void writeLog_free_info(unsigned int order, struct page *page)
{
	unsigned int nr_pages = (1 << order);
	pid_t orig_pid;
	int i;

	if (unlikely(page->onwer_info.magic != PAGE_MAGIC)) 
		return ;
	
	if (unlikely(page->onwer_info.magic == FREED_PAGE_MAGIC))
	{
		printk(KERN_ERR "[%s] page free twice\n", __func__);
		BUG_ON(1);
	}

	orig_pid = page->onwer_info.pid;

	for(i=0 ; i < nr_pages ; i++)
	{
		(page + i)->onwer_info.magic = FREED_PAGE_MAGIC;
		if( orig_pid != (page + i)->onwer_info.pid )
			page_different++;
	}
	mb();

 	if(	!search_task_and_mark( page , nr_pages ) )
	{
		search_dead_task_and_mark( page, nr_pages );
	}
}

void init_task_leak_info(struct task_struct* tsk)
{
	memset(&tsk->leak_detector, 0, sizeof(struct leak_detector_struct));
	tsk->leak_detector.magic = LEAK_LOG_MAGIC;
	rwlock_init(&tsk->leak_detector.ld_lock);

	memset(&tsk->lt_info, 0, sizeof(tsk->lt_info));
	tsk->lt_info.sid = atomic_inc_return(&second_ids);
}

int __init pantech_memlog_init(void)
{
	task_head_offset = offsetof(struct task_struct, tasks);
	thread_group_offset = offsetof(struct task_struct, thread_group);

	printk(KERN_INFO "pantech memory alloc/free log init \n");
	return 0;
}

module_init(pantech_memlog_init);

MODULE_AUTHOR("Lee Horim <horim.lee@pantech.com>");
MODULE_DESCRIPTION("Pantech Memory log for memory leak detection");
MODULE_LICENSE("GPL");
