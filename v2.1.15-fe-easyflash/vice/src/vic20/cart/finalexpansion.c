/*
 * finalexpansion.c -- VIC20 Final Expansion v3.1 emulation.
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

#include "archdep.h"
#include "cartridge.h"
#include "lib.h"
#include "machine.h"
#include "finalexpansion.h"
#include "mem.h"
#include "resources.h"
#include "types.h"
#include "util.h"
#include "vic20cartmem.h"
#include "vic20mem.h"
#include "zfile.h"

/* ------------------------------------------------------------------------- */

/*
 * Cartridge RAM
 *
 * Mapping
 *      RAM                 VIC20
 *   0x0000 - 0x1fff  ->  0xa000 - 0xbfff
 *   0x2000 - 0x7fff  ->  0x2000 - 0x7fff
 *
 */
#define CART_RAM_SIZE 0x80000
static BYTE *cart_ram = NULL;

/*
 * Cartridge ROM
 *
 * Mapping
 *      ROM
 *   0x000000 - 0x0fffff  ->  Low ROM: banks 0x00-0x7f
 *   0x100000 - 0x1fffff  ->  High ROM: banks 0x00-0x7f
 *
 */
#define CART_ROM_SIZE 0x80000
static BYTE *cart_rom = NULL;

/* Cartridge States */
static enum { BUTTON_RESET, SOFTWARE_RESET } reset_mode = BUTTON_RESET;

/* ------------------------------------------------------------------------- */

/* helper pointers */
/* static BYTE *cart_rom_low; */

/* ------------------------------------------------------------------------- */

/* 0x9c00-0x9fff */
BYTE REGPARM1 finalexpansion_io3_read(WORD addr)
{
    return addr >> 8;
}

void REGPARM2 finalexpansion_io3_store(WORD addr, BYTE value)
{
}


void REGPARM2 finalexpansion_ram123_store(WORD addr, BYTE value)
{
}

BYTE REGPARM1 finalexpansion_ram123_read(WORD addr)
{
}

/* 
 * 0x2000-0x7fff
 * 0xa000-0xbfff
 */
void REGPARM2 finalexpansion_mem_store(WORD addr, BYTE value)
{
}

BYTE REGPARM1 finalexpansion_mem_read(WORD addr)
{
    return addr >> 8;
}


void finalexpansion_init(void)
{
    reset_mode = BUTTON_RESET;
}

void finalexpansion_reset(void)
{
    reset_mode = BUTTON_RESET;
}

void finalexpansion_config_setup(BYTE *rawcart)
{
}


static int zfile_load(const char *filename, BYTE *dest, size_t size)
{
    FILE *fd;

    fd = zfile_fopen(filename, MODE_READ);
    if (!fd) {
        return -1;
    }
    if (util_file_length(fd) != size) {
        zfile_fclose(fd);
        return -1;
    }
    if ( fread(dest, size, 1, fd) < 1) {
        zfile_fclose(fd);
        return -1;
    }
    zfile_fclose(fd);
    return 0;
}

int finalexpansion_bin_attach(const char *filename)
{
    if (!cart_ram) {
        cart_ram = lib_malloc(CART_RAM_SIZE);
    }
    if (!cart_rom) {
        cart_rom = lib_malloc(CART_ROM_SIZE);
    }

    if ( zfile_load(filename, cart_rom, (size_t)CART_ROM_SIZE) < 0 ) {
        finalexpansion_detach();
        return -1;
    }

    mem_cart_blocks = VIC_CART_RAM123 |
        VIC_CART_BLK1 | VIC_CART_BLK2 | VIC_CART_BLK3 | VIC_CART_BLK5 |
        VIC_CART_IO3;
    mem_initialize_memory();
    return 0;
}

void finalexpansion_detach(void)
{
    mem_cart_blocks = 0;
    mem_initialize_memory();
    lib_free(cart_ram);
    lib_free(cart_rom);
    cart_ram = NULL;
    cart_rom = NULL;
}

/* ------------------------------------------------------------------------- */
