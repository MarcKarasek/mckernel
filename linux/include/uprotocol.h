#ifndef HEADER_UPROTOCOL_H
#define HEADER_UPROTOCOL_H

#define MCEXEC_UP_PREPARE_IMAGE  0x30a02900
#define MCEXEC_UP_LOAD_IMAGE     0x30a02901
#define MCEXEC_UP_START_IMAGE    0x30a02902
#define MCEXEC_UP_WAIT_SYSCALL   0x30a02903
#define MCEXEC_UP_RET_SYSCALL    0x30a02904
#define MCEXEC_UP_LOAD_SYSCALL   0x30a02905

#define MCEXEC_UP_PREPARE_DMA    0x30a02910

struct program_transfer {
	unsigned long dest;
	void *src;
	unsigned long sz;
};

struct program_image_section {
	unsigned long vaddr;
	unsigned long len;
	unsigned long remote_pa;
	unsigned long filesz, offset;
	void *source;
};

struct program_load_desc {
	int num_sections;
	int status;
	int cpu;
	int pid;
	unsigned long entry;
	unsigned long rprocess;
	char *args;
	unsigned long args_len;
	char *envs;
	unsigned long envs_len;
	struct program_image_section sections[0];
};

struct syscall_request {
	unsigned long valid;
	unsigned long number;
	unsigned long args[6];
};

struct syscall_wait_desc {
	unsigned long cpu;
	struct syscall_request sr;
};

struct syscall_load_desc {
	unsigned long cpu;
	unsigned long src;
	unsigned long dest;
	unsigned long size;
};

struct syscall_response {
	unsigned long status;
	long ret;
};

struct syscall_ret_desc {
	long cpu;
	long ret;
	unsigned long src;
	unsigned long dest;
	unsigned long size;
};

#endif
