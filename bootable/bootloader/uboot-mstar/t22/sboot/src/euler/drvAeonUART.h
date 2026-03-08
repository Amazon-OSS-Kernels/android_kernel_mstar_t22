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

#ifndef _DRVUART_H
#define _DRVUART_H

#include "datatype.h"

#define __USE__AEON_UART__

#ifdef __USE__AEON_UART__
#define UART_RX     0    // In:  Receive buffer (DLAB=0)    (16-byte FIFO)
#define UART_TX     0    // Out: Transmit buffer (DLAB=0)   (16-byte FIFO)
#define UART_DLL    0    // Out: Divisor Latch Low (DLAB=1)
#define UART_DLM    1    // Out: Divisor Latch High (DLAB=1)
#define UART_IER    1    // Out: Interrupt Enable Register
#define UART_IIR    2    // In:  Interrupt ID Register
#define UART_FCR    2    // Out: FIFO Control Register
#define UART_LCR    3    // Out: Line Control Register
#define UART_MCR    4    // Out: Modem Control Register
#define UART_LSR    5    // In:  Line Status Register
#define UART_MSR    6    // In:  Modem Status Register
#define UART_SCR    7    // I/O: Scratch Register
#else
#define UART_RX        (0 * 2)  // In:  Receive buffer (DLAB=0)
#define UART_TX        (0 * 2)  // Out: Transmit buffer (DLAB=0)
#define UART_DLL       (0 * 2)  // Out: Divisor Latch Low (DLAB=1)
#define UART_DLM       (1 * 2)  // Out: Divisor Latch High (DLAB=1)
#define UART_IER       (1 * 2)  // Out: Interrupt Enable Register
#define UART_IIR       (2 * 2)  // In:  Interrupt ID Register
#define UART_FCR       (2 * 2)  // Out: FIFO Control Register
#define UART_LCR       (3 * 2)  // Out: Line Control Register
#define UART_LSR       (5 * 2)  // In:  Line Status Register
#define UART_MSR       (6 * 2)  // In:  Modem Status Register
#endif

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

//
// Line Status Register
//
#define UART_LSR_TEMT               0x40    // Transmitter empty
#define UART_LSR_THRE               0x20    // Transmit-hold-register empty
#define UART_LSR_BI                 0x10    // Break interrupt indicator
#define UART_LSR_FE                 0x08    // Frame error indicator
#define UART_LSR_PE                 0x04    // Parity error indicator
#define UART_LSR_OE                 0x02    // Overrun error indicator
#define UART_LSR_DR                 0x01    // Receiver data ready

//
// Interrupt Identification Register
//
#define UART_IIR_NO_INT             0x01    // No interrupts pending
#define UART_IIR_ID                 0x06    // Mask for the interrupt ID

#define UART_IIR_MSI                0x00    // Modem status interrupt
#define UART_IIR_THRI               0x02    // Transmitter holding register empty
#define UART_IIR_TOI                0x0c    // Receive time out interrupt
#define UART_IIR_RDI                0x04    // Receiver data interrupt
#define UART_IIR_RLSI               0x06    // Receiver line status interrupt

//
// Interrupt Enable Register
//
#define UART_IER_MSI                0x08    // Enable Modem status interrupt
#define UART_IER_RLSI               0x04    // Enable receiver line status interrupt
#define UART_IER_THRI               0x02    // Enable Transmitter holding reg empty int
#define UART_IER_RDI                0x01    // Enable receiver data available interrupt

//
// Modem Control Register
//
#define UART_MCR_LOOP               0x10    // Enable loopback test mode
#define UART_MCR_OUT2               0x08    // Out2 complement
#define UART_MCR_OUT1               0x04    // Out1 complement
#define UART_MCR_RTS                0x02    // RTS complement
#define UART_MCR_DTR                0x01    // DTR complement

//
// Modem Status Register
//
#define UART_MSR_DCD                0x80    // Data Carrier Detect
#define UART_MSR_RI                 0x40    // Ring Indicator
#define UART_MSR_DSR                0x20    // Data Set Ready
#define UART_MSR_CTS                0x10    // Clear to Send
#define UART_MSR_DDCD               0x08    // Delta DCD
#define UART_MSR_TERI               0x04    // Trailing edge ring indicator
#define UART_MSR_DDSR               0x02    // Delta DSR
#define UART_MSR_DCTS               0x01    // Delta CTS
#define UART_MSR_ANY_DELTA          0x0F    // Any of the delta bits!

// both Transmitter empty / Transmit-hold-register empty
#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Clock source
#define UART_CLK_FREQ   (XTAL_CLOCK_FREQ)
// Divisor
#define UART_DIVISOR    (UART_CLK_FREQ/(16 * UART_BAUD_RATE))

#ifdef __USE__AEON_UART__
#define UART_BASE       0x90000000
#define UART_REG8(_x_)   ((volatile unsigned char *)UART_BASE)[_x_]
#else
#define UART_BASE       (0xA0200000+0x1300)
#define UART_REG8(_x_)  ((volatile unsigned char *)(UART_BASE))[((_x_) * 4) - ((_x_) & 1)]
#endif


//-------------------------------------------------------------------------------------------------
// Extern Functions
//-------------------------------------------------------------------------------------------------
/// UART callback function used in interrupt context

void uart_init(U32 aeon_clock, U32 uart_clock);
void uart_putc(char c);
char uart_getc(void);
char uart_kbhit(void);
void uart_interrupt_enable(BOOLEAN bEnable);

#endif //#ifndef _DRVUART_H

