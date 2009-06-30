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

/* Cartridge RAM */
static BYTE cart_ram[0x8000];

/* Cartridge NvRAM */
static BYTE cart_nvram[0x2000];

/* Cartridge ROM */
static BYTE cart_rom[0x200000];
static BYTE *cart_rom_low;
static BYTE *cart_rom_high;

/* Cartridge States */
static enum { BUTTON_RESET, SOFTWARE_RESET } reset_mode = BUTTON_RESET;
static int oe_flop = 0;
static int nvram_flop = 0;
static BYTE bank_reg_low = 0;
static BYTE bank_reg_high = 0;

/* ------------------------------------------------------------------------- */

/* 0x9800-0x9bff */
BYTE REGPARM1 megacart_io2_read(WORD addr)
{
    BYTE value;
    if (nvram_flop) {
        value=cart_nvram[addr & 0xfff];
    } else {
        value=addr>>8;
    }
    return value;
}

void REGPARM2 megacart_io2_store(WORD addr, BYTE value)
{
    if (nvram_flop) {
        cart_nvram[addr & 0xfff]=value;
    }
}

/* 0x9c00-0x9fff */
BYTE REGPARM1 megacart_io3_read(WORD addr)
{
    BYTE value;
    if (nvram_flop) {
        value=cart_nvram[addr & 0xfff];
    } else {
        value=addr>>8;
    }
    return value;
}

void REGPARM2 megacart_io3_store(WORD addr, BYTE value)
{
    if (nvram_flop) {
        cart_nvram[addr & 0xfff]=value;
    }

    if ((addr & 0x180) == 0x080) { /* $9c80 */
        bank_reg_high = value;
    }

    if ((addr & 0x180) == 0x100) { /* $9d00 */
        bank_reg_low = value;
    }

    if ((addr & 0x180) == 0x180) { /* $9d80 */
        nvram_flop = (value & 0x1) ? 0 : 1;
        bank_reg_high = value;
        bank_reg_low = value;
    }

    if ((addr & 0x200) == 0x200) { /* $9e00 */
        /* peform reset */
        oe_flop = !oe_flop;
        reset_mode=SOFTWARE_RESET;
        machine_trigger_reset(MACHINE_RESET_MODE_HARD);
    }
}

/* 
 * 0x0400-0x0fff
 * 0x2000-0x7fff
 * 0xa000-0xbfff
 */

void REGPARM2 megacart_mem_store(WORD addr, BYTE value)
{
    if (nvram_flop && (addr >= 0x0400 && addr < 0x1000) ) {
        cart_nvram[addr & 0x0fff] = value;
    }
    if (addr >= 0x2000 && addr < 0x8000) {
    }
    if (addr >= 0xa000 && addr < 0xc000) {
    }
}

BYTE REGPARM1 megacart_mem_read(WORD addr)
{
    BYTE bank_low;
    BYTE bank_high;

    if (addr >= 0x0400 && addr < 0x1000) {
        if (nvram_flop) {
            return cart_nvram[addr & 0x0fff];
        } else {
            return addr >> 8;
        }
    }

    bank_low=0x7f;
    bank_high=0x7f;
    if (oe_flop) {
        bank_low=bank_reg_low;
        bank_high=bank_reg_high;
    }

    if (addr >= 0x2000 && addr < 0x8000) {
        if ((bank_low & 0x80) == 0x00) {
            return cart_rom_low[(addr & 0x1fff) | (bank_low * 0x2000)];
        }
    }
    if (addr >= 0xa000 && addr < 0xc000) {
        if ((bank_high & 0x80) == 0x00) {
            return cart_rom_high[(addr & 0x1fff) | (bank_high * 0x2000)];
        } else {
            if ((bank_low & 0x80) == 0x00) {
                return cart_rom_low[(addr & 0x1fff) | (bank_low * 0x2000)];
            }
        }
    }
    return 0x00;
}


void megacart_init(void)
{
    int retval=0;

    reset_mode=BUTTON_RESET;
    oe_flop=0;
    memset(cart_rom, 1, 0x200000);
    memset(cart_nvram, 3, 0x2000);
    memset(cart_ram, 2, 0x8000);

    /* kludge: megacart.bin needs to be in the installation directory.
       1 Mbyte low rom + 1 Mbyte high rom concatenated */
    if ((retval = util_file_load("megacart.bin", cart_rom, (size_t)0x200000, UTIL_FILE_LOAD_RAW)) < 0) {
    }
    cart_rom_low=cart_rom;
    cart_rom_high=cart_rom + 0x100000;
}

void megacart_reset(void)
{
    if (reset_mode != SOFTWARE_RESET) {
        oe_flop=0;
    }
    reset_mode=BUTTON_RESET;
}

/* ------------------------------------------------------------------------- */


/* eof */
