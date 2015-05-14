ticated messages
 */
struct ns_tsig_key {
        char name[NS_MAXDNAME], alg[NS_MAXDNAME];
        unsigned char *data;
        int len;
};
typedef struct ns_tsig_key ns_tsig_key;

/*%
 * This structure is used for TSIG authenticated TCP messages
 */
struct ns_tcp_tsig_state {
	int counter;
	struct dst_key *key;
	void *ctx;
	unsigned char sig[NS_PACKETSZ];
	int siglen;
};
typedef struct ns_tcp_tsig_state ns_tcp_tsig_state;

#define NS_TSIG_FUDGE 300
#define NS_TSIG_TCP_COUNT 100
#define NS_TSIG_ALG_HMAC_MD5 "HMAC-MD5.SIG-ALG.REG.INT"

#define NS_TSIG_ERROR_NO_TSIG -10
#define NS_TSIG_ERROR_NO_SPACE -11
#define NS_TSIG_ERROR_FORMERR -12

/*%
 * Currently defined type values for resources and queries.
 */
typedef enum __ns_type {
	ns_t_invalid = 0,	/*%< Cookie. */
	ns_t_a = 1,		/*%< Host address. */
	ns_t_ns = 2,		/*%< Authoritative server. */
	ns_t_md = 3,		/*%< Mail destination. */
	ns_t_mf = 4,		/*%< Mail forwarder. */
	ns_t_cname = 5,		/*%< Canonical name. */
	ns_t_soa = 6,		/*%< Start of authority zone. */
	ns_t_mb = 7,		/*%< Mailbox domain name. */
	ns_t_mg = 8,		/*%< Mail group member. */
	ns_t_mr = 9,		/*%< Mail rename name. */
	ns_t_null = 10,		/*%< Null resource record. */
	ns_t_wks = 11,		/*%< Well known service. */
	ns_t_ptr = 12,		/*%< Domain name pointer. */
	ns_t_hinfo = 13,	/*%< Host information. */
	ns_t_minfo = 14,	/*%< Mailbox information. */
	ns_t_mx = 15,		/*%< Mail routing information. */
	ns_t_txt = 16,		/*%< Text strings. */
	ns_t_rp = 17,		/*%< Responsible person. */
	ns_t_afsdb = 18,	/*%< AFS cell database. */
	ns_t_x25 = 19,		/*%< X_25 calling address. */
	ns_t_isdn = 20,		/*%< ISDN calling address. */
	ns_t_rt = 21,		/*%< Router. */
	ns_t_nsap = 22,		/*%< NSAP address. */
	ns_t_nsap_ptr = 23,	/*%< Reverse NSAP lookup (deprecated). */
	ns_t_sig = 24,		/*%< Security signature. */
	ns_t_key = 25,		/*%< Security key. */
	ns_t_px = 26,		/*%< X.400 mail mapping. */
	ns_t_gpos = 27,		/*%< Geographical position (withdrawn). */
	ns_t_aaaa = 28,		/*%< Ip6 Address. */
	ns_t_loc = 29,		/*%< Location Information. */
	ns_t_nxt = 30,		/*%< Next domain (security). */
	ns_t_eid = 31,		/*%< Endpoint identifier. */
	ns_t_nimloc = 32,	/*%< Nimrod Locator. */
	ns_t_srv = 33,		/*%< Server Selection. */
	ns_t_atma = 34,		/*%< ATM Address */
	ns_t_naptr = 35,	/*%< Naming Authority PoinTeR */
	ns_t_kx = 36,		/*%< Key Exchange */
	ns_t_cert = 37,		/*%< Certification record */
	ns_t_a6 = 38,		/*%< IPv6 address (deprecated, use ns_t_aaaa) */
	ns_t_dname = 39,	/*%< Non-terminal DNAME (for IPv6) */
	ns_t_sink = 40,		/*%< Kitchen sink (experimentatl) */
	ns_t_opt = 41,		/*%< EDNS0 option (meta-RR) */
	ns_t_apl = 42,		/*%< Address prefix list (RFC3123) */
	ns_t_tkey = 249,	/*%< Transaction key */
	ns_t_tsig = 250,	/*%< Transaction signature. */
	ns_t_ixfr = 251,	/*%< Incremental zone transfer. */
	ns_t_axfr = 252,	/*%< Transfer zone of authority. */
	ns_t_mailb = 253,	/*%< Transfer mailbox records. */
	ns_t_maila = 254,	/*%< Transfer mail agent records. */
	ns_t_any = 255,		/*%< Wildcard match. */
	ns_t_zxfr = 256,	/*%< BIND-specific, nonstandard. */
	ns_t_max = 65536
} ns_type;

/* Exclusively a QTYPE? (not also an RTYPE) */
#define	ns_t_qt_p(t) (ns_t_xfr_p(t) || (t) == ns_t_any || \
		      (t) == ns_t_mailb || (t) == ns_t_maila)
/* Some kind of meta-RR? (not a QTYPE, but also not an RTYPE) */
#define	ns_t_mrr_p(t) ((t) == ns_t_tsig || (t) == ns_t_opt)
/* Exclusively an RTYPE? (not also a QTYPE or a meta-RR) */
#define ns_t_rr_p(t) (!ns_t_qt_p(t) && !ns_t_mrr_p(t))
#define ns_t_udp_p(t) ((t) != ns_t_axfr && (t) != ns_t_zxfr)
#define ns_t_xfr_p(t) ((t) == ns_t_axfr || (t) == ns_t_ixfr || \
		       (t) == ns_t_zxfr)

/*%
 * Values for class field
 */
typedef enum __ns_class {
	ns_c_invalid = 0,	/*%< Cookie. */
	ns_c_in = 1,		/*%< Internet. */
	ns_c_2 = 2,		/*%< unallocated/unsupported. */
	ns_c_chaos = 3,		/*%< MIT Chaos-net. */
	ns_c_hs = 4,		/*%< MIT Hesiod. */
	/* Query class values which do not appear in resource records */
	ns_c_none = 254,	/*%< for prereq. sections in update requests */
	ns_c_any = 255,		/*%< Wildcard match. */
	ns_c_max = 65536
} ns_class;

/* DNSSEC constants. */

typedef enum __ns_key_types {
	ns_kt_rsa = 1,		/*%< key type RSA/MD5 */
	ns_kt_dh  = 2,		/*%< Diffie Hellman */
	ns_kt_dsa = 3,		/*%< Digital Signature Standard (MANDATORY) */
	ns_kt_private = 254	/*%< Private key type starts with OID */
} ns_key_types;

typedef enum __ns_cert_types {
	cert_t_pkix = 1,	/*%< PKIX (X.509v3) */
	cert_t_spki = 2,	/*%< SPKI */
	cert_t_pgp  = 3,	/*%< PGP */
	cert_t_url  = 253,	/*%< URL private type */
	cert_t_oid  = 254	/*%< OID private type */
} ns_cert_types;

/* Flags field of the KEY RR rdata. */
#define	NS_KEY_TYPEMASK		0xC000	/*%< Mask for "type" bits */
#define	NS_KEY_TYPE_AUTH_CONF	0x0000	/*%< Key usable for both */
#define	NS_KEY_TYPE_CONF_ONLY	0x8000	/*%< Key usable for confidentiality */
#define	NS_KEY_TYPE_AUTH_ONLY	0x4000	/*%< Key usable for authentication */
#define	NS_KEY_TYPE_NO_KEY	0xC000	/*%< No key usable for either; no key */
/* The type bits can also be interpreted independently, as single bits: */
#define	NS_KEY_NO_AUTH		0x8000	/*%< Key unusable for authentication */
#define	NS_KEY_NO_CONF		0x4000	/*%< Key unusable for confidentiality */
#define	NS_KEY_RESERVED2	0x2000	/* Security is *mandatory* if bit=0 */
#define	NS_KEY_EXTENDED_FLAGS	0x1000	/*%< reserved - must be zero */
#define	NS_KEY_RESERVED4	0x0800  /*%< reserved - must be zero */
#define	NS_KEY_RESERVED5	0x0400  /*%< reserved - must be zero */
#define	NS_KEY_NAME_TYPE	0x0300	/*%< these bits determine the type */
#define	NS_KEY_NAME_USER	0x0000	/*%< key is assoc. with user */
#define	NS_KEY_NAME_ENTITY	0x0200	/*%< key is assoc. with entity eg host */
#define	NS_KEY_NAME_ZONE	0x0100	/*%< key is zone key */
#define	NS_KEY_NAME_RESERVED	0x0300	/*%< reserved meaning */
#define	NS_KEY_RESERVED8	0x0080  /*%< reserved - must be zero */
#define	NS_KEY_RESERVED9	0x0040  /*%< reserved - must be zero */
#define	NS_KEY_RESERVED10	0x0020  /*%< reserved - must be zero */
#define	NS_KEY_RESERVED11	0x0010  /*%< reserved - must be zero */
#define	NS_KEY_SIGNATORYMASK	0x000F	/*%< key can sign RR's of same name */
#define	NS_KEY_RESERVED_BITMASK ( NS_KEY_RESERVED2 | \
				  NS_KEY_RESERVED4 | \
				  NS_KEY_RESERVED5 | \
				  NS_KEY_RESERVED8 | \
				  NS_KEY_RESERVED9 | \
				  NS_KEY_RESERVED10 | \
				  NS_KEY_RESERVED11 )
#define NS_KEY_RESERVED_BITMASK2 0xFFFF /*%< no bits defined here */
/* The Algorithm field of the KEY and SIG RR's is an integer, {1..254} */
#define	NS_ALG_MD5RSA		1	/*%< MD5 with RSA */
#define	NS_ALG_DH               2	/*%< Diffie Hellman KEY */
#define	NS_ALG_DSA              3	/*%< DSA KEY */
#define	NS_ALG_DSS              NS_ALG_DSA
#define	NS_ALG_EXPIRE_ONLY	253	/*%< No alg, no security */
#define	NS_ALG_PRIVATE_OID	254	/*%< Key begins with OID giving alg */
/* Protocol values  */
/* value 0 is reserved */
#define NS_KEY_PROT_TLS         1
#define NS_KEY_PROT_EMAIL       2
#define NS_KEY_PROT_DNSSEC      3
#define NS_KEY_PROT_IPSEC       4
#define NS_KEY_PROT_ANY		255

/* Signatures */
#define	NS_MD5RSA_MIN_BITS	 512	/*%< Size of a mod or exp in bits */
#define	NS_MD5RSA_MAX_BITS	4096
	/* Total of binary mod and exp */
#define	NS_MD5RSA_MAX_BYTES	((NS_MD5RSA_MAX_BITS+7/8)*2+3)
	/* Max length of text sig block */
#define	NS_MD5RSA_MAX_BASE64	(((NS_MD5RSA_MAX_BYTES+2)/3)*4)
#define NS_MD5RSA_MIN_SIZE	((NS_MD5RSA_MIN_BITS+7)/8)
#define NS_MD5RSA_MAX_SIZE	((NS_MD5RSA_MAX_BITS+7)/8)

#define NS_DSA_SIG_SIZE         41
#define NS_DSA_MIN_SIZE         213
#define NS_DSA_MAX_BYTES        405

/* Offsets into SIG record rdata to find various values */
#define	NS_SIG_TYPE	0	/*%< Type flags */
#define	NS_SIG_ALG	2	/*%< Algorithm */
#define	NS_SIG_LABELS	3	/*%< How many labels in name */
#define	NS_SIG_OTTL	4	/*%< Original TTL */
#define	NS_SIG_EXPIR	8	/*%< Expiration time */
#define	NS_SIG_SIGNED	12	/*%< Signature time */
#define	NS_SIG_FOOT	16	/*%< Key footprint */
#define	NS_SIG_SIGNER	18	/*%< Domain name of who signed it */
/* How RR types are represented as bit-flags in NXT records */
#define	NS_NXT_BITS 8
#define	NS_NXT_BIT_SET(  n,p) (p[(n)/NS_NXT_BITS] |=  (0x80>>((n)%NS_NXT_BITS)))
#define	NS_NXT_BIT_CLEAR(n,p) (p[(n)/NS_NXT_BITS] &= ~(0x80>>((n)%NS_NXT_BITS)