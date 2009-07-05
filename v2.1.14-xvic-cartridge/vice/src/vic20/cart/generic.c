/*
 * generic.c -- VIC20 generic cartridge emulation.
 *
 * Written by
 *  Daniel Kahlin <daniel@kahlin.net>
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

#include "cartridge.h"
#include "lib.h"
#include "machine.h"
#include "maincpu.h"
#include "generic.h"
#include "mem.h"
#include "monitor.h"
#include "ram.h"
#include "resources.h"
#include "types.h"
#include "util.h"

/* ------------------------------------------------------------------------- */

/*
 * Cartridge RAM
 *
 * Mapping
 *      RAM                 VIC20
 *   0x0000 - 0x1fff  ->  0xa000 - 0xbfff
 *   0x2000 - 0x7fff  ->  0x2000 - 0x7fff
 *   0x8400 - 0x8fff  ->  0x0400 - 0x0fff
 *
 */
#define CART_RAM_SIZE 0x9000
static BYTE *cart_ram;

/*
 * Cartridge ROM
 *
 * Mapping
 *      ROM                 VIC20
 *   0x0000 - 0x1fff  ->  0xa000 - 0xbfff
 *   0x2000 - 0x7fff  ->  0x2000 - 0x7fff
 *   0x8400 - 0x8fff  ->  0x0400 - 0x0fff
 *
 */
#define CART_ROM_SIZE 0x9000
static BYTE *cart_rom;

/* Cartridge States */

/* ------------------------------------------------------------------------- */

/* 
 * 0x0400-0x0fff
 * 0x2000-0x7fff
 * 0xa000-0xbfff
 */
void REGPARM2 generic_mem_store(WORD addr, BYTE value)
{
    if (addr >= 0x0400 && addr < 0x1000) {
        cart_ram[(addr & 0x0fff) | 0x8000] = value;
    }
    if (addr >= 0x2000 && addr < 0x8000) {
        cart_ram[addr] = value;
    }
    if (addr >= 0xa000 && addr < 0xc000) {
        cart_ram[addr & 0x1fff] = value;
    }
}

BYTE REGPARM1 generic_mem_read(WORD addr)
{
    if (addr >= 0x0400 && addr < 0x1000) {
        return cart_rom[(addr & 0x0fff) | 0x8000];
    }
    if (addr >= 0x2000 && addr < 0x8000) {
        return cart_rom[addr];
    }
    if (addr >= 0xa000 && addr < 0xc000) {
        return cart_rom[addr & 0x1fff];
    }
    return 0x00; /* should never happen */
}


void generic_init(void)
{
}

void generic_reset(void)
{
}

void generic_config_setup(BYTE *rawcart)
{
}

int generic_bin_attach(const char *filename)
{
    cart_ram = lib_malloc(CART_RAM_SIZE);
    cart_rom = lib_malloc(CART_ROM_SIZE);
    if ( util_file_load(filename, cart_rom, (size_t)0x2000, UTIL_FILE_LOAD_RAW) < 0 ) {
        generic_detach();
        return -1;
    }
    return 0;
}

void generic_detach(void)
{
    lib_free(cart_rom);
    lib_free(cart_ram);
}

/* ------------------------------------------------------------------------- */


/* eof */
