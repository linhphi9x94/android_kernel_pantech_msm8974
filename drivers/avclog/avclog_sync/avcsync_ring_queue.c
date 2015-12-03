#include <linux/module.h>                        
#include <linux/fs.h>                            
#include "avcsync_ring_queue.h"
#include "avcsync_minilib.h"

static avcsync_qid_ctl qid_ctl;                

inline int avc_rq_is_correct(avc_ring_queue* rq);

//-----------------------------------------------------------------------------
int avc_rq_init(avc_ring_queue* rq, int size, int maxcnt)
{
    AVCLOG_ASSERT(rq != NULL);
    AVCLOG_ASSERT((0 <= size) && (size <= AVC_RQ_BUFFER_MAX));
    AVCLOG_ASSERT((1 <= maxcnt) && (maxcnt <= AVC_RQ_BUFFER_MAX));
    
    if (AVC_RQ_BUFFER_MAX < size + maxcnt + 1) {
		PRINTF("[%s] memory allocation rejected.\n", __func__);
		return FALSE;
    }
    
    if ((rq->data = (byte*)MEM_ALLOC(size + maxcnt + 1)) == NULL) {
		PRINTF("[%s] memory allocation error\n", __func__);
		return FALSE;
    }
    rq->head     = 0;
    rq->tail     = 0;
    rq->size     = size + maxcnt + 1;
    rq->count    = 0;
    rq->maxcount = maxcnt;
    
//    AVCLOG_ASSERT(avc_rq_is_correct(rq));
    if(avc_rq_is_correct(rq) == FALSE)
		return FALSE;
    
    return TRUE;
}


//-----------------------------------------------------------------------------
void avc_rq_final(avc_ring_queue* rq)
{
//    AVCLOG_ASSERT(avc_rq_is_correct(rq));
    if(avc_rq_is_correct(rq) == FALSE)
		return;
    
    MEM_FREE(rq->data);
    rq->data = NULL;
}


//-----------------------------------------------------------------------------
int avc_rq_is_full(avc_ring_queue* rq, int size)
{
    int  d;
    int  space_left;
    
    
//    AVCLOG_ASSERT(avc_rq_is_correct(rq));
    if(avc_rq_is_correct(rq) == FALSE)
		return FALSE;

    AVCLOG_ASSERT((0 <= size) && (size <= AVC_RQ_BUCKET_MAX));

    if (rq->count == rq->maxcount)
	return TRUE;

    d = rq->head - rq->tail;
    space_left = (0 <= d) ? (rq->size - d - 1) : (-1 - d);
    if (size + sizeof(byte) <= space_left)
	return FALSE;
    else
	return TRUE;
}


//-----------------------------------------------------------------------------
int avc_rq_is_empty(avc_ring_queue* rq)
{
//    AVCLOG_ASSERT(avc_rq_is_correct(rq));
    if(avc_rq_is_correct(rq) == FALSE)
		return FALSE;
    
    if (rq->head == rq->tail)
	return TRUE;
    else
	return FALSE;
}


//-----------------------------------------------------------------------------
int avc_rq_enque(avc_ring_queue* rq, const byte* buf, int size)
{
    int  i;
    
    
//    AVCLOG_ASSERT(avc_rq_is_correct(rq));
    if(avc_rq_is_correct(rq) == FALSE)
		return FALSE;

    AVCLOG_ASSERT((0 <= size) && (size <= AVC_RQ_BUCKET_MAX));

    if (avc_rq_is_full(rq, size))
	return -1;

    rq->data[rq->head++] = (byte)size;
    if (rq->head == rq->size)
	rq->head = 0;
    
    for (i = 0; (i < size) && (rq->head < rq->size); i++)
	rq->data[rq->head++] = *buf++;

    if (rq->head == rq->size)
	rq->head = 0;
    
    for ( ; i < size; i++)
	rq->data[rq->head++] = *buf++;

    rq->count++;

//    AVCLOG_ASSERT(avc_rq_is_correct(rq));
    if(avc_rq_is_correct(rq) == FALSE)
		return FALSE;

    return i;
}


//-----------------------------------------------------------------------------
int avc_rq_deque(avc_ring_queue* rq, byte* buf, int size)
{
    int  i;
    int  bucketsize;
    int  smaller;
    
    
//    AVCLOG_ASSERT(avc_rq_is_correct(rq));
//    AVCLOG_ASSERT(buf != NULL);
    if ((avc_rq_is_correct(rq) == FALSE) && (buf == NULL))
		return FALSE;

    AVCLOG_ASSERT((0 <= size) && (size <= AVC_RQ_BUCKET_MAX));

    if (avc_rq_is_empty(rq))
		return -1;

    bucketsize = (int)rq->data[rq->tail++];
    if (rq->tail == rq->size)
	rq->tail = 0;
    
    AVCLOG_ASSERT((0 <= bucketsize) && (bucketsize <= AVC_RQ_BUCKET_MAX));
    smaller = (size < bucketsize) ? size : bucketsize;
    
    for (i = 0; (i < smaller) && (rq->tail < rq->size); i++)
	*buf++ = rq->data[rq->tail++];

    if (rq->tail == rq->size)
	rq->tail = 0;

    for ( ; i < smaller; i++)
	*buf++ = rq->data[rq->tail++];

    for ( ; (i < bucketsize) && (rq->tail < rq->size); i++)
	rq->tail++;

    if (rq->tail == rq->size)
	rq->tail = 0;
    
    for ( ; i < bucketsize; i++)
	rq->tail++;
	
    rq->count--;

//    AVCLOG_ASSERT(avc_rq_is_correct(rq));
    if(avc_rq_is_correct(rq) == FALSE)
		return FALSE;
    
    return smaller;
}


//-----------------------------------------------------------------------------
int avc_rq_is_correct(avc_ring_queue* rq)
{
    if (rq == NULL)
	return FALSE;

    if (rq->data == NULL)
	return FALSE;

    if ((rq->size < 0) || (AVC_RQ_BUFFER_MAX < rq->size))
	return FALSE;

    if ((rq->head < 0) || (rq->size <= rq->head))
	return FALSE;

    if ((rq->tail < 0) || (rq->size <= rq->tail))
	return FALSE;

    if ((rq->count < 0) || (rq->maxcount < rq->count))
	return FALSE;

    if ((rq->maxcount < 0) || (AVC_RQ_COUNT_MAX < rq->maxcount))
	return FALSE;
    
    return TRUE;
}

void avc_rq_id_init( void )                         
{                                              
    int cnt;                                     

    qid_ctl.lastid = 0;                         
                                               
    for( cnt = 0; cnt < AVC_RQ_IDNUM_MAX; cnt ++ )  
    {                                         
        qid_ctl.avcsync_qid[ cnt ] = NULL;       
    }                                          

}                                               

int avc_rq_id_take( avcsync_fpd* fpd )                 
{                                               
    int     cnt;                                 
    int     id;                                

    id = qid_ctl.lastid;                     

    for( cnt = 0; cnt < AVC_RQ_IDNUM_MAX; cnt ++ )   
    {                                           
        id = (( id + 1 ) & ( AVC_RQ_IDNUM_MASK ));  

        if( qid_ctl.avcsync_qid[ id ] == NULL )     
        {                                      
            fpd->id = ( id | avcsync_op_count );  
            qid_ctl.avcsync_qid[ id ] = fpd;      
                                               
            qid_ctl.lastid = id;                
            return 0;                          
        }                                      

    }                                           

    return -1;                                  
}                                             

int avc_rq_id_free( avcsync_fpd* fpd )                  
{                                               
    int     id;                                

    id = (( fpd->id ) & ( AVC_RQ_IDNUM_MASK ));     
    
    if( qid_ctl.avcsync_qid[ id ] == NULL )          
    {                                           
        return -1;                              
    }                                         

    qid_ctl.avcsync_qid[ id ] = NULL;             
    return 0;                                  
}                                            

int avc_rq_id_get( int id, avcsync_fpd** fpd )                   
{                                              
    int      idnum;                             
    avcsync_fpd* set_fpd;                           
                                                
    idnum = (( id ) & ( AVC_RQ_IDNUM_MASK ));        

    if( qid_ctl.avcsync_qid[ idnum ] == NULL )      
    {                                         
        return -1;                              
    }                                          

    set_fpd = qid_ctl.avcsync_qid[ idnum ];         

    if( set_fpd->id != id )                    
    {                                          
        return -1;                              
    }                                            

    *fpd = set_fpd;                             
    return 0;                                   
}                                               


