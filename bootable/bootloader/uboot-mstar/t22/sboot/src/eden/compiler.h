/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#ifndef __INIT_H__
#define __INIT_H__

#if defined(__GNUC__)

#define __used          __attribute__ ((used))
#define __unused        __attribute__ ((used))
#define __packed        __attirbute__ ((packed))
#define __section(_x_)  __attribute__ ((__section__(_x_)))

#define __likely(_x_)   __builtin_expect (!!(_x_), 1)
#define __unlikely(_x_) __builtin_expect (!!(_x_), 0)

typedef int (*initcall_t)(void);

#define __init          __section (".init.text")
#define __initdata      __section (".init.data")

#define __define_initcall(_cat_, _fn_) \
        static initcall_t __initcall_ ## _fn_ __used \
        __section(".init_" _cat_ ".init") = _fn_

#define core_initcall(fn)   __define_initcall("core", fn)
#define arch_initcall(fn)   __define_initcall("arch", fn)
#define drv_initcall(fn)    __define_initcall("drv", fn)
#define api_initcall(fn)    __define_initcall("api", fn)
#define mw_initcall(fn)     __define_initcall("mw", fn)
#define app_initcall(fn)    __define_initcall("app", fn)

#else   /* !__GNUC__ */

#define __used
#define __unused
#define __packed
#define likely(_x_)     _x_
#define unlikely(_x_)   _x_

#endif

#endif /* __INIT_H__ */
