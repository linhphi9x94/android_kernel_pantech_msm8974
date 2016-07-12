#ifndef RING_QUEUE_H
#define RING_QUEUE_H


//-----------------------------------------------------------------------------
#define AVC_RQ_BUFFER_MAX    8192
#define AVC_RQ_COUNT_MAX     8000
#define AVC_RQ_DATASIZE_MAX  8000
#define AVC_RQ_BUCKET_MAX      128 /*buffersize 64 -> 128 05/05/24 LinuxOST*/
#define AVC_RQ_IDNUM_MAX    0x100                 
                                              
#define AVC_RQ_IDNUM_MASK    AVC_RQ_IDNUM_MAX - 1       

#define AVC_ADD_OP_COUNT  0x10000                 
typedef unsigned char  byte;


typedef struct __avc_ring_queue {
    int    head;
    int    tail;
    int    size;
    int    count;
    int    maxcount;
    byte*  data;
} avc_ring_queue;

typedef struct __avcsync_fpd {                      
    int                allocated;               
    int                status;                   
    int                id;                     
    avc_ring_queue         ringq;                   
    wait_queue_head_t  waitq;                   
    pid_t              tgid;                    
} avcsync_fpd;

typedef struct __avcsync_qid_ctl                   
{                                                
    unsigned int lastid;                       
    avcsync_fpd*     avcsync_qid[ AVC_RQ_IDNUM_MAX ];       
} avcsync_qid_ctl;                                 

int  avc_rq_init(avc_ring_queue* rq, int size, int maxcnt);
void avc_rq_final(avc_ring_queue* rq);
int  avc_rq_enque(avc_ring_queue* rq, const byte* buf, int size);
int  avc_rq_deque(avc_ring_queue* rq, byte* buf, int size);
int  avc_rq_is_empty(avc_ring_queue* rq);
int  avc_rq_is_full(avc_ring_queue* rq, int size);
void avc_rq_id_init( void );                         
int  avc_rq_id_take( avcsync_fpd* fpd );               
int  avc_rq_id_free( avcsync_fpd* fpd );                 
int  avc_rq_id_get( int id,                       
                avcsync_fpd** fpd );                


extern unsigned int avcsync_op_count;          
#endif  // RING_QUEUE_H
