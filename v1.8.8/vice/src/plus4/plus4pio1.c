/*
 * plus4pio1.c -- Plus4 PIO1 handling.
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#include "vice.h"

#include "plus4pio1.h"
#include "types.h"

static BYTE pio1_data = 0xff;

BYTE REGPARM1 pio1_read(ADDRESS addr)
{
    return pio1_data;
}

void REGPARM2 pio1_store(ADDRESS addr, BYTE value)
{
    pio1_data = value;
}

void pio1_set_tape_sense(int sense)
{
    pio1_data = (BYTE)(pio1_data & ~4) | (BYTE)(sense ? 0 : 4);
}

