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

#include "datatype.h"
#include "drvAeonUART.h"
#include "drvGlobal.h"


/*------------------------------------------------------------------------------*/
/*  MACROS                                                                      */
/*------------------------------------------------------------------------------*/

#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

#define WAIT_FOR_XMITR \
        do { \
                lsr = UART_REG8(UART_LSR); \
        } while ((lsr & BOTH_EMPTY) != BOTH_EMPTY)

#define WAIT_FOR_THRE \
        do { \
                lsr = UART_REG8(UART_LSR); \
        } while ((lsr & UART_LSR_THRE) != UART_LSR_THRE)

#define CHECK_FOR_CHAR (UART_REG8(UART_LSR) & UART_LSR_DR)

#define WAIT_FOR_CHAR \
         do { \
                lsr = UART_REG8(UART_LSR); \
         } while ((lsr & UART_LSR_DR) != UART_LSR_DR)

/*------------------------------------------------------------------------------*/
/*  UART APIS                                                                   */
/*------------------------------------------------------------------------------*/
void uart_init(U32 aeon_clock, U32 uart_clock)
{
    int divisor;

    RIU[0x101EA6] = 0xFFF0;
    RIU[0x101EA8] = 0x000F;
    RIU[0x0E12] &= (~BIT12);     //UART mux sel by dig_mux
    RIU[0x0E12] |= BIT11;        //UART mux sel by dig_mux
    /* Reset receiver and transmiter */

#ifdef __USE__AEON_UART__
    // Reset receiver and transmiter
    UART_REG8(UART_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_1;

    // Set 8 bit char, 1 stop bit, no parity
    UART_REG8(UART_LCR) = UART_LCR_WLEN8 & ~(UART_LCR_STOP2 | UART_LCR_PARITY);
    divisor = ((aeon_clock) / (16 * uart_clock));

    UART_REG8(UART_LCR) |= UART_LCR_DLAB;
    UART_REG8(UART_DLL) = divisor & 0xFF;
    UART_REG8(UART_DLM) = (divisor >> 8) & 0xFF;
    UART_REG8(UART_LCR) &= ~(UART_LCR_DLAB);
#else
    // UART mode
    *(volatile U32*)(0xA0200000+(0x0F6E*4)) &= ~0x0F00;

    UART_REG8(UART_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14;
    /* Interrupt Enable Register */
    UART_REG8(UART_IER) = UART_IER_RDI;     // Receive data available
    /* Set 8 bit char, 1 stop bit, no parity */
    UART_REG8(UART_LCR) = UART_LCR_WLEN8 & ~(UART_LCR_STOP2 | UART_LCR_PARITY);
    /* Set baud rate */
    divisor = aeon_clock/(16 * uart_clock);
    UART_REG8(UART_LCR) |= UART_LCR_DLAB;
    UART_REG8(UART_DLL) = divisor & 0x000000ff;
    UART_REG8(UART_DLM) = (divisor >> 8) & 0x000000ff;
    UART_REG8(UART_LCR) &= ~(UART_LCR_DLAB);
#endif
}

void uart_putc(char c)
{
#ifdef __USE__AEON_UART__
    unsigned char lsr;

    WAIT_FOR_THRE;
    UART_REG8(UART_TX) = c;
    WAIT_FOR_XMITR;
#else
    while (!(UART_REG8(UART_LSR) & UART_LSR_THRE));
    UART_REG8(UART_TX) = c;
#endif
}

char uart_getc(void)
{
#ifdef __USE__AEON_UART__
    unsigned char lsr, ier;
    char c;

    ier = UART_REG8(UART_IER);
    UART_REG8(UART_IER) = 0;

    WAIT_FOR_CHAR;
    c = UART_REG8(UART_RX);

    UART_REG8(UART_IER) = ier;

    return c;
#else
    char c;

    while (!(UART_REG8(UART_LSR) & UART_LSR_DR));
    c = UART_REG8(UART_RX);

    return c;
#endif
}

char uart_kbhit(void)
{
    if (UART_REG8(UART_LSR) & UART_LSR_DR)
        return 1;
    else
        return 0;

}
void uart_interrupt_enable(BOOLEAN bEnable)
{
    if (bEnable)
        UART_REG8(UART_IER) = UART_IER_RDI;     // Receive data available
    else
        UART_REG8(UART_IER) = 0;                //Receive data disable
}

