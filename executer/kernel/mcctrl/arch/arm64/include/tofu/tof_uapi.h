#ifndef _TOF_UAPI_H_
#define _TOF_UAPI_H_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#endif

enum tof_sig_errno_cq {
	TOF_TOQ_DIRECT_DESCRIPTOR_EXCEPTION,
	TOF_TOQ_SOURCE_TYPE_EXCEPTION,
	TOF_MRQ_OVERFLOW,
	TOF_CQS_CACHEFLUSH_TIMEOUT,
};

enum tof_sig_errno_bg {
	TOF_NODE_ADDRESS_UNMATCH,
	TOF_BSEQ_UNMATCH,
	TOF_SIGNAL_STATE_ERROR,
	TOF_ERROR_SYNCHRONIZATION_ACKNOWLEDGE,
};

#define TOF_UAPI_VERSION 0x2a00

struct tof_init_cq {
	uint16_t version;
	uint8_t session_mode;
	uint8_t toq_size;
	uint8_t mrq_size;
	uint8_t num_stag;
	uint8_t tcq_cinj;
	uint8_t mrq_cinj;
	void *toq_mem;
	void *tcq_mem;
	void *mrq_mem;
};

struct tof_alloc_stag {
	uint32_t flags;
	int stag;
	uint64_t offset;
	void *va;
	uint64_t len;
};

struct tof_free_stags {
	uint16_t num;
	int *stags;
};

struct tof_addr {
	uint8_t pa;
	uint8_t pb;
	uint8_t pc;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t a;
	uint8_t b;
	uint8_t c;
};

struct tof_set_bg {
	int tni;
	int gate;
	int source_lgate;
	struct tof_addr source_raddr;
	int source_rtni;
	int source_rgate;
	int dest_lgate;
	struct tof_addr dest_raddr;
	int dest_rtni;
	int dest_rgate;
};

struct tof_enable_bch {
	void *addr;
	int bseq;
	int num;
	struct tof_set_bg *bgs;
};

struct tof_set_subnet {
	int res0;
	int res1;
	uint8_t nx;
	uint8_t sx;
	uint8_t lx;
	uint8_t ny;
	uint8_t sy;
	uint8_t ly;
	uint8_t nz;
	uint8_t sz;
	uint8_t lz;
};

struct tof_reg_user {
	uid_t uid;
	uint32_t gpid;
	struct tof_set_subnet subnet;
	uint64_t *cqmask;
	uint64_t *bgmask;
};

struct tof_notify_linkdown {
	int num;
	struct {
		uint8_t x;
		uint8_t y;
		uint8_t z;
		uint8_t a;
		uint8_t b;
		uint8_t c;
		uint16_t ports;
	} *items;
};

struct tof_get_port_stat {
	int port_no;
	uint64_t mask;
	uint64_t pa[31];
};

struct tof_get_cq_stat {
	int tni;
	int cqid;
	uint64_t txbyte;
	uint64_t rxbyte;
};

struct tof_load_register {
	uint64_t pa;
	uint64_t len;
	void *buf;
};

struct tof_load_resource {
	uint64_t rsc_id;
	uint64_t offset;
	uint64_t len;
	void *buf;
};

union tof_trans_table_bitfield {
	struct {
		uint64_t start:36;
		uint64_t len:27;
		uint64_t ps_code:1;
	} bits;
	uint64_t atomic;
};

struct tof_trans_table {
	union tof_trans_table_bitfield steering;
	union tof_trans_table_bitfield mbpt;
};

void tof_utofu_set_linkdown_callback(void (*callback)(int, const void *));
void tof_utofu_unset_linkdown_callback(void);

#define TOF_MMAP_CQ_REGISTER 0
#ifdef __KERNEL__
#define TOF_MMAP_CQ_TRANSTABLE (PAGE_SIZE)
#else
#define TOF_MMAP_CQ_TRANSTABLE (sysconf(_SC_PAGESIZE))
#endif
#define TOF_MMAP_BCH_REGISTER 0
#define TOF_MMAP_XB_STQ 0

#define TOF_ST_RDWR 0x0
#define TOF_ST_RDONLY 0x1
#define TOF_ST_LPG 0x2

#define TOF_STAG_TRANS_PS_CODE_64KB 0
#define TOF_STAG_TRANS_PS_CODE_2MB  1

#define TOF_IOC_MAGIC 'd'
#define TOF_IOCTL_INIT_CQ _IOWR(TOF_IOC_MAGIC, 0, long)
#define TOF_IOCTL_ALLOC_STAG _IOWR(TOF_IOC_MAGIC, 1, long)
#define TOF_IOCTL_FREE_STAGS _IOWR(TOF_IOC_MAGIC, 2, long)
#define TOF_IOCTL_ENABLE_BCH _IOWR(TOF_IOC_MAGIC, 3, long)
#define TOF_IOCTL_DISABLE_BCH _IOWR(TOF_IOC_MAGIC, 4, long)
#define TOF_IOCTL_SET_RT_SIGNAL _IOWR(TOF_IOC_MAGIC, 5, long)
#define TOF_IOCTL_SET_SUBNET _IOWR(TOF_IOC_MAGIC, 6, long)
#define TOF_IOCTL_REG_USER _IOWR(TOF_IOC_MAGIC, 7, long)
#define TOF_IOCTL_NOTIFY_LINKDOWN _IOWR(TOF_IOC_MAGIC, 8, long)
#define TOF_IOCTL_GET_PORT_STAT _IOWR(TOF_IOC_MAGIC, 9, long)
#define TOF_IOCTL_GET_CQ_STAT _IOWR(TOF_IOC_MAGIC, 10, long)
#define TOF_IOCTL_LOAD_REGISTER _IOWR(TOF_IOC_MAGIC, 11, long)
#define TOF_IOCTL_LOAD_RESOURCE _IOWR(TOF_IOC_MAGIC, 12, long)

#ifndef __KERNEL__
#define TOF_INIT_CQ TOF_IOCTL_INIT_CQ
#define TOF_ALLOC_STAG TOF_IOCTL_ALLOC_STAG
#define TOF_FREE_STAGS TOF_IOCTL_FREE_STAGS
#define TOF_ENABLE_BCH TOF_IOCTL_ENABLE_BCH
#define TOF_DISABLE_BCH TOF_IOCTL_DISABLE_BCH
#define TOF_SET_RT_SIGNAL TOF_IOCTL_SET_RT_SIGNAL
#define TOF_SET_SUBNET TOF_IOCTL_SET_SUBNET
#define TOF_REG_USER TOF_IOCTL_REG_USER
#define TOF_NOTIFY_LINKDOWN TOF_IOCTL_NOTIFY_LINKDOWN
#define TOF_GET_PORT_STAT TOF_IOCTL_GET_PORT_STAT
#define TOF_GET_CQ_STAT TOF_IOCTL_GET_CQ_STAT
#define TOF_LOAD_REGISTER TOF_IOCTL_LOAD_REGISTER
#define TOF_LOAD_RESOURCE TOF_IOCTL_LOAD_RESOURCE
#endif

enum {
	/* TOQ (0 - 71) */
	TOF_RSC_TNI0_TOQ0 = 0, TOF_RSC_TNI0_TOQ1, TOF_RSC_TNI0_TOQ2, TOF_RSC_TNI0_TOQ3,
	TOF_RSC_TNI0_TOQ4, TOF_RSC_TNI0_TOQ5, TOF_RSC_TNI0_TOQ6, TOF_RSC_TNI0_TOQ7,
	TOF_RSC_TNI0_TOQ8, TOF_RSC_TNI0_TOQ9, TOF_RSC_TNI0_TOQ10, TOF_RSC_TNI0_TOQ11,
	TOF_RSC_TNI1_TOQ0, TOF_RSC_TNI1_TOQ1, TOF_RSC_TNI1_TOQ2, TOF_RSC_TNI1_TOQ3,
	TOF_RSC_TNI1_TOQ4, TOF_RSC_TNI1_TOQ5, TOF_RSC_TNI1_TOQ6, TOF_RSC_TNI1_TOQ7,
	TOF_RSC_TNI1_TOQ8, TOF_RSC_TNI1_TOQ9, TOF_RSC_TNI1_TOQ10, TOF_RSC_TNI1_TOQ11,
	TOF_RSC_TNI2_TOQ0, TOF_RSC_TNI2_TOQ1, TOF_RSC_TNI2_TOQ2, TOF_RSC_TNI2_TOQ3,
	TOF_RSC_TNI2_TOQ4, TOF_RSC_TNI2_TOQ5, TOF_RSC_TNI2_TOQ6, TOF_RSC_TNI2_TOQ7,
	TOF_RSC_TNI2_TOQ8, TOF_RSC_TNI2_TOQ9, TOF_RSC_TNI2_TOQ10, TOF_RSC_TNI2_TOQ11,
	TOF_RSC_TNI3_TOQ0, TOF_RSC_TNI3_TOQ1, TOF_RSC_TNI3_TOQ2, TOF_RSC_TNI3_TOQ3,
	TOF_RSC_TNI3_TOQ4, TOF_RSC_TNI3_TOQ5, TOF_RSC_TNI3_TOQ6, TOF_RSC_TNI3_TOQ7,
	TOF_RSC_TNI3_TOQ8, TOF_RSC_TNI3_TOQ9, TOF_RSC_TNI3_TOQ10, TOF_RSC_TNI3_TOQ11,
	TOF_RSC_TNI4_TOQ0, TOF_RSC_TNI4_TOQ1, TOF_RSC_TNI4_TOQ2, TOF_RSC_TNI4_TOQ3,
	TOF_RSC_TNI4_TOQ4, TOF_RSC_TNI4_TOQ5, TOF_RSC_TNI4_TOQ6, TOF_RSC_TNI4_TOQ7,
	TOF_RSC_TNI4_TOQ8, TOF_RSC_TNI4_TOQ9, TOF_RSC_TNI4_TOQ10, TOF_RSC_TNI4_TOQ11,
	TOF_RSC_TNI5_TOQ0, TOF_RSC_TNI5_TOQ1, TOF_RSC_TNI5_TOQ2, TOF_RSC_TNI5_TOQ3,
	TOF_RSC_TNI5_TOQ4, TOF_RSC_TNI5_TOQ5, TOF_RSC_TNI5_TOQ6, TOF_RSC_TNI5_TOQ7,
	TOF_RSC_TNI5_TOQ8, TOF_RSC_TNI5_TOQ9, TOF_RSC_TNI5_TOQ10, TOF_RSC_TNI5_TOQ11,

	/* TOQ (72 - 143) */
	TOF_RSC_TNI0_TCQ0, TOF_RSC_TNI0_TCQ1, TOF_RSC_TNI0_TCQ2, TOF_RSC_TNI0_TCQ3,
	TOF_RSC_TNI0_TCQ4, TOF_RSC_TNI0_TCQ5, TOF_RSC_TNI0_TCQ6, TOF_RSC_TNI0_TCQ7,
	TOF_RSC_TNI0_TCQ8, TOF_RSC_TNI0_TCQ9, TOF_RSC_TNI0_TCQ10, TOF_RSC_TNI0_TCQ11,
	TOF_RSC_TNI1_TCQ0, TOF_RSC_TNI1_TCQ1, TOF_RSC_TNI1_TCQ2, TOF_RSC_TNI1_TCQ3,
	TOF_RSC_TNI1_TCQ4, TOF_RSC_TNI1_TCQ5, TOF_RSC_TNI1_TCQ6, TOF_RSC_TNI1_TCQ7,
	TOF_RSC_TNI1_TCQ8, TOF_RSC_TNI1_TCQ9, TOF_RSC_TNI1_TCQ10, TOF_RSC_TNI1_TCQ11,
	TOF_RSC_TNI2_TCQ0, TOF_RSC_TNI2_TCQ1, TOF_RSC_TNI2_TCQ2, TOF_RSC_TNI2_TCQ3,
	TOF_RSC_TNI2_TCQ4, TOF_RSC_TNI2_TCQ5, TOF_RSC_TNI2_TCQ6, TOF_RSC_TNI2_TCQ7,
	TOF_RSC_TNI2_TCQ8, TOF_RSC_TNI2_TCQ9, TOF_RSC_TNI2_TCQ10, TOF_RSC_TNI2_TCQ11,
	TOF_RSC_TNI3_TCQ0, TOF_RSC_TNI3_TCQ1, TOF_RSC_TNI3_TCQ2, TOF_RSC_TNI3_TCQ3,
	TOF_RSC_TNI3_TCQ4, TOF_RSC_TNI3_TCQ5, TOF_RSC_TNI3_TCQ6, TOF_RSC_TNI3_TCQ7,
	TOF_RSC_TNI3_TCQ8, TOF_RSC_TNI3_TCQ9, TOF_RSC_TNI3_TCQ10, TOF_RSC_TNI3_TCQ11,
	TOF_RSC_TNI4_TCQ0, TOF_RSC_TNI4_TCQ1, TOF_RSC_TNI4_TCQ2, TOF_RSC_TNI4_TCQ3,
	TOF_RSC_TNI4_TCQ4, TOF_RSC_TNI4_TCQ5, TOF_RSC_TNI4_TCQ6, TOF_RSC_TNI4_TCQ7,
	TOF_RSC_TNI4_TCQ8, TOF_RSC_TNI4_TCQ9, TOF_RSC_TNI4_TCQ10, TOF_RSC_TNI4_TCQ11,
	TOF_RSC_TNI5_TCQ0, TOF_RSC_TNI5_TCQ1, TOF_RSC_TNI5_TCQ2, TOF_RSC_TNI5_TCQ3,
	TOF_RSC_TNI5_TCQ4, TOF_RSC_TNI5_TCQ5, TOF_RSC_TNI5_TCQ6, TOF_RSC_TNI5_TCQ7,
	TOF_RSC_TNI5_TCQ8, TOF_RSC_TNI5_TCQ9, TOF_RSC_TNI5_TCQ10, TOF_RSC_TNI5_TCQ11,

	/* MRQ (144 - 215) */
	TOF_RSC_TNI0_MRQ0, TOF_RSC_TNI0_MRQ1, TOF_RSC_TNI0_MRQ2, TOF_RSC_TNI0_MRQ3,
	TOF_RSC_TNI0_MRQ4, TOF_RSC_TNI0_MRQ5, TOF_RSC_TNI0_MRQ6, TOF_RSC_TNI0_MRQ7,
	TOF_RSC_TNI0_MRQ8, TOF_RSC_TNI0_MRQ9, TOF_RSC_TNI0_MRQ10, TOF_RSC_TNI0_MRQ11,
	TOF_RSC_TNI1_MRQ0, TOF_RSC_TNI1_MRQ1, TOF_RSC_TNI1_MRQ2, TOF_RSC_TNI1_MRQ3,
	TOF_RSC_TNI1_MRQ4, TOF_RSC_TNI1_MRQ5, TOF_RSC_TNI1_MRQ6, TOF_RSC_TNI1_MRQ7,
	TOF_RSC_TNI1_MRQ8, TOF_RSC_TNI1_MRQ9, TOF_RSC_TNI1_MRQ10, TOF_RSC_TNI1_MRQ11,
	TOF_RSC_TNI2_MRQ0, TOF_RSC_TNI2_MRQ1, TOF_RSC_TNI2_MRQ2, TOF_RSC_TNI2_MRQ3,
	TOF_RSC_TNI2_MRQ4, TOF_RSC_TNI2_MRQ5, TOF_RSC_TNI2_MRQ6, TOF_RSC_TNI2_MRQ7,
	TOF_RSC_TNI2_MRQ8, TOF_RSC_TNI2_MRQ9, TOF_RSC_TNI2_MRQ10, TOF_RSC_TNI2_MRQ11,
	TOF_RSC_TNI3_MRQ0, TOF_RSC_TNI3_MRQ1, TOF_RSC_TNI3_MRQ2, TOF_RSC_TNI3_MRQ3,
	TOF_RSC_TNI3_MRQ4, TOF_RSC_TNI3_MRQ5, TOF_RSC_TNI3_MRQ6, TOF_RSC_TNI3_MRQ7,
	TOF_RSC_TNI3_MRQ8, TOF_RSC_TNI3_MRQ9, TOF_RSC_TNI3_MRQ10, TOF_RSC_TNI3_MRQ11,
	TOF_RSC_TNI4_MRQ0, TOF_RSC_TNI4_MRQ1, TOF_RSC_TNI4_MRQ2, TOF_RSC_TNI4_MRQ3,
	TOF_RSC_TNI4_MRQ4, TOF_RSC_TNI4_MRQ5, TOF_RSC_TNI4_MRQ6, TOF_RSC_TNI4_MRQ7,
	TOF_RSC_TNI4_MRQ8, TOF_RSC_TNI4_MRQ9, TOF_RSC_TNI4_MRQ10, TOF_RSC_TNI4_MRQ11,
	TOF_RSC_TNI5_MRQ0, TOF_RSC_TNI5_MRQ1, TOF_RSC_TNI5_MRQ2, TOF_RSC_TNI5_MRQ3,
	TOF_RSC_TNI5_MRQ4, TOF_RSC_TNI5_MRQ5, TOF_RSC_TNI5_MRQ6, TOF_RSC_TNI5_MRQ7,
	TOF_RSC_TNI5_MRQ8, TOF_RSC_TNI5_MRQ9, TOF_RSC_TNI5_MRQ10, TOF_RSC_TNI5_MRQ11,

	/* PBQ (216 - 221) */
	TOF_RSC_TNI0_PBQ, TOF_RSC_TNI1_PBQ, TOF_RSC_TNI2_PBQ, TOF_RSC_TNI3_PBQ,
	TOF_RSC_TNI4_PBQ, TOF_RSC_TNI5_PBQ,

	/* PRQ (222 - 227) */
	TOF_RSC_TNI0_PRQ, TOF_RSC_TNI1_PRQ, TOF_RSC_TNI2_PRQ, TOF_RSC_TNI3_PRQ,
	TOF_RSC_TNI4_PRQ, TOF_RSC_TNI5_PRQ,

	/* STEERINGTABLE (228 - 299) */
	TOF_RSC_TNI0_STEERINGTABLE0, TOF_RSC_TNI0_STEERINGTABLE1, TOF_RSC_TNI0_STEERINGTABLE2,
	TOF_RSC_TNI0_STEERINGTABLE3, TOF_RSC_TNI0_STEERINGTABLE4, TOF_RSC_TNI0_STEERINGTABLE5,
	TOF_RSC_TNI0_STEERINGTABLE6, TOF_RSC_TNI0_STEERINGTABLE7, TOF_RSC_TNI0_STEERINGTABLE8,
	TOF_RSC_TNI0_STEERINGTABLE9, TOF_RSC_TNI0_STEERINGTABLE10, TOF_RSC_TNI0_STEERINGTABLE11,
	TOF_RSC_TNI1_STEERINGTABLE0, TOF_RSC_TNI1_STEERINGTABLE1, TOF_RSC_TNI1_STEERINGTABLE2,
	TOF_RSC_TNI1_STEERINGTABLE3, TOF_RSC_TNI1_STEERINGTABLE4, TOF_RSC_TNI1_STEERINGTABLE5,
	TOF_RSC_TNI1_STEERINGTABLE6, TOF_RSC_TNI1_STEERINGTABLE7, TOF_RSC_TNI1_STEERINGTABLE8,
	TOF_RSC_TNI1_STEERINGTABLE9, TOF_RSC_TNI1_STEERINGTABLE10, TOF_RSC_TNI1_STEERINGTABLE11,
	TOF_RSC_TNI2_STEERINGTABLE0, TOF_RSC_TNI2_STEERINGTABLE1, TOF_RSC_TNI2_STEERINGTABLE2,
	TOF_RSC_TNI2_STEERINGTABLE3, TOF_RSC_TNI2_STEERINGTABLE4, TOF_RSC_TNI2_STEERINGTABLE5,
	TOF_RSC_TNI2_STEERINGTABLE6, TOF_RSC_TNI2_STEERINGTABLE7, TOF_RSC_TNI2_STEERINGTABLE8,
	TOF_RSC_TNI2_STEERINGTABLE9, TOF_RSC_TNI2_STEERINGTABLE10, TOF_RSC_TNI2_STEERINGTABLE11,
	TOF_RSC_TNI3_STEERINGTABLE0, TOF_RSC_TNI3_STEERINGTABLE1, TOF_RSC_TNI3_STEERINGTABLE2,
	TOF_RSC_TNI3_STEERINGTABLE3, TOF_RSC_TNI3_STEERINGTABLE4, TOF_RSC_TNI3_STEERINGTABLE5,
	TOF_RSC_TNI3_STEERINGTABLE6, TOF_RSC_TNI3_STEERINGTABLE7, TOF_RSC_TNI3_STEERINGTABLE8,
        TOF_RSC_TNI3_STEERINGTABLE9, TOF_RSC_TNI3_STEERINGTABLE10, TOF_RSC_TNI3_STEERINGTABLE11,
        TOF_RSC_TNI4_STEERINGTABLE0, TOF_RSC_TNI4_STEERINGTABLE1, TOF_RSC_TNI4_STEERINGTABLE2,
        TOF_RSC_TNI4_STEERINGTABLE3, TOF_RSC_TNI4_STEERINGTABLE4, TOF_RSC_TNI4_STEERINGTABLE5,
        TOF_RSC_TNI4_STEERINGTABLE6, TOF_RSC_TNI4_STEERINGTABLE7, TOF_RSC_TNI4_STEERINGTABLE8,
        TOF_RSC_TNI4_STEERINGTABLE9, TOF_RSC_TNI4_STEERINGTABLE10, TOF_RSC_TNI4_STEERINGTABLE11,
        TOF_RSC_TNI5_STEERINGTABLE3, TOF_RSC_TNI5_STEERINGTABLE4, TOF_RSC_TNI5_STEERINGTABLE5,
        TOF_RSC_TNI5_STEERINGTABLE6, TOF_RSC_TNI5_STEERINGTABLE7, TOF_RSC_TNI5_STEERINGTABLE8,
        TOF_RSC_TNI5_STEERINGTABLE9, TOF_RSC_TNI5_STEERINGTABLE10, TOF_RSC_TNI5_STEERINGTABLE11,

        /* MBTABLE (300 - 371) */
        TOF_RSC_TNI0_MBTABLE0, TOF_RSC_TNI0_MBTABLE1, TOF_RSC_TNI0_MBTABLE2,
        TOF_RSC_TNI0_MBTABLE3, TOF_RSC_TNI0_MBTABLE4, TOF_RSC_TNI0_MBTABLE5,
        TOF_RSC_TNI0_MBTABLE6, TOF_RSC_TNI0_MBTABLE7, TOF_RSC_TNI0_MBTABLE8,
        TOF_RSC_TNI0_MBTABLE9, TOF_RSC_TNI0_MBTABLE10, TOF_RSC_TNI0_MBTABLE11,
        TOF_RSC_TNI1_MBTABLE0, TOF_RSC_TNI1_MBTABLE1, TOF_RSC_TNI1_MBTABLE2,
        TOF_RSC_TNI1_MBTABLE3, TOF_RSC_TNI1_MBTABLE4, TOF_RSC_TNI1_MBTABLE5,
        TOF_RSC_TNI1_MBTABLE6, TOF_RSC_TNI1_MBTABLE7, TOF_RSC_TNI1_MBTABLE8,
        TOF_RSC_TNI1_MBTABLE9, TOF_RSC_TNI1_MBTABLE10, TOF_RSC_TNI1_MBTABLE11,
        TOF_RSC_TNI2_MBTABLE0, TOF_RSC_TNI2_MBTABLE1, TOF_RSC_TNI2_MBTABLE2,
        TOF_RSC_TNI2_MBTABLE3, TOF_RSC_TNI2_MBTABLE4, TOF_RSC_TNI2_MBTABLE5,
        TOF_RSC_TNI2_MBTABLE6, TOF_RSC_TNI2_MBTABLE7, TOF_RSC_TNI2_MBTABLE8,
        TOF_RSC_TNI2_MBTABLE9, TOF_RSC_TNI2_MBTABLE10, TOF_RSC_TNI2_MBTABLE11,
        TOF_RSC_TNI3_MBTABLE0, TOF_RSC_TNI3_MBTABLE1, TOF_RSC_TNI3_MBTABLE2,
        TOF_RSC_TNI3_MBTABLE3, TOF_RSC_TNI3_MBTABLE4, TOF_RSC_TNI3_MBTABLE5,
        TOF_RSC_TNI3_MBTABLE6, TOF_RSC_TNI3_MBTABLE7, TOF_RSC_TNI3_MBTABLE8,
        TOF_RSC_TNI3_MBTABLE9, TOF_RSC_TNI3_MBTABLE10, TOF_RSC_TNI3_MBTABLE11,
        TOF_RSC_TNI4_MBTABLE0, TOF_RSC_TNI4_MBTABLE1, TOF_RSC_TNI4_MBTABLE2,
        TOF_RSC_TNI4_MBTABLE3, TOF_RSC_TNI4_MBTABLE4, TOF_RSC_TNI4_MBTABLE5,
        TOF_RSC_TNI4_MBTABLE6, TOF_RSC_TNI4_MBTABLE7, TOF_RSC_TNI4_MBTABLE8,
        TOF_RSC_TNI4_MBTABLE9, TOF_RSC_TNI4_MBTABLE10, TOF_RSC_TNI4_MBTABLE11,
        TOF_RSC_TNI5_MBTABLE0, TOF_RSC_TNI5_MBTABLE1, TOF_RSC_TNI5_MBTABLE2,
        TOF_RSC_TNI5_MBTABLE3, TOF_RSC_TNI5_MBTABLE4, TOF_RSC_TNI5_MBTABLE5,
        TOF_RSC_TNI5_MBTABLE6, TOF_RSC_TNI5_MBTABLE7, TOF_RSC_TNI5_MBTABLE8,
        TOF_RSC_TNI5_MBTABLE9, TOF_RSC_TNI5_MBTABLE10, TOF_RSC_TNI5_MBTABLE11,

        TOF_RSC_NUM     /* 372 */
};
#define TOF_RSC_TOQ(TNI, CQID)  (TOF_RSC_TNI0_TOQ0 + (TNI * 12) + CQID)
#define TOF_RSC_TCQ(TNI, CQID)  (TOF_RSC_TNI0_TCQ0 + (TNI * 12) + CQID)
#define TOF_RSC_MRQ(TNI, CQID)  (TOF_RSC_TNI0_MRQ0 + (TNI * 12) + CQID)
#define TOF_RSC_PBQ(TNI)        (TOF_RSC_TNI0_PBQ + TNI)
#define TOF_RSC_PRQ(TNI)        (TOF_RSC_TNI0_PRQ + TNI)
#define TOF_RSC_STT(TNI, CQID)  (TOF_RSC_TNI0_STEERINGTABLE0 + (TNI * 12) + CQID)
#define TOF_RSC_MBT(TNI, CQID)  (TOF_RSC_TNI0_MBTABLE0 + (TNI * 12) + CQID)

#endif

/* vim: set noet ts=8 sw=8 sts=0 tw=0 : */

