/*
 * vdc.c - Fake VDC emulation for the C128 emulator.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
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

/* This is just a kludge to make the C128 kernal believe there is an alive
   VDC in the system during the power-on self test.  */

#include "vice.h"

#include "mem.h"

static int vdc_ptr;
static BYTE vdc[0x100];

void REGPARM2 store_vdc(ADDRESS addr, BYTE value)
{
    if (addr & 1) {
	vdc[vdc_ptr] = value;
	if (vdc_ptr == 0x1e) {
	    vdc[0x12] = mem_read(0xa3d);
	    vdc[0x13] = mem_read(0xa3c);
	}
    } else {
	vdc_ptr = value;
    }
}

BYTE REGPARM1 read_vdc(ADDRESS addr)
{
    if (addr & 1) {
	if (vdc_ptr == 0x12)
	    return mem_read(0xa3d);
	else if (vdc_ptr == 0x13)
	    return mem_read(0xa3c);
	return ((vdc_ptr < 37) ? vdc[vdc_ptr] : 0xff);
    } else
	return 0x9f;
}
