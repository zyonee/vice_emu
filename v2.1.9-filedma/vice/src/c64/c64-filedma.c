/*
 * c64-filedma.c - DMA load/save support for C64
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "filedma.h"

/* does a machine support dma loading into memory? 0=yes -1=no */
int filedma_can_load(void)
{
    return 0;
}

static filedma_basic_t c64basic = {
    0x0801, /* start of basic */
    0xd000, /* max range of basic */
    0x2b,   /* start ptr */
    0xae    /* end ptr */
};

/* load directly into memory of machine. 0=ok -1=error */
int filedma_load(const BYTE *memory, WORD start_addr, WORD end_addr)
{
    return filedma_load_simulate_basic(memory, start_addr, end_addr, &c64basic);
}
