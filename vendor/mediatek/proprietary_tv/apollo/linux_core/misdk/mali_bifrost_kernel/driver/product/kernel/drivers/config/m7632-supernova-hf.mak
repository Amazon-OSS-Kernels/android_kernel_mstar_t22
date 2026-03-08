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

# predefined configs
include $(CONFIG_DIR)/default.mak
include $(CONFIG_DIR)/m7632.mak
include $(CONFIG_DIR)/linux.mak
include $(CONFIG_DIR)/arm.mak

# project config
project=supernova
toolchain=arm-linux-gnueabihf-
dvfs=0
enable_gpu_mfdec=1
