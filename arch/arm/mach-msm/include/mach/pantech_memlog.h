#ifndef __PANTECH_MEMLOG_H
#define __PANTECH_MEMLOG_H


#ifdef CONFIG_PANTECH_MEM_LEAK_TRACE

#include <linux/types.h>
#include <linux/mm_types.h>

#define PAGE_MAGIC					0xABABABAB
#define FREED_PAGE_MAGIC			0xCDCDCDCD
#define LEAK_LOG_MAGIC      		0x89ABCDEF
#define KERNEL_ASID   				0x270F		//Decimal : 9999

#define MAX_DAED_TASKS      		4096
#define TOO_MANY_DATSK_ALRAM_THR	3275 	// About 80% dtask caches
#define BITMAP_SIZE		    		BITS_TO_LONGS(MAX_DAED_TASKS) 
#define LONG_BIT_ORDER      		5


enum { ALLOC, FREE };

struct addtl_task_info {
	char backup_group_leader[16];
	unsigned int backup_asid;
	int sid;
};

struct page_onwer_info {
    pid_t pid;
    pid_t group_leader_pid;   
	int sid;
    unsigned int magic; 
};


struct leak_detector_struct{
    rwlock_t ld_lock;
    unsigned int alloc_cnt;
    unsigned int free_cnt;
    unsigned int alloc_page_cnt;
    unsigned int free_page_cnt;
    unsigned int total_alloc_size;
    unsigned int total_lowmem_alloc_size;
    unsigned int total_highmem_alloc_size;
    unsigned int max_alloc_size;
	unsigned int hi_watermark;
    unsigned int magic;
    unsigned int alloc_trace[1024];
};

struct mem_summary_info
{
	rwlock_t ld_lock;
	unsigned int total_alloc_size;
	unsigned int total_lowmem_alloc_size;
	unsigned int total_highmem_alloc_size;
};


struct dead_task_struct { 
	struct list_head dead_task_list;
	struct rb_node rb_node;
	pid_t pid; 
	int sid;
	unsigned int asid;
	char comm[16];
	char leader[16];
	volatile int addtl_info;
	struct mem_summary_info mem_info;
	unsigned int age;

};

extern atomic_t clean_dtask_list; 
extern struct dead_task_struct init_dtask;
extern spinlock_t dtask_list_lock;

extern void free_dtask(struct dead_task_struct* dtask);
extern void backup_mem_info_if_need(struct task_struct* tsk);
extern void init_task_leak_info(struct task_struct* tsk);

// LOG API
extern void writeLog_alloc_info(unsigned int order, struct page *page);
extern void writeLog_free_info(unsigned int order, struct page *page);

#define need2clean_dtask_list()				(atomic_read(&clean_dtask_list) == 1)
#define set_clean_dtask_list_done()			(atomic_set(&clean_dtask_list, 0))



#define next_dtask(t) \
	list_entry_rcu((t)->dead_task_list.next, struct dead_task_struct, dead_task_list)

#define for_each_dtask(t) \
	for (t = &init_dtask ; (t = next_dtask(t)) != &init_dtask ; ) 

#define prev_dtask(t) \
	list_entry_rcu((t)->dead_task_list.prev, struct dead_task_struct, dead_task_list)

#define for_each_dtask_rev(t) \
	for (t = &init_dtask ; (t = prev_dtask(t)) != &init_dtask ; ) 

#define add_to_dtask_list(t)	\
	list_add_rcu(&(t)->dead_task_list, &init_dtask.dead_task_list)

#define remove_from_dtask_list(t)	\
	list_del_rcu(&(t)->dead_task_list)

#endif	// CONFIG_PANTECH_MEM_LEAK_TRACE

#endif // __PANTECH_MEMLOG_H
