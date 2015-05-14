s;
	int tap1, tap2, tap3, tap4, tap5;
} poolinfo_table[] = {
	/* x^128 + x^103 + x^76 + x^51 +x^25 + x + 1 -- 105 */
	{ 128,	103,	76,	51,	25,	1 },
	/* x^32 + x^26 + x^20 + x^14 + x^7 + x + 1 -- 15 */
	{ 32,	26,	20,	14,	7,	1 },
#if 0
	/* x^2048 + x^1638 + x^1231 + x^819 + x^411 + x + 1  -- 115 */
	{ 2048,	1638,	1231,	819,	411,	1 },

	/* x^1024 + x^817 + x^615 + x^412 + x^204 + x + 1 -- 290 */
	{ 1024,	817,	615,	412,	204,	1 },

	/* x^1024 + x^819 + x^616 + x^410 + x^207 + x^2 + 1 -- 115 */
	{ 1024,	819,	616,	410,	207,	2 },

	/* x^512 + x^411 + x^308 + x^208 + x^104 + x + 1 -- 225 */
	{ 512,	411,	308,	208,	104,	1 },

	/* x^512 + x^409 + x^307 + x^206 + x^102 + x^2 + 1 -- 95 */
	{ 512,	409,	307,	206,	102,	2 },
	/* x^512 + x^409 + x^309 + x^205 + x^103 + x^2 + 1 -- 95 */
	{ 512,	409,	309,	205,	103,	2 },

	/* x^256 + x^205 + x^155 + x^101 + x^52 + x + 1 -- 125 */
	{ 256,	205,	155,	101,	52,	1 },

	/* x^128 + x^103 + x^78 + x^51 + x^27 + x^2 + 1 -- 70 */
	{ 128,	103,	78,	51,	27,	2 },

	/* x^64 + x^52 + x^39 + x^26 + x^14 + x + 1 -- 15 */
	{ 64,	52,	39,	26,	14,	1 },
#endif
};

#define POOLBITS	poolwords*32
#define POOLBYTES	poolwords*4

/*
 * For the purposes of better mixing, we use the CRC-32 polynomial as
 * well to make a twisted Generalized Feedback Shift Reigster
 *
 * (See M. Matsumoto & Y. Kurita, 1992.  Twisted GFSR generators.  ACM
 * Transactions on Modeling and Computer Simulation 2(3):179-194.
 * Also see M. Matsumoto & Y. Kurita, 1994.  Twisted GFSR generators
 * II.  ACM Transactions on Mdeling and Computer Simulation 4:254-266)
 *
 * Thanks to Colin Plumb for suggesting this.
 *
 * We have not analyzed the resultant polynomial to prove it primitive;
 * in fact it almost certainly isn't.  Nonetheless, the irreducible factors
 * of a random large-degree polynomial over GF(2) are more than large enough
 * that periodicity is not a concern.
 *
 * The input hash is much less sensitive than the output hash.  All
 * that we want of it is that it be a good non-cryptographic hash;
 * i.e. it not produce collisions when fed "random" data of the sort
 * we expect to see.  As long as the pool state differs for different
 * inputs, we have preserved the input entropy and done a good job.
 * The fact that an intelligent attacker can construct inputs that
 * will produce controlled alterations to the pool's state is not
 * important because we don't consider such inputs to contribute any
 * randomness.  The only property we need with respect to them is that
 * the attacker can't increase his/her knowledge of the pool's state.
 * Since all additions are reversible (knowing the final state and the
 * input, you can reconstruct the initial state), if an attacker has
 * any uncertainty about the initial state, he/she can only shuffle
 * that uncertainty about, but never cause any collisions (which would
 * decrease the uncertainty).
 *
 * The chosen system lets the state of the pool be (essentially) the input
 * modulo the generator polymnomial.  Now, for random primitive polynomials,
 * this is a universal class of hash functions, meaning that the chance
 * of a collision is limited by the attacker's knowledge of the generator
 * polynomail, so if it is chosen at random, an attacker can never force
 * a collision.  Here, we use a fixed polynomial, but we *can* assume that
 * ###--> it is unknown to the processes generating the input entropy. <-###
 * Because of this important property, this is a good, collision-resistant
 * hash; hash collisions will occur no more often than chance.
 */

/*
 * Static global variables
 */
static DECLARE_WAIT_QUEUE_HEAD(random_read_wait);
static DECLARE_WAIT_QUEUE_HEAD(random_write_wait);
static struct fasync_struct *fasync;

#if 0
static int debug;
module_param(debug, bool, 0644);
#define DEBUG_ENT(fmt, arg...) do { \
	if (debug) \
		printk(KERN_DEBUG "random %04d %04d %04d: " \
		fmt,\
		input_pool.entropy_count,\
		blocking_pool.entropy_count,\
		nonblocking_pool.entropy_count,\
		## arg); } while (0)
#else
#define DEBUG_ENT(fmt, arg...) do {} while (0)
#endif

/**********************************************************************
 *
 * OS independent entropy store.   Here are the functions which handle
 * storing entropy in an entropy pool.
 *
 **********************************************************************/

struct entropy_store;
struct entropy_store {
	/* read-only data: */
	struct poolinfo *poolinfo;
	__u32 *pool;
	const char *name;
	int limit;
	struct entropy_store *pull;

	/* read-write data: */
	spinlock_t lock;
	unsigned add_ptr;
	int entropy_count;
	int input_rotate;
};

static __u32 input_pool_data[INPUT_POOL_WORDS];
static __u32 blocking_pool_data[OUTPUT_POOL_WORDS];
static __u32 nonblocking_pool_data[OUTPUT_POOL_WORDS];

static struct entropy_store input_pool = {
	.poolinfo = &poolinfo_table[0],
	.name = "input",
	.limit = 1,
	.lock = __SPIN_LOCK_UNLOCKED(&input_pool.lock),
	.pool = input_pool_data
};

static struct entropy_store blocking_pool = {
	.poolinfo = &poolinfo_table[1],
	.name = "blocking",
	.limit = 1,
	.pull = &input_pool,
	.lock = __SPIN_LOCK_UNLOCKED(&blocking_pool.lock),
	.pool = blocking_pool_data
};