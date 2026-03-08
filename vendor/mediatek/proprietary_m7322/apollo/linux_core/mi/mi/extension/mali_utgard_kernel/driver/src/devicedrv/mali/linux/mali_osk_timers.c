/*
 * Copyright (C) 2010, 2013, 2015, 2018 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mali_osk_timers.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */

#include <linux/timer.h>
#include <linux/slab.h>
#include "mali_osk.h"
#include "mali_kernel_common.h"

struct _mali_osk_timer_t_struct {
	struct timer_list timer;
};

typedef void (*timer_timeout_function_t)(unsigned long);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
_mali_osk_timer_t *_mali_osk_timer_init(void)
{
	_mali_osk_timer_t *t = (_mali_osk_timer_t *)kmalloc(sizeof(_mali_osk_timer_t), GFP_KERNEL);
	if (NULL != t) init_timer(&t->timer);
	return t;
}
#endif

void _mali_osk_timer_add(_mali_osk_timer_t *tim, unsigned long ticks_to_expire)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,19,0)
	tim->expires = jiffies + ticks_to_expire;
	add_timer(tim);
#else
	tim->timer.expires = jiffies + ticks_to_expire;
	add_timer(&(tim->timer));
#endif
}

void _mali_osk_timer_mod(_mali_osk_timer_t *tim, unsigned long ticks_to_expire)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,19,0)
	mod_timer(tim, jiffies + ticks_to_expire);
#else
	mod_timer(&(tim->timer), jiffies + ticks_to_expire);
#endif
}

void _mali_osk_timer_del(_mali_osk_timer_t *tim)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,19,0)
	del_timer_sync(tim);
#else
	del_timer_sync(&(tim->timer));
#endif
}

void _mali_osk_timer_del_async(_mali_osk_timer_t *tim)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,19,0)
	del_timer(tim);
#else
	del_timer(&(tim->timer));
#endif
}

mali_bool _mali_osk_timer_pending(_mali_osk_timer_t *tim)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,19,0)
	return 1 == timer_pending(tim);
#else
	return 1 == timer_pending(&(tim->timer));
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
void _mali_osk_timer_setcallback(_mali_osk_timer_t *tim, _mali_osk_timer_callback_t callback, void *data)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
	tim->timer.data = (unsigned long)data;
	tim->timer.function = (timer_timeout_function_t)callback;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
void _mali_osk_timer_term(_mali_osk_timer_t *tim)
{
	MALI_DEBUG_ASSERT_POINTER(tim);
	kfree(tim);
}
#endif
