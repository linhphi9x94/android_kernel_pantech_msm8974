#ifndef PANTECH_LOG_SYS_H
#define PANTECH_LOG_SYS_H

/*---------------------------------------------------------------------------------------------------
 ** Define
 **-------------------------------------------------------------------------------------------------*/

/* Log Server <--> Log Client communication */
#define LOGSYS_RSP_PIPE "/dev/skylog_rsp_pipe"
#define LOGSYS_REQ_PIPE "/dev/skylog_req_pipe"
#define PANTECH_DEBUG_SOCK              "/dev/socket/pantech_debug_sock"
#define PANTECH_DEBUG_PARAM_SIZE        16     // size of pkt_type_t.param4
#define MAX_TRY_COUNT                   10       // second
#define MAX_ACCEPT_TRY_COUNT            10
#define TERM_OF_TRY_COUNT               1

/* Error Log Directory & Partition */
#define SKY_ERROR_LOG_DIR                 "skylog"
#define SKY_ERROR_LOG_SECURE_DIR          "/data/securelog"
#define SKY_ERROR_LOG_USER_DIR            "/mnt/sdcard/skylog"
#define SKY_ERROR_LOG_CORE_ROOT_DIR       "/mnt/sdcard/skylog/corelog"
#define SKY_ERROR_LOG_ANR_DIR             "/data/anr"
#define SKY_ERROR_LOG_TOMBSTONES_DIR      "/data/tombstones"
#define SKY_ERROR_LOG_SECURE_PARTITON     "/dev/block/mmcblk0p22"
#define SKY_ERROR_LOG_TIME_INFO           "time.txt"
#define SKY_DUMP_INIT                     "/persist/.dumpcookie"
#define SKY_ERROR_SSR_DUMP_EMMC_DIR       "/data/ramdump"
#define SKY_ERROR_SSR_DUMP_SSR_DIR        "ssrdump"
#define SKY_ERROR_SSR_LOG_SSR_DIR         "ssrlog"
#define SKY_ERROR_CORE_DUMP_EMMC_DIR      "/data/core"
#define SKY_ERROR_CORE_DUMP_DIR           "coredump"

#define ERR_CRASH_DUMP_START_MAGIC_NUM          0xDEADDEAD
#define ERR_CRASH_DUMP_END_MAGIC_NUM             0xEFBEEFBE

#define PKT_DATA_MAX_SIZE 512

/*---------------------------------------------------------------------------------------------------
 ** Type Define
 **-------------------------------------------------------------------------------------------------*/

typedef struct {
    int cmd;
    int param;
    int param2;
    int param3; /* only for response packet. do not use */
    char param4[16];
    int data_size;
    char data[PKT_DATA_MAX_SIZE];
}logsys_pkt_t;

/* error & status case defined by OEM */
enum{
    COMPLETE_COPY_ERROR_LOG     = 0x0001,
    COMPLETE_COPY_MDM_LOG       = 0x0002,
    COMPLETE_COPY_SSRLOG        = 0x0004,
    COMPLETE_SAVE_LOG_MASK      = 0x0007,

    COMPLETE_LOG_DIR_MOUNT      = 0x0010,

    COMPLETE_MDMLOG_DELETE      = 0x0100,
    COMPLETE_APQLOG_DELETE      = 0x0200,
    COMPLETE_SSRLOG_DELETE      = 0x0400,
    COMPLETE_DELETE_LOG_MASK    = 0x0700,

    MDM_LOG_EXIST_FLAG          = 0x1000,
    APQ_LOG_EXIST_FLAG          = 0x2000,
    SSR_LOG_EXIST_FLAG          = 0x4000,
    COREDUMP_LOG_EXIST_FLAG     = 0x8000,
    FILE_EXIST_MASK             = 0xF000
};

enum{
    FIND_DIR       = 0x0001,
    FIND_FILE      = 0x0002,
    FIND_MASK      = 0x0003,
    DIR_OPEN_ERROR = 0x0004,
    DIR_EMPTY      = 0x0008,
    DIR_ERROR_MASK = 0x000C
};

enum{
    REQ_CMD_SYS_RESET_REQ_SW_RESET = 0x2001001,
    REQ_CMD_SYS_SAVE_LOG_TO_SDCARD = 0x20002001,
    REQ_CMD_SYS_SAVE_LOG_TO_SDCARD_CRASH_ONLY = 0x20002003,

    REQ_CMD_SYS_RAMDUMP_ENABLE_SYSFS = 0x20004001,
    REQ_CMD_SYS_USBDUMP_ENABLE_SYSFS = 0x20004002,
    REQ_CMD_SYS_USERDUMP_ENABLE_SYSFS = 0x20004003,  // p15060
    REQ_CMD_SYS_SSR_COMMAND = 0x20004004,         // p15060

    REQ_CMD_SYS_EXCUTE_DUMPSTATE = 0x20005001,

    REQ_CMD_SYS_ANDROID_RESETCNT_ADD = 0x20006001,

    RES_CMD_LOG_SYSTEM_OK = 0x30000001,
    RES_CMD_LOG_SYSTEM_UNKNOWN,
    REQ_CMD_LOG_SYSTEM_MAX = 0xFFFFFFFF
};

// p15060
enum {
    LOGSET_SETCMD                       = 20,
    LOGSET_GETCMD                       = 21,
    LOGSET_USERDUMP_DISABLE             = 22,
    LOGSET_USERDUMP_COREDUMP            = 23,
    LOGSET_USERDUMP_FRAME_COREDUMP      = 24,
    LOGSET_USERDUMP_USER_RAMDUMP        = 25,
    LOGSET_USERDUMP_FRAME_RAMDUMP       = 26,
};

/*---------------------------------------------------------------------------------------------------
 ** struct Define
 **-------------------------------------------------------------------------------------------------*/
#if 0
typedef struct{
  unsigned int err_crash_dump_start_magic_num;         // Start Magic Number
  unsigned int ram_dump_enable;
  unsigned int usb_dump_enable;
  unsigned int temporary_dump_enable;   // by p11219  for temporary ramdump set flag in multi menu.  
  unsigned int user_dump_enable;            // p15060
  unsigned int err_crash_dump_end_magic_num;             // End Magic Number
}err_crash_dump_type;
#endif
/*----------------------------------------------------------------------------------------------------------------------------------
 ** function declare - these functions defined at pantech_logsystem.c
 **---------------------------------------------------------------------------------------------------------------------------------*/

extern char* Sky_MoveLog_TimeCal(void);
extern int Sky_MoveLog_DirCheck(char *pLogDir);
extern int Sky_MoveLog_APPs(char *filename_time);
extern void pantech_write_ramdump_enable_sysfs( unsigned int ramdump );
extern void pantech_write_usbdump_enable_sysfs( unsigned int usbdump  );
extern int SaveLogCmd_Run(char *pCmd);
extern void Sky_ErrorLog_TimeInfoCheck(void* data);
extern void pantech_dump_mode_init(void);
extern void android_reset_count_add_thread(void *data);

// p15060
extern int isDirectoryEmpty(char *dirname);
extern void pantech_write_userdump_enable_sysfs( unsigned int userdump );
extern void pantech_ssr_command( void *reqpkt, int *ret );
extern int Sky_MoveLog_COREDUMPs(char *filename_time);
#if defined(MSM8974_RELEASED_JB)
int Sky_MoveLog_SSRs(char *filename_time);
#endif
extern int movelog_main( void *arg );
#endif // PANTECH_LOG_H
