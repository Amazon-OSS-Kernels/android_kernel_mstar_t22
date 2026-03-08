/*
 *
 * (C) COPYRIGHT 2014-2017 MStar Semiconductor, Inc. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */

/**
 * @file mali_kbase_clock_debugfs.h
 * Header file for mali_debug entry in debugfs
 *
 */

#include <linux/debugfs.h>
#include <linux/seq_file.h>

/**
 * @brief Initialize mali debug debugfs entry
 */
void kbasep_mali_debug_debugfs_init(struct kbase_device *kbdev);

