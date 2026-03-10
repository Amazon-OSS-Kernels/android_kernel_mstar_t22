#include "mstar_mbx_head_general.h"
#include <linux/interrupt.h>

static DEFINE_MUTEX(mbx_slot_mutex);
static struct mbx_slot slot[MAX_SEC_ARM_NUM];

mbx_smc_args *Sec_ARM_MBX_Base = NULL;

TEE_Result register_mbx_base(mbx_smc_args *base)
{
	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] default ENTER\033[m\n");

	if(Sec_ARM_MBX_Base != NULL)
	{
		MSTAR_MBX_PRINTF_DEBUG("\033[0;32;31m [Sec_ARM MBX] Sec_ARM_MBX_Base already init %p\033[m\n",(void *)Sec_ARM_MBX_Base);
		MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] default EXIT\033[m\n");
		return 0;
	}

	if((sizeof(struct mbx_slot) * MAX_SEC_ARM_NUM) != 0)
		memset(slot,0,sizeof(struct mbx_slot) * MAX_SEC_ARM_NUM);

	Sec_ARM_MBX_Base = (mbx_smc_args *)base;

	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] default EXIT\033[m\n");

	return 0;
}

void __attribute__((weak)) fire_mbx_R2T(void) //mboot and linux please impelement
{
	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] default ENTER\033[m\n");
	TRIGGER_MBX_INTERRUPT;
	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] default EXIT\033[m\n");
}

int __attribute__((weak)) REE_Handle(void *data) //linux please impelement
{
	(void)data;

	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] ENTER\033[m\n");
	if(Sec_ARM_MBX_Base == NULL)
	{
		MSTAR_MBX_PRINTF_ERROR("\033[0;32;31m [Sec_ARM MBX] %s %d Error Sec_ARM_MBX_Base not Init\033[m\n",__func__,__LINE__);
		return 0;
	}

	while(1){
		uint32_t i;
		for(i = 0;i < MAX_SEC_ARM_NUM ; i++){
			if(Sec_ARM_MBX_Base[i].cmd_id == CMD_T2R_GENERAL_DONE || Sec_ARM_MBX_Base[i].cmd_id == CMD_T2R_RPC) {
				slot[i].param->a0 = Sec_ARM_MBX_Base[i].a0;
				slot[i].param->a1 = Sec_ARM_MBX_Base[i].a1;
				slot[i].param->a2 = Sec_ARM_MBX_Base[i].a2;
				slot[i].param->a3 = Sec_ARM_MBX_Base[i].a3;
				slot[i].param->a4 = Sec_ARM_MBX_Base[i].a4;
				slot[i].param->a5 = Sec_ARM_MBX_Base[i].a5;
				slot[i].param->a6 = Sec_ARM_MBX_Base[i].a6;
				slot[i].param->a7 = Sec_ARM_MBX_Base[i].a7;
				Sec_ARM_MBX_Base[i].cmd_id = CMD_NULL;
				mutex_lock(&mbx_slot_mutex);
				if(slot[i].event != NULL);
					up(slot[i].event);
				mutex_unlock(&mbx_slot_mutex);
				MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] Clear %d talk to REE\033[m\n",i);
			}
			msleep(1);
		}
	}
	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] EXIT\033[m\n");
	return 0;
}

TEE_Result send_mbx(thread_smc_args *args,OPTEE_MBX_CMD_ID id, uint32_t mbx_slot)
{
	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] ENTER\033[m\n");

	if(mbx_slot >= MAX_SEC_ARM_NUM)
	{
		MSTAR_MBX_PRINTF_ERROR("\033[0;32;31m [Sec_ARM MBX] Error mbx_slot %d over %d\033[m\n",mbx_slot,MAX_SEC_ARM_NUM);
		return 1;
	}

	if(Sec_ARM_MBX_Base == NULL)
	{
		MSTAR_MBX_PRINTF_ERROR("\033[0;32;31m [Sec_ARM MBX] Error Sec_ARM_MBX_Base not Init\033[m\n");
		return 1;
	}

	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] %s %d id %x mbx_slot %d\033[m\n",__func__,__LINE__,id,mbx_slot);
	Sec_ARM_MBX_Base[mbx_slot].a0 = (uint64_t) args->a0;
	Sec_ARM_MBX_Base[mbx_slot].a1 = (uint64_t) args->a1;
	Sec_ARM_MBX_Base[mbx_slot].a2 = (uint64_t) args->a2;
	Sec_ARM_MBX_Base[mbx_slot].a3 = (uint64_t) args->a3;
	Sec_ARM_MBX_Base[mbx_slot].a4 = (uint64_t) args->a4;
	Sec_ARM_MBX_Base[mbx_slot].a5 = (uint64_t) args->a5;
	Sec_ARM_MBX_Base[mbx_slot].a6 = (uint64_t) args->a6;
	Sec_ARM_MBX_Base[mbx_slot].a7 = (uint64_t) args->a7;
	Sec_ARM_MBX_Base[mbx_slot].cmd_id = (uint64_t) id;
	Sec_ARM_MBX_Base[mbx_slot].version = (uint64_t) 0x1;

	FLUSH_CACHE(Sec_ARM_MBX_Base, sizeof(mbx_smc_args) * MAX_SEC_ARM_NUM);

	if(id == CMD_R2T_GENERAL || id == CMD_R2T_RPC_DONE) {
		fire_mbx_R2T();
	} else if(id == CMD_T2R_GENERAL_DONE || id == CMD_T2R_RPC) {
		Sec_ARM_MBX_Base[mbx_slot].cmd_id = CMD_NULL;
		MSTAR_MBX_PRINTF_ERROR("\033[0;32;31m [Sec_ARM MBX] Not Support Mode 0x%x\033[m\n",id);
		return 1;
	} else {
		Sec_ARM_MBX_Base[mbx_slot].cmd_id = CMD_NULL;
		MSTAR_MBX_PRINTF_ERROR("\033[0;32;31m [Sec_ARM MBX] Not Support Mode 0x%x\033[m\n",id);
		return 1;
	}

	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] EXIT\033[m\n");

	return 0;
}
EXPORT_SYMBOL(send_mbx);

uint32_t mbx_slot_alloc(struct semaphore *event,struct smc_param *param)
{
#if MAX_SEC_ARM_NUM > 0
	int i;

	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] default ENTER\033[m\n");

	mutex_lock(&mbx_slot_mutex);
	for (i = 0; i < MAX_SEC_ARM_NUM; i++) {
		if (slot[i].use == 0) {
			slot[i].use = 1;
			slot[i].pid = current->pid;
			slot[i].tgid = current->tgid;
			slot[i].event = event;
			slot[i].param = param;
			mutex_unlock(&mbx_slot_mutex);
			return i;
		}
	}
	mutex_unlock(&mbx_slot_mutex);

	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] EXIT\033[m\n");
#endif

	return 0xffffffff;
}

uint32_t mbx_slot_free(void)
{
	int i;

	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] default ENTER\033[m\n");

	mutex_lock(&mbx_slot_mutex);
	for(i = 0; i < MAX_SEC_ARM_NUM ; i++)
	{
		if(slot[i].use == 1 && slot[i].pid == current->pid && slot[i].tgid == current->tgid)
		{
			memset(&slot[i],0,sizeof(struct mbx_slot));
			mutex_unlock(&mbx_slot_mutex);
			return i;
		}
	}
	mutex_unlock(&mbx_slot_mutex);

	MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] EXIT\033[m\n");

	return 0xffffffff;
}

static void sec_arm_work_handle(struct work_struct *ignored)
{
#if MAX_SEC_ARM_NUM > 0
	int i;

	for (i = 0; i < MAX_SEC_ARM_NUM ; i++){
		if (Sec_ARM_MBX_Base[i].cmd_id == CMD_T2R_GENERAL_DONE ||
				Sec_ARM_MBX_Base[i].cmd_id == CMD_T2R_RPC) {
			slot[i].param->a0 = Sec_ARM_MBX_Base[i].a0;
			slot[i].param->a1 = Sec_ARM_MBX_Base[i].a1;
			slot[i].param->a2 = Sec_ARM_MBX_Base[i].a2;
			slot[i].param->a3 = Sec_ARM_MBX_Base[i].a3;
			slot[i].param->a4 = Sec_ARM_MBX_Base[i].a4;
			slot[i].param->a5 = Sec_ARM_MBX_Base[i].a5;
			slot[i].param->a6 = Sec_ARM_MBX_Base[i].a6;
			slot[i].param->a7 = Sec_ARM_MBX_Base[i].a7;
			Sec_ARM_MBX_Base[i].cmd_id = CMD_NULL;

			mutex_lock(&mbx_slot_mutex);

			if (slot[i].event != NULL)
				up(slot[i].event);

			mutex_unlock(&mbx_slot_mutex);
			MSTAR_MBX_PRINTF_FLOW("\033[0;32;31m [Sec_ARM MBX] Clear %d talk to REE\033[m\n",i);
		}
	}
	//enable_irq(SECARM_IRQ);
#endif
}

static DECLARE_WORK(sec_arm_work, sec_arm_work_handle);
irqreturn_t mbx_optee_irq_handler(int irq,void *devid)
{
	schedule_work(&sec_arm_work);
	//CLEAN_MBX_INTERRUPT;

	return IRQ_HANDLED;
}
