#
# (C) COPYRIGHT 2014-2017 MStar Semiconductor, Inc. All rights reserved.
#
# This program is free software and is provided to you under the terms of the
# GNU General Public License version 2 as published by the Free Software
# Foundation, and any use by you of this program is subject to the terms
# of such GNU licence.
#
# A copy of the licence is included with the program, and can also be obtained
# from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA  02110-1301, USA.
#
#

# default options for all projects
debug ?= 0
dvfs ?= 1

# Set up defaults of MStar features
umm_export ?= 1
skip_jobs ?= 1
use_fixed_devid ?= 1
dvfs_freq_adjustable ?= 0
clock_debug_adjustable ?= 1
deglitch_patch ?= 0