#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include "fts_ts.h"


#define FTS_FW_REG_WRITE	0xB0
#define FTS_FW_REG_READ		0xB2

typedef enum {	
	
	READ_TOUCH_ID = 101,
	
	APPLY_TOUCH_CONFIG = 501,
	DIAG_DEBUG = 502,
	RESET_TOUCH_CONFIG = 503,
	GET_TOUCH_CONFIG = 504,
	SET_TOUCH_CONFIG = 505,
	READ_ITO_TYPE = 506,
	ATMEL_GET_REFERENCE_DATA = 508,
	TOUCH_CHARGER_MODE = 701,
	TOUCH_IOCTL_READ_LASTKEY=1001,	
	TOUCH_IOCTL_DO_KEY,	
	TOUCH_IOCTL_RELEASE_KEY, 
	TOUCH_IOCTL_CLEAN,
	TOUCH_IOCTL_DEBUG,
	TOUCH_IOCTL_RESTART,
	TOUCH_IOCTL_PRESS_TOUCH,
	TOUCH_IOCTL_RELEASE_TOUCH,
	TOUCH_IOCTL_CHARGER_MODE,
	TOUCH_IOCTL_EARJACK_MODE,
	POWER_OFF,
	TOUCH_IOCTL_DELETE_ACTAREA = 2001,
	TOUCH_IOCTL_RECOVERY_ACTAREA,
	TOUCH_IOCTL_SENSOR_X = 2005,
	TOUCH_IOCTL_SENSOR_Y,
	TOUCH_IOCTL_CHECK_BASE,
	TOUCH_IOCTL_READ_IC_VERSION,
	TOUCH_IOCTL_READ_FW_VERSION,
	TOUCH_IOCTL_START_UPDATE,
	TOUCH_IOCTL_SELF_TEST,
	TOUCH_IOCTL_DIAGNOSTIC_MIN_DEBUG,
	TOUCH_IOCTL_DIAGNOSTIC_MAX_DEBUG,
	TOUCH_IOCTL_DIAG_DELTA = 2014,
	TOUCH_IOCTL_SELF_TEST_RESULT = 2015,
	TOUCH_IOCTL_INIT = 3001,
	TOUCH_IOCTL_OFF  = 3002,
	TOUCH_IOCTL_EVENT_AUTOCAL_DISABLE  = 5001,
	TOUCH_IOCTL_MULTI_TSP_OBJECT_SEL  = 6001,
	TOUCH_IOCTL_SUSPEND,
	TOUCH_IOCTL_RESUME,	
	TOUCH_IOCTL_MULTI_TSP_OBJECT_GET,
	TOUCH_IOCTL_HALL_IC_GPIO_GET,
	TOUCH_IOCTL_WIFI_DEBUG_APP_ENABLE	= 7004,
	TOUCH_IOCTL_WIFI_DEBUG_APP_DISABLE	= 7005,	
	TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_ENABLE = 7100,
	TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_DISABLE = 7101,
	TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_GET = 7102,	
	TOUCH_IOCTL_MODEL_COLOR_GET = 7200,
	TOUCH_IOCTL_TOUCH_MODE = 7201,
#ifdef PAN_KNOCK_ON
	TOUCH_IOCTL_KNOCKON_SET_ENABLE = 7400,
	TOUCH_IOCTL_KNOCKON_GET_ENABLE = 7401,

	TOUCH_IOCTL_KNOCKON_SET_SCAN_FRAME_RATE	 = 7415,
	TOUCH_IOCTL_KNOCKON_SET_THRESHOLD		 = 7416,
	TOUCH_IOCTL_KNOCKON_SET_1ST_LEAVE_TOUT	 = 7417,
	TOUCH_IOCTL_KNOCKON_SET_2ND_ENTER_TOUT	 = 7418,
	TOUCH_IOCTL_KNOCKON_SET_2ND_LEAVE_TOUT	 = 7419,	

	//	for STMicro get
	TOUCH_IOCTL_KNOCKON_GET_SCAN_FRAME_RATE	 = 7425,
	TOUCH_IOCTL_KNOCKON_GET_THRESHOLD		 = 7426,
	TOUCH_IOCTL_KNOCKON_GET_1ST_LEAVE_TOUT	 = 7427,
	TOUCH_IOCTL_KNOCKON_GET_2ND_ENTER_TOUT	 = 7428,
	TOUCH_IOCTL_KNOCKON_GET_2ND_LEAVE_TOUT	 = 7429,
#endif

} TOUCH_IOCTL_CMD;

static struct fts_ts_info *io_info=NULL;
extern int touch_error_cnt;
extern char* touch_error_info;
extern int pan_debug_state;



struct pan_io_data {
	int vendor_id;
	int model_id;
	int type_id;
	int raw_data_buffer[493];
#ifdef PAN_KNOCK_ON
	int knock_on_enable;
	int knock_on_framerate;
	int knock_on_threshold;
	int knock_on_1st_leave;
	int knock_on_2nd_enter;
	int knock_on_2nd_leave;
#endif
};
static struct pan_io_data pan_io_data = {
	.vendor_id = 1,
#if defined(CONFIG_MACH_MSM8974_EF63S)
	.model_id = 660,
#elif defined(CONFIG_MACH_MSM8974_EF63K)
	.model_id = 670,
#elif defined(CONFIG_MACH_MSM8974_EF63L)
	.model_id = 680,
#else
	.model_id = 100,
#endif
	.type_id = 0,
	.raw_data_buffer={0,},
#ifdef PAN_KNOCK_ON
	.knock_on_enable = 0,
	.knock_on_framerate = 0,
	.knock_on_threshold = 0,
	.knock_on_1st_leave = 0,
	.knock_on_2nd_enter = 0,
	.knock_on_2nd_leave = 0,
#endif
};

#ifdef CONFIG_KEYBOARD_TC370	
int pan_tm_key_resume(void);
int pan_tm_key_suspend(void);
void pan_tm_set_mode(int mode);
void pan_tm_set_cover_state(int state);
#endif

int ioctl_debug(unsigned long arg);
unsigned char fts_read_raw_data(unsigned int Col, unsigned int Row);
void raw_data_print(void);


#ifdef TOUCH_MONITOR
char printproc_buf[1024];
char touch_info_vendor[] = "stmicro";
char touch_info_chipset[] = "pan_touch_ef63";

CircularBuffer cb;
spinlock_t cb_spinlock;


void cbInit(CircularBuffer *cb, int size) {
	cb->size  = size + 1; /* include empty elem */
	cb->start = 0;
	cb->end   = 0;
	cb->elems = (char*)kmalloc(cb->size * sizeof(char), 
							   GFP_KERNEL | GFP_ATOMIC);
}

void cbFree(CircularBuffer *cb) {
	kfree(cb->elems); /* OK if null */ 
}

int cbIsFull(CircularBuffer *cb) {
	return (cb->end + 1) % cb->size == cb->start; 
}

int cbIsEmpty(CircularBuffer *cb) {
	return cb->end == cb->start; 
}

void cbWrite(CircularBuffer *cb, char *elem) {
	cb->elems[cb->end] = *elem;
	cb->end = (cb->end + 1) % cb->size;
	if (cb->end == cb->start)
		cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
}

void cbRead(CircularBuffer *cb, char *elem) {
	*elem = cb->elems[cb->start];
	cb->start = (cb->start + 1) % cb->size;
}

int read_log(char *page, char **start, off_t off, int count, 
			 int *eof, void *data_unused) {
	char *buf;
	char elem = {0};
	buf = page;

	spin_lock(&cb_spinlock);
	while (!cbIsEmpty(&cb)) {
		cbRead(&cb, &elem);
		buf += sprintf(buf, &elem);
	}
	spin_unlock(&cb_spinlock);
	*eof = 1;
	return buf - page;
}

int read_touch_info(char *page, char **start, off_t off, 
					int count, int *eof, void *data_unused) {
	char *buf;
	buf = page;
	buf += sprintf(buf, "Vendor: \t%s\n", touch_info_vendor);
	buf += sprintf(buf, "Chipset: \t%s\n", touch_info_chipset);

	*eof = 1;
	return buf - page;
}

void printp(const char *fmt, ...) {
	int count = 0;
	int i;
	va_list args;
	spin_lock(&cb_spinlock);
	va_start(args, fmt);
	count += vsnprintf(printproc_buf, 1024, fmt, args);
	for (i = 0; i<count; i++) {
		cbWrite(&cb, &printproc_buf[i]);
	}
	va_end(args);
	spin_unlock(&cb_spinlock);
}

void init_proc(void) { 
	int testBufferSize = 1024;

	struct proc_dir_entry *touch_log_ent;
	struct proc_dir_entry *touch_info_ent;
	
	touch_log_ent = create_proc_entry("touchlog", S_IFREG|S_IRUGO, 0); 
	touch_log_ent->read_proc = read_log;
	
	touch_info_ent = create_proc_entry("touchinfo", S_IFREG|S_IRUGO, 0); 
	touch_info_ent->read_proc = read_touch_info;

	spin_lock_init(&cb_spinlock);
	cbInit(&cb, testBufferSize);
}

void remove_proc(void) {
	remove_proc_entry("touchlog", 0);
	remove_proc_entry("touchinfo", 0);
	cbFree(&cb);
}
#endif

#ifdef PAN_TOUCH_EVENT_WITH_HALL_IC
void pan_hall_ic_event(int state){
  if(io_info->touch_mode_state.smart_cover == state)
    return;
  dbg_cr("pan_hall_ic_event state -> %d\n",state);
  io_info->touch_mode_state.smart_cover = (u8)state;
  mod_timer(&io_info->touch_mode_state.check_smart_cover, jiffies + msecs_to_jiffies(PAN_SMART_COVER_CHECK_TIMER));
}
#endif
void pan_hallic_ui_sync_timer_func(unsigned long data)
{
  if(io_info->touch_mode_state.smart_cover != io_info->touch_mode_state.smart_cover_from_ui){
    dbg_cr(" pan_hallic_ui_sync_timer_func is called. And smart cover state is Mismatched. ic -> %d, ui -> %d \n",
    io_info->touch_mode_state.smart_cover,io_info->touch_mode_state.smart_cover_from_ui);
    io_info->touch_mode_state.smart_cover=io_info->touch_mode_state.smart_cover_from_ui;
  }else{
    dbg_cr(" pan_hallic_ui_sync_timer_func is called\n");
  
  }	
}



void raw_data_print(void)
{
	int i,j;
	for (i=0; i<FTS_TX_LENGTH; i++) {
		for(j=0; j<FTS_RX_LENGTH; j++) {
			printk("%06d ", pan_io_data.raw_data_buffer[i*FTS_RX_LENGTH+j]);
		}
		printk("\n");
	}
}

//Mutual Raw value
unsigned char fts_read_raw_data(unsigned int Col, unsigned int Row)
{
	unsigned char regAdd[3] = {0xD0, 0x00, 0x02};
	unsigned char ReadData[FTS_TX_LENGTH*FTS_RX_LENGTH*2] = {0};
	unsigned int FrameAddress = 0;
	unsigned int StartAddress = 0;
	unsigned int EndAddress = 0;
	unsigned int TotalLength = 0;
	unsigned int remained = 0;
	unsigned int writeAddr = 0;
	unsigned int readbytes = 0xFF;
	unsigned int count = 0;
	int ret = 0;

	TotalLength = Col * Row * 2;
	ret = io_info->fts_read_reg(io_info,regAdd, 3, &ReadData[0], 4);
	
	if(ret<0){ 
	  dbg_cr(" ret(%d) failed \n",ret);
		return ret;
	}

	FrameAddress = ReadData[0] +  (ReadData[1] << 8) + 58;
	StartAddress = FrameAddress;
	EndAddress = FrameAddress + TotalLength;

	dbg_op(" [FTS] Frame start Address = %x\n", StartAddress);
	dbg_op(" [FTS] End Address = %x\n", EndAddress);

	remained = TotalLength;

	mdelay(10);
	memset(&ReadData[0], 0x0, sizeof(ReadData));
	
	for(writeAddr = StartAddress; 
		writeAddr < EndAddress ; writeAddr += READ_CHUNK_SIZE)
	{
		regAdd[1] = (writeAddr >> 8) & 0xFF;
		regAdd[2] = writeAddr & 0xFF;

		if(remained >= READ_CHUNK_SIZE)
			readbytes = READ_CHUNK_SIZE;
		else
			readbytes = remained;

		ret = io_info->fts_read_reg(io_info, regAdd, 3, 
						   &ReadData[count*READ_CHUNK_SIZE], readbytes);
		count++;
		
		if(ret<0){ 
		  dbg_cr("fts_read_reg ret-> %d\n",ret);
			return ret;
		}
		remained -= readbytes;
	}
	for(count=0 ; count < TotalLength; count+=2)
	{
		pan_io_data.raw_data_buffer[count/2] = 
				ReadData[count] + (ReadData[count+1]<<8);
	}
	return 0;
}

#ifdef PAN_KNOCK_ON
int pan_knock_on_param(int cmd, int *param)
{
	//	write format - cmd(1byte), address(2byte), data(nbyte)
	//	read format - refer to procedure_cmd_event in fts_pantech.c
	unsigned char regAdd[5] = {FTS_FW_REG_WRITE, 0x00, 0x00, 0x00, 0x00};	
	int rc = 0;
	
	switch (cmd) {
	case TOUCH_IOCTL_KNOCKON_SET_SCAN_FRAME_RATE:
		regAdd[1] = 0x01;
		regAdd[2] = 0x26;
		regAdd[3] = (*param)&0xFF;
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 4);
		io_info->fts_command(io_info, FTS_CMD_SAVE_FWCONFIG);
		break;
	case TOUCH_IOCTL_KNOCKON_SET_THRESHOLD:
		regAdd[1] = 0x00;
		regAdd[2] = 0x62;
		regAdd[3] = (*param) & 0xFF;
		regAdd[4] = ((*param) >> 8) & 0xFF;
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 5);
		io_info->fts_command(io_info, FTS_CMD_SAVE_FWCONFIG);
		break;
	case TOUCH_IOCTL_KNOCKON_SET_1ST_LEAVE_TOUT:
		regAdd[1] = 0x01;
		regAdd[2] = 0xDD;
		regAdd[3] = (*param)&0xFF;
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 4);
		io_info->fts_command(io_info, FTS_CMD_SAVE_FWCONFIG);
		break;
	case TOUCH_IOCTL_KNOCKON_SET_2ND_ENTER_TOUT:
		regAdd[1] = 0x01;
		regAdd[2] = 0xDE;
		regAdd[3] = (*param)&0xFF;
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 4);
		io_info->fts_command(io_info, FTS_CMD_SAVE_FWCONFIG);
		break;
	case TOUCH_IOCTL_KNOCKON_SET_2ND_LEAVE_TOUT:
		regAdd[1] = 0x01;
		regAdd[2] = 0xDF;
		regAdd[3] = (*param)&0xFF;
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 4);
		io_info->fts_command(io_info, FTS_CMD_SAVE_FWCONFIG);
		break;
	
	case TOUCH_IOCTL_KNOCKON_GET_SCAN_FRAME_RATE:
		regAdd[0] = FTS_FW_REG_READ;
		regAdd[1] = 0x01;
		regAdd[2] = 0x26;
		regAdd[3] = 0x1;
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 4);
		mdelay(5);		
		*param = io_info->cmd_result_int;
		break;
	case TOUCH_IOCTL_KNOCKON_GET_THRESHOLD:
		regAdd[0] = FTS_FW_REG_READ;  // command
		regAdd[1] = 0x00;	// address 1
		regAdd[2] = 0x62;	// address 2
		regAdd[3] = 0x2;	// read data size
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 4);
		mdelay(5);
		*param = io_info->cmd_result_int;
		break;
	case TOUCH_IOCTL_KNOCKON_GET_1ST_LEAVE_TOUT:
		regAdd[0] = FTS_FW_REG_READ;
		regAdd[1] = 0x01;
		regAdd[2] = 0xDD;
		regAdd[3] = 0x1;
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 4);
		mdelay(5);
		*param = io_info->cmd_result_int;
		break;
	case TOUCH_IOCTL_KNOCKON_GET_2ND_ENTER_TOUT:
		regAdd[0] = FTS_FW_REG_READ;
		regAdd[1] = 0x01;
		regAdd[2] = 0xDE;
		regAdd[3] = 0x1;
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 4);
		mdelay(5);
		*param = io_info->cmd_result_int;
		break;
	case TOUCH_IOCTL_KNOCKON_GET_2ND_LEAVE_TOUT:
		regAdd[0] = FTS_FW_REG_READ;
		regAdd[1] = 0x01;
		regAdd[2] = 0xDF;
		regAdd[3] = 0x1;
		rc = io_info->fts_write_reg(io_info, &regAdd[0], 4);
		mdelay(5);
		*param = io_info->cmd_result_int;
		break;
	}

	if( rc < 0){ 
		dbg_cr("%s error..ret-> %d, cmd=%d, param=%d\n", 
			__func__, rc, cmd, *param);		
	}
	return rc;
}
#endif


int pan_fts_io_register(struct fts_ts_info *info)
{
	if (info == NULL) return -ENOMEM;
	io_info = info;

#ifdef  PAN_TOUCH_EVENT_WITH_HALL_IC
  register_notify_func(EMERGENCY_MODE,"smart_cover",pan_hall_ic_event);
#endif

	return 0;
}
EXPORT_SYMBOL(pan_fts_io_register);

int ts_fops_open(struct inode *inode, struct file *filp)
{	
	return 0;
}

long ts_fops_ioctl(struct file *filp, 
				   unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;	
	int send_data=0;	
	dbg_touch("%s: cmd => %d, arg -> %lu\n", __func__, cmd, arg);
	switch (cmd) 
	{
#ifdef PAN_KNOCK_ON
	case TOUCH_IOCTL_SUSPEND:
		if (pan_io_data.knock_on_enable) {
			io_info->fts_command(io_info, FTS_CMD_KNOCK_ON);
			enable_irq_wake(io_info->irq);
		}
		else {
			io_info->suspend(io_info);
		}
#if defined(CONFIG_KEYBOARD_TC370) && !defined(CONFIG_KEYBOARD_TC370_SLEEP)
		pan_tm_key_suspend();
#endif
		
		break;
	case TOUCH_IOCTL_RESUME:
		if (pan_io_data.knock_on_enable) {
			disable_irq_wake(io_info->irq);
			io_info->suspend(io_info);
			io_info->resume(io_info);
		}
		else {
			io_info->resume(io_info);
		}
#if defined(CONFIG_KEYBOARD_TC370) && !defined(CONFIG_KEYBOARD_TC370_SLEEP)
		pan_tm_key_resume();
#endif		
		break;
#else
	case TOUCH_IOCTL_SUSPEND:
		io_info->suspend(io_info);		
#if defined(CONFIG_KEYBOARD_TC370) && !defined(CONFIG_KEYBOARD_TC370_SLEEP)
		pan_tm_key_suspend();
#endif
		break;
	case TOUCH_IOCTL_RESUME:
		io_info->resume(io_info);		
#if defined(CONFIG_KEYBOARD_TC370) && !defined(CONFIG_KEYBOARD_TC370_SLEEP)
		pan_tm_key_resume();
#endif		
		break;
#endif

	case TOUCH_IOCTL_DO_KEY:
		dbg_ioctl("TOUCH_IOCTL_DO_KEY  = %d\n",(int)argp);			
		if ( (int)argp == KEY_NUMERIC_STAR )
			input_report_key(io_info->input_dev, 0xe3, 1);
		else if ( (int)argp == KEY_NUMERIC_POUND )
			input_report_key(io_info->input_dev, 0xe4, 1);
		else
			input_report_key(io_info->input_dev, (int)argp, 1);
		input_sync(io_info->input_dev); 
		break;
	case TOUCH_IOCTL_RELEASE_KEY:		
		dbg_ioctl("TOUCH_IOCTL_RELEASE_KEY  = %d\n",(int)argp);
		if ( (int)argp == 0x20a )
			input_report_key(io_info->input_dev, 0xe3, 0);
		else if ( (int)argp == 0x20b )
			input_report_key(io_info->input_dev, 0xe4, 0);
		else
			input_report_key(io_info->input_dev, (int)argp, 0);
		input_sync(io_info->input_dev); 
		break;		
	
	case TOUCH_IOCTL_PRESS_TOUCH:
		input_report_abs(io_info->input_dev, ABS_MT_POSITION_X, (int)(arg&0x0000FFFF));
		input_report_abs(io_info->input_dev, ABS_MT_POSITION_Y, (int)((arg >> 16) & 0x0000FFFF));
		input_report_abs(io_info->input_dev, ABS_MT_TOUCH_MAJOR, 255);
		input_report_abs(io_info->input_dev, ABS_MT_WIDTH_MAJOR, 1);			
		input_sync(io_info->input_dev);
		break;
	case TOUCH_IOCTL_RELEASE_TOUCH:		
		input_report_abs(io_info->input_dev, ABS_MT_POSITION_X, (int)(arg&0x0000FFFF));
		input_report_abs(io_info->input_dev, ABS_MT_POSITION_Y, (int)((arg >> 16) & 0x0000FFFF));
		input_report_abs(io_info->input_dev, ABS_MT_TOUCH_MAJOR, 0);
		input_report_abs(io_info->input_dev, ABS_MT_WIDTH_MAJOR, 1);			
		input_sync(io_info->input_dev); 
		break;			
	case POWER_OFF:
	//	pm_power_off();
		break;
	
	case TOUCH_IOCTL_INIT:
		dbg_cr("Touch init \n");
		io_info->mflag_init_test=true;
	  //io_info->suspend(io_info);
	  //msleep(100);	  
	  //io_info->resume(io_info);	  
		break;

	case TOUCH_IOCTL_OFF:
		dbg_ioctl("Touch off \n");
	//	TSP_PowerOff();
		break;		

	case TOUCH_IOCTL_SENSOR_X:
		{
		  send_data= FTS_TX_LENGTH;
			if (copy_to_user(argp, &send_data, sizeof(send_data)))
				return false;
		}
		break;
	case TOUCH_IOCTL_SENSOR_Y:
		{
			send_data = FTS_RX_LENGTH;
			if (copy_to_user(argp, &send_data, sizeof(send_data)))
				return false;
		}
		break;
  case TOUCH_IOCTL_SELF_TEST_RESULT :
  
		io_info->fts_systemreset(io_info);
		msleep(10);
		io_info->fts_command(io_info, SLEEPOUT);
		io_info->fts_command(io_info, FTS_CMD_PANEL_ITO_TEST);
		msleep(500);
		if(copy_to_user(argp, io_info->ito_test,
						 sizeof(io_info->ito_test))){
				dbg_cr("TOUCH_IOCTL_SELF_TEST_RESULT failed\n");
			  return false;			 
	  }
	  io_info->suspend(io_info);
	  io_info->resume(io_info);	
    break;	
  case TOUCH_IOCTL_SELF_TEST :
  
    fts_read_raw_data(17,29);    
    if (copy_to_user(argp, &pan_io_data.raw_data_buffer, 
						 sizeof(pan_io_data.raw_data_buffer))) {
				return false;
		}		
    break;
  case TOUCH_IOCTL_DIAGNOSTIC_MIN_DEBUG :
      return FTS_MIN_RAWDATA;
	case TOUCH_IOCTL_DIAGNOSTIC_MAX_DEBUG :
      return FTS_MAX_RAWDATA;    
	case TOUCH_IOCTL_CHECK_BASE:
	case TOUCH_IOCTL_START_UPDATE:
		break;

	case TOUCH_IOCTL_DIAG_DELTA:
		{
			break;
		}

	case TOUCH_IOCTL_MULTI_TSP_OBJECT_SEL:
	  {
	    dbg_cr("Touch Mode is %d\n",(int)arg); 
      switch(arg){
#ifdef PAN_TOUCH_SET_PEN_MODE  
      case PAN_TOUCH_MODE_FINGER  :
        if(io_info->touch_mode_state.touch == PAN_TOUCH_MODE_GLOVE){
          // set disable glove mode
          io_info->fts_command(io_info, FTS_CMD_MSHOVER_OFF);
          io_info->mshover_enabled = 0;
        }else if(io_info->touch_mode_state.touch == PAN_TOUCH_MODE_PEN){
          // set disable pen mode
          io_info->fts_pen_mode_set(io_info,0);
        }
        io_info->touch_mode_state.touch = (int)arg;
        #ifdef CONFIG_KEYBOARD_TC370  
        pan_tm_set_mode(PAN_TOUCH_MODE_FINGER);
        #endif
        break;
        
      case PAN_TOUCH_MODE_PEN     :
        if(io_info->touch_mode_state.touch == PAN_TOUCH_MODE_GLOVE){
          io_info->fts_command(io_info, FTS_CMD_MSHOVER_OFF);
          io_info->mshover_enabled = 0; 
          io_info->fts_pen_mode_set(io_info,1);
        }else if(io_info->touch_mode_state.touch == PAN_TOUCH_MODE_FINGER){
           io_info->fts_pen_mode_set(io_info,1);          
        }
        io_info->touch_mode_state.touch = (int)arg;
        #ifdef CONFIG_KEYBOARD_TC370  
        pan_tm_set_mode(PAN_TOUCH_MODE_GLOVE); // Touch is Pen mode but tm is glove mode.
        #endif
        break;
      case PAN_TOUCH_MODE_GLOVE   :
        if(io_info->touch_mode_state.touch != (int)arg){
          if(io_info->touch_mode_state.touch != PAN_TOUCH_MODE_FINGER){
            io_info->fts_pen_mode_set(io_info,0);
          }
          io_info->touch_mode_state.touch = (int)arg;
          io_info->fts_command(io_info, FTS_CMD_MSHOVER_ON);
          io_info->mshover_enabled = 1;
        }
        #ifdef CONFIG_KEYBOARD_TC370  
        pan_tm_set_mode((int)arg);
        #endif
        break;
#else
        case PAN_TOUCH_MODE_FINGER  :
        case PAN_TOUCH_MODE_PEN     :
          if(io_info->touch_mode_state.touch == PAN_TOUCH_MODE_GLOVE){
            io_info->fts_command(io_info, FTS_CMD_MSHOVER_OFF);
			      io_info->mshover_enabled = 0;
            io_info->touch_mode_state.touch = (int)arg;
          }else if(io_info->touch_mode_state.touch == PAN_TOUCH_MODE_FINGER || io_info->touch_mode_state.touch == PAN_TOUCH_MODE_PEN){
          }
          #ifdef CONFIG_KEYBOARD_TC370	
          pan_tm_set_mode((int)arg);
          #endif
          break;
        case PAN_TOUCH_MODE_GLOVE   :
          if(io_info->touch_mode_state.touch != (int)arg){
            io_info->touch_mode_state.touch = (int)arg;
            io_info->fts_command(io_info, FTS_CMD_MSHOVER_ON);
			      io_info->mshover_enabled = 1;
          }
          #ifdef CONFIG_KEYBOARD_TC370	
          pan_tm_set_mode((int)arg);
          #endif
          break;
#endif          
        case PAN_SMART_COVER_STATE_OPENED :
        case PAN_SMART_COVER_STATE_CLOSED :
          io_info->touch_mode_state.smart_cover_from_ui= (int)arg - PAN_SMART_COVER_STATE_OPENED ;
          if(io_info->touch_mode_state.smart_cover_from_ui != io_info->touch_mode_state.smart_cover){
            dbg_cr("smart cover state is not equal. ic -> %d, ui -> %d\n",io_info->touch_mode_state.smart_cover,io_info->touch_mode_state.smart_cover_from_ui);
            io_info->touch_mode_state.smart_cover=io_info->touch_mode_state.smart_cover_from_ui;
          }
          pan_tm_set_cover_state(io_info->touch_mode_state.smart_cover_from_ui);
          del_timer_sync(&io_info->touch_mode_state.check_smart_cover);
          break;
        default :
          dbg_cr("Pan Touch Mode wrong value -> %d\n",(int)arg);
          break;
      }
      break;
	  }
	case TOUCH_IOCTL_MULTI_TSP_OBJECT_GET:
	  {
	    send_data = io_info->touch_mode_state.touch + io_info->touch_mode_state.smart_cover*10;
	    if (copy_to_user(argp, &send_data, sizeof(send_data))) {
            dbg_ioctl(" TOUCH_IOCTL_MULTI_TSP_OBJECT_GET error\n");
      }		
      break;
	    
	  }
	case TOUCH_IOCTL_HALL_IC_GPIO_GET:
	  send_data = gpio_get_value(PAN_SMART_COVER_GPIO);
    if (copy_to_user(argp, &send_data, sizeof(send_data))) {
        dbg_ioctl("TOUCH_IOCTL_HALL_IC_GPIO_GET error\n");
    }
    dbg_op("[%s] Hall ic Status: %d\n", __func__, send_data);
    break;

#ifdef PAN_KNOCK_ON
	case TOUCH_IOCTL_KNOCKON_SET_ENABLE:		
		pan_io_data.knock_on_enable = arg;
		dbg_cr("Knock On state is %s.\n", 
			pan_io_data.knock_on_enable ? "ON": "OFF");
		break;
	case TOUCH_IOCTL_KNOCKON_GET_ENABLE:
		if (copy_to_user(argp, &pan_io_data.knock_on_enable, sizeof(int))) {
			dbg_cr("TOUCH_IOCTL_KNOCKON_GET_ENABLE\n");
		}
		break;

	case TOUCH_IOCTL_KNOCKON_SET_SCAN_FRAME_RATE:
		if ( pan_io_data.knock_on_framerate != arg ) {
			pan_io_data.knock_on_framerate = arg;
			pan_knock_on_param(cmd, &pan_io_data.knock_on_framerate);
		}
		dbg_cr("Knock On scan frame rate is %d msec.\n", 
			pan_io_data.knock_on_framerate);
		break;
	case TOUCH_IOCTL_KNOCKON_SET_THRESHOLD:
		if ( pan_io_data.knock_on_threshold != arg ) {
			pan_io_data.knock_on_threshold = arg;
			pan_knock_on_param(cmd, &pan_io_data.knock_on_threshold);
		}
		dbg_cr("Knock On finger threshold is %d.\n", 
			pan_io_data.knock_on_threshold);
		break;
	case TOUCH_IOCTL_KNOCKON_SET_1ST_LEAVE_TOUT:
		if ( pan_io_data.knock_on_1st_leave != arg) {
			pan_io_data.knock_on_1st_leave = arg;
			pan_knock_on_param(cmd, &pan_io_data.knock_on_1st_leave);
		}		
		dbg_cr("Knock On 1st finger leave timeout is %d msec.\n", 
			pan_io_data.knock_on_1st_leave);
		break;
	case TOUCH_IOCTL_KNOCKON_SET_2ND_ENTER_TOUT:
		if (pan_io_data.knock_on_2nd_enter != arg) {
			pan_io_data.knock_on_2nd_enter = arg;
			pan_knock_on_param(cmd, &pan_io_data.knock_on_2nd_enter);
		}
		dbg_cr("Knock On 2nd finger enter timeout is %d msec.\n", 
			pan_io_data.knock_on_2nd_enter);
		break;
	case TOUCH_IOCTL_KNOCKON_SET_2ND_LEAVE_TOUT:
		if (pan_io_data.knock_on_2nd_leave != arg) {
			pan_io_data.knock_on_2nd_leave = arg;
			pan_knock_on_param(cmd, &pan_io_data.knock_on_2nd_leave);
		}		
		dbg_cr("Knock On 2nd finger leave timeout is %d msec.\n", 
			pan_io_data.knock_on_2nd_leave);
		break;

	//	for STMicro get
	case TOUCH_IOCTL_KNOCKON_GET_SCAN_FRAME_RATE:
		pan_knock_on_param(cmd, &pan_io_data.knock_on_framerate);		
		dbg_cr("Knock On scan frame rate is %d msec.\n", 
			pan_io_data.knock_on_framerate);
		if (copy_to_user(argp, &pan_io_data.knock_on_framerate, sizeof(int))) {
			dbg_cr("TOUCH_IOCTL_KNOCKON_GET_SCAN_FRAME_RATE\n");
		}
		break;
	case TOUCH_IOCTL_KNOCKON_GET_THRESHOLD:
		pan_knock_on_param(cmd, &pan_io_data.knock_on_threshold);
		dbg_cr("Knock On finger threshold is %d.\n", 
			pan_io_data.knock_on_threshold);
		if (copy_to_user(argp, &pan_io_data.knock_on_threshold, sizeof(int))) {
			dbg_cr("TOUCH_IOCTL_KNOCKON_GET_THRESHOLD\n");
		}
		break;
	case TOUCH_IOCTL_KNOCKON_GET_1ST_LEAVE_TOUT:
		pan_knock_on_param(cmd, &pan_io_data.knock_on_1st_leave);
		dbg_cr("Knock On 1nd finger leave timeout is %d msec.\n", 
			pan_io_data.knock_on_1st_leave);
		if (copy_to_user(argp, &pan_io_data.knock_on_1st_leave, sizeof(int))) {
			dbg_cr("TOUCH_IOCTL_KNOCKON_GET_1ST_LEAVE_TOUT\n");
		}
		break;
	case TOUCH_IOCTL_KNOCKON_GET_2ND_ENTER_TOUT:
		pan_knock_on_param(cmd, &pan_io_data.knock_on_2nd_enter);
		dbg_cr("Knock On 2nd finger enter timeout is %d msec.\n", 
			pan_io_data.knock_on_2nd_enter);
		if (copy_to_user(argp, &pan_io_data.knock_on_2nd_enter, sizeof(int))) {
			dbg_cr("TOUCH_IOCTL_KNOCKON_GET_2ND_ENTER_TOUT\n");
		}
		break;
	case TOUCH_IOCTL_KNOCKON_GET_2ND_LEAVE_TOUT:
		pan_knock_on_param(cmd, &pan_io_data.knock_on_2nd_leave);
		dbg_cr("Knock On 2nd finger leave timeout is %d msec.\n", 
			pan_io_data.knock_on_2nd_leave);
		if (copy_to_user(argp, &pan_io_data.knock_on_2nd_leave, sizeof(int))) {
			dbg_cr("TOUCH_IOCTL_KNOCKON_GET_2ND_LEAVE_TOUT\n");
		}
		break;
#endif
#if defined (FTS_SUPPORT_TA_MODE_PANTECH)
  case TOUCH_IOCTL_CHARGER_MODE :
  if (arg) {
		io_info->fts_command(io_info, FTS_CMD_CHARGER_PLUGGED);
		io_info->TA_Pluged = true;
		dbg_cr("%s: device_control : CHARGER CONNECTED, ta_status : %d\n",
			 __func__, (int)arg);
	} else {
		io_info->fts_command(io_info, FTS_CMD_CHARGER_UNPLUGGED);
		io_info->TA_Pluged = false;
		dbg_cr("%s: device_control : CHARGER DISCONNECTED, ta_status : %d\n",
			 __func__, (int)arg);
	}
	break;
#endif  

	default:
	  dbg_cr(" Touch IOCTL CMD is nonregistered value -> %d\n",(int)arg);
		break;
	}
	return 1;
}

int open(struct inode *inode, struct file *file) 
{
	return 0; 
}

int release(struct inode *inode, struct file *file) 
{
	return 0; 
}

ssize_t write(struct file *file, 
			  const char *buf, size_t count, loff_t *ppos)
{
	int nBufSize=0, i;
	
	if((size_t)(*ppos) > 0) 
		return 0;

	dbg_cr(" Touch IO Write function\n");	

	if(buf != NULL){
		nBufSize=strlen(buf);
		if(strncmp(buf, "debug_", 6) == 0)
		{ 
			if(buf[6] > '0'){
				i = buf[6] - '1';

				if(pan_debug_state & 0x00000001 <<i) {
					pan_debug_state &= ~(0x00000001 <<i);	
				}else{
					pan_debug_state |= (0x00000001 <<i);
				}
			}
			dbg_cr("pan_debug_state->0x%X, i->%d\n", pan_debug_state, i);
		}

		else if(strncmp(buf, "raw_data",8) == 0)
		{
			dbg_cr(" raw_data \n");
			fts_read_raw_data(17,29);
			raw_data_print();
		}
                else if(strncmp(buf, "self_test",9) == 0)
		{
			dbg_cr(" self_test \n");

			io_info->fts_systemreset(io_info);
			msleep(10);
			io_info->fts_command(io_info, SLEEPOUT);
                        io_info->fts_command(io_info, FTS_CMD_PANEL_ITO_TEST);
                }
		else if(strncmp(buf, "touch_info",10) == 0)
		{
		  dbg_cr("touch mode -> %d\n",io_info->touch_mode_state.touch);
		  dbg_cr("smart cover state -> %d\n",io_info->touch_mode_state.smart_cover);
		  dbg_cr("touch irq state is %s\n",io_info->irq_enabled?"enabled":"disabled");
#ifdef PAN_KNOCK_ON		  
		  dbg_cr("Knock On state is %s.\n",pan_io_data.knock_on_enable ? "ON": "OFF");
#endif
#if defined (FTS_SUPPORT_TA_MODE_PANTECH)
      dbg_cr("TA state -> %d\n",(int)io_info->TA_Pluged);
#endif

	}	
	  else if(strncmp(buf, "glove_on",8) == 0){
	    dbg_cr("glove on\n");
	    io_info->fts_command(io_info, FTS_CMD_MSHOVER_ON);
			io_info->mshover_enabled = 1;	    
	  }else if(strncmp(buf, "glove_off",9) == 0){
	    dbg_cr("glove off\n");
	    io_info->fts_command(io_info, FTS_CMD_MSHOVER_OFF);
			io_info->mshover_enabled = 0;	
	  }
	  else if(strncmp(buf, "pen_on",6) == 0){
	    dbg_cr("pen on\n");
	    io_info->fts_pen_mode_set(io_info,1);
	      
	  }else if(strncmp(buf, "pen_off",7) == 0){
	    dbg_cr("pen off\n");
	    io_info->fts_pen_mode_set(io_info,0);	
	  }

    else if(strncmp(buf, "error_check",11) == 0){
      dbg_cr("touch_error_cnt -> %d\n",touch_error_cnt);
      if(touch_error_cnt)
        dbg_cr("touch error message -> %s",touch_error_info);
    }

	}	

	*ppos +=nBufSize;
	return nBufSize;
}

ssize_t read(struct file *file, 
			 char *buf, size_t count, loff_t *ppos)
{
	return 0; 
}

long ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	void __user *argp = (void __user *)arg;
	dbg_ioctl("%s: cmd => %d, arg -> %lu\n", __func__, cmd, arg);
	
	switch (cmd)
	{
	case READ_TOUCH_ID:
	    return 
		((pan_io_data.vendor_id<<16) +
		 (pan_io_data.model_id<<4) +
		  pan_io_data.type_id);
	    break;
	case TOUCH_IOCTL_DEBUG:
		return ioctl_debug(arg);
		break;
	
	case ATMEL_GET_REFERENCE_DATA:
		if (copy_to_user(argp, &pan_io_data.raw_data_buffer, 
						 sizeof(pan_io_data.raw_data_buffer))) {
			dbg_cr("[TOUCH IO] stmicro raw_data_buffer is failed\n");
		}
		break;

	case DIAG_DEBUG:
		dbg_ioctl(" DIAG_DEBUG arg => %d\n",(int)arg);
		if (arg == 5010) 
		{
			fts_read_raw_data(17,29);
			return 0;
		}
		if (arg == 5011) 
		{
			fts_read_raw_data(17,29);
			return 0;
		}
		else if (arg > 224-1)
		{
			dbg_ioctl(" ERROR! arg -> %d",(int)arg);
			return 0;
		}
		break;	
	}
	return 0;
}

int ioctl_debug(unsigned long arg) 
{		
	return 0;
}
