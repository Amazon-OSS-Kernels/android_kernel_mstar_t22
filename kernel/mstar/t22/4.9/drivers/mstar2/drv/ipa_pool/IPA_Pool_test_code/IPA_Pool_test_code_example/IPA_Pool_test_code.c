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


/* Test main file */





/*
 *
 * Copyright (c) 2010 Alan Ott <alan@signal11.us>
 * Copyright (c) 2010 Signal 11 Software
 *
 * The code may be used by anyone for any purpose,
 * and can serve as a starting point for developing
 * applications using hidraw.
 */

/* Linux */
//#include <linux/types.h>

/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 *
 * If you need this, please have your distro update the kernel headers.
 */

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
//#include "ipa_api.h"

#include <sys/mman.h>  
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "mdrv_ipa_pool_uapi.h"
#include "MsTypes.h"
#include<signal.h> 
#include <time.h>
#include <sys/time.h>

/*
    dev_name
    return value, file number
*/
char dev_name_default[]="/dev/ipapool";

int dev = 0;//global variable
struct timeval tvstart;
struct timeval tvend;

//inside utopia API,no need  function be similar with ipa_open ,for it will be done in MApi_IPA_Pool_Init.
int ipa_open()
{
    int fd = open(dev_name_default, O_RDWR);
    return fd;
}

//inside utopia API,no need  function be similar with ipa_close ,Never close.
int ipa_close(int fd)
{
    close(fd);
    return 0;
}

struct IPA_Pool_Polling_Param
{
    MS_U32 pool_handle_id;
    void (*polling_callback)(MS_U32 pool_handle_id,MS_U64 start,MS_U64 length);
};
#define MAX_IPAPOOLSIZE 16UL
enum IPA_SPACE_TYPE
{
    IPA_SPACE_TYPE_PA = 0,//directly PA space
    IPA_SPACE_TYPE_CMA = 1, //CMA space 
    IPA_IPA_SPACE_TYPE_INVALID = 0xFFFFFFFF//force enum size to be 4 byte alignment
};
#define MAX_CLIENT_MAP_NUM 8UL

struct VIRT_MAP_INFO
{
	MS_U64 virt_addr;
	MS_U64 length;
	MS_BOOL bNonCache;
	MS_U64 Physaddr;
};

struct IPA_Pool_Init_Param_No_P
{
    MS_U32 space_id;     //in: space id the pool will be created in
    //MS_U64 pool_name;//in: global identify name for pool to shared between multiple process
    MS_U8 pool_name[IPAPOOL_NAME_MAX_LONG];
	
    MS_U64 offset_in_heap;    //in: pool location in space
    MS_U64 len;       //in: pool length in  space 

    MS_U32 pool_handle_id; //out: generate pool id based on space specified by space_id
    MS_U32 miu;  //out: miu id this space belongs, index from 0.
    enum IPA_SPACE_TYPE space_type;//out: return space type to application
    MS_S32 error_code; // error code when pool init failed

    MS_U64 space_length; //out: space leagth
    MS_U64 space_miu_start_offset; //out: space start offset in miu
};
enum PA_In_IPA_Pool_State_Param
{
    PA_NOT_IN_MIU_Param = (1<<0),//PA not in any miu
    PA_NOT_IN_HEAP_Param =(1<<1),//PA in a miu,but not in any heap
    PA_NOT_IN_POOL_Param =(1<<2),//PA in a heap ,but not in any pool(means not allocated)
    PA_ALLOCATED_Param =(1<<3),//PA in a pool(means pa allocated)
    PA_CASE_NUM_Param
};

struct PA_In_IPA_Pool_Param
{
    MS_U64 PA;//in:   PA value
    MS_S32 miu;//out:  in which miu
    
    MS_BOOL in_heap;//out:  whether in heap(space).If not, relative value can not get.
    MS_S32 space_id;     //out: space id the pa in 
    enum IPA_SPACE_TYPE space_type;//out: return space type to application
    MS_U64 space_miu_start_offset; //out: space start offset in miu    
    MS_U64 space_length; //out: space leagth
    MS_U64 pa_offset_in_heap;    //out: pa location in heap
    
    MS_BOOL allocated;//out :whether already allocated,if not , relative value can not get.
    //MS_S32 pool_handle_id; //out: allocated by which pool.
    char pool_name[128];//out: name of pool.
    MS_U64 pool_len;       //out: pool length in  space 
    MS_U64 pool_offset_in_heap;    //out: pool location in heap
    MS_U64 pa_offset_in_pool;    //out: pa location in pool

    enum PA_In_IPA_Pool_State_Param pa_state;//out:pa state summary
    MS_S32 error_code; // error code when pool init failed
};


/*N.B.
 PA:base ARM_MIU0_BUS_BASE/ARM_MIU1_BUS_BASE/ARM_MIU2_BUS_BASE 
if PA value inside [ARM_MIU0_BUS_BASE,ARM_MIU1_BUS_BASE),PA is in miu0
if PA value inside [ARM_MIU1_BUS_BASE,ARM_MIU2_BUS_BASE),PA is in miu1
......


to meet modules less use/store global variables request,
input of this API only have pool_handle_id and offset_in_pool.
*/
struct Pool_To_PA_Param
{
    MS_U32 pool_handle_id;//in : pa in which pool.
    MS_U64 offset_in_pool;//in: pa offset in pool.

    MS_U64 PA;//out:   PA value.
    MS_S32 error_code; // out： reason why failed.

    //give more out ,which nowadays no modules use,but may future use.
    MS_S32 miu;//out:  in which miu.
    MS_U32 heap_id;     //out: heap id the pa in .After get this,with the help of MApi_IPA_Pool_HEAP_ATTR,can get more more info.
};



//no need code about this mutex,for in test code,in one process we will 
//fork its child process.
//to avoid dead lock,no longer use this mutex.
//static pthread_mutex_t  _IPA_POOL_Mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    struct IPA_Pool_Init_Param_No_P Init_Param;
    MS_BOOL bIsUsed;
    struct VIRT_MAP_INFO map_info[MAX_CLIENT_MAP_NUM];
    volatile MS_BOOL polling_thread_delete_task_flag;
    pthread_t pthIPAPollingId;
    void (*polling_callback)(MS_U32 pool_handle_id,MS_U64 start,MS_U64 length);
} IPAPOOL_INFO;

static IPAPOOL_INFO IPAPool_Info[MAX_IPAPOOLSIZE];

static MS_BOOL _findEmpty_IPA_Pool_Entry(MS_U32 *index)
{
    MS_BOOL find = FALSE;
    MS_U32 i;

    *index = 0;
    for (i = 0; i < MAX_IPAPOOLSIZE; i++)
    {
        if(IPAPool_Info[i].bIsUsed == FALSE)
        {
            find = TRUE;
            *index = i;
            break;
        }
    }

    if(find == FALSE)
        printf("Not enough IPAPool, must increase MAX_IPAPOOLSIZE!!\n");
    
    return find;
}


static MS_BOOL _findPoolHandleId_InIPA_Pool_Table(MS_U32 pool_handle_id, MS_U32 *index)
{
    MS_BOOL find = FALSE;
    MS_U32 i;

    *index = 0;
    for (i = 0; i < MAX_IPAPOOLSIZE; i++)
    {
        if((IPAPool_Info[i].bIsUsed == TRUE) && (IPAPool_Info[i].Init_Param.pool_handle_id == pool_handle_id))
        {
            find = TRUE;
            *index = i;
            break;
        }
    }
    
    return find;
}

static MS_BOOL _findHeapId_InIPA_Pool_Table(struct IPA_Pool_Init_Args Init_Param,MS_U32 *index)
{
    MS_BOOL find = FALSE;
    MS_U32 i;

    *index = 0;
    for (i = 0; i < MAX_IPAPOOLSIZE; i++)
    {
        //pool name 的比较直接用名字所在的地址粗暴比较即可。
        if((IPAPool_Info[i].bIsUsed == TRUE) 
			&& (IPAPool_Info[i].Init_Param.space_id == Init_Param.heap_id)
			&&(!strncmp((char *)IPAPool_Info[i].Init_Param.pool_name, Init_Param.pool_name,strlen(Init_Param.pool_name)))
			/*&&(IPAPool_Info[i].Init_Param.pool_name == Init_Param.pool_name)*/
			&& (IPAPool_Info[i].Init_Param.offset_in_heap == Init_Param.offset_in_heap)
			&& (IPAPool_Info[i].Init_Param.len == Init_Param.len))
			
        {
            find = TRUE;
            *index = i;
            break;
        }
    }
    
    return find;
}


MS_BOOL __attribute__((weak)) PA_in_IPA_POOL_info(struct PA_In_IPA_Pool_Param * in_ipa_pool_info)
{
    MS_BOOL ret = FALSE;
    int res = 0;
    struct PA_In_IPA_Pool_Args in_ipa_pool_info_args;
    if (dev <= 0)//may never open device
    {
        if ((dev = open("/dev/ipapool", O_RDWR)) < 0)
        {
            printf("open /dev/ipapool fail\n");
            ret =  FALSE;
            goto PA_in_IPA_POOL_info;
        }

        memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);
    }
    in_ipa_pool_info_args.PA = in_ipa_pool_info->PA;
    res = ioctl(dev, IPA_POOL_IOC_PA_INFO, &in_ipa_pool_info_args);
    if (res < 0 ||  in_ipa_pool_info_args.error_code < 0) 
    {
        ret =  FALSE;
        goto PA_in_IPA_POOL_info;     

    }
    printf(" %s  PA:0x%llx   pa_state=%d\n",__FUNCTION__, in_ipa_pool_info_args.PA,in_ipa_pool_info_args.pa_state);

     printf("%s:%d debug   in_ipa_pool_info_args  PA=0x%lx,miu=0x%x,space_id=0x%x,pool_offset_in_heap=0x%lx,pa_offset_in_pool=0x%lx   pa_state=%d\n",__FUNCTION__,__LINE__,(unsigned long)in_ipa_pool_info_args.PA,in_ipa_pool_info_args.miu,in_ipa_pool_info_args.space_id,(unsigned long long)in_ipa_pool_info_args.pool_offset_in_heap,(unsigned long long)in_ipa_pool_info_args.pa_offset_in_pool,in_ipa_pool_info_args.pa_state);

     in_ipa_pool_info->pa_state= in_ipa_pool_info_args.pa_state;
     in_ipa_pool_info->miu= in_ipa_pool_info_args.miu;
     if(in_ipa_pool_info->miu < 0)
     {
        printf("ipa error: %s:%d failed, unsigned long PA 0x%lx,miu=0x%x\n",__FUNCTION__,__LINE__, (unsigned long)in_ipa_pool_info_args.PA,in_ipa_pool_info_args.miu);
        in_ipa_pool_info->in_heap = FALSE;
        in_ipa_pool_info->allocated = FALSE;
        ret =  FALSE;
        goto PA_in_IPA_POOL_info;     
     }
     in_ipa_pool_info->in_heap= in_ipa_pool_info_args.in_heap;
     if(FALSE == in_ipa_pool_info->in_heap)
     {
        printf("ipa error: %s failed, PA 0x%lx,miu=%d,in_heap is FALSE\n",__FUNCTION__,(unsigned long)in_ipa_pool_info_args.PA,in_ipa_pool_info_args.miu);
        in_ipa_pool_info->allocated = FALSE;
        ret =  FALSE;
        goto PA_in_IPA_POOL_info;       
     }
     else
     {
         in_ipa_pool_info->space_id= in_ipa_pool_info_args.space_id;
         in_ipa_pool_info->space_type= in_ipa_pool_info_args.space_type;
         in_ipa_pool_info->space_miu_start_offset= in_ipa_pool_info_args.space_miu_start_offset;
         in_ipa_pool_info->space_length= in_ipa_pool_info_args.space_length;
         in_ipa_pool_info->pa_offset_in_heap= in_ipa_pool_info_args.pa_offset_in_heap;
        
         in_ipa_pool_info->allocated = in_ipa_pool_info_args.allocated;
         if(FALSE == in_ipa_pool_info->allocated)
         {
             printf("ipa error: %s failed, unsigned long PA 0x%lx,miu=%d,in_ipa_pool_info_args.space_id=%d,allocated is FALSE\n",__FUNCTION__, (unsigned long)in_ipa_pool_info_args.PA,in_ipa_pool_info_args.miu,in_ipa_pool_info_args.space_id);
             ret =  FALSE;
             goto PA_in_IPA_POOL_info;                    
         }
         else
         {
             //in_ipa_pool_info->pool_handle_id= in_ipa_pool_info_args.pool_handle_id;
#if defined (__aarch64__)
            strcpy((char *)in_ipa_pool_info->pool_name , in_ipa_pool_info_args.pool_name);
#else
            strcpy((char *)in_ipa_pool_info->pool_name ,in_ipa_pool_info_args.pool_name);
#endif
            in_ipa_pool_info->pool_len = in_ipa_pool_info_args.pool_len;
            in_ipa_pool_info->pool_offset_in_heap = in_ipa_pool_info_args.pool_offset_in_heap;
            in_ipa_pool_info->pa_offset_in_pool = in_ipa_pool_info_args.pa_offset_in_pool;
        }
    }
    ret =  TRUE;

PA_in_IPA_POOL_info:
    
   return ret;
}

MS_BOOL __attribute__((weak)) IN_IPA_POOL_To_PA(struct Pool_To_PA_Param * pool_to_pa_param)
{
    MS_BOOL ret = TRUE;
    struct Pool_To_PA_Args pool_to_pa_args;
    int res = 0;

    printf("%s handle_id 0x%x offset_in_pool 0x%lx\n", 
        __FUNCTION__,pool_to_pa_param->pool_handle_id, (unsigned long)pool_to_pa_param->offset_in_pool);

    printf("%s  %d\n",__FUNCTION__,__LINE__);
    //pthread_mutex_lock(&_IPA_POOL_Mutex);  
    if (dev < 0)
    {
        ret = FALSE;
        printf("%s  %d",__FUNCTION__,__LINE__);
        goto POOL_To_PA_DONE;
    }
    printf("%s  %d\n",__FUNCTION__,__LINE__);

    //input
    pool_to_pa_args.pool_handle_id = pool_to_pa_param->pool_handle_id;  
    pool_to_pa_args.offset_in_pool = pool_to_pa_param->offset_in_pool;
    printf("%s  %d\n",__FUNCTION__,__LINE__);
    res = ioctl(dev, IPA_POOL_IOC_POOL_TO_PA, &pool_to_pa_args);
    printf("%s  %d\n",__FUNCTION__,__LINE__);
    if (res < 0 || pool_to_pa_args.error_code != IPAERROR_OK)
    {  
        printf("%s fail: pool_handle_id %u, offset 0x%lx, error_code=0x%x  res=%d\n",__FUNCTION__, pool_to_pa_param->pool_handle_id, 
            (unsigned long)pool_to_pa_param->offset_in_pool,pool_to_pa_args.error_code,res);
        ret = FALSE;
        goto POOL_To_PA_DONE;
    }

    //output
    pool_to_pa_param->PA = pool_to_pa_args.PA;
    pool_to_pa_param->error_code = pool_to_pa_args.error_code;
    pool_to_pa_param->miu = pool_to_pa_args.miu;
    pool_to_pa_param->heap_id = pool_to_pa_args.heap_id;
    
POOL_To_PA_DONE:
    //pthread_mutex_unlock(&_IPA_POOL_Mutex);
    
    return ret;
}


//this API is similay with MApi_IPA_Pool_Init.
int ipa_init(MS_U32 heap_id, char *pool_name, MS_U64 offset, MS_U64 len, MS_U32 * pool_id)
{
    int fd =dev;
    int res;
    struct IPA_Pool_Init_Args init_para;

    init_para.heap_id = heap_id;
    strcpy(init_para.pool_name,pool_name);
    init_para.offset_in_heap = offset;
    init_para.len = len;
    MS_U32 idx = 0;
    MS_BOOL ret;
    printf("%s:%d  \n",__FUNCTION__,__LINE__);
    //pthread_mutex_lock(&_IPA_POOL_Mutex);
    printf("%s:%d  \n",__FUNCTION__,__LINE__);
    ret = _findHeapId_InIPA_Pool_Table(init_para, &idx);
    printf("%s:%d  \n",__FUNCTION__,__LINE__);
    if(ret == TRUE)
    {
        init_para.heap_id = IPAPool_Info[idx].Init_Param.space_id;
        
        //init_para.pool_name = (char *)IPAPool_Info[idx].Init_Param.pool_name;
        strcpy(init_para.pool_name ,IPAPool_Info[idx].Init_Param.pool_name);
        
        init_para.offset_in_heap=IPAPool_Info[idx].Init_Param.offset_in_heap;
        init_para.len = IPAPool_Info[idx].Init_Param.len;
        *pool_id = init_para.pool_handle_id = IPAPool_Info[idx].Init_Param.pool_handle_id;
        init_para.miu = IPAPool_Info[idx].Init_Param.miu;
        init_para.error_code = IPAPool_Info[idx].Init_Param.error_code;
        init_para.heap_type = IPAPool_Info[idx].Init_Param.space_type;
        init_para.heap_length= IPAPool_Info[idx].Init_Param.space_length;
        init_para.heap_miu_start_offset = IPAPool_Info[idx].Init_Param.space_miu_start_offset;
        
        //pthread_mutex_unlock(&_IPA_POOL_Mutex);
        printf("pool_handle_id %u already init!\n", IPAPool_Info[idx].Init_Param.pool_handle_id);

        return 0;    
    }
    printf("%s:%d  \n",__FUNCTION__,__LINE__);
    res = ioctl(fd, IPA_POOL_IOC_INIT, &init_para);
    printf("%s:%d  \n",__FUNCTION__,__LINE__);
    if (res < 0 || init_para.error_code != IPAERROR_OK) 
    {
        printf("ipa init failed, error_code %d\n", init_para.error_code);
        //pthread_mutex_unlock(&_IPA_POOL_Mutex);

        return -1;
    }
    *pool_id = init_para.pool_handle_id; 
    printf("MApi_CMA_Pool_Init after ioctl  *pool_id =%u\n",*pool_id );    

                ret = _findEmpty_IPA_Pool_Entry(&idx);
            printf("MApi_CMA_Pool_Init after _findEmpty_IPA_Pool_Entry\n");        
                if(ret == FALSE)
                {
                    printf("ipa error: pool_handle_id %u init failed!\n",init_para.pool_handle_id);
                    //pthread_mutex_unlock(&_IPA_POOL_Mutex);
                    return -1;
                }
                printf("MApi_CMA_Pool_Init after _findEmpty_IPA_Pool_Entry  idx=%u\n",idx);    


            IPAPool_Info[idx].Init_Param.pool_handle_id =*pool_id;
    
            IPAPool_Info[idx].Init_Param.space_id = init_para.heap_id;
//#if defined (__aarch64__)
        strcpy(IPAPool_Info[idx].Init_Param.pool_name , (MS_U64)init_para.pool_name);
//#else
//        IPAPool_Info[idx].Init_Param.pool_name = (MS_U32)init_para.pool_name;
//#endif
            IPAPool_Info[idx].Init_Param.offset_in_heap =init_para.offset_in_heap;
            IPAPool_Info[idx].Init_Param.len =init_para.len;
            IPAPool_Info[idx].Init_Param.pool_handle_id =init_para.pool_handle_id;
            IPAPool_Info[idx].Init_Param.miu =init_para.miu;
            IPAPool_Info[idx].Init_Param.space_type =init_para.heap_type;
            IPAPool_Info[idx].Init_Param.error_code =init_para.error_code;
            IPAPool_Info[idx].Init_Param.space_length =init_para.heap_length;
            IPAPool_Info[idx].Init_Param.space_miu_start_offset =init_para.heap_miu_start_offset;
            IPAPool_Info[idx].bIsUsed = TRUE; 
            printf("%s:%d  IPAPool_Info[%d].Init_Param.pool_handle_id=%u, IPAPool_Info[%d].bIsUsed=%d\n",__FUNCTION__,__LINE__,idx,
            IPAPool_Info[idx].Init_Param.pool_handle_id, idx ,!!IPAPool_Info[idx].bIsUsed);
            IPAPool_Info[idx].pthIPAPollingId = -1;//in init,no polling id yet.
            IPAPool_Info[idx].polling_thread_delete_task_flag = FALSE;
 
   // printf("miu %d miu_start_offset 0x%llx len 0x%llx type %u\n", init_para.miu, 
   //     init_para.heap_miu_start_offset, init_para.len, init_para.heap_type);
    printf("init heap id %u success, pool handle %u\n", heap_id, init_para.pool_handle_id);
    //pthread_mutex_unlock(&_IPA_POOL_Mutex);

    return 0;
}

//If timeout is 0,this API is similay with MApi_IPA_Pool_GetMem.
//If timeout is larger than 0, this API is similay with MApi_IPA_Pool_GetMem_Timeout.
int ipa_alloc(MS_U32 pool_handle_id, MS_U64 start, MS_U64 len,MS_U32 timeout)
{
	int res;
       int fd = dev;

	struct IPA_Pool_Alloc_Args ipa_alloc_args;

	ipa_alloc_args.pool_handle_id = pool_handle_id;	
    ipa_alloc_args.offset_in_pool = start;
    ipa_alloc_args.length = len;

	ipa_alloc_args.timeout = timeout;
//pthread_mutex_lock(&_IPA_POOL_Mutex);  	
    res = ioctl(fd, IPA_POOL_IOC_ALLOC, &ipa_alloc_args);
    if (res < 0  || ipa_alloc_args.error_code != IPAERROR_OK) 
    {
       printf("will tvstart\n");
       gettimeofday( &tvstart, NULL);  
       printf("after tvstart  [%ld,%ld] \n",tvstart.tv_sec,tvstart.tv_usec);

    
       MS_U32 each_delay_ms = 50*1000;
       MS_U32 now_delay_ms=0;
       while(1)
       {    
           // delay and try again    
           printf("%s:%d  each_delay_ms=%u, delay and try alloc again now_delay_ms=%u\n",__FUNCTION__,__LINE__,each_delay_ms,now_delay_ms);
       
           ipa_alloc_args.timeout = 0;//into kernel will not consider timeout again
           printf("before usleep\n");
           usleep(each_delay_ms);
           printf("after usleep and will ioctl\n");
           res = ioctl(fd, IPA_POOL_IOC_ALLOC, &ipa_alloc_args);
           printf("after ioctl\n");
           if ((res == 0) && (ipa_alloc_args.error_code == IPAERROR_OK))
               break;
           now_delay_ms += each_delay_ms;
           if(now_delay_ms >= timeout*1000)
                break;
       }
       if (res < 0  || ipa_alloc_args.error_code != IPAERROR_OK) 
        {
        printf("ipa alloc failed, error %d", ipa_alloc_args.error_code);
        //pthread_mutex_unlock(&_IPA_POOL_Mutex);
        return -1;
        }
       printf("second alloc success\n");
    }

    printf("ipa alloc[0x%llx 0x%llx] success\n", start, len);
    //pthread_mutex_unlock(&_IPA_POOL_Mutex);
    return 0;
}



void *IPA_Pool_Polling_Task(void  *argc)
{
    MS_BOOL ret = TRUE;
    int res=0;
    MS_U32 idx = 0;
    struct IPA_Pool_Event_Args pool_event_args;
    MS_U32  pool_handle_id = (*(MS_U32  *)argc);
    printf("%s   201510211955\n",__FUNCTION__);
    //pthread_detach(pthread_self());   
    
    //pthread_mutex_lock(&_IPA_POOL_Mutex);
    if (dev < 0)
    {
        printf("%s  fail  \n",__FUNCTION__);      	
        ret = FALSE;
        return  NULL;
    } 
    pool_event_args.pool_handle_id = pool_handle_id;


    printf("%s , %d  before _findPoolHandleId_InIPA_Pool_Table  pool_event_args.pool_handle_id=%u\n",__FUNCTION__,__LINE__,pool_event_args.pool_handle_id);
    ret = _findPoolHandleId_InIPA_Pool_Table(pool_event_args.pool_handle_id, &idx);
    
    if(ret == FALSE)
    {
        printf("ipa pool IPA_Pool_Polling_Task  fail not find in pool table\n");
        return NULL;
    }
         
    while(1)
    {  
        //ioctl for event and after callback,shoud ioctl again.
        printf("%s , %d before ioctl IPA_POOL_IOC_POLL   pid %d ppid %d             pool_handle_id=%u\n",__FUNCTION__,__LINE__,getpid(),getppid(),pool_event_args.pool_handle_id);
        //before each ioctl,should clear invalid event value.
        memset(&pool_event_args,0,sizeof(struct IPA_Pool_Event_Args));
        pool_event_args.pool_handle_id = pool_handle_id;
        printf("%s , %d before2 ioctl IPA_POOL_IOC_POLL   pid %d ppid %d             pool_handle_id=%u\n",__FUNCTION__,__LINE__,getpid(),getppid(),pool_event_args.pool_handle_id);

        res = ioctl(dev,IPA_POOL_IOC_POLL,&pool_event_args);
        printf("%s , %d after ioctl IPA_POOL_IOC_POLL  pid %d ppid %d             pool_handle_id=%u       start=0x%llx,length=0x%llx\n",__FUNCTION__,__LINE__,getpid(),getppid(),pool_event_args.pool_handle_id,pool_event_args.start,pool_event_args.length);
       // printf("%s , %d  will sleep \n",__FUNCTION__,__LINE__);
       // sleep(5);
        //printf("%s , %d  after sleep \n",__FUNCTION__,__LINE__);
        if(res < 0)
        {
            printf("ipa pool IPA_Pool_Polling_Task  fail\n");
            ret = FALSE;
            goto IPA_POOL_POLLING_TASK_DONE;   
        }

        if(IPA_EVENT_NO_WAIT== pool_event_args.event )
        {
            printf("%s , %d  IPA_EVENT_NO_WAIT\n",__FUNCTION__,__LINE__);
            break; 
        }
 
        else if(IPA_EVENT_CONFLICT == pool_event_args.event )
        {
            printf("%s , %d before polling_callback\n",__FUNCTION__,__LINE__);
            //invoke callback corresponding to that pool event
            IPAPool_Info[idx].polling_callback(pool_event_args.pool_handle_id,pool_event_args.start,pool_event_args.length);//call callback
            printf("%s , %d after polling_callback\n",__FUNCTION__,__LINE__);


            printf("will tvend\n");
	gettimeofday( &tvend, NULL);
    
    printf("after tvend  [%ld,%ld] \n",tvend.tv_sec,tvend.tv_usec);

    
	printf("cost %ld ms   [%ld,%ld],[%ld,%ld]\n",((tvend.tv_sec*1000)+(tvend.tv_usec/1000))-((tvstart.tv_sec*1000)+(tvstart.tv_usec/1000)),
        tvend.tv_sec,tvend.tv_usec,tvstart.tv_sec,tvstart.tv_usec);           

        }
        else
        {
            printf("%s error ,event %d is invalid !!! but continue while    [%llx  %llx]\n",__FUNCTION__,pool_event_args.event,pool_event_args.start,pool_event_args.length);
        }
    }
    printf("%s , %d \n",__FUNCTION__,__LINE__);
IPA_POOL_POLLING_TASK_DONE:
    //pthread_mutex_unlock(&_IPA_POOL_Mutex); 
    printf("%s , %d before set  idx=%d ,  polling_thread_delete_task_flag=%d\n",__FUNCTION__,__LINE__,idx,IPAPool_Info[idx].polling_thread_delete_task_flag);
    //before return ,set this flag.And this flag will be checked in MApi_IPA_Pool_Release.
    IPAPool_Info[idx].polling_thread_delete_task_flag = TRUE;
    printf("%s , %d after set    idx=%d ,  polling_thread_delete_task_flag=%d\n",__FUNCTION__,__LINE__,idx,IPAPool_Info[idx].polling_thread_delete_task_flag);
    printf("%s , %d \n",__FUNCTION__,__LINE__);
    return NULL;

}

//this API is similay with utopia API MApi_IPA_Pool_Register_Notify.
int MApi_IPA_Pool_Register_Notify(struct IPA_Pool_Polling_Param *polling_param)
{
    MS_BOOL ret = TRUE;
    pthread_t pthIPAPollingId = -1;
    MS_U32 idx = 0;
    
    printf("%s  polling_param->pool_handle_id=%u\n",__FUNCTION__,polling_param->pool_handle_id);

    if(!polling_param->polling_callback)
    {
        printf("%u have no polling_callback function , %s return directly\n",polling_param->pool_handle_id,__FUNCTION__);
        return TRUE;
    }

    ret = _findPoolHandleId_InIPA_Pool_Table(polling_param->pool_handle_id, &idx);
    
    if(ret == FALSE)//not find idx
    {
         printf(" %s ipa pool fail not find pool_handle_id %u in pool table\n",__FUNCTION__,polling_param->pool_handle_id);
         return FALSE;
    }

    if( -1 == IPAPool_Info[idx].pthIPAPollingId) //no polling id refer to pool handle id
    {
    
        //create polling thread

        int err;
        printf("%s , %d  before pthread_create\n",__FUNCTION__,__LINE__);
        err = pthread_create(&pthIPAPollingId,
                                                NULL,
                                                IPA_Pool_Polling_Task,
                                                &polling_param->pool_handle_id);
        printf("%s , %d  after pthread_create\n",__FUNCTION__,__LINE__);
        if(err != 0)
        {
            printf("can't create thread :%s\n",strerror(err));
            return err;
        }
        
       printf("%s , %d  \n",__FUNCTION__,__LINE__);


        if(pthIPAPollingId < 0)
        {
            printf("CreateTask fail pthIPAPollingId=%d,pthIPAPollingId=%d\n",pthIPAPollingId,pthIPAPollingId);
            ret= FALSE;
        }
        else
        {
            //corresponding pthIPAPollingId to pool_handle_id  
            IPAPool_Info[idx].pthIPAPollingId = pthIPAPollingId;
            IPAPool_Info[idx].polling_callback = polling_param->polling_callback;
            printf("%s:%d   pthIPAPollingId=0x%x \n",__FUNCTION__,__LINE__,IPAPool_Info[idx].pthIPAPollingId);
        }
        printf("%s , %d  \n",__FUNCTION__,__LINE__);
    }
    else//already have polling id refer to pool handle id.
    {
        ret=FALSE;
        printf("pool handle id %u has its polling id %d ,error,will not set callback again\n",polling_param->pool_handle_id,IPAPool_Info[idx].pthIPAPollingId);
    }
    printf("%s , %d  b 5 pthread_exit\n",__FUNCTION__,__LINE__);
    //sleep(5);
    //pthread_exit(NULL);
    printf("%s , %d  a pthread_exit\n",__FUNCTION__,__LINE__);
    return ret;
}

//this API is similay with utopia API MApi_IPA_Pool_PutMem.
int ipa_free(MS_U32 pool_handle_id, MS_U64 start, MS_U64 len)
{
	int res;
       int fd =dev; 

	struct IPA_Pool_free_Args free_para;

	free_para.pool_handle_id = pool_handle_id;	
    free_para.offset_in_pool = start;
    free_para.length = len;
    //pthread_mutex_lock(&_IPA_POOL_Mutex);		
    res = ioctl(fd, IPA_POOL_IOC_FREE, &free_para);
    if (res < 0)
    {
        perror("ipa free failed");
        //pthread_mutex_unlock(&_IPA_POOL_Mutex);
        return res;
    }

    printf("ipa free[0x%llx 0x%llx] success\n", start, len);
    //pthread_mutex_unlock(&_IPA_POOL_Mutex);
    return 0;
}

//this API is similay with utopia API MApi_IPA_Pool_MapVA.
int ipa_map(MS_U32 pool_handle_id, MS_U64 start, MS_U64 len, MS_U32 map_va_type, MS_U64 * va)
                      
{
    int res;
    int fd=dev;
    struct IPA_Pool_Map_Args map_para;

    map_para.pool_handle_id = pool_handle_id;
	map_para.offset_in_pool = start;
	map_para.length = len;
	map_para.map_va_type = map_va_type;
//pthread_mutex_lock(&_IPA_POOL_Mutex);	
    res = ioctl(fd, IPA_POOL_IOC_MAP, &map_para);
    if (res < 0 || map_para.error_code != IPAERROR_OK)
    {
        printf("ipa map failed, error %d\n", map_para.error_code);
        //pthread_mutex_unlock(&_IPA_POOL_Mutex);  
        return -1;
    }
    *va = map_para.virt_addr;
	
	printf("ipa map start 0x%llx len 0x%llx type %u va %llx success\n", 
        start, len, map_va_type, *va);
    //pthread_mutex_unlock(&_IPA_POOL_Mutex);  
    return 0;
}

//this API is similay with utopia API MApi_IPA_Pool_UnmapVA.
int ipa_unmap( MS_U64 virt_addr, MS_U64 len)
                      
{
    int res;
    int fd=dev;
    struct IPA_Pool_Unmap_Args unmap_para;

    unmap_para.virt_addr = virt_addr;
	unmap_para.length = len;
	//pthread_mutex_lock(&_IPA_POOL_Mutex);
    res = ioctl(fd, IPA_POOL_IOC_UNMAP, &unmap_para);
    if (res < 0)
    {
        perror("ipa unmap failed");
        //pthread_mutex_unlock(&_IPA_POOL_Mutex); 
        return res;
    }

	printf("ipa unmap virt_addr 0x%llx len 0x%llx success\n", virt_addr, len);
    //pthread_mutex_unlock(&_IPA_POOL_Mutex); 
    return 0;
}

//this API is similay with utopia API MApi_IPA_Pool_DCacheFlush
int ipa_flush( MS_U64 virt_addr, MS_U64 len, MS_U32 flush_type)                      
{
    int res;
    int fd=dev;
    struct IPA_Pool_DCacheFlush_Args flush_para;

	flush_para.virt_addr = virt_addr;
	flush_para.length = len;
	flush_para.flush_type = flush_type;
	//pthread_mutex_lock(&_IPA_POOL_Mutex);
    res = ioctl(fd, IPA_POOL_IOC_FLUSH, &flush_para);
    if (res < 0)
    {
        perror("ipa flush failed");
        //pthread_mutex_unlock(&_IPA_POOL_Mutex); 	
        return res;
    }

	printf("ipa flush virt 0x%llx len 0x%llx flush_type %u success\n", virt_addr, len, flush_type);
    //pthread_mutex_unlock(&_IPA_POOL_Mutex); 	
    return 0;
}

//this API is similay with utopia API MApi_IPA_Pool_Release.
int ipa_deinit(MS_U32 pool_handle_id)
{
    int res;
    int fd=dev;
    MS_BOOL ret = TRUE;
    MS_U32 idx = 0;
    struct IPA_Pool_Deinit_Args deinit_para;

    deinit_para.pool_handle_id = pool_handle_id;	
    //pthread_mutex_lock(&_IPA_POOL_Mutex);

    printf("%s:%d   pool_handle_id=%u\n",__FUNCTION__,__LINE__,pool_handle_id);
    ret = _findPoolHandleId_InIPA_Pool_Table(pool_handle_id, &idx);
    printf("%s:%d   \n",__FUNCTION__,__LINE__);
    if(ret == TRUE)
    {
		     // if same client all maps have been unmapped ,set  IPAPool_Info[idx].bIsUsed be FALSE
        	 IPAPool_Info[idx].bIsUsed = FALSE;
        	 IPAPool_Info[idx].Init_Param.pool_handle_id = 0;  		
    }



    printf("%s:%d   \n",__FUNCTION__,__LINE__);
    if(IPAPool_Info[idx].pthIPAPollingId != -1)
    {
        if(TRUE == IPAPool_Info[idx].polling_thread_delete_task_flag)
            printf("%s:%d  before ioctl deinit delete_task_flag is ture\n",__FUNCTION__,__LINE__);
        else
            printf("%s:%d  before ioctl deinit delete_task_flag is false\n",__FUNCTION__,__LINE__);
        
    }
    res = ioctl(fd, IPA_POOL_IOC_DEINIT, &deinit_para);
    printf("%s:%d   res=%d\n",__FUNCTION__,__LINE__,res);
    if (res < 0)
    {
        perror("ipa deinit failed");
        //pthread_mutex_unlock(&_IPA_POOL_Mutex);
        return res;
    }

    //after deinit, if have polling thread,should delete task
    if(IPAPool_Info[idx].pthIPAPollingId != -1)
    {
        #if 0
        printf("%s:%d  idx=%d  polling_thread_delete_task_flag=%d\n",__FUNCTION__,__LINE__,idx,IPAPool_Info[idx].polling_thread_delete_task_flag);
        while(1)
        {
            printf("%s:%d  while for delete_task_flag   polling_thread_delete_task_flag=%d\n",__FUNCTION__,__LINE__,IPAPool_Info[idx].polling_thread_delete_task_flag);
            if(TRUE == IPAPool_Info[idx].polling_thread_delete_task_flag)
                {     
               printf("%s:%d  while will break   polling_thread_delete_task_flag=%d\n",__FUNCTION__,__LINE__,IPAPool_Info[idx].polling_thread_delete_task_flag);
                break;
                }
            //else
                //printf("while for delete_task_flag set \n");
        }
        #endif
        
        printf("%s:%d  idx=%d  polling_thread_delete_task_flag=%d\n",__FUNCTION__,__LINE__,idx,IPAPool_Info[idx].polling_thread_delete_task_flag);
        printf("%s:%d    pthIPAPollingId = 0x%x  10131359\n",__FUNCTION__,__LINE__,IPAPool_Info[idx].pthIPAPollingId);
        //polling_thread_delete_task_flag has been set true,now we can delete task.
        
        //pthread_kill(IPAPool_Info[idx].pthIPAPollingId,SIGKILL);
        //pthread_join(IPAPool_Info[idx].pthIPAPollingId, NULL);
            printf("will pthread_join\n");
            pthread_join(IPAPool_Info[idx].pthIPAPollingId, NULL);
            printf("after pthread_join\n");

           #if 0
                   //build SN lib can find pthread_cancel,
        //but build AN lib,can not find pthread_cancel,
        //in fact only need pthread_join,no need pthread_cancel.
            printf("will pthread_cancel \n");
            pthread_cancel(IPAPool_Info[idx].pthIPAPollingId);//delete polling task  
            printf("after pthread_cancel\n");
            #endif



            if(TRUE != IPAPool_Info[idx].polling_thread_delete_task_flag)
            {
                printf("error !!!!!!   pthread_cancel and pthread_join  polling_thread_delete_task_flag is not TRUE!!!\n");
                return -1;
            }
        
        printf("%s:%d after delete  \n",__FUNCTION__,__LINE__);
    }
    else
        printf("%s:%d    pthIPAPollingId is -1 \n",__FUNCTION__,__LINE__);



	printf("ipa deinit poolid %u success\n", pool_handle_id);
    //pthread_mutex_unlock(&_IPA_POOL_Mutex);
    return 0;
}




//#define DFB_TEMP_HID 27
#define DFB_TEMP_HID 28

#define PAGE 4096
#define MILLION (1<<20)
static char * Pool_Name1 = "DFB_TEMP";
static char * Pool_Name1_2 = "DFB_TEMP_2";
static char * Pool_Name1_3 = "DFB_TEMP_3";

static char * GO_STRING = "go";
/*
static inline void stop_to_gets(int line_num)
{
    char str[3];

    while(1)
    {
        fgets(str, 3, stdin);
        if(strcmp(GO_STRING, str) == 0)
        {   
            printf("\n\n\ngo! Line num %d\n", line_num);
            fflush(stdout);
            return;
        }
    }
}
*/


void *DFB_callback_id_1(MS_U32 pool_handle_id,MS_U64 start,MS_U64 length)
{
    int fd=dev;
    printf("inside %s ,pool_handle_id=%u\n",__FUNCTION__,pool_handle_id);
}

void *DFB_callback_id_1_free(MS_U32 pool_handle_id,MS_U64 start,MS_U64 length)
{
    int res=0;
    int fd=dev;
    printf("inside %s ,pool_handle_id=%u\n",__FUNCTION__,pool_handle_id);
    res=ipa_free(pool_handle_id,start,length);
    printf("inside %s ,pool_handle_id=%u  start=%llx, length=%llx     res=%d\n",__FUNCTION__,pool_handle_id,start,length,res);
    if (res < 0)
    {
        perror("ipa free failed");
        //pthread_mutex_unlock(&_IPA_POOL_Mutex);
    }    
}

void *DFB_callback_id_2(MS_U32 pool_handle_id,MS_U64 start,MS_U64 length)
{
    int fd=dev;
    printf("inside %s ,pool_handle_id=%u\n",__FUNCTION__,pool_handle_id);
}

void *DFB_callback_id_2_free(MS_U32 pool_handle_id,MS_U64 start,MS_U64 length)
{
    int res=0;
    int fd=dev;
    printf("inside %s ,pool_handle_id=%u\n",__FUNCTION__,pool_handle_id);
    printf("inside %s ,pool_handle_id=%u\n",__FUNCTION__,pool_handle_id);
    res=ipa_free(pool_handle_id,start,length);
    printf("inside %s ,pool_handle_id=%u  start=%llx, length=%llx     res=%d\n",__FUNCTION__,pool_handle_id,start,length,res);
    if (res < 0)
    {
        perror("ipa free failed");
        //pthread_mutex_unlock(&_IPA_POOL_Mutex);
    }    
    
}


void *DFB_callback_id_3(MS_U32 pool_handle_id,MS_U64 start,MS_U64 length)
{
    int fd = dev;
    printf("inside %s ,pool_handle_id=%u\n",__FUNCTION__,pool_handle_id);
}



#if 1
int test_case_1(int argc, char **argv)
{
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 2000;
    pid_t pid = 0;
    int status;
    printf("samson debug in %s \n",__FUNCTION__);
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, PAGE,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param polling_param;
    polling_param.pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param.polling_callback=DFB_callback_id_1;

    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
    MApi_IPA_Pool_Register_Notify(&polling_param);
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);


    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }
	
#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}
#endif

#if 1
int test_case_2(int argc, char **argv)
{

    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 2;
    pid_t pid = 0;
    int status;
    printf("samson debug in main \n");
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, /*(MS_U32)(intptr_t)*/Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, PAGE,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }
#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param polling_param;
    polling_param.pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param.polling_callback=DFB_callback_id_1;
    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify  polling_param.pool_handle_id=%u\n",__FUNCTION__,__LINE__,polling_param.pool_handle_id);
    MApi_IPA_Pool_Register_Notify((struct IPA_Pool_Polling_Param *)(&polling_param));
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

     //在free之前必须delay，保证polling已经在睡眠之后才free。
     printf("samson before sleep\n");
     sleep(5);//wait for pthread that code in MApi_IPA_Pool_Register_Notify to finish.
     printf("samson after sleep\n");
    printf("[%s:%d] before ipa_free\n",__FUNCTION__,__LINE__);	
    ipa_free(pool_handle_id1, 0, PAGE);


    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

#if 1	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(pool_handle_id1);
    printf("samson main after ipa_deinit  err=%d\n",err);
    if(err < 0)
    	return 0;
#endif



 #if 1
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);
    printf("samson main after ipa_close\n");

   // printf("samson before sleep\n");
   // sleep(5);//wait for pthread that code in MApi_IPA_Pool_Register_Notify to finish.
   // printf("samson after sleep\n");
    
#endif
      
    //while(1)
    //{
    //    sleep(1);//just for test
    //}

    return 0;
}
#endif

#if 1
int test_case_3(int argc, char **argv)
{
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 2000;
    pid_t pid = 0;
    int status;
    printf("samson debug in %s \n",__FUNCTION__);
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, PAGE,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param polling_param;
    polling_param.pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param.polling_callback=DFB_callback_id_1;

    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
    MApi_IPA_Pool_Register_Notify(&polling_param);
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

    printf("[%s:%d] before fork pid %d, ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    pid = fork();
    printf("[%s:%d] after fork  pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    if(pid < 0)
    {
        printf("fork fail\n");
        printf("will while 1\n");
        while(1);
        goto PARENT_RUN;
    }
    else if(pid == 0) //child
    {
        printf("about child pid=%d\n",pid);
        //second client conflict with first
        MS_U32 pool_handle_id1_2 = 0;
          
//#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
//不同的pool 就无所谓了.
#if 1
        printf("about child pid=%d before ipa_init\n",pid);
        err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_2, 6.5*MILLION, 8*MILLION, &pool_handle_id1_2);
        printf("about child pid=%d after ipa_init\n",pid);
        if(err < 0)
        {
            printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
            return 0;
        }
#endif
          
        //pool1 allocations
        MS_U64 va1_2 = NULL;
        printf("[%s:%d] will ipa_map  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2);
        err = ipa_map(pool_handle_id1_2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_2);
        if(err < 0)
        {
            printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
            return 0;  
        }

        printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
        timeout_ms = 2000;
        err = ipa_alloc(pool_handle_id1_2, 0, PAGE,timeout_ms);
        printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
        if(err < 0)
        {
            printf("[%s:%d] ipa_alloc FAIL err=%d \n",__FUNCTION__,__LINE__,err);
            printf("will while 1\n");
            while(1);
            return 0;
        }

        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        struct IPA_Pool_Polling_Param polling_param_2;
        polling_param_2.pool_handle_id = pool_handle_id1_2;
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        polling_param_2.polling_callback=DFB_callback_id_2;
        printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        MApi_IPA_Pool_Register_Notify(&polling_param_2);
        printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        printf("will while 1\n");
        while(1);
    }
    else //parent
    {
        printf("inside parent\n");
        waitpid(pid, &status, 0);
        //sleep(5);
        printf("parent wake pid %d ppid %d==========================================\n",
                      getpid(),getppid());
        printf("will while 1\n");
        while(1);

        goto PARENT_RUN;
    }    
    printf("child return pid=%d\n",pid);
    return 0;
          
PARENT_RUN:   

    printf("inside parent_run\n");

          
    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}
#endif

#if 1
int test_case_4(int argc, char **argv)
{
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 2000;
    pid_t pid = 0;
    int status;
    printf("samson debug in main \n");
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, PAGE,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

    err = ipa_alloc(pool_handle_id1, 3*PAGE, PAGE,timeout_ms);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param polling_param;
    polling_param.pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param.polling_callback=DFB_callback_id_1;

    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
    MApi_IPA_Pool_Register_Notify(&polling_param);
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

    printf("[%s:%d] before fork pid %d, ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    pid = fork();
    printf("[%s:%d] after fork  pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    if(pid < 0)
    {
        printf("fork fail\n");
        printf("will while 1\n");
        while(1);
        goto PARENT_RUN;
    }
    else if(pid == 0) //child
    {
        printf("about child pid=%d\n",pid);
        //second client conflict with first
        MS_U32 pool_handle_id1_2 = 0;
          
//#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
//不同的pool 就无所谓了.
#if 1
        printf("about child pid=%d before ipa_init\n",pid);
        err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_2, 6.5*MILLION, 8*MILLION, &pool_handle_id1_2);
        printf("about child pid=%d after ipa_init\n",pid);
        if(err < 0)
        {
            printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
            return 0;
        }
#endif
          
        //pool1 allocations
        MS_U64 va1_2 = NULL;
        printf("[%s:%d] will ipa_map  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2);
        err = ipa_map(pool_handle_id1_2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_2);
        if(err < 0)
        {
            printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
            return 0;  
        }
          
        printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
        timeout_ms = 2000;
        err = ipa_alloc(pool_handle_id1_2, 0, 8*PAGE,timeout_ms);
        printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
        if(err < 0)
        {
            printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
            printf("will while 1\n");
            while(1);
            return 0;
        }
          
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
          
        struct IPA_Pool_Polling_Param polling_param_2;
        polling_param_2.pool_handle_id = pool_handle_id1_2;
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        polling_param_2.polling_callback=DFB_callback_id_2;
          
        printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        MApi_IPA_Pool_Register_Notify(&polling_param_2);
        printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        printf("will while 1\n");
        while(1);
    }
    else //parent
    {
        printf("inside parent\n");
        waitpid(pid, &status, 0);
        //sleep(5);
        printf("parent wake pid %d ppid %d==========================================\n",
                      getpid(),getppid());
        printf("will while 1\n");
        while(1);
        goto PARENT_RUN;
    }
    printf("child return pid=%d\n",pid);
    return 0;
          
PARENT_RUN:   

    printf("inside parent_run\n");


#if 0//another cleint init and alloc.
            printf("2[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);
            MS_U32 pool_handle_id2 = 0;
            err = ipa_init(dev, DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id2);
            if(err < 0)
                {
                printf("2[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
                return 0;
                }
        
        
            //pool1 allocations
         MS_U64 va2 = NULL;
            printf("2[%s:%d] will ipa_map\n",__FUNCTION__,__LINE__);
        err = ipa_map(dev, pool_handle_id2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va2);
            if(err < 0)
                {
                printf("2[%s:%d] err,return\n",__FUNCTION__,__LINE__);
                return 0;  
                }
        
         printf("2[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
         timeout = 3;
            err = ipa_alloc(dev, pool_handle_id2, 0, PAGE,timeout);
            printf("2[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
            if(err < 0)
                {
                printf("2[%s:%d] err return\n",__FUNCTION__,__LINE__);
                return 0;
                }
#endif



          
    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}
#endif

#if 1
int test_case_5(int argc, char **argv)
{
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 2000;
    pid_t pid = 0;
    int status;
    printf("samson debug in main \n");
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, PAGE,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

    err = ipa_alloc(pool_handle_id1, 3*PAGE, PAGE,timeout_ms);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param polling_param;
    polling_param.pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param.polling_callback=DFB_callback_id_1;

    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
    MApi_IPA_Pool_Register_Notify(&polling_param);
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

    printf("[%s:%d] before fork pid %d, ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    pid = fork();
    printf("[%s:%d] after fork  pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    if(pid < 0)
    {
        printf("fork fail\n");
        printf("%s:%d    will while 1",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }
    else if(pid == 0) //child
    {
        printf("about child pid=%d\n",pid);
        //second client conflict with first
        MS_U32 pool_handle_id1_2 = 0;
          
//#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
//不同的pool 就无所谓了.
#if 1
        printf("about child pid=%d before ipa_init\n",pid);
        err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_2, 6.5*MILLION, 8*MILLION, &pool_handle_id1_2);
        printf("about child pid=%d after ipa_init\n",pid);
        if(err < 0)
        {
            printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
            return 0;
        }
#endif
          
        //pool1 allocations
        MS_U64 va1_2 = NULL;
        printf("[%s:%d] will ipa_map  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2);
        err = ipa_map(pool_handle_id1_2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_2);
        if(err < 0)
        {
            printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
            return 0;  
        }

        printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
        timeout_ms = 2000;
        err = ipa_alloc(pool_handle_id1_2, 8*PAGE, 8*PAGE,timeout_ms);
        printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
        if(err < 0)
        {
            printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
            return 0;
        }

        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        struct IPA_Pool_Polling_Param polling_param_2;
        polling_param_2.pool_handle_id = pool_handle_id1_2;
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        polling_param_2.polling_callback=DFB_callback_id_2;
          
        printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        MApi_IPA_Pool_Register_Notify(&polling_param_2);
        printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        printf("%s:%d    will while 1",__FUNCTION__,__LINE__);
        while(1);

    }
    else //parent
    {        
        printf("inside parent\n");
        waitpid(pid, &status, 0);
        //sleep(5);
        printf("parent wake pid %d ppid %d==========================================\n",
                      getpid(),getppid());
        printf("%s:%d    will while 1",__FUNCTION__,__LINE__);
        while(1);            
        goto PARENT_RUN;
    }
    printf("child return pid=%d\n",pid);
    return 0;
          
PARENT_RUN:   

    printf("inside parent_run\n");

    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}
#endif

#if 1
int test_case_6(int argc, char **argv)
{
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 1000;
    pid_t pid = 0;
    int status;
    printf("samson debug in %s \n",__FUNCTION__);
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 ;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, PAGE,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

    err = ipa_alloc(pool_handle_id1, 3*PAGE, PAGE,timeout_ms);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

#endif
#if 1
//after map and alloc,test write data and read data.
        MS_U64 temp;
        MS_U64 temp_r;
    
        temp_r = temp = va1;
        int tmp_i=0;
        int w_d[2]={0x12345678,0x46267};
        int  read_data;
        printf("will write data:0x%x   0x%x ",w_d[0],w_d[1]);
        //*((MS_U32 *)temp) = 0x12345678;
      
    
        *((MS_U32 *)temp) = w_d[0];
        temp += 3*PAGE; 
        *((MS_U32 *)temp) = w_d[1];
    
    
        printf("read data from pool :");
        read_data = *((MS_U32 *)temp_r);
        printf("0x%x  ", read_data);
        temp_r += 3*PAGE; 
        read_data = *((MS_U32 *)temp_r);
        printf("0x%x", read_data);    
        printf("\n");
    
#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param polling_param;
    //polling_param = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
    polling_param.pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param.polling_callback=DFB_callback_id_1;

    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
    MApi_IPA_Pool_Register_Notify(&polling_param);
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

    printf("[%s:%d] before fork pid %d, ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    pid = fork();
    printf("[%s:%d] after fork  pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    if(pid < 0)
    {
        printf("fork fail\n");
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }
    else if(pid == 0) //child
    {
        printf("about child pid=%d\n",pid);
        //second client conflict with first
        MS_U32 pool_handle_id1_2 = 0;
          
//#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
//不同的pool 就无所谓了.
#if 1
        printf("about child pid=%d before ipa_init\n",pid);
        err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_2, 6.5*MILLION, 8*MILLION, &pool_handle_id1_2);
        printf("about child pid=%d after ipa_init\n",pid);
        if(err < 0)
        {
            printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
            return 0;
        }
#endif
          
        //pool1 allocations
        MS_U64 va1_2 ;
        printf("[%s:%d] will ipa_map  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2);
        err = ipa_map(pool_handle_id1_2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_2);
        if(err < 0)
        {
            printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
            return 0;  
        }
          
        printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
        timeout_ms = 1000;
        err = ipa_alloc(pool_handle_id1_2, 8*PAGE, 8*PAGE,timeout_ms);
        printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
        if(err < 0)
        {
            printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
            return 0;
        }
         
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
          
        struct IPA_Pool_Polling_Param polling_param_2;
        polling_param_2.pool_handle_id = pool_handle_id1_2;
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        polling_param_2.polling_callback=DFB_callback_id_2;

        printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        MApi_IPA_Pool_Register_Notify(&polling_param_2);
        printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

        pid_t pid2 = 0;
        pid2 = fork();
        printf("[%s:%d] after fork  SSSS pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
        if(pid2 < 0)
        {
            printf("fork fail\n");
            printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
            while(1);
            goto CHIND_AS_PARENT_RUN;
        }
        else if(pid2 == 0) //child
        {  
            MS_U32 pool_handle_id1_3 = 0;
                                  
                        //#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
                        //不同的pool 就无所谓了.
#if 1
            printf("about child pid2=%d before ipa_init\n",pid2);
            err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_3, 6.5*MILLION, 8*MILLION, &pool_handle_id1_3);
            printf("about child pid2=%d after ipa_init\n",pid2);
            if(err < 0)
            {
                printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
                return 0;
            }
#endif
                                  
            //pool1 allocations
            MS_U64 va1_3 ;
            printf("[%s:%d] will ipa_map  pool_handle_id1_3=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_3);
            err = ipa_map(pool_handle_id1_3, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_3);
            if(err < 0)
            {
                printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
                return 0;  
            }
            printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
            timeout_ms = 1000;
            err = ipa_alloc(pool_handle_id1_3, 0, 10*PAGE,timeout_ms);
            printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
            if(err < 0)
            {
                printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
                //goto CHIND_AS_PARENT_TO_PP;
				
				
				//test from PA get info.
				/*
				//use messi 091B chip to test 
				shell@watermelon:/data # cat /proc/cmdline                                     
console=ttyS0,115200 androidboot.console=ttyS0 root=/dev/ram rw rootwait init=/init 
CORE_DUMP_PATH=/data/core_dump.%%p.gz KDebug=1 delaylogo=true androidboot.selinux=permissive swiotlb=512
 CMA6=heap00,miu=1,hid=28,sz=0x1400000,st=0xA800000 LX_MEM=0xEB00000 LX_MEM2=0xA0400000,0x1FC00000 
 EMAC_MEM=0x40000 PM51_ADDR=0x20010000 PM51_LEN=0x10000 CMA0=OTHERS,
 miu=0,hid=23,sz=0x1c00000,st=0x6c00000 CMA1=VDEC_XC_STR_MBOOT,miu=0,hid=20,sz=0x3000000,st=0x8800000 CMA2=RETURN_ENUM0,miu=0,hid=26,sz=0x2400000,st=0xb800000
 CMA3=RETURN_ENUM2,miu=1,hid=27,sz=0x1400000,st=0x400000 CMA4=VDEC1,miu=1,hid=19,sz=0x7000000,st=0x1800000 CMA5=RETURN_ENUM,miu=1,hid=25,sz=0x2000000,st=0x8800000
 OOTLOGO_IN_MBOOT ENV_VAR_OFFSET=0x0 ENV_VAR_SIZE=0x10000 ENV=EMMC SECURITY=ON BOOTTIME_SBOOT=367 BOOTTIME_UBOOT=1544

 shell@watermelon:/data # 
shell@watermelon:/data # cat /sys/kernel/debug/IPA_Heap/heap-28                
heapid 28 name heap00 information---------------------------------------------
miu 1  miu_offset 0xa800000  heap_len 0x1400000  heap_type 1

dump pool name DFB_TEMP_3 id 6: memory allocation range-------------------------


dump pool name DFB_TEMP_2 id 5: memory allocation range-------------------------
alloc[0x688000 0x8000] 

dump pool name DFB_TEMP id 4: memory allocation range-------------------------
alloc[0x680000 0x1000] alloc[0x683000 0x1000] 

dump heap 28 hal tree memory allocation range----------------------------------
alloc[0x680000 0x4000] alloc[0x688000 0x8000] 

shell@watermelon:/data # 


in kernel code mdrv_system.h:
#define ARM_MIU0_BUS_BASE                      0x20000000UL
#define ARM_MIU1_BUS_BASE                      0xA0000000UL
#define ARM_MIU2_BUS_BASE                      0xFFFFFFFFUL



MIU_base + heap_offset_in_miu  +  (heap_pos)
0xA0000000 + 0xA800000 +  (0x680000  + 0x100) =  0xAAE80100   allocated by DFB_TEMP      offset_in_pool 0x100
0xA0000000 + 0xA800000 +  (0x688000  + 0x4000)=  0xAAE8C000   allocated by DFB_TEMP_2    offset_in_pool 0x8000+0x4000=0xC000
0xA0000000 + 0xA800000 +  (0x688000  + 0x9000)=  0xAA891000   in heap,not allocated
0xA0000000 + 0xA800000 +  (0x1400000  +0x100)=   0xABC00100   not in heap
				*/
    		  MS_U64 PA=0xAAE80100;
    		  MS_BOOL bool_info_ret;
    		  struct PA_In_IPA_Pool_Param in_ipa_pool_info;
    		  in_ipa_pool_info.PA = PA;
    		  bool_info_ret=PA_in_IPA_POOL_info(&in_ipa_pool_info);
        	  printf("PA=0x%lx   bool_info_ret =%s   \n",(unsigned long)PA,(bool_info_ret==true)?"true":"false");
    		  if(in_ipa_pool_info.miu > 0)
    		  {
    		      printf("in_ipa_pool_info.miu =%d\n",in_ipa_pool_info.miu);
    		      if(true == in_ipa_pool_info.in_heap)
                    {
                        printf("in heap\n");
                        printf("space_id=%d,space_miu_start_offset=0x%lx,space_length=0x%lx\n",in_ipa_pool_info.space_id,(unsigned long)in_ipa_pool_info.space_miu_start_offset,(unsigned long)in_ipa_pool_info.space_length);
                        if(true == in_ipa_pool_info.allocated)
                        {
                            printf("allocated by following pool:\n");
                            printf("pool_len=0x%lx   pool_offset_in_heap=0x%lx,  pa_offset_in_pool=0x%lx\n",(unsigned long)in_ipa_pool_info.pool_len,(unsigned long)in_ipa_pool_info.pool_offset_in_heap,(unsigned long)in_ipa_pool_info.pa_offset_in_pool);
                            printf("pool_name %s\n",(char *)in_ipa_pool_info.pool_name);
                        }
                        else
                        {
                            printf("not allocated\n");
                        }
                    }
                    else
                    {
                        printf("not in heap\n");
                    }
    		  }
                else
                {
                    printf("not in miu!\n");
                }


                //////////////////////
                /*
                id 4,offset 0x100    == >PA:  0xAAE80100
                
                id 5,offset 0x4000   == >PA:  0xAAE8C000
                
                */
                //////////////////////
                                MS_BOOL pool_to_pa_ret;
                                struct Pool_To_PA_Param pool_to_pa_param;
                                pool_to_pa_param.pool_handle_id =4;
                                pool_to_pa_param.offset_in_pool = 0x100;
                                printf("pool_handle_id=%x , offset_in_pool=0x%lx   to get PA:\n",pool_to_pa_param.pool_handle_id,pool_to_pa_param.offset_in_pool);
                                pool_to_pa_ret = IN_IPA_POOL_To_PA(&pool_to_pa_param);
                                if(false == pool_to_pa_ret || pool_to_pa_param.error_code < 0)
                                    {
                                    printf("IN_IPA_POOL_To_PA fail\n");
                                    }
                                else
                                    {
                                    printf("result PA=0x%lx     miu=0x%x  heap_id=0x%x\n",(unsigned long)pool_to_pa_param.PA,pool_to_pa_param.miu,pool_to_pa_param.heap_id);
                                    }
                                pool_to_pa_param.pool_handle_id =5;
                                pool_to_pa_param.offset_in_pool = 0xC000;
                                printf("pool_handle_id=%x , offset_in_pool=0x%lx   to get PA:\n",pool_to_pa_param.pool_handle_id,pool_to_pa_param.offset_in_pool);                
                                pool_to_pa_ret = IN_IPA_POOL_To_PA(&pool_to_pa_param);
                                if(false == pool_to_pa_ret || pool_to_pa_param.error_code < 0)
                                    {
                                    printf("IN_IPA_POOL_To_PA fail\n");
                                    }
                                else
                                    {
                                    printf("result PA=0x%lx     miu=0x%x  heap_id=0x%x\n",(unsigned long)pool_to_pa_param.PA,pool_to_pa_param.miu,pool_to_pa_param.heap_id);
                                    }



                
                printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
                while(1);
            }

            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
            struct IPA_Pool_Polling_Param polling_param_3;
            //polling_param_3 = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
            polling_param_3.pool_handle_id = pool_handle_id1_3;
            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
            polling_param_3.polling_callback=DFB_callback_id_3;

            printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
            MApi_IPA_Pool_Register_Notify(&polling_param_3);  
                      
        }
        else
        {
            printf("inside child as parent\n");
            waitpid(pid2, &status, 0);
            //sleep(5);
            printf("parent wake pid %d ppid %d==========================================\n",
                                  getpid(),getppid());
            printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
            while(1);
            goto CHIND_AS_PARENT_RUN;
        }
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        return 0;
CHIND_AS_PARENT_RUN:  
        printf("inside CHIND_AS_PARENT_RUN\n");
    }
    else //parent
    {
        printf("inside parent\n");
        waitpid(pid, &status, 0);
        // sleep(5);
        printf("parent wake pid %d ppid %d==========================================\n",
                      getpid(),getppid());
        
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }    

//CHIND_AS_PARENT_TO_PP:

            
    printf("child return pid=%d\n",pid);
    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);

    return 0;
          
PARENT_RUN:   

    printf("inside parent_run\n");
    // free(polling_param);

    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

	
	
	
#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}
#endif

#if 1
int test_case_7(int argc, char **argv)
{
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 5000;
    pid_t pid = 0;
    int status;
    printf("samson debug in %s \n",__FUNCTION__);
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, PAGE,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

    err = ipa_alloc(pool_handle_id1, 3*PAGE, PAGE,timeout_ms);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param polling_param;
    polling_param.pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param.polling_callback=DFB_callback_id_1_free;

    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
    MApi_IPA_Pool_Register_Notify(&polling_param);
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

    printf("[%s:%d] before fork pid %d, ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    pid = fork();
    printf("[%s:%d] after fork  pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    if(pid < 0)
    {
        printf("fork fail\n");
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }
    else if(pid == 0) //child
    {
#if 1
        printf("about child pid=%d\n",pid);
        //second client conflict with first
        MS_U32 pool_handle_id1_2 = 0;
          
//#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
//不同的pool 就无所谓了.
#if 1
        printf("about child pid=%d before ipa_init\n",pid);
        err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_2, 6.5*MILLION, 8*MILLION, &pool_handle_id1_2);
        printf("about child pid=%d after ipa_init\n",pid);
        if(err < 0)
        {
            printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
            return 0;
        }
#endif
          
        //pool1 allocations
        MS_U64 va1_2 = NULL;
        printf("[%s:%d] will ipa_map  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2);
        err = ipa_map(pool_handle_id1_2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_2);
        if(err < 0)
        {
            printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
            return 0;  
        }

        printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
        timeout_ms = 5000;
        err = ipa_alloc(pool_handle_id1_2, 8*PAGE, 8*PAGE,timeout_ms);
        printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
        if(err < 0)
        {
            printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
            return 0;
        }

        printf("[%s:%d] \n",__FUNCTION__,__LINE__);

        struct IPA_Pool_Polling_Param polling_param_2;
        polling_param_2.pool_handle_id = pool_handle_id1_2;
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        polling_param_2.polling_callback=DFB_callback_id_2_free;
        printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        MApi_IPA_Pool_Register_Notify(&polling_param_2);
        printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
                  
#endif

        pid_t pid2 = 0;
        pid2 = fork();
        printf("[%s:%d] after fork  SSSS pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
        if(pid2 < 0)
        {
             printf("fork fail\n");
             printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
             while(1);
             goto CHIND_AS_PARENT_RUN;
        }
        else if(pid2 == 0) //child
        {
            MS_U32 pool_handle_id1_3 = 0;
                                  
            //#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
            //不同的pool 就无所谓了.
#if 1
            printf("about child pid2=%d before ipa_init\n",pid2);
            err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_3, 6.5*MILLION, 8*MILLION, &pool_handle_id1_3);
            printf("about child pid2=%d after ipa_init\n",pid2);
            if(err < 0)
            {
                printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
                return 0;
            }
#endif
                                  
            //pool1 allocations
            MS_U64 va1_3 = NULL;
            printf("[%s:%d] will ipa_map  pool_handle_id1_3=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_3);
            err = ipa_map(pool_handle_id1_3, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_3);
            if(err < 0)
            {
                printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
                return 0;  
            }

            printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
            timeout_ms = 5000;
            err = ipa_alloc(pool_handle_id1_3, 0, 10*PAGE,timeout_ms);
            printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
            if(err < 0)
            {
                printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
                //goto CHIND_AS_PARENT_TO_PP;
                                          
                //printf("[%s:%d] ipa_alloc    tryyyyyy again\n",__FUNCTION__,__LINE__);
                // err = ipa_alloc(dev, pool_handle_id1_3, 0, 10*PAGE,timeout_ms);
                // if(err < 0)
                {
                //      printf("[%s:%d] ipa_alloc    tryyyyyy again  failllll\n",__FUNCTION__,__LINE__);
                printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
                while(1);
                }
            }
                                 
            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
                                  
            struct IPA_Pool_Polling_Param polling_param_3;
            polling_param_3.pool_handle_id = pool_handle_id1_3;
            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
            polling_param_3.polling_callback=DFB_callback_id_3;
            printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
            MApi_IPA_Pool_Register_Notify(&polling_param_3);
        }
        else
        {
            printf("inside child as parent\n");
            waitpid(pid2, &status, 0);
            //sleep(5);
            printf("parent wake pid %d ppid %d==========================================\n",
                                  getpid(),getppid());
            printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
            while(1);
            goto CHIND_AS_PARENT_RUN;
        }
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        return 0;
CHIND_AS_PARENT_RUN:
    printf("inside CHIND_AS_PARENT_RUN\n");


    }
    else //parent
    {
        printf("inside parent\n");
        waitpid(pid, &status, 0);
        // sleep(5);
        printf("parent wake pid %d ppid %d==========================================\n",
                      getpid(),getppid());
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }    

//CHIND_AS_PARENT_TO_PP:

            
    printf("child return pid=%d\n",pid);
    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);

    return 0;
          
PARENT_RUN:   

    printf("inside parent_run\n");
    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

	
#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}
#endif


#if 1
int test_case_8(int argc, char **argv)
{
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 5000;
    pid_t pid = 0;
    pid_t pid2 = 0;
    int status;
    printf("samson debug in %s \n",__FUNCTION__);
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, PAGE,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

    err = ipa_alloc(pool_handle_id1, 3*PAGE, PAGE,timeout_ms);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param polling_param;
    polling_param.pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param.polling_callback=DFB_callback_id_1_free;

    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
    MApi_IPA_Pool_Register_Notify(&polling_param);
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

    printf("[%s:%d] before fork pid %d, ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    pid = fork();
    printf("[%s:%d] after fork  pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    if(pid < 0)
    {
        printf("fork fail\n");
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }
    else if(pid == 0) //child
    {
#if 1
        printf("about child pid=%d\n",pid);
        //second client conflict with first
        MS_U32 pool_handle_id1_2 = 0;
          
//#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
//不同的pool 就无所谓了.
#if 1
        printf("about child pid=%d before ipa_init\n",pid);
        err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_2, 6.5*MILLION, 8*MILLION, &pool_handle_id1_2);
        printf("about child pid=%d after ipa_init\n",pid);
        if(err < 0)
        {
            printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
            return 0;
        }
#endif
          
        //pool1 allocations
        MS_U64 va1_2 = NULL;
        printf("[%s:%d] will ipa_map  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2);
        err = ipa_map(pool_handle_id1_2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_2);
        if(err < 0)
        {
            printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
            return 0;  
        }

        printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
        timeout_ms = 5000;
        err = ipa_alloc(pool_handle_id1_2, 0, PAGE,timeout_ms);
        printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
        if(err < 0)
        {
            printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
            printf("2 conflict will for 3\n");
            goto  SECOND_FAIL_TO_THIRD;
            return 0;
        }

        printf("[%s:%d] \n",__FUNCTION__,__LINE__);

        struct IPA_Pool_Polling_Param polling_param_2;
        polling_param_2.pool_handle_id = pool_handle_id1_2;
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        polling_param_2.polling_callback=DFB_callback_id_2_free;
        printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        MApi_IPA_Pool_Register_Notify(&polling_param_2);
        printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
                  
#endif

SECOND_FAIL_TO_THIRD:

        
        pid2 = fork();
        printf("[%s:%d] after fork  SSSS pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
        if(pid2 < 0)
        {
             printf("fork fail\n");
             printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
             while(1);
             goto CHIND_AS_PARENT_RUN;
        }
        else if(pid2 == 0) //child
        {
            MS_U32 pool_handle_id1_3 = 0;
                                  
            //#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
            //不同的pool 就无所谓了.
#if 1
            printf("about child pid2=%d before ipa_init\n",pid2);
            err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_3, 6.5*MILLION, 8*MILLION, &pool_handle_id1_3);
            printf("about child pid2=%d after ipa_init\n",pid2);
            if(err < 0)
            {
                printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
                return 0;
            }
#endif
                                  
            //pool1 allocations
            MS_U64 va1_3 = NULL;
            printf("[%s:%d] will ipa_map  pool_handle_id1_3=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_3);
            err = ipa_map(pool_handle_id1_3, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_3);
            if(err < 0)
            {
                printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
                return 0;  
            }

            printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
            timeout_ms = 5000;
            err = ipa_alloc(pool_handle_id1_3, 3*PAGE, PAGE,timeout_ms);
            printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
            if(err < 0)
            {
                printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
                //goto CHIND_AS_PARENT_TO_PP;
                                          
                //printf("[%s:%d] ipa_alloc    tryyyyyy again\n",__FUNCTION__,__LINE__);
                // err = ipa_alloc(dev, pool_handle_id1_3, 0, 10*PAGE,timeout_ms);
                // if(err < 0)
                {
                //      printf("[%s:%d] ipa_alloc    tryyyyyy again  failllll\n",__FUNCTION__,__LINE__);
                printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
                while(1);
                }
            }
                                 
            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
                                  
            struct IPA_Pool_Polling_Param polling_param_3;
            polling_param_3.pool_handle_id = pool_handle_id1_3;
            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
            polling_param_3.polling_callback=DFB_callback_id_3;
            printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
            MApi_IPA_Pool_Register_Notify(&polling_param_3);
        }
        else
        {
            printf("inside child as parent\n");
            waitpid(pid2, &status, 0);
            //sleep(5);
            printf("parent wake pid %d ppid %d==========================================\n",
                                  getpid(),getppid());
            printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
            while(1);
            goto CHIND_AS_PARENT_RUN;
        }
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        return 0;
CHIND_AS_PARENT_RUN:
    printf("inside CHIND_AS_PARENT_RUN\n");


    }
    else //parent
    {
        printf("inside parent\n");
        waitpid(pid, &status, 0);
        // sleep(5);
        printf("parent wake pid %d ppid %d==========================================\n",
                      getpid(),getppid());
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }    

//CHIND_AS_PARENT_TO_PP:

            
    printf("child return pid=%d\n",pid);
    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);

    return 0;
          
PARENT_RUN:   

    printf("inside parent_run\n");
    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

	
#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}
#endif

//test_case_9和test_case_8基本相同，差别在于
//MApi_IPA_Pool_Register_Notify的参数是直接用指针，MApi_IPA_Pool_Register_Notify之后都是立刻free
int test_case_9(int argc, char **argv)
{
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 10;
    pid_t pid = 0;
    pid_t pid2 = 0;
    int status;
    printf("samson debug in %s \n",__FUNCTION__);
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, PAGE,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

    err = ipa_alloc(pool_handle_id1, 3*PAGE, PAGE,timeout_ms);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param *polling_param = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
    polling_param->pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param->polling_callback=DFB_callback_id_1_free;

    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
    MApi_IPA_Pool_Register_Notify(polling_param);
    free(polling_param);
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

    printf("[%s:%d] before fork pid %d, ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    pid = fork();
    printf("[%s:%d] after fork  pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    if(pid < 0)
    {
        printf("fork fail\n");
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }
    else if(pid == 0) //child
    {
#if 1
        printf("about child pid=%d\n",pid);
        //second client conflict with first
        MS_U32 pool_handle_id1_2 = 0;
          
//#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
//不同的pool 就无所谓了.
#if 1
        printf("about child pid=%d before ipa_init\n",pid);
        err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_2, 6.5*MILLION, 8*MILLION, &pool_handle_id1_2);
        printf("about child pid=%d after ipa_init\n",pid);
        if(err < 0)
        {
            printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
            return 0;
        }
#endif
          
        //pool1 allocations
        MS_U64 va1_2 = NULL;
        printf("[%s:%d] will ipa_map  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2);
        err = ipa_map(pool_handle_id1_2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_2);
        if(err < 0)
        {
            printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
            return 0;  
        }

        printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
        timeout_ms = 10;
        err = ipa_alloc(pool_handle_id1_2, 0, PAGE,timeout_ms);
        printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
        if(err < 0)
        {
            printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
            printf("2 conflict will for 3\n");
            goto  SECOND_FAIL_TO_THIRD;
            return 0;
        }

        printf("[%s:%d] \n",__FUNCTION__,__LINE__);

        struct IPA_Pool_Polling_Param *polling_param_2 = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
        polling_param_2->pool_handle_id = pool_handle_id1_2;
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        polling_param_2->polling_callback=DFB_callback_id_2_free;
        printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        MApi_IPA_Pool_Register_Notify(polling_param_2);
        free(polling_param_2);
        printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
                  
#endif

SECOND_FAIL_TO_THIRD:

        
        pid2 = fork();
        printf("[%s:%d] after fork  SSSS pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
        if(pid2 < 0)
        {
             printf("fork fail\n");
             printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
             while(1);
             goto CHIND_AS_PARENT_RUN;
        }
        else if(pid2 == 0) //child
        {
            MS_U32 pool_handle_id1_3 = 0;
                                  
            //#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
            //不同的pool 就无所谓了.
#if 1
            printf("about child pid2=%d before ipa_init\n",pid2);
            err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_3, 6.5*MILLION, 8*MILLION, &pool_handle_id1_3);
            printf("about child pid2=%d after ipa_init\n",pid2);
            if(err < 0)
            {
                printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
                return 0;
            }
#endif
                                  
            //pool1 allocations
            MS_U64 va1_3 = NULL;
            printf("[%s:%d] will ipa_map  pool_handle_id1_3=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_3);
            err = ipa_map(pool_handle_id1_3, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_3);
            if(err < 0)
            {
                printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
                return 0;  
            }

            printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
            timeout_ms = 10;
            err = ipa_alloc(pool_handle_id1_3, 3*PAGE, PAGE,timeout_ms);
            printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
            if(err < 0)
            {
                printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
                //goto CHIND_AS_PARENT_TO_PP;
                                          
                //printf("[%s:%d] ipa_alloc    tryyyyyy again\n",__FUNCTION__,__LINE__);
                // err = ipa_alloc(dev, pool_handle_id1_3, 0, 10*PAGE,timeout_ms);
                // if(err < 0)
                {
                //      printf("[%s:%d] ipa_alloc    tryyyyyy again  failllll\n",__FUNCTION__,__LINE__);
                printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
                while(1);
                }
            }
                                 
            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
                                  
            struct IPA_Pool_Polling_Param *polling_param_3 = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
            polling_param_3->pool_handle_id = pool_handle_id1_3;
            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
            polling_param_3->polling_callback=DFB_callback_id_3;
            printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
            MApi_IPA_Pool_Register_Notify(polling_param_3);
            free(polling_param_3);
        }
        else
        {
            printf("inside child as parent\n");
            waitpid(pid2, &status, 0);
            //sleep(5);
            printf("parent wake pid %d ppid %d==========================================\n",
                                  getpid(),getppid());
            printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
            while(1);
            goto CHIND_AS_PARENT_RUN;
        }
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        return 0;
CHIND_AS_PARENT_RUN:
    printf("inside CHIND_AS_PARENT_RUN\n");


    }
    else //parent
    {
        printf("inside parent\n");
        waitpid(pid, &status, 0);
        // sleep(5);
        printf("parent wake pid %d ppid %d==========================================\n",
                      getpid(),getppid());
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }    

//CHIND_AS_PARENT_TO_PP:

            
    printf("child return pid=%d\n",pid);
    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);

    return 0;
          
PARENT_RUN:   

    printf("inside parent_run\n");
    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

	
#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}

int test_case_10(int argc, char **argv)
{
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout_ms = 10;
    pid_t pid = 0;
    pid_t pid2 = 0;
    int status;
    printf("samson debug in %s \n",__FUNCTION__);
    memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1, 0, 20*MILLION, &pool_handle_id1);
    if(err < 0)
    {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
    }


    //pool1 allocations
    MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
    err = ipa_map(pool_handle_id1, 0, 20*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
    {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
    }

    printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(pool_handle_id1, 0, 15*MILLION,timeout_ms);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

    err = ipa_alloc(pool_handle_id1, 18*MILLION, 2*MILLION,timeout_ms);
    if(err < 0)
    {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
    }

#endif

    printf("[%s:%d] \n",__FUNCTION__,__LINE__);

    struct IPA_Pool_Polling_Param *polling_param = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
    polling_param->pool_handle_id = pool_handle_id1;
    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
    polling_param->polling_callback=DFB_callback_id_1_free;

    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
    MApi_IPA_Pool_Register_Notify(polling_param);
    free(polling_param);
    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

    printf("[%s:%d] before fork pid %d, ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    pid = fork();
    printf("[%s:%d] after fork  pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
    if(pid < 0)
    {
        printf("fork fail\n");
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }
    else if(pid == 0) //child
    {
#if 1
        printf("about child pid=%d\n",pid);
        //second client conflict with first
        MS_U32 pool_handle_id1_2 = 0;
          
//#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
//不同的pool 就无所谓了.
#if 1
        printf("about child pid=%d before ipa_init\n",pid);
        err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_2, 0, 20*MILLION, &pool_handle_id1_2);
        printf("about child pid=%d after ipa_init\n",pid);
        if(err < 0)
        {
            printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
            return 0;
        }
#endif
          
        //pool1 allocations
        MS_U64 va1_2 = NULL;
        printf("[%s:%d] will ipa_map  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2);
        err = ipa_map(pool_handle_id1_2, 0, 20*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_2);
        if(err < 0)
        {
            printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
            return 0;  
        }

        printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
        timeout_ms = 10;
        err = ipa_alloc(pool_handle_id1_2, 0, 15*MILLION,timeout_ms);
        printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
        if(err < 0)
        {
            printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
            printf("2 conflict will for 3\n");
            goto  SECOND_FAIL_TO_THIRD;
            return 0;
        }

        printf("[%s:%d] \n",__FUNCTION__,__LINE__);

        struct IPA_Pool_Polling_Param *polling_param_2 = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
        polling_param_2->pool_handle_id = pool_handle_id1_2;
        printf("[%s:%d] \n",__FUNCTION__,__LINE__);
        polling_param_2->polling_callback=DFB_callback_id_2_free;
        printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
        MApi_IPA_Pool_Register_Notify(polling_param_2);
        free(polling_param_2);
        printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
                  
#endif

SECOND_FAIL_TO_THIRD:

        
        pid2 = fork();
        printf("[%s:%d] after fork  SSSS pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
        if(pid2 < 0)
        {
             printf("fork fail\n");
             printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
             while(1);
             goto CHIND_AS_PARENT_RUN;
        }
        else if(pid2 == 0) //child
        {
            MS_U32 pool_handle_id1_3 = 0;
                                  
            //#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
            //不同的pool 就无所谓了.
#if 1
            printf("about child pid2=%d before ipa_init\n",pid2);
            err = ipa_init(DFB_TEMP_HID, (MS_U32)(intptr_t)Pool_Name1_3, 0, 20*MILLION, &pool_handle_id1_3);
            printf("about child pid2=%d after ipa_init\n",pid2);
            if(err < 0)
            {
                printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
                return 0;
            }
#endif
                                  
            //pool1 allocations
            MS_U64 va1_3 = NULL;
            printf("[%s:%d] will ipa_map  pool_handle_id1_3=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_3);
            err = ipa_map(pool_handle_id1_3, 0, 20*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_3);
            if(err < 0)
            {
                printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
                return 0;  
            }

            printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
            timeout_ms = 10;
            err = ipa_alloc(pool_handle_id1_3, 18*MILLION, 2*MILLION,timeout_ms);
            printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
            if(err < 0)
            {
                printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
                //goto CHIND_AS_PARENT_TO_PP;
                                          
                //printf("[%s:%d] ipa_alloc    tryyyyyy again\n",__FUNCTION__,__LINE__);
                // err = ipa_alloc(dev, pool_handle_id1_3, 0, 10*PAGE,timeout_ms);
                // if(err < 0)
                {
                //      printf("[%s:%d] ipa_alloc    tryyyyyy again  failllll\n",__FUNCTION__,__LINE__);
                printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
                while(1);
                }
            }
                                 
            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
                                  
            struct IPA_Pool_Polling_Param *polling_param_3 = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
            polling_param_3->pool_handle_id = pool_handle_id1_3;
            printf("[%s:%d] \n",__FUNCTION__,__LINE__);
            polling_param_3->polling_callback=DFB_callback_id_3;
            printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
            MApi_IPA_Pool_Register_Notify(polling_param_3);
            free(polling_param_3);
        }
        else
        {
            printf("inside child as parent\n");
            waitpid(pid2, &status, 0);
            //sleep(5);
            printf("parent wake pid %d ppid %d==========================================\n",
                                  getpid(),getppid());
            printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
            while(1);
            goto CHIND_AS_PARENT_RUN;
        }
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        return 0;
CHIND_AS_PARENT_RUN:
    printf("inside CHIND_AS_PARENT_RUN\n");


    }
    else //parent
    {
        printf("inside parent\n");
        waitpid(pid, &status, 0);
        // sleep(5);
        printf("parent wake pid %d ppid %d==========================================\n",
                      getpid(),getppid());
        printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
        while(1);
        goto PARENT_RUN;
    }    

//CHIND_AS_PARENT_TO_PP:

            
    printf("child return pid=%d\n",pid);
    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);

    return 0;
          
PARENT_RUN:   

    printf("inside parent_run\n");
    printf("%s:%d will while 1\n",__FUNCTION__,__LINE__);
    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

	
#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}


#if 0
struct pthread_info
{
    int dev; 
};

void *create_thread3(void  *argc)
{
    MS_BOOL ret = TRUE;
    int res;
    MS_U32 timeout = 2;
    int err=0;
    struct IPA_Pool_Event_Args pool_event_args;
    struct pthread_info *thread3_info = (struct IPA_Pool_Polling_Param *)argc;
    
    
                                      MS_U32 pool_handle_id1_3 = 0;
                                      printf("%s   \n",__FUNCTION__);
                                      
                            //#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
                            //不同的pool 就无所谓了.
#if 1
                                      printf("about child before ipa_init\n");
                                          err = ipa_init(thread3_info->dev, DFB_TEMP_HID, (MS_U32)Pool_Name1_3, 6.5*MILLION, 8*MILLION, &pool_handle_id1_3);
                                          printf("about child  after ipa_init\n");
                                          if(err < 0)
                                              {
                                              printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
                                              return 0;
                                              }
          #endif
                                      
                                          //pool1 allocations
                                       MS_U64 va1_3 = NULL;
                                          printf("[%s:%d] will ipa_map  pool_handle_id1_3=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_3);
                                      err = ipa_map(thread3_info->dev, pool_handle_id1_3, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_3);
                                          if(err < 0)
                                              {
                                              printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
                                              return 0;  
                                              }
                                      
                                       printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
                                       timeout = 2;
                                          err = ipa_alloc(thread3_info->dev, pool_handle_id1_3, 0, 10*PAGE,timeout);
                                          printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
                                          if(err < 0)
                                              {
                                              printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
                                              return 0;
                                              }
                                     
                                                printf("[%s:%d] \n",__FUNCTION__,__LINE__);
                                      

}


int test_case_8(int argc, char **argv)
{


    int dev = 0;
    MS_U32 pool_handle_id1 = 0;
    int err;
    MS_U32 timeout = 2;
        pid_t pid = 0;
    int status;
	printf("201510141533  samson debug in %s \n",__FUNCTION__);
	memset(IPAPool_Info, 0, sizeof(IPAPOOL_INFO)*MAX_IPAPOOLSIZE);

    //open ion device
    dev = ipa_open();
    if(dev < 0)
    {
        printf("open ipapool device fail\n");
        return 0;
    }      
#if 1
    printf("[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);

    err = ipa_init(dev, DFB_TEMP_HID, (MS_U32)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id1);
    if(err < 0)
        {
        printf("[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
        return 0;
        }


    //pool1 allocations
 MS_U64 va1 = NULL;
    printf("[%s:%d] will ipa_map  pool_handle_id1=%u\n",__FUNCTION__,__LINE__,pool_handle_id1);
err = ipa_map(dev, pool_handle_id1, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1);
    if(err < 0)
        {
        printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
        return 0;  
        }

 printf("[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
    err = ipa_alloc(dev, pool_handle_id1, 0, PAGE,timeout);
    printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
    if(err < 0)
        {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
        }

    err = ipa_alloc(dev, pool_handle_id1, 3*PAGE, PAGE,timeout);
    if(err < 0)
        {
        printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
        return 0;
        }
    /*
    err = ipa_alloc(dev, pool_handle_id1, 2.5*MILLION, PAGE,timeout);
    if(err < 0)
        return 0;
    err = ipa_alloc(dev, pool_handle_id1, 3.5*MILLION-PAGE, PAGE,timeout);
    if(err < 0)
        return 0;*/
#endif

          printf("[%s:%d] \n",__FUNCTION__,__LINE__);

          struct IPA_Pool_Polling_Param polling_param;
          //polling_param = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
          polling_param.pool_handle_id = pool_handle_id1;
          polling_param.fd=dev;
          printf("[%s:%d] \n",__FUNCTION__,__LINE__);
          polling_param.polling_callback=DFB_callback_id_1;

          printf("[%s:%d] will MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
          MApi_IPA_Pool_Register_Notify(&polling_param);
          printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);

          printf("[%s:%d] before fork pid %d, ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
          pid = fork();
          printf("[%s:%d] after fork  pid %d,  ppid %d\n",__FUNCTION__,__LINE__,getpid(),getppid());
          if(pid < 0)
          {
              printf("fork fail\n");
              goto PARENT_RUN;
          }
          else if(pid == 0) //child
          {
             printf("about child pid=%d\n",pid);
          //second client conflict with first
          MS_U32 pool_handle_id1_2 = 0;
          
//#if 0//同一个pool ，init过了一次就不再init了。要想再init就必须先deinit了再说.
//不同的pool 就无所谓了.
#if 1
          printf("about child pid=%d before ipa_init\n",pid);
              err = ipa_init(dev, DFB_TEMP_HID, (MS_U32)Pool_Name1_2, 6.5*MILLION, 8*MILLION, &pool_handle_id1_2);
              printf("about child pid=%d after ipa_init\n",pid);
              if(err < 0)
                  {
                  printf("[%s:%d] ipa_init fail 2\n",__FUNCTION__,__LINE__);
                  return 0;
                  }
          #endif
          
              //pool1 allocations
           MS_U64 va1_2 = NULL;
              printf("[%s:%d] will ipa_map  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2);
          err = ipa_map(dev, pool_handle_id1_2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va1_2);
              if(err < 0)
                  {
                  printf("[%s:%d] err,return\n",__FUNCTION__,__LINE__);
                  return 0;  
                  }
          
           printf("[%s:%d] will ipa_alloc  pool_handle_id1_2=%u\n",__FUNCTION__,__LINE__);
           timeout = 2;
              err = ipa_alloc(dev, pool_handle_id1_2, 8*PAGE, 8*PAGE,timeout);
              printf("[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
              if(err < 0)
                  {
                  printf("[%s:%d] err return\n",__FUNCTION__,__LINE__);
                  return 0;
                  }
         
                    printf("[%s:%d] \n",__FUNCTION__,__LINE__);
          
                    struct IPA_Pool_Polling_Param polling_param_2;
                    //polling_param_2 = (struct IPA_Pool_Polling_Param *)malloc(sizeof(struct IPA_Pool_Polling_Param));
                    polling_param_2.pool_handle_id = pool_handle_id1_2;
                    polling_param_2.fd=dev;
                    printf("[%s:%d]  ,pool_handle_id1_2=%u ,polling_param_2.pool_handle_id=%u\n",__FUNCTION__,__LINE__,pool_handle_id1_2,polling_param_2.pool_handle_id);
                    polling_param_2.polling_callback=DFB_callback_id_2;
          
                    printf("[%s:%d] will MApi_IPA_Pool_Register_Notify polling_param_2->pool_handle_id=%u\n",__FUNCTION__,__LINE__,polling_param_2.pool_handle_id);
                    MApi_IPA_Pool_Register_Notify(&polling_param_2);
                    printf("[%s:%d] after MApi_IPA_Pool_Register_Notify\n",__FUNCTION__,__LINE__);
                  
                   // free(polling_param_2);

            }
            else //parent
              {
                   printf("inside parent\n");
                   waitpid(pid, &status, 0);
                  // sleep(5);
                   printf("parent wake pid %d ppid %d==========================================\n",
                      getpid(),getppid());
                   goto PARENT_RUN;
              }    
              printf("child return pid=%d\n",pid);
              return 0;
          
PARENT_RUN:   

          printf("inside parent_run\n");
          //free(polling_param);


               struct pthread_info thread3_info;
               
               thread3_info.dev =dev;

          pthread_t ntid;
               err = pthread_create(&ntid,
                                                       NULL,
                                                       create_thread3,
                                                       &thread3_info);




#if 0//another cleint init and alloc.
            printf("2[%s:%d] will ipa_init\n",__FUNCTION__,__LINE__);
            MS_U32 pool_handle_id2 = 0;
            err = ipa_init(dev, DFB_TEMP_HID, (MS_U32)Pool_Name1, 6.5*MILLION, 8*MILLION, &pool_handle_id2);
            if(err < 0)
                {
                printf("2[%s:%d] ipa_init fail \n",__FUNCTION__,__LINE__);
                return 0;
                }
        
        
            //pool1 allocations
         MS_U64 va2 = NULL;
            printf("2[%s:%d] will ipa_map\n",__FUNCTION__,__LINE__);
        err = ipa_map(dev, pool_handle_id2, 0, 1*MILLION, IPA_VA_CACHE_NONE_CACHE, &va2);
            if(err < 0)
                {
                printf("2[%s:%d] err,return\n",__FUNCTION__,__LINE__);
                return 0;  
                }
        
         printf("2[%s:%d] will ipa_alloc\n",__FUNCTION__,__LINE__);
         timeout = 3;
            err = ipa_alloc(dev, pool_handle_id2, 0, PAGE,timeout);
            printf("2[%s:%d] after ipa_alloc\n",__FUNCTION__,__LINE__);
            if(err < 0)
                {
                printf("2[%s:%d] err return\n",__FUNCTION__,__LINE__);
                return 0;
                }
#endif



          
	    while(1);//just for test

    printf("[%s:%d] will ipa_unmap\n",__FUNCTION__,__LINE__);	
    err = ipa_unmap(dev, va1, 1*MILLION);
    if(err < 0)
    {
        printf("[%s:%d] ipa_unmap fail err=%d\n",__FUNCTION__,__LINE__,err);
        return 0;
    }

	
	
	
#if 0	
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_deinit\n");
    err = ipa_deinit(dev, pool_handle_id1);
    printf("samson main after ipa_deinit\n");
    if(err < 0)
    	return 0;
#endif
    
    //stop_to_gets(__LINE__);
    printf("samson main before ipa_close\n");

    ipa_close(dev);//娉ㄦ剰鍦╟lose涔嬪墠蹇呴』鎵�鏈夌浉鍏崇殑閮藉緱deinit鎺夈�?
    printf("samson main after ipa_close\n");

    return 0;
}
#endif

int main(int argc, char **argv)
{
     return test_case_6(argc,argv);
}


