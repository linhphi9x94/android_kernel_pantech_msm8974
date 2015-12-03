#ifndef __PANTECH_SYS_INFO_H
#define __PANTECH_SYS_INFO_H


#define MMU_SCRIPT_BUF_SIZE 512
#define P_INFO_BUFFER_SIZE 0x80

/*******************************************************************************
**  VENDOR2 Shared memory structure for log info
*******************************************************************************/
typedef struct _pantech_log_header{
    unsigned int magic;
    unsigned int version;
    unsigned int *klog_buf_address;
    unsigned int *klog_end_idx;
    unsigned int klog_size;
    unsigned int *mlogcat_buf_address;
    unsigned int *mlogcat_w_off;
    unsigned int mlogcat_size;
    unsigned int *slogcat_buf_address;
    unsigned int *slogcat_w_off;
    unsigned int slogcat_size;
    unsigned int *rlogcat_buf_address;
    unsigned int *rlogcat_w_off;
    unsigned int rlogcat_size;
    
	char mmu_cmm_script[MMU_SCRIPT_BUF_SIZE];
	int mmu_cmm_size;    
	unsigned int *pantech_dbg_addr;
	unsigned int pantech_dbg_size;

    unsigned int start_phone_info_log;
    char information[P_INFO_BUFFER_SIZE];
    unsigned int end_phone_info_log;

}pantech_log_header;

#endif
