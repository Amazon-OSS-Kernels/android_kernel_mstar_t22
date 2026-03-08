#include "ns_uboot_common.h"

#if defined(CONFIG_OPTEE_TEELOADER)
#define __postloader_text__ __attribute__((section(".ldr.teepostloader.text")))
#else
#define __postloader_text__
#endif

void LDR_SHOWTIME()
{
    int piuMs = 0;
    long long piuClk=((unsigned int)RREG16(0x1F006090)|((unsigned int)RREG16(0x1F006094)<<16));
    for(;piuClk>0;piuClk-=12000)
    {
        piuMs++;
    }
    LDR_PUTDW(piuMs);
}

void LDR_PUTC(int c)
{
    while((RREG16(REG_UART_LSR) & UART_LSR_THRE) != UART_LSR_THRE)
        ;
    WREG16(REG_UART_TX, c);
    while((RREG16(REG_UART_LSR) & (UART_LSR_TEMT | UART_LSR_THRE)) != (UART_LSR_TEMT | UART_LSR_THRE))
        ;
}

void LDR_PUTX(char val)
{
    if (val > 0x9)
        LDR_PUTC(0x41+(val-10));
    else
        LDR_PUTC(0x30+val);
}

void LDR_PUTDW(unsigned int val)
{
    unsigned char value;

    value = (val>>28)&0xF;
    LDR_PUTX(value);
    value = (val>>24)&0xF;
    LDR_PUTX(value);
    value = (val>>20)&0xF;
    LDR_PUTX(value);
    value = (val>>16)&0xF;
    LDR_PUTX(value);
    value = (val>>12)&0xF;
    LDR_PUTX(value);
    value = (val>>8)&0xF;
    LDR_PUTX(value);
    value = (val>>4)&0xF;
    LDR_PUTX(value);
    value = (val)&0xF;
    LDR_PUTX(value);
}

void LDR_PUTS(const char *s)
{
    while(*s) {
        if(*s == '\n')
            LDR_PUTC('\r');
        LDR_PUTC(*s++);
    }
}

void LDR_DUMP(const char *s,int count)
{
    unsigned int i;
    int rByte=0;
    unsigned char value;
    for(i=0;s && i<count ;i++)
    {
        value = (s[i]>>4)&0xF;
        LDR_PUTX(value);
        value = (s[i])&0xF;
        LDR_PUTX(value);
        LDR_PUTC(' ');
        rByte++;
        if(rByte==16)
        {
            rByte=0;
            LDR_PUTS("\n");
        }
    }
    if(rByte>0)
    {
        LDR_PUTS("\n");
    }
}

void *_memcpy(void *d, void *s, unsigned int n)
{

#ifdef MEMCPY_BDMA
    __BDMA_MIU0Copy2MIU0((U32)__BA2PA((unsigned long long)s), (U32)__BA2PA((unsigned long long)d), n);
#else
    int *dest = (int*)d;
    int *src = (int*)s;
    char *source_char = NULL;
    char *dst_char = NULL;
    int wordnum = n >> 2;
    int slice = n & 0x03;

    if(((unsigned int)s & 0x00000003)==0 && ((unsigned int)d & 0x00000003)==0)//Check if both of d and s are aligned to 4byte
    {
        while (wordnum-- > 0)
        {
            *dest++ = *src++;
        }
    }
    else//either d or s is not aligned to 4byte
    {
        slice = n;
    }

    source_char = (char*)src;
    dst_char = (char*)dest;
    while (slice-- > 0)
    {
        *(dst_char++) = *(source_char++);
    }
#endif
#ifdef CONFIG_MCU_MIPS32
    flush_cache(d, n);
#endif
    return d;
}

int _memcmp(const void * cs,const void * ct, unsigned int count)
{
    const unsigned char *su1, *su2;
    int res = 0;

    for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
        res |= (*su1 ^ *su2);
    return res;
}

void __postloader_text__ *_memset (void *s, int c, unsigned int n)
{
    unsigned int *start4Byte = s;
    unsigned char *startByte = NULL;
    int wordnum = n >> 2;
    int slice = n & 0x03;
    if(((unsigned int)start4Byte & 0x00000003)==0)//Check if s is aligned to 4byte
    {
        while (wordnum-- > 0)
        {
            *(start4Byte++) = (unsigned int)c;
        }
    }
    else//s is not aligned to 4byte
    {
        slice = n;
    }
    startByte = (unsigned char*)start4Byte;
    while (slice-- > 0)
    {
        *(startByte++) = c;
    }
#ifdef CONFIG_MCU_MIPS32
    flush_cache(s, n);
#endif
    return s;
}

void array_reverse(unsigned char* ary, unsigned int len)
{
    int i, j;
    char tmp;
    for (i=0, j=len-1; i<j; ++i, --j)
    {
        tmp = ary[i];
        ary[i] = ary[j];
        ary[j] = tmp;
    }
}

#if defined(__aarch64__)
unsigned long long __BA2PA(unsigned long long u64BusAddr)
{
    unsigned long long u64PhyAddr = 0x0;
#ifdef CONFIG_MSTAR_MASERATI
    if( (u64BusAddr >= CONFIG_MIU0_BUSADDR) && (u64BusAddr < CONFIG_MIU1_BUSADDR) ) // MIU0
        u64PhyAddr = u64BusAddr - CONFIG_MIU0_BUSADDR + MIU0_PA_BASE;
    else if( (u64BusAddr >= CONFIG_MIU0_BUSADDR) && (u64BusAddr < CONFIG_MIU2_BUSADDR) )// MIU1
        u64PhyAddr = u64BusAddr - CONFIG_MIU1_BUSADDR + MIU1_PA_BASE;
    else                                                                        //MIU2
        u64PhyAddr = u64BusAddr - CONFIG_MIU2_BUSADDR + MIU2_PA_BASE;
#else
    if( (u64BusAddr >= CONFIG_MIU0_BUSADDR) && (u64BusAddr < CONFIG_MIU1_BUSADDR) ) // MIU0
        u64PhyAddr = u64BusAddr - CONFIG_MIU0_BUSADDR + MIU0_PA_BASE;
    else if( (u64BusAddr >= CONFIG_MIU1_BUSADDR) )// MIU1
        u64PhyAddr = u64BusAddr - CONFIG_MIU1_BUSADDR + MIU1_PA_BASE;
#endif
    return u64PhyAddr;
}

unsigned long long __PA2BA(unsigned long long u64PhyAddr)
{
    unsigned long long u64BusAddr = 0x0;
#ifdef CONFIG_MSTAR_MASERATI
    if( u64PhyAddr < MIU1_PA_BASE ) // MIU0
        u64BusAddr = u64PhyAddr - MIU0_PA_BASE + CONFIG_MIU0_BUSADDR;
    else if ( (u64PhyAddr >= MIU1_PA_BASE) && (u64PhyAddr < MIU2_PA_BASE) ) // MIU1
        u64BusAddr = u64PhyAddr - MIU1_PA_BASE + CONFIG_MIU1_BUSADDR;
    else
        u64BusAddr = u64PhyAddr - MIU2_PA_BASE + CONFIG_MIU2_BUSADDR;
#else
    if( u64PhyAddr < MIU1_PA_BASE ) // MIU0
        u64BusAddr = u64PhyAddr - MIU0_PA_BASE + CONFIG_MIU0_BUSADDR;
    else if ( (u64PhyAddr >= MIU1_PA_BASE) ) // MIU1
        u64BusAddr = u64PhyAddr - MIU1_PA_BASE + CONFIG_MIU1_BUSADDR;
#endif
    return u64BusAddr;
}
#else
unsigned int __BA2PA(unsigned int u32BusAddr)
{
    unsigned int u32PhyAddr = 0x0;
#ifdef CONFIG_MSTAR_MASERATI
    if( (u32BusAddr >= CONFIG_MIU0_BUSADDR) && (u32BusAddr < CONFIG_MIU1_BUSADDR) ) // MIU0
        u32PhyAddr = u32BusAddr - CONFIG_MIU0_BUSADDR + MIU0_PA_BASE;
    else if( (u32BusAddr >= CONFIG_MIU0_BUSADDR) && (u32BusAddr < CONFIG_MIU2_BUSADDR) )// MIU1
        u32PhyAddr = u32BusAddr - CONFIG_MIU1_BUSADDR + MIU1_PA_BASE;
    else                                                                        //MIU2
        u32PhyAddr = u32BusAddr - CONFIG_MIU2_BUSADDR + MIU2_PA_BASE;
#else
    if( (u32BusAddr >= CONFIG_MIU0_BUSADDR) && (u32BusAddr < CONFIG_MIU1_BUSADDR) ) // MIU0
        u32PhyAddr = u32BusAddr - CONFIG_MIU0_BUSADDR + MIU0_PA_BASE;
    else if( (u32BusAddr >= CONFIG_MIU1_BUSADDR) )// MIU1
        u32PhyAddr = u32BusAddr - CONFIG_MIU1_BUSADDR + MIU1_PA_BASE;
#endif
    return u32PhyAddr;
}

unsigned int __PA2BA(unsigned int u32PhyAddr)
{
    unsigned int u32BusAddr = 0x0;
#ifdef CONFIG_MSTAR_MASERATI
    if( u32PhyAddr < MIU1_PA_BASE ) // MIU0
        u32BusAddr = u32PhyAddr - MIU0_PA_BASE + CONFIG_MIU0_BUSADDR;
    else if ( (u32PhyAddr >= MIU1_PA_BASE) && (u32PhyAddr < MIU2_PA_BASE) ) // MIU1
        u32BusAddr = u32PhyAddr - MIU1_PA_BASE + CONFIG_MIU1_BUSADDR;
    else
        u32BusAddr = u32PhyAddr - MIU2_PA_BASE + CONFIG_MIU2_BUSADDR;
#else
    if( u32PhyAddr < MIU1_PA_BASE ) // MIU0
        u32BusAddr = u32PhyAddr - MIU0_PA_BASE + CONFIG_MIU0_BUSADDR;
    else if ( (u32PhyAddr >= MIU1_PA_BASE) ) // MIU1
        u32BusAddr = u32PhyAddr - MIU1_PA_BASE + CONFIG_MIU1_BUSADDR;
#endif
    return u32BusAddr;
}
#endif

void __cyg_profile_func_enter(void *this_fn,void *call_site) __attribute__((__no_instrument_function__)) __postloader_text__;
void __cyg_profile_func_exit(void *this_fn,void *call_site) __attribute__((__no_instrument_function__)) __postloader_text__;
void __cyg_profile_func_enter(void *this_fn,void *call_site)
{
    LDR_PUTC('[');
    LDR_SHOWTIME();
    LDR_PUTC(']');
    LDR_PUTS("Enter  ");
    LDR_PUTDW((unsigned int)this_fn-1);
    LDR_PUTS("\n");
}

void __cyg_profile_func_exit(void *this_fn,void *call_site)
{
    LDR_PUTC('[');
    LDR_SHOWTIME();
    LDR_PUTC(']');
    LDR_PUTS("Exit  ");
    LDR_PUTDW((unsigned int)this_fn-1);
    LDR_PUTS("\n");
}