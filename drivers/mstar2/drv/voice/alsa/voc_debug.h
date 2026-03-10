///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2017 MStar Semiconductor, Inc.
// This program is free software.
// You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _VOC_DEBUG_H_
#define _VOC_DEBUG_H_
//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------
#include "chip_int.h"

#if 0
#define TRACE_LEVEL_TAG        1
#define ERROR_LEVEL_TAG        1
#define DEBUG_LEVEL_TAG        1
#define IRQ_LEVEL_TAG          1
#else
#define TRACE_LEVEL_TAG        1
#define ERROR_LEVEL_TAG        1
#define DEBUG_LEVEL_TAG        0
#define IRQ_LEVEL_TAG          0
#endif

#define NONE			"\033[m"
#define RED				"\033[0;32;31m"
#define LIGHT_RED		"\033[1;31m"
#define GREEN			"\033[0;32;32m"
#define LIGHT_GREEN		"\033[1;32m"
#define BLUE			"\033[0;32;34m"
#define LIGHT_BLUE		"\033[1;34m"
#define DARY_GRAY		"\033[1;30m"
#define CYAN			"\033[0;36m"
#define LIGHT_CYAN		"\033[1;36m"
#define PURPLE			"\033[0;35m"
#define LIGHT_PURPLE	"\033[1;35m"
#define BROWN			"\033[0;33m"
#define YELLOW			"\033[1;33m"
#define LIGHT_GRAY		"\033[0;37m"
#define WHITE			"\033[1;37m"

#define TRACE_LEVEL          LIGHT_GREEN"[VOC TRACE]"NONE
#define ERROR_LEVEL          LIGHT_RED"[VOC ERROR]"NONE
#define DEBUG_LEVEL          LIGHT_BLUE"[VOC DEBUG]"NONE
#define IRQ_LEVEL            "[VOC IRQ]"


#define LOG_MSG 					1
#if LOG_MSG
#define VOC_PRINTF(level ,fmt, arg...)		if (level##_TAG) printk(KERN_ERR level fmt, ##arg);
#else
#define VOC_PRINTF(level ,fmt, arg...)
#endif

//#define __VOICE_AEC_SW_SYNC__
#define __VOICE_I2S__
#define __VOICE_HW_AEC__
//#define __VOICE_MSYS_ALLOC__
#define __VOICE_DA__

#endif  /* _VOC_DEBUG_H_ */
