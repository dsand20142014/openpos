#ifndef _LINUX_TTY_H
#define _LINUX_TTY_H

/*
 * 'tty.h' defines some structures used by tty_io.c and some defines.
 */

#ifdef __KERNEL__
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/termios.h>
#include <linux/workqueue.h>
#include <linux/tty_driver.h>
#include <linux/tty_ldisc.h>
#include <linux/mutex.h>

#include <asm/system.h>


/*
 * (Note: the *_driver.minor_start values 1, 64, 128, 192 are
 * hardcoded at present.)
 */
#define NR_UNIX98_PTY_DEFAULT	4096      /* Default maximum for Unix98 ptys */
#define NR_UNIX98_PTY_MAX	(1 << MINORBITS) /* Absolute limit */
#define NR_LDISCS		19

/* line disciplines */
#define N_TTY		0
#define N_SLIP		1
#define N_MOUSE		2
#define N_PPP		3
#define N_STRIP		4
#define N_AX25		5
#define N_X25		6	/* X.25 async */
#define N_6PACK		7
#define N_MASC		8	/* Reserved for Mobitex module <kaz@cafe.net> */
#define N_R3964		9	/* Reserved for Simatic R3964 module */
#define N_PROFIBUS_FDL	10	/* Reserved for Profibus */
#define N_IRDA		11	/* Linux IrDa - http://irda.sourceforge.net/ */
#define N_SMSBLOCK	12	/* SMS block mode - for talking to GSM data */
				/* cards about SMS messages */
#define N_HDLC		13	/* synchronous HDLC */
#define N_SYNC_PPP	14	/* synchronous PPP */
#define N_HCI		15	/* Bluetooth HCI UART */
#define N_GIGASET_M101	16	/* Siemens Gigaset M101 serial DECT adapter */
#define N_SLCAN		17	/* Serial / USB serial CAN Adaptors */
#define N_PPS		18	/* Pulse per Second */

/*
 * This character is the same as _POSIX_VDISABLE: it cannot be used as
 * a c_cc[] character, but indicates that a particular special character
 * isn't in use (eg VINTR has no character etc)
 */
#define __DISABLED_CHAR '\0'

struct tty_buffer {
	struct tty_buffer *next;
	char *char_buf_ptr;
	unsigned char *flag_buf_ptr;
	int used;
	int size;
	int commit;
	int read;
	/* Data points here */
	unsigned long data[0];
};

struct tty_bufhead {
	struct delayed_work work;
	spinlock_t lock;
	struct tty_buffer *head;	/* Queue head */
	struct tty_buffer *tail;	/* Active buffer */
	struct tty_buffer *free;	/* Free queue head */
	int memory_used;		/* Buffer space used excluding
								free queue */
};
/*
 * When a break, frame error, or parity error happens, these codes are
 * stuffed into the flags buffer.
 */
#define TTY_NORMAL	0
#define TTY_BREAK	1
#define TTY_FRAME	2
#define TTY_PARITY	3
#define TTY_OVERRUN	4

#define INTR_CHAR(tty) ((tty)->termios->c_cc[VINTR])
#define QUIT_CHAR(tty) ((tty)->termios->c_cc[VQUIT])
#define ERASE_CHAR(tty) ((tty)->termios->c_cc[VERASE])
#define KILL_CHAR(tty) ((tty)->termios->c_cc[VKILL])
#define EOF_CHAR(tty) ((tty)->termios->c_cc[VEOF])
#define TIME_CHAR(tty) ((tty)->termios->c_cc[VTIME])
#define MIN_CHAR(tty) ((tty)->termios->c_cc[VMIN])
#define SWTC_CHAR(tty) ((tty)->termios->c_cc[VSWTC])
#define START_CHAR(tty) ((tty)->termios->c_cc[VSTART])
#define STOP_CHAR(tty) ((tty)->termios->c_cc[VSTOP])
#define SUSP_CHAR(tty) ((tty)->termios->c_cc[VSUSP])
#define EOL_CHAR(tty) ((tty)->termios->c_cc[VEOL])
#define REPRINT_CHAR(tty) ((tty)->termios->c_cc[VREPRINT])
#define DISCARD_CHAR(tty) ((tty)->termios->c_cc[VDISCARD])
#define WERASE_CHAR(tty) ((tty)->termios->c_cc[VWERASE])
#define LNEXT_CHAR(tty)	((tty)->termios->c_cc[VLNEXT])
#define EOL2_CHAR(tty) ((tty)->termios->c_cc[VEOL2])

#define _I_FLAG(tty, f)	((tty)->termios->c_iflag & (f))
#define _O_FLAG(tty, f)	((tty)->termios->c_oflag & (f))
#define _C_FLAG(tty, f)	((tty)->termios->c_cflag & (f))
#define _L_FLAG(tty, f)	((tty)->termios->c_lflag & (f))

#define I_IGNBRK(tty)	_I_FLAG((tty), IGNBRK)
#define I_BRKINT(tty)	_I_FLAG((tty), BRKINT)
#define I_IGNPAR(tty)	_I_FLAG((tty), IGNPAR)
#define I_PARMRK(tty)	_I_FLAG((tty), PARMRK)
#define I_INPCK(tty)	_I_FLAG((tty), INPCK)
#define I_ISTRIP(tty)	_I_FLAG((tty), ISTRIP)
#define I_INLCR(tty)	_I_FLAG((tty), INLCR)
#define I_IGNCR(tty)	_I_FLAG((tty), IGNCR)
#define I_ICRNL(tty)	_I_FLAG((tty), ICRNL)
#define I_IUCLC(tty)	_I_FLAG((tty), IUCLC)
#define I_IXON(tty)	_I_FLAG((tty), IXON)
#define I_IXANY(tty)	_I_FLAG((tty), IXANY)
#define I_IXOFF(tty)	_I_FLAG((tty), IXOFF)
#define I_IMAXBEG_ACCEPTED) {
			/* drp_ie is companion */
			if (!bitmap_equal(rsv->mas.bm, mas.bm, UWB_NUM_MAS))
				/* stroke companion */
				uwb_rsv_set_state(rsv, UWB_RSV_STATE_T_EXPANDING_ACCEPTED);	
		} else {
			if (!bitmap_equal(rsv->mas.bm, mas.bm, UWB_NUM_MAS)) {
				if (uwb_drp_avail_reserve_pending(rc, &mas) == -EBUSY) {
					/* FIXME: there is a conflict, find
					 * the conflicting reservations and
					 * take a sensible action. Consider
					 * that in drp_ie there is the
					 * "neighbour" */
					uwb_drp_handle_all_conflict_rsv(rc, drp_evt, drp_ie, &mas);
				} else {
					/* accept the extra reservation */
					bitmap_copy(mv->companion_mas.bm, mas.bm, UWB_NUM_MAS);
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_T_EXPANDING_ACCEPTED);
				}
			} else {
				if (status) {
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_T_ACCEPTED);
				}
			}
			
		}
		break;

	case UWB_DRP_REASON_MODIFIED:
		/* check to see if we have already modified the reservation */
		if (bitmap_equal(rsv->mas.bm, mas.bm, UWB_NUM_MAS)) {
			uwb_rsv_set_state(rsv, UWB_RSV_STATE_T_ACCEPTED);
			break;
		}

		/* find if the owner wants to expand or reduce */
		if (bitmap_subset(mas.bm, rsv->mas.bm, UWB_NUM_MAS)) {
			/* owner is reducing */
			bitmap_andnot(mv->companion_mas.bm, rsv->mas.bm, mas.bm, UWB_NUM_MAS);
			uwb_drp_avail_release(rsv->rc, &mv->companion_mas);
		}

		bitmap_copy(rsv->mas.bm, mas.bm, UWB_NUM_MAS);
		uwb_rsv_set_state(rsv, UWB_RSV_STATE_T_RESIZED);
		break;
	default:
		dev_warn(dev, "ignoring invalid DRP IE state (%d/%d)\n",
			 reason_code, status);
	}
}

/*
 * Based on the DRP IE, transition an owner reservation to a new
 * state.
 */
static void uwb_drp_process_owner(struct uwb_rc *rc, struct uwb_rsv *rsv,
				  struct uwb_dev *src, struct uwb_ie_drp *drp_ie,
				  struct uwb_rc_evt_drp *drp_evt)
{
	struct device *dev = &rc->uwb_dev.dev;
	struct uwb_rsv_move *mv = &rsv->mv;
	int status;
	enum uwb_drp_reason reason_code;
	struct uwb_mas_bm mas;

	status = uwb_ie_drp_status(drp_ie);
	reason_code = uwb_ie_drp_reason_code(drp_ie);
	uwb_drp_ie_to_bm(&mas, drp_ie);

	if (status) {
		switch (reason_code) {
		case UWB_DRP_REASON_ACCEPTED:
			switch (rsv->state) {
			case UWB_RSV_STATE_O_PENDING:
			case UWB_RSV_STATE_O_INITIATED:
			case UWB_RSV_STATE_O_ESTABLISHED:
				uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_ESTABLISHED);
				break;
			case UWB_RSV_STATE_O_MODIFIED:
				if (bitmap_equal(mas.bm, rsv->mas.bm, UWB_NUM_MAS)) {
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_ESTABLISHED);
				} else {
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_MODIFIED);	
				}
				break;
				
			case UWB_RSV_STATE_O_MOVE_REDUCING: /* shouldn' t be a problem */
				if (bitmap_equal(mas.bm, rsv->mas.bm, UWB_NUM_MAS)) {
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_ESTABLISHED);
				} else {
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_MOVE_REDUCING);	
				}
				break;
			case UWB_RSV_STATE_O_MOVE_EXPANDING:
				if (bitmap_equal(mas.bm, mv->companion_mas.bm, UWB_NUM_MAS)) {
					/* Companion reservation accepted */
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_MOVE_COMBINING);
				} else {
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_MOVE_EXPANDING);
				}
				break;
			case UWB_RSV_STATE_O_MOVE_COMBINING:
				if (bitmap_equal(mas.bm, rsv->mas.bm, UWB_NUM_MAS))
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_MOVE_REDUCING);
				else
					uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_MOVE_COMBINING);
				break;
			default:
				break;	
			}
			break;
		default:
			dev_warn(dev, "ignoring invalid DRP IE state (%d/%d)\n",
				 reason_code, status);
		}
	} else {
		switch (reason_code) {
		case UWB_DRP_REASON_PENDING:
			uwb_rsv_set_state(rsv, UWB_RSV_STATE_O_PENDING);
			break;
		case UWB_DRP_REASON_DENIED:
			uwb_rsv_set_state(rsv, UWB_RSV_STATE_NONE);
			break;
		case UWB_DRP_REASON_CONFLICT:
			/* resolve the conflict */
			bitmap_complement(mas.bm, src->last_availability_bm,
					  UWB_NUM_MAS);
			uwb_drp_handle_conflict_rsv(rc, rsv, drp_evt, drp_ie, &mas);
			break;
		default:
			dev_warn(dev, "ignoring invalid DRP IE state (%d/%d)\n",
				 reason_code, status);
		}
	}
}

static voiruct ktermios *termios, *termios_locked;
	struct termiox *termiox;	/* May be NULL for unsupported */
	char name[64];
	struct pid *pgrp;		/* Protected by ctrl lock */
	struct pid *session;
	unsigned long flags;
	int count;
	struct winsize winsize;		/* termios mutex */
	unsigned char stopped:1, hw_stopped:1, flow_stopped:1, packet:1;
	unsigned char low_latency:1, warned:1;
	unsigned char ctrl_status;	/* ctrl_lock */
	unsigned int receive_room;	/* Bytes free for queue */

	struct tty_struct *link;
	struct fasync_struct *fasync;
	struct tty_bufhead buf;		/* Locked internally */
	int alt_speed;		/* For magic substitution of 38400 bps */
	wait_queue_head_t write_wait;
	wait_queue_head_t read_wait;
	struct work_struct hangup_work;
	void *disc_data;
	void *driver_data;
	struct list_head tty_files;

#define N_TTY_BUF_SIZE 4096

	/*
	 * The following is data for the N_TTY line discipline.  For
	 * historical reasons, this is included in the tty structure.
	 * Mostly locked by the BKL.
	 */
	unsigned int column;
	unsigned char lnext:1, erasing:1, raw:1, real_raw:1, icanon:1;
	unsigned char closing:1;
	unsigned char echo_overrun:1;
	unsigned short minimum_to_wake;
	unsigned long overrun_time;
	int num_overrun;
	unsigned long process_char_map[256/(8*sizeof(unsigned long))];
	char *read_buf;
	int read_head;
	int read_tail;
	int read_cnt;
	unsigned long read_flags[N_TTY_BUF_SIZE/(8*sizeof(unsigned long))];
	unsigned char *echo_buf;
	unsigned int echo_pos;
	unsigned int echo_cnt;
	int canon_data;
	unsigned long canon_head;
	unsigned int canon_column;
	struct mutex atomic_read_lock;
	struct mutex atomic_write_lock;
	struct mutex output_lock;
	struct mutex echo_lock;
	unsigned char *write_buf;
	int write_cnt;
	spinlock_t read_lock;
	/* If the tty has a pending do_SAK, queue it here - akpm */
	struct work_struct SAK_work;
	struct tty_port *port;
};

/* tty magic number */
#define TTY_MAGIC		0x5401

/*
 * These bits are used in the flags field of the tty structure.
 *
 * So that interrupts won't be able to mess up the queues,
 * copy_to_cooked must be atomic with respect to itself, as must
 * tty->write.  Thus, you must use the inline functions set_bit() and
 * clear_bit() to make things atomic.
 */
#define TTY_THROTTLED 		0	/* Call unthrottle() at threshold min */
#define TTY_IO_ERROR 		1	/* Cause an I/O error (may be no ldisc too) */
#define TTY_OTHER_CLOSED 	2	/* Other side (if any) has closed */
#define TTY_EXCLUSIVE 		3	/* Exclusive open mode */
#define TTY_DEBUG 		4	/* Debugging */
#define TTY_DO_WRITE_WAKEUP 	5	/* Call write_wakeup after queuing new */
#define TTY_PUSH 		6	/* n_tty private */
#define TTY_CLOSING 		7	/* ->close() in progress */
#define TTY_LDISC 		9	/* Line discipline attached */
#define TTY_LDISC_CHANGING 	10	/* Line discipline changing */
#define TTY_LDISC_OPEN	 	11	/* Line discipline is open */
#define TTY_HW_COOK_OUT 	14	/* Hardware can do output cooking */
#define TTY_HW_COOK_IN 		15	/* Hardware can do input cooking */
#define TTY_PTY_LOCK 		16	/* pty private */
#define TTY_NO_WRITE_SPLIT 	17	/* Preserve write boundaries to driver */
#define TTY_HUPPED 		18	/* Post driver->hangup() */
#define TTY_FLUSHING		19	/* Flushing to ldisc in progress */
#define TTY_FLUSHPENDING	20	/* Queued buffer flush pending */

#define TTY_WRITE_FLUSH(tty) tty_write_flush((tty))

extern void tty_write_flush(struct tty_struct *);

extern struct ktermios tty_std_termios;

extern int kmsg_redirect;

extern void console_init(void);
extern int vcs_init(void);

extern struct class *tty_class;

/**
 *	tty_kref_get		-	get a tty reference
 *	@tty: tty device
 *
 *	Return a new reference to a tty object. The caller must hold
 *	sufficient locks/counts to ensure that their existing reference cannot
 *	go away
 */

static inline struct tty_struct *tty_kref_get(struct tty_struct *tty)
{
	if (tty)
		kref_get(&tty->kref);
	return tty;
}
extern void tty_kref_put(struct tty_struct *tty);

extern int tty_paranoia_check(struct tty_struct *tty, struct inode *inode,
			      const char *routine);
extern char *tty_name(struct tty_struct *tty, char *buf);
extern void tty_wait_until_sent(struct tty_struct *tty, long timeout);
extern int tty_check_change(struct tty_struct *tty);
extern void stop_tty(struct tty_struct *tty);
extern void start_tty(struct tty_struct *tty);
extern int tty_register_driver(struct tty_driver *driver);
extern int tty_unregister_driver(struct tty_driver *driver);
extern struct device *tty_register_device(struct tty_driver *driver,
					  unsigned index, struct device *dev);
extern void tty_unregister_device(struct tty_driver *driver, unsigned index);
extern int tty_read_raw_data(struct tty_struct *tty, unsigned char *bufp,
			     int buflen);
extern void tty_write_message(struct tty_struct *tty, char *msg);
extern int tty_put_char(struct tty_struct *tty, unsigned char c);
extern int tty_chars_in_buffer(struct tty_struct *tty);
extern int tty_write_room(struct tty_struct *tty);
extern void tty_driver_flush_buffer(struct tty_struct *tty);
extern void tty_throttle(struct tty_struct *tty);
extern void tty_unthrottle(struct tty_struct *tty);
extern int tty_do_resize(struct tty_struct *tty, struct winsize *ws);
extern void tty_shutdown(struct tty_struct *tty);
extern void tty_free_termios(struct tty_struct *tty);
extern int is_current_pgrp_orphaned(void);
extern struct pid *tty_get_pgrp(struct tty_struct *tty);
extern int is_ignored(int sig);
extern int tty_signal(int sig, struct tty_struct *tty);
extern void tty_hangup(struct tty_struct *tty);
extern void tty_vhangup(struct tty_struct *tty);
extern void tty_vhangup_self(void);
extern void tty_unhangup(struct file *filp);
extern int tty_hung_up_p(struct file *filp);
extern void do_SAK(struct tty_struct *tty);
extern void __do_SAK(struct tty_struct *tty);
extern void disassociate_ctty(int priv);
extern void no_tty(void);
extern void tty_flip_buffer_push(struct tty_struct *tty);
extern void tty_flush_to_ldisc(struct tty_struct *tty);
extern void tty_buffer_free_all(struct tty_struct *tty);
extern void tty_buffer_flush(struct tty_struct *tty);
extern void tty_buffer_init(struct tty_struct *tty);
extern speed_t tty_get_baud_rate(struct tty_struct *tty);
extern speed_t tty_termios_baud_rate(struct ktermios *termios);
extern speed_t tty_termios_input_baud_rate(struct ktermios *termios);
extern void tty_termios_encode_baud_rate(struct ktermios *termios,
						speed_t ibaud, speed_t obaud);
extern void tty_encode_baud_rate(struct tty_struct *tty,
						speed_t ibaud, speed_t obaud);
extern void tty_termios_copy_hw(struct ktermios *new, struct ktermios *old);
extern int tty_termios_hw_change(struct ktermios *a, struct ktermios *b);

extern struct tty_ldisc *tty_ldisc_ref(struct tty_struct *);
extern void tty_ldisc_deref(struct tty_ldisc *);
extern struct tty_ldisc *tty_ldisc_ref_wait(struct tty_struct *);
extern void tty_ldisc_hangup(struct tty_struct *tty);
extern const struct file_operations tty_ldiscs_proc_fops;

extern void tty_wakeup(struct tty_struct *tty);
extern void tty_ldisc_flush(struct tty_struct *tty);

extern long tty_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
extern int tty_mode_ioctl(struct tty_struct *tty, struct file *file,
			unsigned int cmd, unsigned long arg);
extern int tty_perform_flush(struct tty_struct *tty, unsigned long arg);
extern dev_t tty_devnum(struct tty_struct *tty);
extern void proc_clear_tty(struct task_struct *p);
extern struct tty_struct *get_current_tty(void);
extern void tty_default_fops(struct file_operations *fops);
extern struct tty_struct *alloc_tty_struct(void);
extern void free_tty_struct(struct tty_struct *tty);
extern void initialize_tty_struct(struct tty_struct *tty,
		struct tty_driver *driver, int idx);
extern struct tty_struct *tty_init_dev(struct tty_driver *driver, int idx,
								int first_ok);
extern void tty_release_dev(struct file *filp);
extern int tty_init_termios(struct tty_struct *tty);

extern struct tty_struct *tty_pair_get_tty(struct tty_struct *tty);
extern struct tty_struct *tty_pair_get_pty(struct tty_struct *tty);

extern struct mutex tty_mutex;

extern void tty_write_unlock(struct tty_struct *tty);
extern int tty_write_lock(struct tty_struct *tty, int ndelay);
#define tty_is_writelocked(tty)  (mutex_is_locked(&tty->atomic_write_lock))

extern void tty_port_init(struct tty_port *port);
extern int tty_port_alloc_xmit_buf(struct tty_port *port);
extern void tty_port_free_xmit_buf(struct tty_port *port);
extern struct tty_struct *tty_port_tty_get(struct tty_port *port);
extern void tty_port_tty_set(struct tty_port *port, struct tty_struct *tty);
extern int tty_port_carrier_raised(struct tty_port *port);
extern void tty_port_raise_dtr_rts(struct tty_port *port);
extern void tty_port_lower_dtr_rts(struct tty_port *port);
extern void tty_port_hangup(struct tty_port *port);
extern int tty_port_block_til_ready(struct tty_port *port,
				struct tty_struct *tty, struct file *filp);
extern int tty_port_close_start(struct tty_port *port,
				struct tty_struct *tty, struct file *filp);
extern void tty_port_close_end(struct tty_port *port, struct tty_struct *tty);

extern int tty_register_ldisc(int disc, struct tty_ldisc_ops *new_ldisc);
extern int tty_unregister_ldisc(int disc);
extern int tty_set_ldisc(struct tty_struct *tty, int ldisc);
extern int tty_ldisc_setup(struct tty_struct *tty, struct tty_struct *o_tty);
extern void tty_ldisc_release(struct tty_struct *tty, struct tty_struct *o_tty);
extern void tty_ldisc_init(struct tty_struct *tty);
extern void tty_ldisc_begin(void);
/* This last one is just for the tty layer internals and shouldn't be used elsewhere */
extern void tty_ldisc_enable(struct tty_struct *tty);


/* n_tty.c */
extern struct tty_ldisc_ops tty_ldisc_N_TTY;

/* tty_audit.c */
#ifdef CONFIG_AUDIT
extern void tty_audit_add_data(struct tty_struct *tty, unsigned char *data,
			       size_t size);
extern void tty_audit_exit(void);
extern void tty_audit_fork(struct signal_struct *sig);
extern void tty_audit_tiocsti(struct tty_struct *tty, char ch);
extern void tty_audit_push(struct tty_struct *tty);
extern void tty_audit_push_task(struct task_struct *tsk,
					uid_t loginuid, u32 sessionid);
#else
static inline void tty_audit_add_data(struct tty_struct *tty,
				      unsigned char *data, size_t size)
{
}
static inline void tty_audit_tiocsti(struct tty_struct *tty, char ch)
{
}
static inline void tty_audit_exit(void)
{
}
static inline void tty_audit_fork(struct signal_struct *sig)
{
}
static inline void tty_audit_push(struct tty_struct *tty)
{
}
static inline void tty_audit_push_task(struct task_struct *tsk,
					uid_t loginuid, u32 sessionid)
{
}
#endif

/* tty_ioctl.c */
extern int n_tty_ioctl_helper(struct tty_struct *tty, struct file *file,
		       unsigned int cmd, unsigned long arg);

/* serial.c */

extern void serial_console_init(void);

/* pcxx.c */

extern int pcxe_open(struct tty_struct *tty, struct file *filp);

/* printk.c */

extern void console_print(const char *);

/* vt.c */

extern int vt_ioctl(struct tty_struct *tty, struct file *file,
		    unsigned int cmd, unsigned long arg);

#endif /* __KERNEL__ */
#endif
