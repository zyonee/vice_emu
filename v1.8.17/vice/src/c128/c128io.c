/*
 * c128io.c -- C128 io handling.
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
 *
 * Based on the original work in VICE 0.11.0 by
 *  Jouko Valta <jopi@stekt.oulu.fi>
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

#include <stdio.h>
#include <stdlib.h>

#include "c128-resources.h"
#include "c128io.h"
#include "c64acia.h"
#include "c64cart.h"
#include "c64tpi.h"
#include "cartridge.h"
#include "emuid.h"
#include "reu.h"
#include "sid-resources.h"
#include "sid.h"
#include "types.h"


BYTE REGPARM1 io1_read(ADDRESS addr)
{
    if (sid_stereo
        && addr >= sid_stereo_address_start
        && addr < sid_stereo_address_end)
        return sid2_read(addr);
    if (mem_cartridge_type != CARTRIDGE_NONE)
        return cartridge_read_io1(addr);
#ifdef HAVE_RS232
    if (acia_de_enabled)
        return acia1_read(addr & 0x03);
#endif
    return 0xff;  /* rand(); - C64 has rand(), which is correct? */
}

void REGPARM2 io1_store(ADDRESS addr, BYTE value)
{
    if (sid_stereo
        && addr >= sid_stereo_address_start
        && addr < sid_stereo_address_end)
        sid2_store(addr, value);
    if (mem_cartridge_type != CARTRIDGE_NONE)
        cartridge_store_io1(addr, value);
#ifdef HAVE_RS232
    if (acia_de_enabled)
        acia1_store(addr & 0x03, value);
#endif
    return;
}

BYTE REGPARM1 io2_read(ADDRESS addr)
{
    if (sid_stereo
        && addr >= sid_stereo_address_start
        && addr < sid_stereo_address_end)
        return sid2_read(addr);
    if (mem_cartridge_type != CARTRIDGE_NONE)
        return cartridge_read_io2(addr);
    if (emu_id_enabled && addr >= 0xdfa0) {
        addr &= 0xff;
        if (addr == 0xff)
            emulator_id[addr - 0xa0] ^= 0xff;
        return emulator_id[addr - 0xa0];
    }
    if (reu_enabled)
        return reu_read((ADDRESS)(addr & 0x0f));
    if (ieee488_enabled)
        return tpi_read((ADDRESS)(addr & 0x07));

    return 0xff;  /* rand(); - C64 has rand(), which is correct? */
}

void REGPARM2 io2_store(ADDRESS addr, BYTE value)
{
    if (sid_stereo
        && addr >= sid_stereo_address_start
        && addr < sid_stereo_address_end)
        sid2_store(addr, value);
    if (mem_cartridge_type != CARTRIDGE_NONE) {
        cartridge_store_io2(addr, value);
        return;
    }
    if (reu_enabled)
        reu_store((ADDRESS)(addr & 0x0f), value);
    if (ieee488_enabled) {
        tpi_store((ADDRESS)(addr & 0x07), value);
    }
    return;
}

