#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

static int _debug;
module_param_named(debug, _debug, int, 0444);
static int _limit = 0x100;	/* sould be power of 2, eg. 0x100=256 */
module_param_named(limit, _limit, int, 0444);
static int _msleep = 200;
module_param_named(msleep, _msleep, int, 0444);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/kobject.h>
#include <linux/delay.h>
#include "dev.c"

#ifndef UNUSED
#define UNUSED(x)  (void *)(x)
#endif

#define MAKE_NODE_DEBUG_NODE (1 << 0)

struct kset *mknode_kset;
struct kobject *dev_kobj;

#define MAX_ENV_LEN 128

enum {
	ENV_MAJOR,
	ENV_MINOR,
	ENV_DEVPATH,
	ENV_SUBSYSTEM,
	ENV_LAST
};

#define TAG "devnode"

static char *dev_envp[ENV_LAST+1];

static void dynamic_kobj_release(struct kobject *kobj)
{
	pr_warn("kobject: (%p): %s\n", kobj, __func__);
	kfree(kobj);
}

static struct kobj_type dynamic_kobj_ktype = {
	.release	= dynamic_kobj_release,
	.sysfs_ops	= &kobj_sysfs_ops,
};

static struct kobject *create_kobj(char *name)
{
	struct kobject *kobj;

	kobj = kzalloc(sizeof(*kobj), GFP_KERNEL);
	if (kobj) {
		if (kobject_init_and_add(
			    kobj, &dynamic_kobj_ktype, NULL, "%s", name))
			pr_err(TAG ":cannot init_and_add '%s' kobject\n", name);

		kobj->kset = mknode_kset;
		list_add_tail(&kobj->entry, &kobj->kset->list);
	} else {
		pr_err(TAG ":cannot alloc '%s' kobject\n", name);
	}
	return kobj;
}

static void free_kobj(struct kobject *kobj)
{
	if (kobj) {
		kobject_del(kobj);
		kfree(kobj);
	}
}

static void init_dev_env(void)
{
	int i;

	dev_envp[0] = kzalloc(MAX_ENV_LEN*ENV_LAST, GFP_KERNEL);

	for (i = 1; i < ENV_LAST; i++)
		dev_envp[i] = (dev_envp[i-1]+MAX_ENV_LEN);
	dev_envp[i] = 0;
}

static void set_dev_env(int major, int minor, const char *subsystem,
		const char *devpath)
{
	snprintf(dev_envp[ENV_MAJOR], MAX_ENV_LEN, "MAJOR=%d", major);
	snprintf(dev_envp[ENV_MINOR], MAX_ENV_LEN, "MINOR=%d", minor);
	if (subsystem[0])
		snprintf(dev_envp[ENV_SUBSYSTEM], MAX_ENV_LEN,
				"SUBSYSTEM=%s", subsystem);
	if (devpath[0])
		snprintf(dev_envp[ENV_DEVPATH], MAX_ENV_LEN,
				"DEVPATH=%s", devpath);
}

static int uevent_add(const char *filename, unsigned int dev)
{
	static const char * const subsys[] = { "block", "misc" };
	const char *basename;
	int error = 0;
	bool is_char = filename[0] == 'c';

	filename++;
	basename = strrchr(filename, '/') + 1;
	if (basename == (void *)1)
		basename = filename;

	set_dev_env(MAJOR(dev), MINOR(dev), subsys[is_char], basename);
	kobject_uevent_env(dev_kobj, KOBJ_ADD, dev_envp);

	return error;
}

/* This is a kthread function and it should be ended with do_exit(0) */
int uevent_node(void *dummy)
{
	int i;
	int ret;

	init_dev_env();

	mknode_kset = kset_create_and_add("mknode", NULL, kernel_kobj);
	if (!mknode_kset) {
		pr_err("%s: cannot create mknode_kset\n", __func__);
		return -2;
	}

	dev_kobj = create_kobj("dev_obj");
	for (i = 0; i < ARRAY_SIZE(node_array); i++) {
		struct node_data *node = &node_array[i];

		if (i && (i % _limit) == 0) {
			if (_debug & MAKE_NODE_DEBUG_NODE)
				pr_info("%s: msleep %d\n", __func__, _msleep);
			msleep(_msleep);
		}
		if (node->name == (void *)0)
			break;
		switch (node->name[0]) {
		case 'c':
		case 'b':
			ret = uevent_add(node->name, node->dev);
			if (_debug & MAKE_NODE_DEBUG_NODE)
				pr_info("%s: %3d event %c %s (%d,%d). ret %d\n",
					__func__, i,
					node->name[0], node->name+1,
					MAJOR(node->dev), MINOR(node->dev), ret);
			break;
		default:
			pr_err("%s: %3d unknown entry %s 0x%08x\n", __func__,
					i, node->name, (u32)node->dev);
			break;
		}
	}
	pr_crit("[devnode] %s: broadcast uevent done\n", __func__);

	do_exit(0);
	return 0;
}

static int __init devnode_init(void)
{
	struct task_struct *k_mknod;

	pr_crit("[devnode] %s: start\n", __func__);
	k_mknod = kthread_run(uevent_node, NULL, "kworker_devnode");
	pr_crit("[devnode] %s: end\n", __func__);
	return 0;
}
static void __exit devnode_exit(void)
{
	kfree(dev_envp[0]);
	if (dev_kobj)
		free_kobj(dev_kobj);
	if (mknode_kset)
		kset_unregister(mknode_kset);
}
#else
static int __init devnode_init(void)
{
	return 0;
}
static void __init devnode_exit(void)
{
}
#endif

module_init(devnode_init);
module_exit(devnode_exit);

MODULE_LICENSE("GPL");
