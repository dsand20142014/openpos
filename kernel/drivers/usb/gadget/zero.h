#define BYTES_PER_BULK 600

struct  otg_fifo
{
	struct cdev cdev;
	spinlock_t      kfifo_lock;                         ///
  unsigned int    kfifo_order;                        ///
  struct kfifo *  kfifo;                              ///
  unsigned char * kfifo_buffer;                       ///
  unsigned int    kfifo_maxlen;                       ///PAGESIZE<<order	
  unsigned char line_buf[BYTES_PER_BULK]; 
};
