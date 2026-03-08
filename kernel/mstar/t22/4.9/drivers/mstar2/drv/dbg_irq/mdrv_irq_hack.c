#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include "chip_int.h"

extern void irq_enable(struct irq_desc *desc);

spinlock_t irq_hacking_lock;
extern ptrdiff_t mstar_pm_base;
int test_once = 1;

irqreturn_t irq_hacking(int irq, void *dev_id)
{
	unsigned long flags;
	int i = 3000;
	i *= test_once;
	test_once = 0;

	spin_lock_irqsave(&irq_hacking_lock, flags);
	while (i > 0) {
		mdelay(1);
		i--;
	}
	spin_unlock_irqrestore(&irq_hacking_lock, flags);

	return IRQ_HANDLED;
}

static int irq_hack_show(struct seq_file *s, void *data)
{
	seq_printf(s, "\n");
	seq_printf(s,   "####### Abnormal Interrupt Detector #######\n"
			"----------------- Setting -----------------\n"
			"[irq_cnt]     : The threshold (times per second) of abnormal interrupt frequency\n"
			"[irq_interval]: The threshold (milliseconds) of abnormal interrupt latency \n"
			"---------------- Self-test ----------------\n"
			"echo test > /sys/kernel/debug/dbg_irqirq_hack\n");
	seq_printf(s, "\n");
	return 0;
}

static int irq_hack_open(struct inode *inode, struct file *file)
{
	return single_open(file, irq_hack_show, NULL);
}

static ssize_t irq_hack_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char buffer[32];
	volatile void __iomem *reg = (volatile void __iomem *) (REG_IRQ_MASK_L - (4 << 2));

	if (!count || count > 16)// out of bound
		return count;

	if (copy_from_user(buffer, buf, count))
		return -EFAULT;

	buffer[count] = '\0';

	if (!strncmp(buffer, "test", 4)) {
		pr_err("Abnormal Interrupt detector is testing irq\n");

		free_irq(34, NULL);
		spin_lock_init(&irq_hacking_lock);
		request_irq(34, irq_hacking, SA_INTERRUPT, "irq hack", NULL);

		test_once = 1;
		__raw_writeb(0, reg);
		irq_enable(irq_to_desc(34));
		__raw_writeb(4, reg);
	}

	return count;
}

static const struct file_operations irq_hack_fops = {
	.owner      = THIS_MODULE,
	.open       = irq_hack_open,
	.read       = seq_read,
	.write      = irq_hack_write,
	.llseek     = seq_lseek,
	.release    = single_release,
};

static struct dentry *mstar_dbg_irq_dir;
extern unsigned int mst_dbg_irq_cnt;
extern unsigned int mst_dbg_irq_interval;

static int __init mstar_dbg_irq_init(void)
{
	struct dentry *dentry;
	mstar_dbg_irq_dir = debugfs_create_dir("dbg_irq", NULL);
	if (!mstar_dbg_irq_dir)
		return -ENOMEM;

	dentry = debugfs_create_file("irq_hack", S_IRUGO | S_IWUGO, mstar_dbg_irq_dir, NULL, &irq_hack_fops);
	if (IS_ERR(dentry))
		return -ENOMEM;

	dentry = debugfs_create_u32("irq_cnt", S_IRUGO | S_IWUGO, mstar_dbg_irq_dir, &mst_dbg_irq_cnt);
	if (IS_ERR(dentry))
		return -ENOMEM;

	dentry = debugfs_create_u32("irq_interval", S_IRUGO | S_IWUGO, mstar_dbg_irq_dir, &mst_dbg_irq_interval);
	if (IS_ERR(dentry))
		return -ENOMEM;

	return 0;
}

static void __exit mstar_dbg_irq_exit(void)
{
	debugfs_remove_recursive(mstar_dbg_irq_dir);
}

module_init(mstar_dbg_irq_init);
module_exit(mstar_dbg_irq_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("Abnormal Interrupt Detector");
MODULE_LICENSE("GPL");
