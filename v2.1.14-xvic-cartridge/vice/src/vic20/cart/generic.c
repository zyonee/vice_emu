/*
 * generic.c -- VIC20 generic cartridge emulation.
 *
 * Written by
 *  Daniel Kahlin <daniel@kahlin.net>
 *
 * Original individual file code by
 *  André Fachat <fachat@physik.tu-chemnitz.de>
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
#include "maincpu.h"
#include "generic.h"
#include "mem.h"
#include "vic20mem.h"
#include "vic20cartmem.h"
#include "monitor.h"
#include "ram.h"
#include "resources.h"
#include "types.h"
#include "util.h"
#include "zfile.h"

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
static BYTE *cart_ram = NULL;

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
static BYTE *cart_rom = NULL;

/* Cartridge States */
int generic_ram_blocks = 0;
int generic_rom_blocks = 0;

/* filenames of separate binaries. */
static char *cartfile2 = NULL;
static char *cartfile4 = NULL;
static char *cartfile6 = NULL;
static char *cartfileA = NULL;
static char *cartfileB = NULL;

/* ------------------------------------------------------------------------- */

void REGPARM2 generic_ram123_store(WORD addr, BYTE value)
{
    if (generic_ram_blocks & VIC_CART_RAM123) {
        cart_ram[(addr & 0x0fff) | 0x8000] = value;
    }
}

BYTE REGPARM1 generic_ram123_read(WORD addr)
{
    if (generic_ram_blocks & VIC_CART_RAM123) {
        return cart_ram[(addr & 0x0fff) | 0x8000];
    }
    return cart_rom[(addr & 0x0fff) | 0x8000];
}

void REGPARM2 generic_blk1_store(WORD addr, BYTE value)
{
    if (generic_ram_blocks & VIC_CART_BLK1) {
        cart_ram[addr] = value;
    }
}

BYTE REGPARM1 generic_blk1_read(WORD addr)
{
    if (generic_ram_blocks & VIC_CART_BLK1) {
        return cart_ram[addr];
    }
    return cart_rom[addr];
}

void REGPARM2 generic_blk2_store(WORD addr, BYTE value)
{
    if (generic_ram_blocks & VIC_CART_BLK2) {
        cart_ram[addr] = value;
    }
}

BYTE REGPARM1 generic_blk2_read(WORD addr)
{
    if (generic_ram_blocks & VIC_CART_BLK2) {
        return cart_ram[addr];
    }
    return cart_rom[addr];
}

void REGPARM2 generic_blk3_store(WORD addr, BYTE value)
{
    if (generic_ram_blocks & VIC_CART_BLK3) {
        cart_ram[addr] = value;
    }
}

BYTE REGPARM1 generic_blk3_read(WORD addr)
{
    if (generic_ram_blocks & VIC_CART_BLK3) {
        return cart_ram[addr];
    }
    return cart_rom[addr];
}

void REGPARM2 generic_blk5_store(WORD addr, BYTE value)
{
    if (generic_ram_blocks & VIC_CART_BLK5) {
        cart_ram[addr & 0x1fff] = value;
    }
}

BYTE REGPARM1 generic_blk5_read(WORD addr)
{
    if (generic_ram_blocks & VIC_CART_BLK5) {
        return cart_ram[addr & 0x1fff];
    }
    return cart_rom[addr & 0x1fff];
}

/* ------------------------------------------------------------------------- */

void generic_init(void)
{
}

void generic_reset(void)
{
}

void generic_config_setup(BYTE *rawcart)
{
}


/* ------------------------------------------------------------------------- */

static int attach_image(int type, const char *filename)
{
    BYTE rawcart[0x4000];
    FILE *fd;
    int addr;
    size_t n;
    int type2 = CARTRIDGE_VIC20_DETECT;

    fd = zfile_fopen(filename, MODE_READ);
    if (!fd)
        return -1;

    addr = fgetc(fd);
    addr = (addr & 0xff) | ((fgetc(fd) << 8) & 0xff00);

    if (addr == 0x6000 || addr == 0x7000) {
        type2 = CARTRIDGE_VIC20_16KB_6000;
    } else if (addr == 0xA000) {
        type2 = CARTRIDGE_VIC20_8KB_A000;
    } else if (addr == 0x2000 || addr == 0x3000) {
        type2 = CARTRIDGE_VIC20_16KB_2000;
    } else if (addr == 0xB000) {
        type2 = CARTRIDGE_VIC20_4KB_B000;
    } else if (addr == 0x4000 || addr == 0x5000) {
        type2 = CARTRIDGE_VIC20_16KB_4000;
    }
    if (type2 == CARTRIDGE_VIC20_DETECT) {
        /* rewind to the beginning of the file (no load address) */
        fseek(fd, 0, SEEK_SET);
    }
    if (type == CARTRIDGE_VIC20_DETECT) {
        type = type2;
    }

    memset(rawcart, 0xff, 0x4000);

    switch (type) {
      case CARTRIDGE_VIC20_16KB_4000:
        if ((n = fread(rawcart, 0x1000, 4, fd)) < 1) {
            zfile_fclose(fd);
            return -1;
        }
        if (n < 4) {
            type = CARTRIDGE_VIC20_8KB_4000;
            if (n < 2) {
                memcpy(rawcart + 0x1000, rawcart, 0x1000);
            }
        }
        util_string_set(&cartfile4, filename);
        break;
      case CARTRIDGE_VIC20_16KB_2000:
        if ((n = fread(rawcart, 0x1000, 4, fd)) < 1) {
            zfile_fclose(fd);
            return -1;
        }
        if (n < 4) {
            type = CARTRIDGE_VIC20_8KB_2000;
            if (n < 2) {
                /* type = CARTRIDGE_VIC20_4KB_2000; */
                memcpy(rawcart + 0x1000, rawcart, 0x1000);
            }
        }
        util_string_set(&cartfile2, filename);
        break;
      case CARTRIDGE_VIC20_16KB_6000:
        if ((n = fread(rawcart, 0x1000, 4, fd)) < 1) {
            zfile_fclose(fd);
            return -1;
        }
        if (n < 4) {
            type = CARTRIDGE_VIC20_8KB_6000;
            if (n < 2) {
                /* type = CARTRIDGE_VIC20_4KB_6000; */
                memcpy(rawcart + 0x1000, rawcart, 0x1000);
            }
        }
        util_string_set(&cartfile6, filename);
        break;
      case CARTRIDGE_VIC20_8KB_A000:
        if ((n = fread(rawcart, 0x1000, 2, fd)) < 1) {
            zfile_fclose(fd);
            return -1;
        }
        if (n < 2) {
            if (cartfileB) {
                type = CARTRIDGE_VIC20_4KB_A000;
            } else {
                memcpy(rawcart + 0x1000, rawcart, 0x1000);
            }
        }
        util_string_set(&cartfileA, filename);
        break;
      case CARTRIDGE_VIC20_4KB_B000:
        if ((n = fread(rawcart, 0x1000, 1, fd)) < 1) {
            zfile_fclose(fd);
            return -1;
        }
        util_string_set(&cartfileB, filename);
        break;
      default:
        zfile_fclose(fd);
        return -1;
    }

    zfile_fclose(fd);

    /* attach cartridge data */
    switch (type) {
    case CARTRIDGE_VIC20_8KB_2000:
        memcpy(cart_rom+0x2000, rawcart, 0x2000);
        generic_rom_blocks |= VIC_CART_BLK1;
        break;
    case CARTRIDGE_VIC20_8KB_4000:
        memcpy(cart_rom+0x4000, rawcart, 0x2000);
        generic_rom_blocks |= VIC_CART_BLK2;
        break;
    case CARTRIDGE_VIC20_8KB_6000:
        memcpy(cart_rom+0x6000, rawcart, 0x2000);
        generic_rom_blocks |= VIC_CART_BLK3;
        break;
    case CARTRIDGE_VIC20_8KB_A000:
        memcpy(cart_rom+0x0000, rawcart, 0x2000);
        generic_rom_blocks |= VIC_CART_BLK5;
        break;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

int generic_bin_attach(int type, const char *filename)
{
    if (!cart_ram) {
        cart_ram = lib_malloc(CART_RAM_SIZE);
    }
    if (!cart_rom) {
        cart_rom = lib_malloc(CART_ROM_SIZE);
    }

    if ( attach_image(type, filename) < 0 ) {
        generic_detach();
        return -1;
    }

    mem_cart_blocks = generic_ram_blocks | generic_rom_blocks;
    mem_initialize_memory();
    return 0;
}

void generic_detach(void)
{
    generic_ram_blocks = 0;
    generic_rom_blocks = 0;
    mem_cart_blocks = 0;
    mem_initialize_memory();
    lib_free(cart_ram);
    lib_free(cart_rom);
    cart_ram = NULL;
    cart_rom = NULL;

    util_string_set(&cartfile2, NULL);
    util_string_set(&cartfile4, NULL);
    util_string_set(&cartfile6, NULL);
    util_string_set(&cartfileA, NULL);
    util_string_set(&cartfileB, NULL);
}

/* ------------------------------------------------------------------------- */


/* eof */
