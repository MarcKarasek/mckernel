/* get_rusage.c COPYRIGHT FUJITSU LIMITED 2018 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ihklib.h>

static void usage(char *cmd)
{
	printf("Usage:\n");
	printf("  %s <os_number>\n", cmd);
}

int main(int argc, char *argv[])
{
	int os_num = 0;
	int ret = -1;
	int i = 0;
	struct ihk_os_rusage mck_rusage;

	if (argc != 2) {
		usage(argv[0]);
		goto out;
	}

	os_num = atoi(argv[1]);
	if (os_num < 0 || 63 < os_num) {
		printf("Invalid argument 1 os_num.\n");
		usage(argv[0]);
		goto out;
	}

	memset(&mck_rusage, 0, sizeof(mck_rusage));
	ret = ihk_os_getrusage(os_num, &mck_rusage, sizeof(mck_rusage));
	if (ret) {
		perror("ihk_os_getrusage()");
		goto out;
	}

	for (i = 0; i < IHK_MAX_NUM_PGSIZES; i++) {
		printf("memory_stat_rss[%d] : 0x%lx\n", i,
			mck_rusage.memory_stat_rss[i]);
	}

	for (i = 0; i < IHK_MAX_NUM_PGSIZES; i++) {
		printf("memory_stat_mapped_file[%d] : 0x%lx\n", i,
			mck_rusage.memory_stat_mapped_file[i]);
	}

	printf("memory_max_usage : 0x%lx\n", mck_rusage.memory_max_usage);
	printf("memory_kmem_usage : 0x%lx\n", mck_rusage.memory_kmem_usage);
	printf("memory_kmem_max_usage : 0x%lx\n", mck_rusage.memory_kmem_max_usage);

	for (i = 0; i < IHK_MAX_NUM_NUMA_NODES; i++) {
		printf("memory_numa_stat[%d] : 0x%lx\n", i,
			mck_rusage.memory_numa_stat[i]);
	}

	printf("cpuacct_stat_system : 0x%lx\n", mck_rusage.cpuacct_stat_system);
	printf("cpuacct_stat_user : 0x%lx\n", mck_rusage.cpuacct_stat_user);
	printf("cpuacct_usage : 0x%lx\n", mck_rusage.cpuacct_usage);

	for (i = 0; i < IHK_MAX_NUM_CPUS; i++) {
		printf("cpuacct_usage_percpu[%d] : 0x%lx\n", i,
			mck_rusage.cpuacct_usage_percpu[i]);
	}

	printf("num_threads : 0x%x\n", mck_rusage.num_threads);
	printf("max_num_threads : 0x%x\n", mck_rusage.max_num_threads);

out:
	return ret;
}
