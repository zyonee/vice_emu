/*
 * megacart.c -- VIC20 Mega-Cart emulation.
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
#include "machine.h"
#include "maincpu.h"
#include "megacart.h"
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
 *
 * (by reasoning around minimal decoding, may be different on actual HW)
 */
static BYTE cart_ram[0x8000];

/*
 * Cartridge NvRAM
 *
 * Mapping
 *      NvRAM                 VIC20
 *   0x0400 - 0x0fff  ->  0x0400 - 0x0fff
 *   0x1000 - 0x17ff  ->  0x9800 - 0x9fff
 *
 * (by reasoning around minimal decoding, may be different on actual HW)
 */
static BYTE cart_nvram[0x2000];

/*
 * Cartridge ROM
 *
 * Mapping
 *      ROM
 *   0x000000 - 0x0fffff  ->  Low ROM: banks 0x00-0x7f
 *   0x100000 - 0x1fffff  ->  High ROM: banks 0x00-0x7f
 *
 */
static BYTE cart_rom[0x200000];

/* Cartridge States */
static enum { BUTTON_RESET, SOFTWARE_RESET } reset_mode = BUTTON_RESET;
static int oe_flop = 0;
static int nvram_en_flop = 0;
static BYTE bank_low_reg = 0;
static BYTE bank_high_reg = 0;

/* ------------------------------------------------------------------------- */

/* helper pointers */
static BYTE *cart_rom_low;
static BYTE *cart_rom_high;

/* ------------------------------------------------------------------------- */

/* 0x9800-0x9bff */
BYTE REGPARM1 megacart_io2_read(WORD addr)
{
    BYTE value;
    if (nvram_en_flop) {
        value = cart_nvram[addr & 0x0fff];
    } else {
        value = addr >> 8;
    }
    return value;
}

void REGPARM2 megacart_io2_store(WORD addr, BYTE value)
{
    if (nvram_en_flop) {
        cart_nvram[addr & 0x0fff] = value;
    }
}

/* 0x9c00-0x9fff */
BYTE REGPARM1 megacart_io3_read(WORD addr)
{
    BYTE value;
    if (nvram_en_flop) {
        value = cart_nvram[addr & 0x0fff];
    } else {
        value = addr >> 8;
    }
    return value;
}

void REGPARM2 megacart_io3_store(WORD addr, BYTE value)
{
    if (nvram_en_flop) {
        cart_nvram[addr & 0x0fff] = value;
    }

    if ((addr & 0x180) == 0x080) { /* $9c80 */
        bank_high_reg = value;
    }

    if ((addr & 0x180) == 0x100) { /* $9d00 */
        bank_low_reg = value;
    }

    if ((addr & 0x180) == 0x180) { /* $9d80 */
        nvram_en_flop = (value & 0x1) ? 0 : 1;
        bank_high_reg = value;
        bank_low_reg = value;
    }

    if ((addr & 0x200) == 0x200) { /* $9e00 */
        /* peform reset */
        reset_mode = SOFTWARE_RESET;
        machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    }
}


/* 
 * 0x0400-0x0fff
 * 0x2000-0x7fff
 * 0xa000-0xbfff
 */
void REGPARM2 megacart_mem_store(WORD addr, BYTE value)
{
    BYTE bank_low;
    BYTE bank_high;
    int ram_low_en;
    int ram_high_en;
    int ram_wp;

    if (addr >= 0x0400 && addr < 0x1000) {
        if (nvram_en_flop) {
            cart_nvram[addr & 0x0fff] = value;
        }
    }

    /* get bank registers */
    bank_low = (oe_flop) ? bank_low_reg : 0x7f;
    bank_high = (oe_flop) ? bank_high_reg : 0x7f;

    /* determine flags from bank registers. */
    ram_low_en = (bank_low & 0x80) ? 1 : 0;
    ram_high_en = (bank_high & 0x80) ? 1 : 0;
    ram_wp = (bank_high & 0x40) ? 0 : 1;

    if (addr >= 0x2000 && addr < 0x8000) {
        if (!ram_wp && (ram_low_en && ram_high_en) ) {
            cart_ram[addr] = value;
        }
    }
    if (addr >= 0xa000 && addr < 0xc000) {
        if (!ram_wp && (ram_low_en && ram_high_en) ) {
            cart_ram[addr & 0x1fff] = value;
        }
    }
}

BYTE REGPARM1 megacart_mem_read(WORD addr)
{
    BYTE bank_low;
    BYTE bank_high;
    int ram_low_en;
    int ram_high_en;

    if (addr >= 0x0400 && addr < 0x1000) {
        if (nvram_en_flop) {
            return cart_nvram[addr & 0x0fff];
        } else {
            return addr >> 8;
        }
    }

    /* get bank registers */
    bank_low = (oe_flop) ? bank_low_reg : 0x7f;
    bank_high = (oe_flop) ? bank_high_reg : 0x7f;

    /* determine flags from bank registers. */
    ram_low_en = (bank_low & 0x80) ? 1 : 0;
    ram_high_en = (bank_high & 0x80) ? 1 : 0;

    if (addr >= 0x2000 && addr < 0x8000) {
        if (!ram_low_en) {
            return cart_rom_low[(addr & 0x1fff) | (bank_low * 0x2000)];
        } else {
            if (ram_high_en) {
                return cart_ram[addr];
            } else {
                return addr >> 8;
            }
        }
    }
    if (addr >= 0xa000 && addr < 0xc000) {
        if (!ram_high_en) {
            return cart_rom_high[(addr & 0x1fff) | (bank_high * 0x2000)];
        } else {
            if (!ram_low_en) {
                return cart_rom_low[(addr & 0x1fff) | (bank_low * 0x2000)];
            } else {
                return cart_ram[addr & 0x1fff];
            }
        }
    }
    return 0x00; /* should never happen */
}


void megacart_init(void)
{
    int retval = 0;

    reset_mode = BUTTON_RESET;
    oe_flop = 0;

    memset(cart_rom, 0, 0x200000);
    memset(cart_nvram, 0, 0x2000);
    memset(cart_ram, 0, 0x8000);

    /* kludge: megacart.bin needs to be in the installation directory.
       1 Mbyte low rom + 1 Mbyte high rom concatenated */
    if ((retval = util_file_load("megacart.bin", cart_rom, (size_t)0x200000, UTIL_FILE_LOAD_RAW)) < 0) {
    }
    cart_rom_low = cart_rom;
    cart_rom_high = cart_rom + 0x100000;
}

void megacart_reset(void)
{
    if (reset_mode == SOFTWARE_RESET) {
        oe_flop = !oe_flop;
    } else {
        oe_flop = 0;
    }
    reset_mode = BUTTON_RESET;
}

/* ------------------------------------------------------------------------- */


/* eof */
