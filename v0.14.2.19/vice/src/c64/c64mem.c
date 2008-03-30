/*
 * c64mem.c -- C64 memory handling.
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

#ifndef CBM64
#define	CBM64		/* for mkdep */
#endif

#include "vice.h"

#include <stdio.h>

#include "vmachine.h"
#include "interrupt.h"
#include "true1541.h"
#include "c64mem.h"
#include "resources.h"
#include "cmdline.h"
#include "vicii.h"
#include "cia.h"
#include "sid.h"
#include "stdlib.h"
#include "memutils.h"
#include "maincpu.h"
#include "parallel.h"
#include "tpi.h"
#include "reu.h"
#include "tapeunit.h"
#include "mon.h"
#include "utils.h"
#include "patchrom.h"

/* ------------------------------------------------------------------------- */

/* C64 memory-related resources.  */

/* Name of the character ROM.  */
static char *chargen_rom_name;

/* Name of the BASIC ROM.  */
static char *basic_rom_name;

/* Name of the Kernal ROM.  */
static char *kernal_rom_name;

/* Kernal revision for ROM patcher.  */
static char *kernal_revision;

/* Flag: Do we enable the Emulator ID?  */
static int emu_id_enabled;

/* Flag: Do we enable the IEEE488 interface emulation?  */
static int ieee488_enabled;

/* Flag: Do we enable the internal REU?  */
static int reu_enabled;

/* FIXME: Should load the new character ROM.  */
static int set_chargen_rom_name(resource_value_t v)
{
    const char *name = (const char *) v;

    if (chargen_rom_name != NULL && name != NULL
        && strcmp(name, chargen_rom_name) == 0)
        return 0;

    string_set(&chargen_rom_name, name);
    return 0;
}

/* FIXME: Should load the new Kernal ROM.  */
static int set_kernal_rom_name(resource_value_t v)
{
    const char *name = (const char *) v;

    if (kernal_rom_name != NULL && name != NULL
        && strcmp(name, kernal_rom_name) == 0)
        return 0;

    string_set(&kernal_rom_name, name);
    return 0;
}

/* FIXME: Should load the new BASIC ROM.  */
static int set_basic_rom_name(resource_value_t v)
{
    const char *name = (const char *) v;

    if (basic_rom_name != NULL && name != NULL
        && strcmp(name, basic_rom_name) == 0)
        return 0;

    string_set(&basic_rom_name, name);
    return 0;
}

static int set_emu_id_enabled(resource_value_t v)
{
    emu_id_enabled = (int) v;
    return 0;
}

static int set_ieee488_enabled(resource_value_t v)
{
    if (!(int)v) {
        ieee488_enabled = 0;
        return 0;
    } else if (!reu_enabled) {
        ieee488_enabled = 1;
        return 0;
    } else {
	/* The REU and the IEEE488 interface share the same address space, so
	   they cannot be enabled at the same time.  */
        return -1;
    }
}

/* FIXME: Should initialize the REU when turned on.  */
static int set_reu_enabled(resource_value_t v)
{
    if (!(int)v) {
        reu_enabled = 0;
        return 0;
    } else if (!ieee488_enabled) {
        reu_enabled = 1;
        return 0;
    } else {
	/* The REU and the IEEE488 interface share the same address space, so
	   they cannot be enabled at the same time.  */
        return -1;
    }
}

/* FIXME: Should patch the ROM on-the-fly.  */
static int set_kernal_revision(resource_value_t v)
{
    const char *rev = (const char *) v;

    string_set(&kernal_revision, rev);
    return 0;
}

static resource_t resources[] = {
    { "ChargenName", RES_STRING, (resource_value_t) "chargen",
      (resource_value_t *) &chargen_rom_name, set_chargen_rom_name },
    { "KernalName", RES_STRING, (resource_value_t) "kernal",
      (resource_value_t *) &kernal_rom_name, set_kernal_rom_name },
    { "BasicName", RES_STRING, (resource_value_t) "basic",
      (resource_value_t *) &basic_rom_name, set_basic_rom_name },
    { "REU", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &reu_enabled, set_reu_enabled },
    { "IEEE488", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &ieee488_enabled, set_ieee488_enabled },
    { "EmuID", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &emu_id_enabled, set_emu_id_enabled },
    { "KernalRev", RES_STRING, (resource_value_t) NULL,
      (resource_value_t *) &kernal_revision, set_kernal_revision },
    { NULL }
};

int c64_mem_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------- */

/* C64 memory-related command-line options.  */
/* FIXME: Maybe the `-kernal', `-basic' and `-chargen' options should not
   really affect resources.  */

static cmdline_option_t cmdline_options[] = {
    { "-kernal", SET_RESOURCE, 1, NULL, NULL, "KernalName", NULL,
      "<name>", "Specify name of Kernal ROM image" },
    { "-basic", SET_RESOURCE, 1, NULL, NULL, "BasicName", NULL,
      "<name>", "Specify name of BASIC ROM image" },
    { "-chargen", SET_RESOURCE, 1, NULL, NULL, "ChargenName", NULL,
      "<name>", "Specify name of character generator ROM image" },
    { "-reu", SET_RESOURCE, 0, NULL, NULL, "REU", (resource_value_t) 1,
      NULL, "Enable the 512K RAM expansion unit" },
    { "+reu", SET_RESOURCE, 0, NULL, NULL, "REU", (resource_value_t) 0,
      NULL, "Disable the 512K RAM expansion unit" },
    { "-emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (resource_value_t) 1,
      NULL, "Enable emulator identification" },
    { "+emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (resource_value_t) 0,
      NULL, "Disable emulator identification" },
    { "-ieee488", SET_RESOURCE, 0, NULL, NULL, "IEEE488", (resource_value_t) 1,
      NULL, "Enable the IEEE488 interface emulation" },
    { "+ieee488", SET_RESOURCE, 0, NULL, NULL, "IEEE488", (resource_value_t) 0,
      NULL, "Disable the IEEE488 interface emulation" },
    { "-kernalrev", SET_RESOURCE, 1, NULL, NULL, "KernalRev", NULL,
      "<revision>", "Patch the Kernal ROM to the specified <revision>" },
    { NULL }
};

int c64_mem_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/* Number of possible memory configurations.  */
#define NUM_CONFIGS	8
/* Number of possible video banks (16K each).  */
#define NUM_VBANKS	4

/* The C64 memory. */
BYTE ram[C64_RAM_SIZE];
BYTE basic_rom[C64_BASIC_ROM_SIZE];
BYTE kernal_rom[C64_KERNAL_ROM_SIZE];
BYTE chargen_rom[C64_CHARGEN_ROM_SIZE];

/* Size of */
int ram_size = C64_RAM_SIZE;

/* Flag: nonzero if the Kernal and BASIC ROMs have been loaded. */
int rom_loaded = 0;

/* Pointers to the currently used memory read and write tables. */
read_func_ptr_t *_mem_read_tab_ptr;
store_func_ptr_t *_mem_write_tab_ptr;
BYTE **_mem_read_base_tab_ptr;

/* Memory read and write tables. */
static store_func_ptr_t mem_write_tab[NUM_VBANKS][NUM_CONFIGS][0x101];
static read_func_ptr_t mem_read_tab[NUM_CONFIGS][0x101];
static BYTE *mem_read_base_tab[NUM_CONFIGS][0x101];

static store_func_ptr_t mem_write_tab_watch[NUM_VBANKS][NUM_CONFIGS][0x101];
static read_func_ptr_t mem_read_tab_watch[NUM_CONFIGS][0x101];

/* Processor port. */
static struct {
    BYTE dir, data;
} pport;

/* Current video bank (0, 1, 2 or 3). */
static int vbank;

/* Current memory configuration. */
static int mem_config;

/* Emulation identification string. */
#if defined (EMULATOR_ID)
static BYTE emulator_id[] = {
    0x56, 0x49, 0x43, 0x45, 0x20, 0x30, 0x2e, 0x31,
    0x34, 0x0d, 0x28, 0x43, 0x29, 0x20, 0x31, 0x39, 	/* VICE 0.14.(C) 19 */
    0x39, 0x33, 0x2d, 0x31, 0x39, 0x39, 0x38, 0x20,
    0x45, 0x2e, 0x50, 0x45, 0x52, 0x41, 0x5a, 0x5a, 	/* 93-1998 E.PERAZZ */
    0x4f, 0x4c, 0x49, 0x2f, 0x54, 0x2e, 0x52, 0x41,
    0x4e, 0x54, 0x41, 0x4e, 0x45, 0x4e, 0x2f, 0x41, 	/* OLI/T.RANTANEN/A */
    0x2e, 0x46, 0x41, 0x43, 0x48, 0x41, 0x54, 0x2f,
    0x4a, 0x2e, 0x56, 0x41, 0x4c, 0x54, 0x41, 0x2f, 	/* .FACHAT/J.VALTA/ */
    0x4a, 0x2e, 0x53, 0x4f, 0x4e, 0x4e, 0x49, 0x4e,
    0x45, 0x4e, 0x2f, 0x44, 0x2e, 0x53, 0x4c, 0x41, 	/* J.SONNINEN/D.SLA */
    0x44, 0x49, 0x43, 0x0d, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x14, 0x56, 0x55	/* DIC...........VU */
};
#endif

/* Tape sense status: 1 = some button pressed, 0 = no buttons pressed. */
static int tape_sense = 0;

/* ------------------------------------------------------------------------- */

BYTE REGPARM1 read_zero_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_zero(addr);
}

BYTE REGPARM1 read_ram_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_ram(addr);
}

BYTE REGPARM1 read_basic_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_basic(addr);
}

BYTE REGPARM1 read_chargen_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_chargen(addr);
}

BYTE REGPARM1 read_io2_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_io2(addr);
}

BYTE REGPARM1 read_kernal_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_kernal(addr);
}

BYTE REGPARM1 read_sid_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_sid(addr);
}

BYTE REGPARM1 read_vic_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_vic(addr);
}

BYTE REGPARM1 read_colorram_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_colorram(addr);
}

BYTE REGPARM1 read_cia1_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_cia1(addr);
}

BYTE REGPARM1 read_cia2_watch(ADDRESS addr)
{
    watch_push_load_addr(addr,e_comp_space);
    return read_cia2(addr);
}

void REGPARM2 store_zero_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_zero(addr, value);
}

void REGPARM2 store_ram_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_ram(addr, value);
}

void REGPARM2 store_ram_hi_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_ram_hi(addr, value);
}

void REGPARM2 store_io2_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_io2(addr, value);
}

void REGPARM2 store_sid_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_sid(addr, value);
}

void REGPARM2 store_vbank_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_vbank(addr, value);
}

void REGPARM2 store_vbank_39xx_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_vbank_39xx(addr, value);
}

void REGPARM2 store_vbank_3fxx_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_vbank_3fxx(addr, value);
}

void REGPARM2 store_vic_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_vic(addr, value);
}

void REGPARM2 store_colorram_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_colorram(addr, value);
}

void REGPARM2 store_cia1_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_cia1(addr, value);
}

void REGPARM2 store_cia2_watch(ADDRESS addr, BYTE value)
{
    watch_push_store_addr(addr,e_comp_space);
    store_cia2(addr, value);
}

/* ------------------------------------------------------------------------- */

static inline void pport_changed(void)
{
    mem_config = (~pport.dir | pport.data) & 0x7;

    /* Bit 4: tape sense.  0 = some button pressed, 1 = no buttons pressed. */
    if (tape_sense)
	ram[1] = (pport.data | ~pport.dir) & 0x2f;
    else
	ram[1] = (pport.data | ~pport.dir) & 0x3f;

    ram[0] = pport.dir;

    if (any_watchpoints_load(e_comp_space)
	|| any_watchpoints_store(e_comp_space)) {
       _mem_read_tab_ptr = mem_read_tab_watch[mem_config];
       _mem_write_tab_ptr = mem_write_tab_watch[vbank][mem_config];
    } else {
       _mem_read_tab_ptr = mem_read_tab[mem_config];
       _mem_write_tab_ptr = mem_write_tab[vbank][mem_config];
    }

    _mem_read_base_tab_ptr = mem_read_base_tab[mem_config];
}

void mem_toggle_watchpoints(int flag)
{
    if (flag) {
        _mem_read_tab_ptr = mem_read_tab_watch[mem_config];
        _mem_write_tab_ptr = mem_write_tab_watch[vbank][mem_config];
    } else {
        _mem_read_tab_ptr = mem_read_tab[mem_config];
        _mem_write_tab_ptr = mem_write_tab[vbank][mem_config];
    }
}

BYTE REGPARM1 read_zero(ADDRESS addr)
{
    return ram[addr & 0xff];
}

void REGPARM2 store_zero(ADDRESS addr, BYTE value)
{
    addr &= 0xff;

    switch((BYTE)addr) {
      case 0:
	if (pport.dir != value) {
	    pport.dir = value;
	    pport_changed();
	}
	break;
      case 1:
	if (pport.data != value) {
	    pport.data = value;
	    pport_changed();
	}
	break;
      default:
	ram[addr] = value;
    }
}

BYTE REGPARM1 read_basic(ADDRESS addr)
{
    return basic_rom[addr & 0x1fff];
}

BYTE REGPARM1 read_kernal(ADDRESS addr)
{
    return kernal_rom[addr & 0x1fff];
}

BYTE REGPARM1 read_chargen(ADDRESS addr)
{
    return chargen_rom[addr & 0xfff];
}

BYTE REGPARM1 read_ram(ADDRESS addr)
{
    return ram[addr];
}

void REGPARM2 store_ram(ADDRESS addr, BYTE value)
{
    ram[addr] = value;
}

void REGPARM2 store_ram_hi(ADDRESS addr, BYTE value)
{
    ram[addr] = value;
    if (reu_enabled && addr == 0xff00)
	reu_dma(-1);
}

void REGPARM2 store_io2(ADDRESS addr, BYTE value)
{
    if ((addr & 0xff00) == 0xdf00) {
	if (reu_enabled)
	    store_reu(addr & 0x0f, value);
	if (ieee488_enabled)
	    store_tpi(addr & 0x07, value);
    }

    return;
}

BYTE REGPARM1 read_io2(ADDRESS addr)
{
    if (emu_id_enabled && addr >= 0xdfa0) {
	addr &= 0xff;
	if (addr == 0xff)
	    emulator_id[addr - 0xa0] ^= 0xff;
	return emulator_id[addr - 0xa0];
    }
    if ((addr & 0xff00) == 0xdf00) {
	if (reu_enabled)
	    return read_reu(addr & 0x0f);
	if(ieee488_enabled)
	    return read_tpi(addr & 0x07);
    }

    return rand();
}

BYTE REGPARM1 read_rom(ADDRESS addr)
{
    switch (addr & 0xf000) {
      case 0xa000:
      case 0xb000:
	return read_basic(addr);
      case 0xd000:
	return read_chargen(addr);
      case 0xe000:
      case 0xf000:
	return read_kernal(addr);
    }

    return 0;
}

void REGPARM2 store_rom(ADDRESS addr, BYTE value)
{
    switch (addr & 0xf000) {
      case 0xa000:
      case 0xb000:
	basic_rom[addr & 0x1fff] = value;
	break;
      case 0xd000:
	chargen_rom[addr & 0x0fff] = value;
	break;
      case 0xe000:
      case 0xf000:
	kernal_rom[addr & 0x1fff] = value;
	break;
    }
}

/* ------------------------------------------------------------------------- */

/* Generic memory access.  */

void REGPARM2 mem_store(ADDRESS addr, BYTE value)
{
    _mem_write_tab_ptr[addr >> 8](addr, value);
}

BYTE REGPARM1 mem_read(ADDRESS addr)
{
    return _mem_read_tab_ptr[addr >> 8](addr);
}

/* ------------------------------------------------------------------------- */

static void set_write_hook(int config, int page, store_func_t *f,
                           store_func_t *f2)
{
    int i;

    for (i = 0; i < NUM_VBANKS; i++) {
	mem_write_tab[i][config][page] = f;
	mem_write_tab_watch[i][config][page] = f2;
    }
}

void initialize_memory(void)
{
    int i, j, k;

    /* Default is RAM. */
    for (i = 0; i < NUM_CONFIGS; i++) {
	set_write_hook(i, 0, store_zero, store_zero_watch);
	mem_read_tab[i][0] = read_zero;
	mem_read_tab_watch[i][0] = read_zero_watch;
	mem_read_base_tab[i][0] = ram;
	for (j = 1; j <= 0xfe; j++) {
	    mem_read_tab[i][j] = read_ram;
	    mem_read_tab_watch[i][j] = read_ram_watch;
	    mem_read_base_tab[i][j] = ram + (j << 8);
	    for (k = 0; k < NUM_VBANKS; k++) {
		if ((j & 0xc0) == (k << 6)) {
                    switch (j & 0x3fff) {
                      case 0x39:
                        mem_write_tab[k][i][j] = store_vbank_39xx;
                        mem_write_tab_watch[k][i][j] = store_vbank_39xx_watch;
                        break;
                      case 0x3f:
                        mem_write_tab[k][i][j] = store_vbank_3fxx;
                        mem_write_tab_watch[k][i][j] = store_vbank_3fxx_watch;
                        break;
                      default:
                        mem_write_tab[k][i][j] = store_vbank;
                        mem_write_tab_watch[k][i][j] = store_vbank_watch;
                    }
		} else {
		    mem_write_tab[k][i][j] = store_ram;
		    mem_write_tab_watch[k][i][j] = store_ram_watch;
                }
	    }
	}
	mem_read_tab[i][0xff] = read_ram;
	mem_read_tab_watch[i][0xff] = read_ram_watch;
	mem_read_base_tab[i][0xff] = ram + 0xff00;

	/* FIXME: we do not care about vbank writes here, but we probably
           should.  Anyway, the $FFxx addresses are not so likely to contain
           sprites or other stuff that really needs the special handling, and
           it's much easier this way. */
	set_write_hook(i, 0xff, store_ram_hi, store_ram_hi_watch);
    }

    /* Setup BASIC ROM at $A000-$BFFF (memory configs 3, 7). */
    for (i = 0xa0; i <= 0xbf; i++) {
	mem_read_tab[3][i] = read_basic;
	mem_read_tab[7][i] = read_basic;
	mem_read_tab_watch[3][i] = read_basic_watch;
	mem_read_tab_watch[7][i] = read_basic_watch;
	mem_read_base_tab[3][i] = basic_rom + ((i & 0x1f) << 8);
	mem_read_base_tab[7][i] = basic_rom + ((i & 0x1f) << 8);
    }

    /* Setup character generator ROM at $D000-$DFFF (memory configs 1, 2,
       3). */
    for (i = 0xd0; i <= 0xdf; i++) {
	mem_read_tab[1][i] = read_chargen;
	mem_read_tab[2][i] = read_chargen;
	mem_read_tab[3][i] = read_chargen;
	mem_read_tab_watch[1][i] = read_chargen_watch;
	mem_read_tab_watch[2][i] = read_chargen_watch;
	mem_read_tab_watch[3][i] = read_chargen_watch;
	mem_read_base_tab[1][i] = chargen_rom + ((i & 0x0f) << 8);
	mem_read_base_tab[2][i] = chargen_rom + ((i & 0x0f) << 8);
	mem_read_base_tab[3][i] = chargen_rom + ((i & 0x0f) << 8);
    }

    /* Setup I/O at $D000-$DFFF (memory configs 5, 6, 7). */
    for (j = 5; j <= 7; j++) {
	for (i = 0xd0; i <= 0xd3; i++) {
	    mem_read_tab[j][i] = read_vic;
	    mem_read_tab_watch[j][i] = read_vic_watch;
	    set_write_hook(j, i, store_vic, store_vic_watch);
	}
	for (i = 0xd4; i <= 0xd7; i++) {
	    mem_read_tab[j][i] = read_sid;
	    mem_read_tab_watch[j][i] = read_sid_watch;
	    set_write_hook(j, i, store_sid, store_sid_watch);
	}
	for (i = 0xd8; i <= 0xdb; i++) {
	    mem_read_tab[j][i] = read_colorram;
	    mem_read_tab_watch[j][i] = read_colorram_watch;
	    set_write_hook(j, i, store_colorram, store_colorram_watch);
	}

	mem_read_tab[j][0xdc] = read_cia1;
	mem_read_tab_watch[j][0xdc] = read_cia1_watch;
	set_write_hook(j, 0xdc, store_cia1, store_cia1_watch);
	mem_read_tab[j][0xdd] = read_cia2;
	mem_read_tab_watch[j][0xdd] = read_cia2_watch;
	set_write_hook(j, 0xdd, store_cia2, store_cia2_watch);

	mem_read_tab[j][0xde] = read_io2;
	mem_read_tab_watch[j][0xde] = read_io2_watch;
	set_write_hook(j, 0xde, store_io2, store_io2_watch);
	mem_read_tab[j][0xdf] = read_io2;
	mem_read_tab_watch[j][0xdf] = read_io2_watch;
	set_write_hook(j, 0xdf, store_io2, store_io2_watch);

	for (i = 0xd0; i <= 0xdf; i++)
	    mem_read_base_tab[j][i] = NULL;
    }

    /* Setup Kernal ROM at $E000-$FFFF (memory configs 2, 3, 6, 7). */
    for (i = 0xe0; i <= 0xff; i++) {
	mem_read_tab[2][i] = read_kernal;
	mem_read_tab[3][i] = read_kernal;
	mem_read_tab[6][i] = read_kernal;
	mem_read_tab[7][i] = read_kernal;
	mem_read_tab_watch[2][i] = read_kernal_watch;
	mem_read_tab_watch[3][i] = read_kernal_watch;
	mem_read_tab_watch[6][i] = read_kernal_watch;
	mem_read_tab_watch[7][i] = read_kernal_watch;
	mem_read_base_tab[2][i] = kernal_rom + ((i & 0x1f) << 8);
	mem_read_base_tab[3][i] = kernal_rom + ((i & 0x1f) << 8);
	mem_read_base_tab[6][i] = kernal_rom + ((i & 0x1f) << 8);
	mem_read_base_tab[7][i] = kernal_rom + ((i & 0x1f) << 8);
    }

    for (i = 0; i < 8; i++) {
	mem_read_tab[i][0x100] = mem_read_tab[i][0];
	mem_read_tab_watch[i][0x100] = mem_read_tab_watch[i][0];
	for (j = 0; j < NUM_VBANKS; j++) {
	    mem_write_tab[j][i][0x100] = mem_write_tab[j][i][0];
	    mem_write_tab_watch[j][i][0x100] = mem_write_tab_watch[j][i][0];
        }
	mem_read_base_tab[i][0x100] = mem_read_base_tab[i][0];
    }

    _mem_read_tab_ptr = mem_read_tab[7];
    _mem_write_tab_ptr = mem_write_tab[vbank][7];
    _mem_read_base_tab_ptr = mem_read_base_tab[7];

    pport.data = 0xff;
    pport.dir = 0x0;

    /* Setup initial memory configuration. */
    pport_changed();
}

/* ------------------------------------------------------------------------- */

/* Initialize RAM for power-up. */
void mem_powerup(void)
{
    int i;

#ifndef __MSDOS__
    printf("Initializing RAM for power-up...\n");
#endif

    for (i = 0; i < 0x10000; i += 0x80) {
	memset(ram + i, 0, 0x40);
	memset(ram + i + 0x40, 0xff, 0x40);
    }
}

/* Load ROMs at startup.  This is half-stolen from the old `load_mem()' in
   `memory.c'. */
int mem_load(void)
{
    WORD sum;			/* ROM checksum */
    int	id;			/* ROM identification number */
    int i;

    mem_powerup();

    /* Load Kernal ROM. */
    if (mem_load_sys_file(kernal_rom_name,
			  kernal_rom, C64_KERNAL_ROM_SIZE,
			  C64_KERNAL_ROM_SIZE) < 0) {
	fprintf(stderr,"Couldn't load kernal ROM `%s'.\n",
                kernal_rom_name);
	return -1;
    }

    /* Check Kernal ROM.  */
    for (i = 0, sum = 0; i < C64_KERNAL_ROM_SIZE; i++)
	sum += kernal_rom[i];

    id = read_rom(0xff80);

    printf("Kernal rev #%d.\n", id);

    if ((id == 0
	 && sum != C64_KERNAL_CHECKSUM_R00)
	|| (id == 3
	    && sum != C64_KERNAL_CHECKSUM_R03
	    && sum != C64_KERNAL_CHECKSUM_R03swe)
	|| (id == 0x43
	    && sum != C64_KERNAL_CHECKSUM_R43)
	|| (id == 0x64
	    && sum != C64_KERNAL_CHECKSUM_R64)) {
	fprintf(stderr,
                "Warning: Unknown Kernal image `%s'.  Sum: %d ($%04X)\n",
		kernal_rom_name, sum, sum);
    } else if (kernal_revision != NULL) {
        if (patch_rom(kernal_revision) < 0)
            return -1;
    }

    /* Load Basic ROM. */
    if (mem_load_sys_file(basic_rom_name,
			  basic_rom, C64_BASIC_ROM_SIZE,
			  C64_BASIC_ROM_SIZE) < 0) {
	fprintf(stderr, "Couldn't load basic ROM `%s'.\n",
                basic_rom_name);
	return -1;
    }

    /* Check Basic ROM. */

    for (i = 0, sum = 0; i < C64_BASIC_ROM_SIZE; i++)
	sum += basic_rom[i];

    if (sum != C64_BASIC_CHECKSUM)
	fprintf(stderr,
                "Warning: Unknown Basic image `%s'.  Sum: %d ($%04X)\n",
		basic_rom_name, sum, sum);

    /* Load chargen ROM. */

    if (mem_load_sys_file(chargen_rom_name,
			  chargen_rom, C64_CHARGEN_ROM_SIZE,
			  C64_CHARGEN_ROM_SIZE) < 0) {
	fprintf(stderr, "Couldn't load character ROM `%s'.\n",
                chargen_rom_name);
	return -1;
    }

    rom_loaded = 1;

    return 0;
}

/* ------------------------------------------------------------------------- */

/* Change the current video bank. */
void mem_set_vbank(int new_vbank)
{
    if (new_vbank != vbank) {
	vbank = new_vbank;
	_mem_write_tab_ptr = mem_write_tab[new_vbank][mem_config];
	vic_ii_set_vbank(new_vbank);
    }
}

/* Set the tape sense status. */
void mem_set_tape_sense(int sense)
{
    tape_sense = sense;
    pport_changed();
}

/* Enable/disable the REU.  FIXME: should initialize the REU if necessary?  */
void mem_toggle_reu(int flag)
{
    reu_enabled = flag;
}

/* Enable/disable the IEEE488 interface.  */
void mem_toggle_ieee488(int flag)
{
    ieee488_enabled = flag;
}

/* Enable/disable the Emulator ID.  */
void mem_toggle_emu_id(int flag)
{
    emu_id_enabled = flag;
}

/* ------------------------------------------------------------------------- */

/* FIXME: this part needs to be checked. */

void mem_get_basic_text(ADDRESS *start, ADDRESS *end)
{
    if (start != NULL)
	*start = ram[0x2b] | (ram[0x2c] << 8);
    if (end != NULL)
	*end = ram[0x2d] | (ram[0x2e] << 8);
}

void mem_set_basic_text(ADDRESS start, ADDRESS end)
{
    ram[0x2b] = ram[0xac] = start & 0xff;
    ram[0x2c] = ram[0xad] = start >> 8;
    ram[0x2d] = ram[0x2f] = ram[0x31] = ram[0xae] = end & 0xff;
    ram[0x2e] = ram[0x30] = ram[0x32] = ram[0xaf] = end >> 8;
}
