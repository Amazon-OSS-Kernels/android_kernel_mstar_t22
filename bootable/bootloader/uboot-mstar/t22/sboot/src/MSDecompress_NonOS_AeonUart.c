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

#ifndef ENABLE_LOADER2_AEON_UART
    #define ENABLE_LOADER2_AEON_UART    0
#endif

#if( defined(__aeon__) && ENABLE_LOADER2_AEON_UART )

#define UART_BASE       0x90000000
#define UART_REG8(_x_)   ((volatile unsigned char *)UART_BASE)[_x_]


#define UART_TX     0    // Out: Transmit buffer (DLAB=0)   (16-byte FIFO)
#define UART_DLL    0    // Out: Divisor Latch Low (DLAB=1)
#define UART_DLM    1    // Out: Divisor Latch High (DLAB=1)
#define UART_FCR    2    // Out: FIFO Control Register
#define UART_LCR    3    // Out: Line Control Register

#define UART_LSR    5    // In:  Line Status Register

//
// FIFO Control Register
//
#define UART_FCR_ENABLE_FIFO        0x01    // Enable the FIFO
#define UART_FCR_CLEAR_RCVR         0x02    // Clear the RCVR FIFO
#define UART_FCR_CLEAR_XMIT         0x04    // Clear the XMIT FIFO
#define UART_FCR_DMA_SELECT         0x08    // For DMA applications
#define UART_FCR_TRIGGER_MASK       0xC0    // Mask for the FIFO trigger range
#define UART_FCR_TRIGGER_1          0x00    // Mask for trigger set at 1
#define UART_FCR_TRIGGER_4          0x40    // Mask for trigger set at 4
#define UART_FCR_TRIGGER_8          0x80    // Mask for trigger set at 8
#define UART_FCR_TRIGGER_14         0xC0    // Mask for trigger set at 14

//
// Line Control Register
// Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting
// UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
//
#define UART_LCR_DLAB               0x80    // Divisor latch access bit
#define UART_LCR_SBC                0x40    // Set break control
#define UART_LCR_SPAR               0x20    // Stick parity (?)
#define UART_LCR_EPAR               0x10    // Even parity select
#define UART_LCR_PARITY             0x08    // Parity Enable
#define UART_LCR_STOP1              0x00    // Stop bits: 0=1 stop bit, 1= 2 stop bits
#define UART_LCR_STOP2              0x04    // Stop bits: 0=1 stop bit, 1= 2 stop bits
#define UART_LCR_WLEN5              0x00    // Wordlength: 5 bits
#define UART_LCR_WLEN6              0x01    // Wordlength: 6 bits
#define UART_LCR_WLEN7              0x02    // Wordlength: 7 bits
#define UART_LCR_WLEN8              0x03    // Wordlength: 8 bits

#define UART_LSR_TEMT               0x40    // Transmitter empty
#define UART_LSR_THRE               0x20    // Transmit-hold-register empty

// both Transmitter empty / Transmit-hold-register empty
#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)


#define WAIT_FOR_XMITR \
        do { \
                lsr = UART_REG8(UART_LSR); \
        } while ((lsr & BOTH_EMPTY) != BOTH_EMPTY)

#define WAIT_FOR_THRE \
        do { \
                lsr = UART_REG8(UART_LSR); \
        } while ((lsr & UART_LSR_THRE) != UART_LSR_THRE)



void __loader_2nd loader2_Aeon_uart_putc(char ch);
void __loader_2nd loader2_Aeon_uart_putc(char ch)
{
#if 1//(ENABLE_MSTAR_EDEN||ENABLE_MSTAR_WHISKY)
    unsigned char lsr;

    WAIT_FOR_THRE;
    UART_REG8(UART_TX) = ch;
    WAIT_FOR_XMITR;
#else
    ch = ch;
#endif
}

// Aeon SPR :
#define MAX_SPRS_PER_GRP_BITS   (11)
#define SPRGROUP_TT             (10<< MAX_SPRS_PER_GRP_BITS)

/* Tick Timer group */
#define SPR_TTMR                (SPRGROUP_TT + 0)
#define SPR_TTCR                (SPRGROUP_TT + 1)

#define mtspr(spr, value) \
    __asm__ __volatile__ ("l.mtspr\t\t%0,%1,0" : : "r" (spr), "r" (value))

void __loader_2nd Loader2_MDrv_Uart_Init( void );
void __loader_2nd Loader2_MDrv_Uart_Init( void )
{
#ifdef __AEONR2__
    U32 aeon_clock = 216000000;
    U32 uart_clock = 115200;
#else
    U32 aeon_clock = 172800000;
    U32 uart_clock = 115200;
#endif

#if 1//(ENABLE_MSTAR_EDEN||ENABLE_MSTAR_WHISKY)

    int divisor;

    // Reset receiver and transmiter
    UART_REG8(UART_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_1;

    // Set 8 bit char, 1 stop bit, no parity
    UART_REG8(UART_LCR) = UART_LCR_WLEN8 & ~(UART_LCR_STOP2 | UART_LCR_PARITY);
    divisor = ((aeon_clock) / (16 * uart_clock));

    UART_REG8(UART_LCR) |= UART_LCR_DLAB;
    UART_REG8(UART_DLL) = divisor & 0xFF;
    UART_REG8(UART_DLM) = (divisor >> 8) & 0xFF;
    UART_REG8(UART_LCR) &= ~(UART_LCR_DLAB);

    mtspr( SPR_TTMR, 0x40000000 | (aeon_clock/1000) );
#endif
}
#endif // ENABLE_LOADER2_AEON_UART

