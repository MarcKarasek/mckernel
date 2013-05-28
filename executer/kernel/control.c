#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <asm/msr.h>
#include <asm/io.h>
#include "mcctrl.h"

#ifdef DEBUG
#define dprintk printk
#else
#define dprintk(...)
#endif

//static DECLARE_WAIT_QUEUE_HEAD(wq_prepare);
//extern struct mcctrl_channel *channels;
int mcctrl_ikc_set_recv_cpu(ihk_os_t os, int cpu);

static long mcexec_prepare_image(ihk_os_t os,
                                 struct program_load_desc * __user udesc)
{
	struct program_load_desc desc, *pdesc;
	struct ikc_scd_packet isp;
	void *args, *envs;
	long ret = 0;
	struct mcctrl_usrdata *usrdata = ihk_host_os_get_usrdata(os);

	if (copy_from_user(&desc, udesc,
	                    sizeof(struct program_load_desc))) {
		return -EFAULT;
	}
	if (desc.num_sections <= 0 || desc.num_sections > 16) {
		printk("# of sections: %d\n", desc.num_sections);
		return -EINVAL;
	}
	pdesc = kmalloc(sizeof(struct program_load_desc) + 
	                sizeof(struct program_image_section)
	                * desc.num_sections, GFP_KERNEL);
	memcpy(pdesc, &desc, sizeof(struct program_load_desc));
	if (copy_from_user(pdesc->sections, udesc->sections,
	                   sizeof(struct program_image_section)
	                   * desc.num_sections)) {
		kfree(pdesc);
		return -EFAULT;
	}

	pdesc->pid = task_tgid_vnr(current);

	if (reserve_user_space(usrdata, &pdesc->user_start, &pdesc->user_end)) {
		kfree(pdesc);
		return -ENOMEM;
	}

	args = kmalloc(pdesc->args_len, GFP_KERNEL);
	if (copy_from_user(args, pdesc->args, pdesc->args_len)) {
		kfree(args);
		kfree(pdesc);
		return -EFAULT;
	}
	
	envs = kmalloc(pdesc->envs_len, GFP_KERNEL);
	if (copy_from_user(envs, pdesc->envs, pdesc->envs_len)) {
		ret = -EFAULT;	
		goto free_out;
	}

	pdesc->args = (void*)virt_to_phys(args);
	printk("args: 0x%lX\n", (unsigned long)pdesc->args);
	printk("argc: %d\n", *(int*)args);
	pdesc->envs = (void*)virt_to_phys(envs);
	printk("envs: 0x%lX\n", (unsigned long)pdesc->envs);
	printk("envc: %d\n", *(int*)envs);

	isp.msg = SCD_MSG_PREPARE_PROCESS;
	isp.ref = pdesc->cpu;
	isp.arg = virt_to_phys(pdesc);

	printk("# of sections: %d\n", pdesc->num_sections);
	printk("%p (%lx)\n", pdesc, isp.arg);
	
	pdesc->status = 0;
	mcctrl_ikc_send(os, pdesc->cpu, &isp);

	wait_event_interruptible(usrdata->wq_prepare, pdesc->status);

	usrdata->rpgtable = pdesc->rpgtable;
	if (copy_to_user(udesc, pdesc, sizeof(struct program_load_desc) + 
	             sizeof(struct program_image_section) * desc.num_sections)) {
		ret = -EFAULT;	
		goto free_out;
	}

	ret = 0;

free_out:
	kfree(args);
	kfree(pdesc);
	kfree(envs);

	return ret;
}

int mcexec_load_image(ihk_os_t os, struct program_transfer *__user upt)
{
	struct program_transfer pt;
	unsigned long phys, ret = 0;
	void *rpm;
#if 0	
	unsigned long dma_status = 0;
	ihk_dma_channel_t channel;
	struct ihk_dma_request request;
	void *p;

	channel = ihk_device_get_dma_channel(ihk_os_to_dev(os), 0);
	if (!channel) {
		return -EINVAL;
	}
#endif	

	if (copy_from_user(&pt, upt, sizeof(pt))) {
		return -EFAULT;
	}
	
	phys = ihk_device_map_memory(ihk_os_to_dev(os), pt.dest, PAGE_SIZE);
#ifdef CONFIG_MIC
	rpm = ioremap_wc(phys, PAGE_SIZE);
#else
	rpm = ihk_device_map_virtual(ihk_os_to_dev(os), phys, PAGE_SIZE, NULL, 0);
#endif

	if (copy_from_user(rpm, pt.src, PAGE_SIZE)) {
		ret = -EFAULT;
	}

#ifdef CONFIG_MIC
	iounmap(rpm);
#else
	ihk_device_unmap_virtual(ihk_os_to_dev(os), rpm, PAGE_SIZE);
#endif
	ihk_device_unmap_memory(ihk_os_to_dev(os), phys, PAGE_SIZE);	

	return ret;

#if 0	
	p = (void *)__get_free_page(GFP_KERNEL);

	if (copy_from_user(p, pt.src, PAGE_SIZE)) {
		return -EFAULT;
	}

	memset(&request, 0, sizeof(request));
	request.src_os = NULL;
	request.src_phys = virt_to_phys(p);
	request.dest_os = os;
	request.dest_phys = pt.dest;
	request.size = PAGE_SIZE;
	request.notify = (void *)virt_to_phys(&dma_status);
	request.priv = (void *)1;

	ihk_dma_request(channel, &request);

	while (!dma_status) {
		mb();
		udelay(1);
	}

	free_page((unsigned long)p);

	return 0;
#endif
}

//extern unsigned long last_thread_exec;

static long mcexec_start_image(ihk_os_t os,
                                 struct program_load_desc * __user udesc)
{
	struct program_load_desc desc;
	struct ikc_scd_packet isp;
	struct mcctrl_channel *c;
	struct mcctrl_usrdata *usrdata = ihk_host_os_get_usrdata(os);

	if (copy_from_user(&desc, udesc,
	                   sizeof(struct program_load_desc))) {
		return -EFAULT;
	}

	c = usrdata->channels + desc.cpu;

	mcctrl_ikc_set_recv_cpu(os, desc.cpu);

	usrdata->last_thread_exec = desc.cpu;
	
	isp.msg = SCD_MSG_SCHEDULE_PROCESS;
	isp.ref = desc.cpu;
	isp.arg = desc.rprocess;

	mcctrl_ikc_send(os, desc.cpu, &isp);

	return 0;
}

int mcexec_syscall(struct mcctrl_channel *c, unsigned long arg)
{
	c->req = 1;
	wake_up(&c->wq_syscall);

	return 0;
}

#ifndef DO_USER_MODE
int __do_in_kernel_syscall(ihk_os_t os, struct mcctrl_channel *c,
                           struct syscall_request *sc);
// static int remaining_job, base_cpu, job_pos;
#endif

// extern int num_channels;
// extern int mcctrl_dma_abort;

int mcexec_wait_syscall(ihk_os_t os, struct syscall_wait_desc *__user req)
{
	struct syscall_wait_desc swd;
	struct mcctrl_channel *c;
	struct mcctrl_usrdata *usrdata = ihk_host_os_get_usrdata(os);
#ifndef DO_USER_MODE
	unsigned long s, w, d;
#endif
	
//printk("mcexec_wait_syscall swd=%p req=%p size=%d\n", &swd, req, sizeof(swd.cpu));
	if (copy_from_user(&swd, req, sizeof(swd.cpu))) {
		return -EFAULT;
	}

if(swd.cpu >= usrdata->num_channels)return -EINVAL;

	c = usrdata->channels + swd.cpu;

#ifdef DO_USER_MODE
	wait_event_interruptible(c->wq_syscall, c->req);
	c->req = 0;
#else
	while (1) {
		c = usrdata->channels + swd.cpu;
		rdtscll(s);
		if (!usrdata->remaining_job) {
			while (!(*c->param.doorbell_va)) {
				mb();
				cpu_relax();
				rdtscll(w);
				if (w > s + 1024UL * 1024 * 1024 * 10) {
					return -EINTR;
				}
			}
			d = (*c->param.doorbell_va) - 1;
			*c->param.doorbell_va = 0;

			if (d < 0 || d >= usrdata->num_channels) {
				d = 0;
			}
			usrdata->base_cpu = d;
			usrdata->job_pos = 0;
			usrdata->remaining_job = 1;
		} else { 
			usrdata->job_pos++;
		}
		
		for (; usrdata->job_pos < usrdata->num_channels; usrdata->job_pos++) {
			if (base_cpu + job_pos >= num_channels) {
				c = usrdata->channels + 
					(usrdata->base_cpu + usrdata->job_pos - usrdata->num_channels);
			} else {
				c = usrdata->channels + usrdata->base_cpu + usrdata->job_pos;
			}
			if (!c) {
				continue;
			}
			if (c->param.request_va &&
			    c->param.request_va->valid) {
				c->param.request_va->valid = 0; /* ack */
				dprintk("SC #%lx, %lx\n",
				        c->param.request_va->number,
				        c->param.request_va->args[0]);
		if (__do_in_kernel_syscall(os, c, c->param.request_va)) {
#endif
			if (copy_to_user(&req->sr, c->param.request_va,
			                 sizeof(struct syscall_request))) {
				return -EFAULT;
			}
#ifndef DO_USER_MODE
			return 0;
		}
		if (usrdata->mcctrl_dma_abort) {
			return -2;
		}
			}
		}
		usrdata->remaining_job = 0;
	}
#endif
	return 0;
}

long mcexec_pin_region(ihk_os_t os, unsigned long *__user arg)
{
	struct prepare_dma_desc desc;
	int pin_shift = 16;
	int order;
	unsigned long a;

	if (copy_from_user(&desc, arg, sizeof(struct prepare_dma_desc))) {
		return -EFAULT;
	}

	order =  pin_shift - PAGE_SHIFT;
	if(desc.size > 0){
		order = get_order (desc.size);
	}

	a = __get_free_pages(GFP_KERNEL, order);
	if (!a) {
		return -ENOMEM;
	}

	a = virt_to_phys((void *)a);

	if (copy_to_user((void*)desc.pa, &a, sizeof(unsigned long))) {
		return -EFAULT;
	}
	return 0;
}

long mcexec_free_region(ihk_os_t os, unsigned long *__user arg)
{
	struct free_dma_desc desc;
	int pin_shift = 16;
	int order;

	if (copy_from_user(&desc, arg, sizeof(struct free_dma_desc))) {
		return -EFAULT;
	}

	order =  pin_shift - PAGE_SHIFT;
	if(desc.size > 0){
		order = get_order (desc.size);
	}

	if(desc.pa > 0){
		free_pages((unsigned long)phys_to_virt(desc.pa), order);
	}
	return 0;
}

long mcexec_load_syscall(ihk_os_t os, struct syscall_load_desc *__user arg)
{
	struct syscall_load_desc desc;
	unsigned long phys;
	void *rpm;
	
	if (copy_from_user(&desc, arg, sizeof(struct syscall_load_desc))) {
		return -EFAULT;
	}
	
	phys = ihk_device_map_memory(ihk_os_to_dev(os), desc.src, desc.size);
#ifdef CONFIG_MIC
	rpm = ioremap_wc(phys, desc.size);
#else
	rpm = ihk_device_map_virtual(ihk_os_to_dev(os), phys, desc.size, NULL, 0);
#endif

	dprintk("mcexec_load_syscall: %s (desc.size: %d)\n", rpm, desc.size);

	if (copy_to_user((void *__user)desc.dest, rpm, desc.size)) {
		return -EFAULT;
	}

#ifdef CONFIG_MIC
	iounmap(rpm);
#else
	ihk_device_unmap_virtual(ihk_os_to_dev(os), rpm, desc.size);
#endif
	
	ihk_device_unmap_memory(ihk_os_to_dev(os), phys, desc.size);	
	
/*
	ihk_dma_channel_t channel;
	struct ihk_dma_request request;
	unsigned long dma_status = 0;

	channel = ihk_device_get_dma_channel(ihk_os_to_dev(os), 0);
	if (!channel) {
		return -EINVAL;
	}

	memset(&request, 0, sizeof(request));
	request.src_os = os;
	request.src_phys = desc.src;
	request.dest_os = NULL;
	request.dest_phys = desc.dest;
	request.size = desc.size;
	request.notify = (void *)virt_to_phys(&dma_status);
	request.priv = (void *)1;

	ihk_dma_request(channel, &request);

	while (!dma_status) {
		mb();
		udelay(1);
	}
*/

	return 0;
}

long mcexec_ret_syscall(ihk_os_t os, struct syscall_ret_desc *__user arg)
{
	struct syscall_ret_desc ret;
	struct mcctrl_channel *mc;
	struct mcctrl_usrdata *usrdata = ihk_host_os_get_usrdata(os);
#if 0	
	ihk_dma_channel_t channel;
	struct ihk_dma_request request;

	channel = ihk_device_get_dma_channel(ihk_os_to_dev(os), 0);
	if (!channel) {
		return -EINVAL;
	}
#endif	

	if (copy_from_user(&ret, arg, sizeof(struct syscall_ret_desc))) {
		return -EFAULT;
	}
	mc = usrdata->channels + ret.cpu;
	if (!mc) {
		return -EINVAL;
	}

	mc->param.response_va->ret = ret.ret;

	if (ret.size > 0) {
		/* Host => Accel. Write is fast. */
		unsigned long phys;
		void *rpm;

		phys = ihk_device_map_memory(ihk_os_to_dev(os), ret.dest,
		                             ret.size);
#ifdef CONFIG_MIC
		rpm = ioremap_wc(phys, ret.size);
#else
		rpm = ihk_device_map_virtual(ihk_os_to_dev(os), phys, 
		                             ret.size, NULL, 0);
#endif
		
		if (copy_from_user(rpm, (void *__user)ret.src, ret.size)) {
			return -EFAULT;
		}

		mc->param.response_va->status = 1;

#ifdef CONFIG_MIC
		iounmap(rpm);
#else
		ihk_device_unmap_virtual(ihk_os_to_dev(os), rpm, ret.size);
#endif
		ihk_device_unmap_memory(ihk_os_to_dev(os), phys, ret.size);

/*
		memset(&request, 0, sizeof(request));
		request.src_os = NULL;
		request.src_phys = ret.src;
		request.dest_os = os;
		request.dest_phys = ret.dest;
		request.size = ret.size;
		request.notify_os = os;
		request.notify = (void *)mc->param.response_rpa;
		request.priv = (void *)1;
		
		ihk_dma_request(channel, &request);
*/
	} else {
		mc->param.response_va->status = 1;
	}

	return 0;
}

long __mcctrl_control(ihk_os_t os, unsigned int req, unsigned long arg)
{
	switch (req) {
	case MCEXEC_UP_PREPARE_IMAGE:
		return mcexec_prepare_image(os,
		                            (struct program_load_desc *)arg);
	case MCEXEC_UP_LOAD_IMAGE:
		return mcexec_load_image(os, (struct program_transfer *)arg);

	case MCEXEC_UP_START_IMAGE:
		return mcexec_start_image(os, (struct program_load_desc *)arg);

	case MCEXEC_UP_WAIT_SYSCALL:
		return mcexec_wait_syscall(os, (struct syscall_wait_desc *)arg);

	case MCEXEC_UP_RET_SYSCALL:
		return mcexec_ret_syscall(os, (struct syscall_ret_desc *)arg);

	case MCEXEC_UP_LOAD_SYSCALL:
		return mcexec_load_syscall(os, (struct syscall_load_desc *)arg);

	case MCEXEC_UP_PREPARE_DMA:
		return mcexec_pin_region(os, (unsigned long *)arg);

	case MCEXEC_UP_FREE_DMA:
		return mcexec_free_region(os, (unsigned long *)arg);
	}
	return -EINVAL;
}

void mcexec_prepare_ack(ihk_os_t os, unsigned long arg)
{
	struct program_load_desc *desc = phys_to_virt(arg);
	struct mcctrl_usrdata *usrdata = ihk_host_os_get_usrdata(os);

	desc->status = 1;
	
	wake_up_all(&usrdata->wq_prepare);
}

