#ifndef __MBX_LINUX_H__
#define __MBX_LINUX_H__

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <tee_mutex_wait.h>
#include <tee_wait_queue.h>
#include "../tee_tz_priv.h"
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/irqreturn.h>
#include <mstar_mbx_head_hal.h>

#ifndef uint64_t
	#define uint64_t unsigned long long
#endif

#ifndef uint32_t
	#define uint32_t unsigned int
#endif

#define TEE_Result int

typedef struct smc_param thread_smc_args;

struct mbx_slot {
	pid_t pid;
	pid_t tgid;
	uint32_t use;
	//event
	struct semaphore *event;
	struct smc_param *param;
};

uint32_t mbx_slot_alloc(struct semaphore *event,struct smc_param *param);
uint32_t mbx_slot_free(void);
void __attribute__((weak)) fire_mbx_R2T(void); //mboot and linux please impelement
int __attribute__((weak)) REE_Handle(void *data); //linux please impelement

irqreturn_t mbx_optee_irq_handler(int irq,void *devid);
extern ptrdiff_t   mstar_pm_base;

#define MSTAR_MBX_PRINTF_DEBUG(fmt, args...)  printk(KERN_DEBUG "[DEBUG] %s %d" fmt,__func__,__LINE__, ## args);
#define MSTAR_MBX_PRINTF_FLOW(fmt, args...)  printk(KERN_DEBUG "[FLOW] %s %d" fmt,__func__,__LINE__, ## args);
#define MSTAR_MBX_PRINTF_ERROR(fmt, args...)  printk(KERN_ERR "[ERROR] %s %d" fmt,__func__,__LINE__, ## args);

extern void Chip_Flush_Miu_Pipe(void);

#define FLUSH_CACHE(addr, len) \
					do{ \
						Chip_Flush_Miu_Pipe(); \
					}while(0)

#define TEESMC_OPTEE_REVISION_MAJOR 1

#endif
