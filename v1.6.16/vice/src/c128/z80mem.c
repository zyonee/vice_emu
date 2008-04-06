/*
 * z80mem.c
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

#include <stdio.h>
#include <string.h>

#include "c128mem.h"
#include "c128mmu.h"
#include "c64cia.h"
#include "cmdline.h"
#include "log.h"
#include "mem.h"
#include "resources.h"
#include "sid.h"
#include "sysfile.h"
#include "types.h"
#include "utils.h"
#include "vdc-mem.h"
#include "vdc.h"
#include "vicii-mem.h"
#include "vicii.h"
#include "z80mem.h"

/* Z80 boot BIOS.  */
static BYTE z80bios_rom[0x1000];

/* Name of the character ROM.  */
static char *z80bios_rom_name = NULL;

/* Logging.  */
static log_t z80mem_log = LOG_ERR;

/* Adjust this pointer when the MMU changes banks.  */
static BYTE **bank_base;
static int *bank_limit = NULL;
unsigned int z80_old_reg_pc;

/* Pointers to the currently used memory read and write tables.  */
read_func_ptr_t *_z80mem_read_tab_ptr;
store_func_ptr_t *_z80mem_write_tab_ptr;
BYTE **_z80mem_read_base_tab_ptr;
int *z80mem_read_limit_tab_ptr;

#define NUM_CONFIGS 8

/* Memory read and write tables.  */
static store_func_ptr_t mem_write_tab[NUM_CONFIGS][0x101];
static read_func_ptr_t mem_read_tab[NUM_CONFIGS][0x101];
static BYTE *mem_read_base_tab[NUM_CONFIGS][0x101];
static int mem_read_limit_tab[NUM_CONFIGS][0x101];

store_func_ptr_t io_write_tab[0x101];
read_func_ptr_t io_read_tab[0x101];

static int z80_rom_loaded = 0;

#define IS_NULL(s)  (s == NULL || *s == '\0')

static int z80mem_load_bios(void)
{
    if (!z80_rom_loaded)
        return 0;

    if (!IS_NULL(z80bios_rom_name)) {
        if (sysfile_load(z80bios_rom_name,
            z80bios_rom, 4096, 4096) < 0) {
            log_error(z80mem_log, "Couldn't load Z80 BIOS ROM `%s'.",
                  z80bios_rom_name);
            return -1;
        }
    }
    return 0;
}

static int set_z80bios_rom_name(resource_value_t v, void *param)
{
    const char *name = (const char *) v;

    if (z80bios_rom_name != NULL && name != NULL
        && strcmp(name, z80bios_rom_name) == 0)
        return 0;

    string_set(&z80bios_rom_name, name);

    return z80mem_load_bios();
}

static resource_t resources[] = {
    { "Z80BiosName", RES_STRING, (resource_value_t) "z80bios",
      (resource_value_t *) &z80bios_rom_name,
      set_z80bios_rom_name, NULL },
    { NULL }
};

int z80mem_init_resources(void)
{
    return resources_register(resources);
}

static cmdline_option_t cmdline_options[] =
{
    { "-z80bios", SET_RESOURCE, 1, NULL, NULL, "Z80BiosName", NULL,
      "<name>", "Specify name of Z80 BIOS ROM image" },
    { NULL }
};

int z80mem_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/* Generic memory access.  */
static void REGPARM2 z80mem_store(ADDRESS addr, BYTE value)
{
    _z80mem_write_tab_ptr[addr >> 8](addr, value);
}

static BYTE REGPARM1 z80mem_read(ADDRESS addr)
{
    return _z80mem_read_tab_ptr[addr >> 8](addr);
}

static BYTE REGPARM1 read_ram(ADDRESS addr)
{
    return ram_bank[addr];
}

static void REGPARM2 store_ram(ADDRESS addr, BYTE value)
{
    ram_bank[addr] = value;
}

static BYTE REGPARM1 read_ram0(ADDRESS addr)
{
    return ram[addr];
}

static void REGPARM2 store_ram0(ADDRESS addr, BYTE value)
{
    ram[addr] = value;
}

BYTE REGPARM1 read_bios(ADDRESS addr)
{
    return z80bios_rom[addr & 0x0fff];
}

void REGPARM2 store_bios(ADDRESS addr, BYTE value)
{
    z80bios_rom[addr] = value;
}

static BYTE REGPARM1 z80_read_zero(ADDRESS addr)
{
    return page_zero[addr];
}

static void REGPARM2 z80_store_zero(ADDRESS addr, BYTE value)
{
    page_zero[addr] = value;
}

static BYTE REGPARM1 read_one(ADDRESS addr)
{
    return page_one[addr - 0x100];
}

static void REGPARM2 store_one(ADDRESS addr, BYTE value)
{
    page_one[addr - 0x100] = value;
}

static BYTE REGPARM1 read_unconnected_io(ADDRESS addr)
{
    log_message(z80mem_log, "Read from unconnected IO %04x", addr);
    return 0;
}

static void REGPARM2 store_unconnected_io(ADDRESS addr, BYTE value)
{
    log_message(z80mem_log, "Store to unconnected IO %04x %02x", addr, value);
}

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

void z80mem_initialize(void)
{
    int i, j;

    /* Memory addess space.  */

    for (j = 0; j < NUM_CONFIGS; j++) {
        for (i = 0; i <= 0x100; i++) {
            mem_read_base_tab[j][i] = NULL;
            mem_read_limit_tab[j][i] = -1;
        }
    }

    mem_read_tab[0][0] = read_bios;
    mem_write_tab[0][0] = z80_store_zero;
    mem_read_tab[1][0] = read_bios;
    mem_write_tab[1][0] = z80_store_zero;
    mem_read_tab[2][0] = z80_read_zero;
    mem_write_tab[2][0] = z80_store_zero;
    mem_read_tab[3][0] = z80_read_zero;
    mem_write_tab[3][0] = z80_store_zero;
    mem_read_tab[4][0] = z80_read_zero;
    mem_write_tab[4][0] = z80_store_zero;
    mem_read_tab[5][0] = z80_read_zero;
    mem_write_tab[5][0] = z80_store_zero;
    mem_read_tab[6][0] = z80_read_zero;
    mem_write_tab[6][0] = z80_store_zero;
    mem_read_tab[7][0] = z80_read_zero;
    mem_write_tab[7][0] = z80_store_zero;
 
    mem_read_tab[0][1] = read_bios;
    mem_write_tab[0][1] = store_one;
    mem_read_tab[1][1] = read_bios;
    mem_write_tab[1][1] = store_one;
    mem_read_tab[2][1] = read_one;
    mem_write_tab[2][1] = store_one;
    mem_read_tab[3][1] = read_one;
    mem_write_tab[3][1] = store_one;
    mem_read_tab[4][1] = read_one;
    mem_write_tab[4][1] = store_one;
    mem_read_tab[5][1] = read_one;
    mem_write_tab[5][1] = store_one;
    mem_read_tab[6][1] = read_one;
    mem_write_tab[6][1] = store_one;
    mem_read_tab[7][1] = read_one;
    mem_write_tab[7][1] = store_one;

    for (i = 2; i < 0x10; i++) {
        mem_read_tab[0][i] = read_bios;
        mem_write_tab[0][i] = store_ram;
        mem_read_tab[1][i] = read_bios;
        mem_write_tab[1][i] = store_ram;
        mem_read_tab[2][i] = read_lo;
        mem_write_tab[2][i] = store_lo;
        mem_read_tab[3][i] = read_lo;
        mem_write_tab[3][i] = store_lo;
        mem_read_tab[4][i] = read_ram;
        mem_write_tab[4][i] = store_ram;
        mem_read_tab[5][i] = read_ram;
        mem_write_tab[5][i] = store_ram;
        mem_read_tab[6][i] = read_lo;
        mem_write_tab[6][i] = store_lo;
        mem_read_tab[7][i] = read_lo;
        mem_write_tab[7][i] = store_lo;
    }

    for (i = 0x10; i <= 0x13; i++) {
        mem_read_tab[0][i] = read_ram;
        mem_write_tab[0][i] = store_ram;
        mem_read_tab[1][i] = colorram_read;
        mem_write_tab[1][i] = colorram_store;
        mem_read_tab[2][i] = read_lo;
        mem_write_tab[2][i] = store_lo;
        mem_read_tab[3][i] = colorram_read;
        mem_write_tab[3][i] = colorram_store;
        mem_read_tab[4][i] = read_ram;
        mem_write_tab[4][i] = store_ram;
        mem_read_tab[5][i] = colorram_read;
        mem_write_tab[5][i] = colorram_store;
        mem_read_tab[6][i] = read_lo;
        mem_write_tab[6][i] = store_lo;
        mem_read_tab[7][i] = colorram_read;
        mem_write_tab[7][i] = colorram_store;
    }

    for (i = 0x14; i <= 0x3f; i++) {
        mem_read_tab[0][i] = read_ram;
        mem_write_tab[0][i] = store_ram;
        mem_read_tab[1][i] = read_ram;
        mem_write_tab[1][i] = store_ram;
        mem_read_tab[2][i] = read_lo;
        mem_write_tab[2][i] = store_lo;
        mem_read_tab[3][i] = read_lo;
        mem_write_tab[3][i] = store_lo;
        mem_read_tab[4][i] = read_ram;
        mem_write_tab[4][i] = store_ram;
        mem_read_tab[5][i] = read_ram;
        mem_write_tab[5][i] = store_ram;
        mem_read_tab[6][i] = read_lo;
        mem_write_tab[6][i] = store_lo;
        mem_read_tab[7][i] = read_lo;
        mem_write_tab[7][i] = store_lo;
    }

    for (j = 0; j < NUM_CONFIGS; j++) {
        for (i = 0x40; i <= 0xbf; i++) {
            mem_read_tab[j][i] = read_ram;
            mem_write_tab[j][i] = store_ram;
        }
    }

    for (i = 0xc0; i <= 0xcf; i++) {
        mem_read_tab[0][i] = read_ram;
        mem_write_tab[0][i] = store_ram;
        mem_read_tab[1][i] = read_ram;
        mem_write_tab[1][i] = store_ram;
        mem_read_tab[2][i] = read_top_shared;
        mem_write_tab[2][i] = store_top_shared;
        mem_read_tab[3][i] = read_top_shared;
        mem_write_tab[3][i] = store_top_shared;
        mem_read_tab[4][i] = read_ram;
        mem_write_tab[4][i] = store_ram;
        mem_read_tab[5][i] = read_ram;
        mem_write_tab[5][i] = store_ram;
        mem_read_tab[6][i] = read_top_shared;
        mem_write_tab[6][i] = store_top_shared;
        mem_read_tab[7][i] = read_top_shared;
        mem_write_tab[7][i] = store_top_shared;
    }

    for (i = 0xd0; i <= 0xdf; i++) {
        mem_read_tab[0][i] = read_ram;
        mem_write_tab[0][i] = store_ram;
        mem_read_tab[1][i] = read_ram;
        mem_write_tab[1][i] = store_ram;
        mem_read_tab[2][i] = read_top_shared;
        mem_write_tab[2][i] = store_top_shared;
        mem_read_tab[3][i] = read_top_shared;
        mem_write_tab[3][i] = store_top_shared;
        mem_read_tab[4][i] = read_ram;
        mem_write_tab[4][i] = store_ram;
        mem_read_tab[5][i] = read_ram;
        mem_write_tab[5][i] = store_ram;
        mem_read_tab[6][i] = read_top_shared;
        mem_write_tab[6][i] = store_top_shared;
        mem_read_tab[7][i] = read_top_shared;
        mem_write_tab[7][i] = store_top_shared;
    }

    for (i = 0xe0; i <= 0xfe; i++) {
        mem_read_tab[0][i] = read_ram;
        mem_write_tab[0][i] = store_ram;
        mem_read_tab[1][i] = read_ram;
        mem_write_tab[1][i] = store_ram;
        mem_read_tab[2][i] = read_top_shared;
        mem_write_tab[2][i] = store_top_shared;
        mem_read_tab[3][i] = read_top_shared;
        mem_write_tab[3][i] = store_top_shared;
        mem_read_tab[4][i] = read_ram;
        mem_write_tab[4][i] = store_ram;
        mem_read_tab[5][i] = read_ram;
        mem_write_tab[5][i] = store_ram;
        mem_read_tab[6][i] = read_top_shared;
        mem_write_tab[6][i] = store_top_shared;
        mem_read_tab[7][i] = read_top_shared;
        mem_write_tab[7][i] = store_top_shared;
    }

    for (j = 0; j < NUM_CONFIGS; j++) {
        mem_read_tab[j][0xff] = mmu_ffxx_read_z80;
        mem_write_tab[j][0xff] = mmu_ffxx_store;

        mem_read_tab[j][0x100] = mem_read_tab[j][0x0];
        mem_write_tab[j][0x100] = mem_write_tab[j][0x0];
    }

    _z80mem_read_tab_ptr = mem_read_tab[0];
    _z80mem_write_tab_ptr = mem_write_tab[0];
    _z80mem_read_base_tab_ptr = mem_read_base_tab[0];
    z80mem_read_limit_tab_ptr = mem_read_limit_tab[0];

    /* IO address space.  */

    /* At least we know what happens.  */
    for (i = 0; i <= 0x100; i++) {
        io_read_tab[i] = read_unconnected_io;
        io_write_tab[i] = store_unconnected_io;
    }
/*
    io_read_tab[0x10] = colorram_read;
    io_write_tab[0x10] = colorram_store;
    io_read_tab[0x11] = colorram_read;
    io_write_tab[0x11] = colorram_store;
    io_read_tab[0x12] = colorram_read;
    io_write_tab[0x12] = colorram_store;
    io_read_tab[0x13] = colorram_read;
    io_write_tab[0x13] = colorram_store;
*/
    io_read_tab[0xd0] = vic_read;
    io_write_tab[0xd0] = vic_store;
    io_read_tab[0xd1] = vic_read;
    io_write_tab[0xd1] = vic_store;
    io_read_tab[0xd2] = vic_read;
    io_write_tab[0xd2] = vic_store;
    io_read_tab[0xd3] = vic_read;
    io_write_tab[0xd3] = vic_store;

    io_read_tab[0xd4] = sid_read;
    io_write_tab[0xd4] = sid_store;

    io_read_tab[0xd5] = mmu_read;
    io_write_tab[0xd5] = mmu_store;

    io_read_tab[0xd6] = vdc_read;
    io_write_tab[0xd6] = vdc_store;
    io_read_tab[0xd7] = d7xx_read;
    io_write_tab[0xd7] = d7xx_store;

    io_read_tab[0xd8] = colorram_read;
    io_write_tab[0xd8] = colorram_store;
    io_read_tab[0xd9] = colorram_read;
    io_write_tab[0xd9] = colorram_store;
    io_read_tab[0xda] = colorram_read;
    io_write_tab[0xda] = colorram_store;
    io_read_tab[0xdb] = colorram_read;
    io_write_tab[0xdb] = colorram_store;

    io_read_tab[0xdc] = cia1_read;
    io_write_tab[0xdc] = cia1_store;
    io_read_tab[0xdd] = cia2_read;
    io_write_tab[0xdd] = cia2_store;

    io_read_tab[0xde] = io1_read;
    io_write_tab[0xde] = io1_store;
    io_read_tab[0xdf] = io2_read;
    io_write_tab[0xdf] = io2_store;
}

#ifdef _MSC_VER
#pragma optimize("",on)
#endif

void z80mem_set_bank_pointer(BYTE **base, int *limit)
{
    bank_base = base;
    bank_limit = limit;
}

void z80mem_update_config(int config)
{
    _z80mem_read_tab_ptr = mem_read_tab[config];
    _z80mem_write_tab_ptr = mem_write_tab[config];
    _z80mem_read_base_tab_ptr = mem_read_base_tab[config];
    z80mem_read_limit_tab_ptr = mem_read_limit_tab[config];
/*
    if (bank_limit != NULL) {
        *bank_base = _z80mem_read_base_tab_ptr[z80_old_reg_pc >> 8];
        if (*bank_base != 0)
            *bank_base = _z80mem_read_base_tab_ptr[z80_old_reg_pc >> 8]
                         - (z80_old_reg_pc & 0xff00);
        *bank_limit = z80mem_read_limit_tab_ptr[z80_old_reg_pc >> 8];
    }
*/
}

int z80mem_load(void)
{
    if (z80mem_log == LOG_ERR)
        z80mem_log = log_open("Z80MEM");

    z80mem_initialize();

    z80_rom_loaded = 1;

    if (z80mem_load_bios() < 0)
        return -1;

    return 0;
}

