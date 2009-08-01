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
#include "flash040.h"
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

/* Cartridge States */
static flash040_context_t flash_state;
static enum { BUTTON_RESET, SOFTWARE_RESET } reset_mode = BUTTON_RESET;
static BYTE register_a;
static BYTE register_b;
static BYTE lock_bit;


/* ------------------------------------------------------------------------- */

/* Register A ($9c02) */

#define MODE_MASK        0xe0
#define MODE_START       0x00  /* Start Modus         (000zzzzz) [Mod_ROM_A] */
#define MODE_FLASH_WRITE 0x20  /* Flash-Schreib-Modus (001zzzzz) [Mod_Prog]  */
#define MODE_FLASH_READ  0x40  /* Flash-Lese-Modus    (010zzzzz) [Mod_ROM]   */
#define MODE_RAM         0x80  /* RAM Modus           (100zzzzz) [Mod_RAM]   */
#define MODE_SUPER_RAM   0xa0  /* Super RAM Modus     (101zzzzz) [Mod_RAM2]  */


/* Register B ($9c03) */
#define REGB_BLK0_OFF    0x01
#define REGB_BLK1_OFF    0x02
#define REGB_BLK2_OFF    0x04
#define REGB_BLK3_OFF    0x08
#define REGB_BLK5_OFF    0x10
#define REGB_INV_A13     0x20
#define REGB_INV_A14     0x40
#define REGB_REG_OFF     0x80

/* ------------------------------------------------------------------------- */

/* helper pointers */
/* static BYTE *cart_rom_low; */


/* ------------------------------------------------------------------------- */

/* 0x9c00-0x9fff */
static int is_locked(void)
{
    if (register_b & REGB_REG_OFF) {
        return 1;
    }
    if ((register_a & MODE_MASK) == MODE_START) {
        return lock_bit;
    }
    return 0;
}

BYTE REGPARM1 finalexpansion_io3_read(WORD addr)
{   
    addr &= 0x03;
    if (!is_locked()) {
        switch (addr) {
        case 0x02:
            return register_a;
        case 0x03:
            return register_b;
        }
    }
    return addr >> 8;
}

void REGPARM2 finalexpansion_io3_store(WORD addr, BYTE value)
{
    addr &= 0x03;
    if (!is_locked()) {
        switch (addr) {
        case 0x02:
            register_a = value;
        case 0x03:
            register_b = value;
        }
    }
}

/* ------------------------------------------------------------------------- */

void REGPARM2 finalexpansion_ram123_store(WORD addr, BYTE value)
{
    if ( !(register_b & REGB_BLK0_OFF) ) {

    }
}

BYTE REGPARM1 finalexpansion_ram123_read(WORD addr)
{
    if ( !(register_b & REGB_BLK0_OFF) ) {

    }
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

void REGPARM2 finalexpansion_blk5_store(WORD addr, BYTE value)
{
    int bank;

    lock_bit = 0;

    switch (register_a & MODE_MASK) {
    case MODE_FLASH_WRITE:
    case MODE_SUPER_RAM:
        bank = register_a & 0x1f;
        break;
    default:
        bank = 1;
        break;
    }

    addr = ((addr & 0x1fff) | 0x6000) | (bank * 0x8000);

    switch (register_a & MODE_MASK) {
    case MODE_FLASH_WRITE:
        flash040core_store(&flash_state, addr, value);
        break;
    case MODE_START:
    case MODE_FLASH_READ:
    case MODE_RAM:
    case MODE_SUPER_RAM:
        cart_ram[addr] = value;
        break;
    default:
        break;
    }
}

BYTE REGPARM1 finalexpansion_blk5_read(WORD addr)
{
    BYTE value;
    int bank;

    lock_bit = 1;

    switch (register_a & MODE_MASK) {
    case MODE_START:
    case MODE_FLASH_READ:
    case MODE_FLASH_WRITE:
    case MODE_SUPER_RAM:
        bank = register_a & 0x1f;
        break;
    default:
        bank = 1;
        break;
    }

    addr = ((addr & 0x1fff) | 0x6000) | (bank * 0x8000);

    switch (register_a & MODE_MASK) {
    case MODE_START:
    case MODE_FLASH_READ:
    case MODE_FLASH_WRITE:
        value = flash040core_read(&flash_state, addr);
        break;
    case MODE_RAM:
    case MODE_SUPER_RAM:
        value = cart_ram[addr];
        break;
    default:
        value = addr >> 8;
        break;
    }
    return value;
}

void finalexpansion_init(void)
{
    reset_mode = BUTTON_RESET;
    register_a = 0x00;
    register_b = 0x00;
    lock_bit = 1;
}

void finalexpansion_reset(void)
{
    reset_mode = BUTTON_RESET;
    register_a = 0x00;
    register_b = 0x00;
    lock_bit = 1;
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
    /* should probably guard this */
    flash040core_init(&flash_state, FLASH040_TYPE_B);

    if ( zfile_load(filename, flash_state.flash_data, (size_t)CART_ROM_SIZE) < 0 ) {
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
    flash040core_shutdown(&flash_state);
    lib_free(cart_ram);
    cart_ram = NULL;
}

/* ------------------------------------------------------------------------- */
