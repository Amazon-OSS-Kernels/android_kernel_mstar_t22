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
#ifndef _VOC_PCM_H_
#define _VOC_PCM_H_

//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------
#define DMA_EMPTY 		   0
#define DMA_UNDERRUN		 1
#define DMA_OVERRUN      2
#define DMA_FULL         3
#define DMA_NORMAL 		   4


//------------------------------------------------------------------------------
//  Variables
//------------------------------------------------------------------------------
extern unsigned long long g_nCapStartTime;

struct voc_pcm_runtime_data
{
    spinlock_t            lock;
    volatile unsigned int         state;
    size_t                dma_level_count;
    size_t                int_level_count;
    struct voc_pcm_dma_data *dma_data;
    void *private_data;
};

struct voc_pcm_dma_data
{
  unsigned char *name;    /* stream identifier */
  unsigned long channel;    /* Channel ID */
};


#endif /* _VOC_PCM_H_ */
