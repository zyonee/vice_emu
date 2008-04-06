/*
 * c128mem.c -- Memory handling for the C128 emulator.
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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
#include <string.h>

#include "c128mem.h"
#include "c128mmu.h"
#include "c64cart.h"
#include "c64cia.h"
#include "c64tpi.h"
#include "cmdline.h"
#include "emuid.h"
#include "log.h"
#include "maincpu.h"
#include "parallel.h"
#include "resources.h"
#include "reu.h"
#include "rs232.h"
#include "sid.h"
#include "snapshot.h"
#include "sysfile.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "vdc-mem.h"
#include "vdc.h"
#include "vicii-mem.h"
#include "vicii.h"
#include "z80mem.h"

#ifdef HAVE_RS232
#include "c64acia.h"
#endif

/* #define DEBUG_MMU */

#ifdef DEBUG_MMU
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x)
#endif

#define IS_NULL(s)  (s == NULL || *s == '\0')

/* ------------------------------------------------------------------------- */

static int mem_load_kernal(void);
static int mem_load_basic(void);
static int mem_load_chargen(void);

/* ------------------------------------------------------------------------- */

const char *mem_romset_resources_list[] = {
    "KernalName", "ChargenName", "BasicName",
    "DosName2031", "DosName1001",
    "DosName1541", "DosName1571", "DosName1581", "DosName1541ii",
    NULL
};

/* ------------------------------------------------------------------------- */

/* C128 memory-related resources.  */

/* Name of the character ROM.  */
static char *chargen_rom_name;

/* Name of the BASIC ROM.  */
static char *basic_rom_name;

/* Name of the Kernal ROM.  */
static char *kernal_rom_name;

/* Flag: Do we enable the Emulator ID?  */
static int emu_id_enabled;

/* Flag: Do we enable the IEEE488 interface emulation?  */
static int ieee488_enabled;

/* Flag: Do we enable the external REU?  */
static int reu_enabled;

#ifdef HAVE_RS232
/* Flag: Do we enable the $DE** ACIA RS232 interface emulation?  */
static int acia_de_enabled;

#if 0
/* Flag: Do we enable the $D7** ACIA RS232 interface emulation?  */
static int acia_d7_enabled;
#endif
#endif

static int set_chargen_rom_name(resource_value_t v, void *param)
{
    const char *name = (const char *) v;

    if (chargen_rom_name != NULL && name != NULL
        && strcmp(name, chargen_rom_name) == 0)
        return 0;

    string_set(&chargen_rom_name, name);

    return mem_load_chargen();
}

static int set_kernal_rom_name(resource_value_t v, void *param)
{
    const char *name = (const char *) v;

    if (kernal_rom_name != NULL && name != NULL
        && strcmp(name, kernal_rom_name) == 0)
        return 0;

    string_set(&kernal_rom_name, name);

    return mem_load_kernal();
}

static int set_basic_rom_name(resource_value_t v, void *param)
{
    const char *name = (const char *) v;

    if (basic_rom_name != NULL && name != NULL
        && strcmp(name, basic_rom_name) == 0)
        return 0;

    string_set(&basic_rom_name, name);

    return mem_load_basic();
}

static int set_emu_id_enabled(resource_value_t v, void *param)
{
    if (!(int) v) {
        emu_id_enabled = 0;
        return 0;
    } else {
        emu_id_enabled = 1;
        return 0;
    }
}

static int set_ieee488_enabled(resource_value_t v, void *param)
{
    if (!(int) v) {
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
static int set_reu_enabled(resource_value_t v, void *param)
{
    if (!(int) v) {
        reu_enabled = 0;
        reu_deactivate();
        return 0;
    } else if (!ieee488_enabled) {
        reu_enabled = 1;
        reu_activate();
        return 0;
    } else {
        /* The REU and the IEEE488 interface share the same address space, so
           they cannot be enabled at the same time.  */
        return -1;
    }
}

#ifdef HAVE_RS232
#if 0
static int set_acia_d7_enabled(resource_value_t v, void *param)
{
    acia_d7_enabled = (int) v;
    return 0;
}
#endif

static int set_acia_de_enabled(resource_value_t v, void *param)
{
    acia_de_enabled = (int) v;
    return 0;
}
#endif

static resource_t resources[] =
{
    { "ChargenName", RES_STRING, (resource_value_t) "chargen",
      (resource_value_t *) & chargen_rom_name,
      set_chargen_rom_name, NULL },
    { "KernalName", RES_STRING, (resource_value_t) "kernal",
      (resource_value_t *) & kernal_rom_name,
      set_kernal_rom_name, NULL },
    { "BasicName", RES_STRING, (resource_value_t) "basic",
      (resource_value_t *) & basic_rom_name,
      set_basic_rom_name, NULL },
    { "REU", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) & reu_enabled,
      set_reu_enabled, NULL },
    { "IEEE488", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) & ieee488_enabled,
      set_ieee488_enabled, NULL },
    { "EmuID", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) & emu_id_enabled,
      set_emu_id_enabled, NULL },
#ifdef HAVE_RS232
    { "AciaDE", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) & acia_de_enabled,
      set_acia_de_enabled, NULL },
#if 0
    { "AciaD7", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) & acia_d7_enabled,
      set_acia_d7_enabled, NULL },
#endif
#endif
    { NULL }
};

int c128_mem_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------- */

/* C128 memory-related command-line options.  */

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
#ifdef HAVE_RS232
    { "-acia1", SET_RESOURCE, 0, NULL, NULL, "AciaDE", (resource_value_t) 1,
      NULL, "Enable the $DE** ACIA RS232 interface emulation" },
    { "+acia1", SET_RESOURCE, 0, NULL, NULL, "AciaDE", (resource_value_t) 0,
      NULL, "Disable the $DE** ACIA RS232 interface emulation" },
#if 0
    { "-acia2", SET_RESOURCE, 0, NULL, NULL, "AciaD7", (resource_value_t) 1,
      NULL, "Enable the $D7** ACIA RS232 interface emulation" },
    { "+acia2", SET_RESOURCE, 0, NULL, NULL, "AciaD7", (resource_value_t) 0,
      NULL, "Disable the $D7** ACIA RS232 interface emulation" },
#endif
#endif
    { NULL }
};

int c128_mem_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/* Number of possible video banks (16K each).  */
#define NUM_VBANKS      4

/* The C128 memory.  */
BYTE ram[C128_RAM_SIZE];
BYTE basic_rom[C128_BASIC_ROM_SIZE + C128_EDITOR_ROM_SIZE];
BYTE kernal_rom[C128_KERNAL_ROM_SIZE];
BYTE chargen_rom[C128_CHARGEN_ROM_SIZE];

/* Size of RAM...  */
int ram_size = C128_RAM_SIZE;

/* Currently selected RAM bank.  */
BYTE *ram_bank;

/* Shared memory.  */
static ADDRESS top_shared_limit, bottom_shared_limit;

/* Tape sense status: 1 = some button pressed, 0 = no buttons pressed.  */
static int tape_sense = 0;

/* Pointers to pages 0 and 1 (which can be physically placed anywhere).  */
BYTE *page_zero, *page_one;

/* Flag: nonzero if the Kernal and BASIC ROMs have been loaded.  */
static int rom_loaded = 0;

/* Adjust this pointer when the MMU changes banks.  */
static BYTE **bank_base;
static int *bank_limit = NULL;
unsigned int old_reg_pc;

/* Pointers to the currently used memory read and write tables.  */
read_func_ptr_t *_mem_read_tab_ptr;
store_func_ptr_t *_mem_write_tab_ptr;
BYTE **_mem_read_base_tab_ptr;
int *mem_read_limit_tab_ptr;

#define NUM_CONFIGS 32

/* Memory read and write tables.  */
static store_func_ptr_t mem_write_tab[NUM_CONFIGS][0x101];
static read_func_ptr_t mem_read_tab[NUM_CONFIGS][0x101];
static BYTE *mem_read_base_tab[NUM_CONFIGS][0x101];
static int mem_read_limit_tab[NUM_CONFIGS][0x101];

/* Current video bank (0, 1, 2 or 3).  */
static int vbank;

/* Cartridge memory interface.  FIXME: Not implemented yet.  */
/* Exansion port ROML/ROMH images.  */
BYTE roml_banks[1], romh_banks[1];

/* Expansion port ROML/ROMH/RAM banking.  */
int roml_bank, romh_bank, export_ram;

/* Flag: Ultimax (VIC-10) memory configuration enabled.  */
int ultimax = 0;

/* Logging goes here.  */
static log_t c128_mem_log = LOG_ERR;

extern BYTE mmu[11];

/* ------------------------------------------------------------------------- */

void mem_update_config(int config)
{
    _mem_read_tab_ptr = mem_read_tab[config];
    _mem_write_tab_ptr = mem_write_tab[config];
    _mem_read_base_tab_ptr = mem_read_base_tab[config];
    mem_read_limit_tab_ptr = mem_read_limit_tab[config];

    if (bank_limit != NULL) {
        *bank_base = _mem_read_base_tab_ptr[old_reg_pc >> 8];
        if (*bank_base != 0)
            *bank_base = _mem_read_base_tab_ptr[old_reg_pc >> 8]
                         - (old_reg_pc & 0xff00);
        *bank_limit = mem_read_limit_tab_ptr[old_reg_pc >> 8];
    }
}

void mem_set_bank_pointer(BYTE **base, int *limit)
{
    bank_base = base;
    bank_limit = limit;
}

void mem_set_ram_config(BYTE value)
{
    int shared_size;

    /* XXX: We only support 128K here.  */
    vic_ii_set_ram_base(ram + ((value & 0x40) << 10));

    DEBUG_PRINT(("MMU: Store RCR = $%02x\n", value));
    DEBUG_PRINT(("MMU: VIC-II base at $%05X\n", ((value & 0xc0) << 2)));

    if ((value & 0x3) == 0)
        shared_size = 1024;
    else
        shared_size = 0x1000 << ((value & 0x3) - 1);

    /* Share high memory?  */
    if (value & 0x8) {
        top_shared_limit = 0xffff - shared_size;
        DEBUG_PRINT(("MMU: Sharing high RAM from $%04X\n",
              top_shared_limit + 1));
    } else {
        top_shared_limit = 0xffff;
        DEBUG_PRINT(("MMU: No high shared RAM\n"));
    }

    /* Share low memory?  */
    if (value & 0x4) {
        bottom_shared_limit = shared_size;
        DEBUG_PRINT(("MMU: Sharing low RAM up to $%04X\n",
              bottom_shared_limit - 1));
    } else {
        bottom_shared_limit = 0;
        DEBUG_PRINT(("MMU: No low shared RAM\n"));
    }
}

/* ------------------------------------------------------------------------- */

/* External memory access functions.  */

BYTE REGPARM1 read_basic(ADDRESS addr)
{
    return basic_rom[addr - 0x4000];
}

void REGPARM2 store_basic(ADDRESS addr, BYTE value)
{
    basic_rom[addr - 0x4000] = value;
}

BYTE REGPARM1 read_kernal(ADDRESS addr)
{
    return kernal_rom[addr & 0x1fff];
}

void REGPARM2 store_kernal(ADDRESS addr, BYTE value)
{
    kernal_rom[addr & 0x1fff] = value;
}

BYTE REGPARM1 read_chargen(ADDRESS addr)
{
    return chargen_rom[addr & 0x0fff];
}

BYTE REGPARM1 rom_read(ADDRESS addr)
{
    switch (addr & 0xf000) {
      case 0x0000:
        return read_bios(addr);
      case 0xe000:
      case 0xf000:
        return read_kernal(addr);
      case 0x4000:
      case 0x5000:
      case 0x6000:
      case 0x7000:
      case 0x8000:
      case 0x9000:
      case 0xa000:
      case 0xb000:
        return read_basic(addr);
    }

    return 0;
}

void REGPARM2 rom_store(ADDRESS addr, BYTE value)
{
    switch (addr & 0xf000) {
      case 0x0000:
        store_bios(addr, value);
        break;
      case 0xe000:
      case 0xf000:
        store_kernal(addr, value);
        break;
      case 0x4000:
      case 0x5000:
      case 0x6000:
      case 0x7000:
      case 0x8000:
      case 0x9000:
      case 0xa000:
      case 0xb000:
        store_basic(addr, value);
        break;
    }
}

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

/* CPU Memory interface.  */

/* The MMU can basically do the following:

   - select one of the two (four) memory banks as the standard
   (non-shared) memory;

   - turn ROM and I/O on and off;

   - enable/disable top/bottom shared RAM (from 1K to 16K, so bottom
   shared RAM cannot go beyond $3FFF and top shared RAM cannot go
   under $C000);

   - move pages 0 and 1 to any physical address.  */

#define READ_TOP_SHARED(addr)                           \
    ((addr) > top_shared_limit ? ram[(addr)]            \
                                : ram_bank[(addr)])

#define STORE_TOP_SHARED(addr, value)                           \
    ((addr) > top_shared_limit ? (ram[(addr)] = (value))        \
                               : (ram_bank[(addr)] = (value)))

#define READ_BOTTOM_SHARED(addr)                        \
    ((addr) < bottom_shared_limit ? ram[(addr)]         \
                                  : ram_bank[(addr)])

#define STORE_BOTTOM_SHARED(addr, value)                                \
    ((addr) < bottom_shared_limit ? (ram[(addr)] = (value))             \
                                  : (ram_bank[(addr)] = (value)))

BYTE REGPARM1 read_zero(ADDRESS addr)
{
    return page_zero[addr];
}

void REGPARM2 store_zero(ADDRESS addr, BYTE value)
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


/* $0200 - $3FFF: RAM (normal or shared).  */

BYTE REGPARM1 read_lo(ADDRESS addr)
{
    return READ_BOTTOM_SHARED(addr);
}

void REGPARM2 store_lo(ADDRESS addr, BYTE value)
{
    STORE_BOTTOM_SHARED(addr, value);
}

static BYTE REGPARM1 read_ram(ADDRESS addr)
{
    return ram_bank[addr];
}

static void REGPARM2 store_ram(ADDRESS addr, BYTE value)
{
    ram_bank[addr] = value;
}


/* $4000 - $7FFF: RAM or low BASIC ROM.  */
static BYTE REGPARM1 read_basic_lo(ADDRESS addr)
{
        return basic_rom[addr - 0x4000];
}

static void REGPARM2 store_basic_lo(ADDRESS addr, BYTE value)
{
    ram_bank[addr] = value;
}


/* $8000 - $BFFF: RAM or high BASIC ROM.  */
static BYTE REGPARM1 read_basic_hi(ADDRESS addr)
{
    return basic_rom[addr - 0x4000];
}

static void REGPARM2 store_basic_hi(ADDRESS addr, BYTE value)
{
    ram_bank[addr] = value;
}


/* $C000 - $CFFF: RAM (normal or shared) or Editor ROM.  */
static BYTE REGPARM1 read_editor(ADDRESS addr)
{
    return basic_rom[addr - 0x4000];
}

static void REGPARM2 store_editor(ADDRESS addr, BYTE value)
{
    STORE_TOP_SHARED(addr, value);
}

BYTE REGPARM1 d7xx_read(ADDRESS addr)
{
#if 0                           /*def HAVE_RS232 */
    if (acia_d7_enabled)
        return acia2_read(addr);
#endif
    return 0xff;
}

void REGPARM2 d7xx_store(ADDRESS addr, BYTE value)
{
#if 0                           /*def HAVE_RS232 */
        if (acia_d7_enabled)
            acia2_read(addr, value);
#endif
}

/* $E000 - $FFFF: RAM or Kernal.  */
static BYTE REGPARM1 read_hi(ADDRESS addr)
{
    return kernal_rom[addr & 0x1fff];
}

static void REGPARM2 store_hi(ADDRESS addr, BYTE value)
{
    STORE_TOP_SHARED(addr, value);
}

BYTE REGPARM1 read_top_shared(ADDRESS addr)
{
    return READ_TOP_SHARED(addr);
}

void REGPARM2 store_top_shared(ADDRESS addr, BYTE value)
{
    STORE_TOP_SHARED(addr, value);
}

/* ------------------------------------------------------------------------- */
/* those are approximate copies from the c64 versions....
 * they leave out the cartridge support
 */

void REGPARM2 io1_store(ADDRESS addr, BYTE value)
{
#ifdef HAVE_RS232
        if (acia_de_enabled)
            acia1_store(addr & 0x03, value);
#endif
    return;
}

BYTE REGPARM1 io1_read(ADDRESS addr)
{
#ifdef HAVE_RS232
        if (acia_de_enabled)
            return acia1_read(addr & 0x03);
#endif
        return 0xff;  /* rand(); - C64 has rand(), which is correct? */
}

void REGPARM2 io2_store(ADDRESS addr, BYTE value)
{
    if (reu_enabled)
        reu_store(addr & 0x0f, value);
    if (ieee488_enabled) {
        tpi_store(addr & 0x07, value);
    }
    return;
}

BYTE REGPARM1 io2_read(ADDRESS addr)
{
    if (emu_id_enabled && addr >= 0xdfa0) {
        addr &= 0xff;
        if (addr == 0xff)
            emulator_id[addr - 0xa0] ^= 0xff;
        return emulator_id[addr - 0xa0];
    }
    if (reu_enabled)
        return reu_read(addr & 0x0f);
    if (ieee488_enabled)
        return tpi_read(addr & 0x07);

    return 0xff;  /* rand(); - C64 has rand(), which is correct? */
}

/* ------------------------------------------------------------------------- */

#ifdef _MSC_VER
#pragma optimize("",off);
#endif

void initialize_memory(void)
{
    int i, j, k;

    int limit_tab[13][NUM_CONFIGS] = {
    /* 0000-01ff */
    {     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1 },
    /* 0200-03ff */
    { 0x03fd, 0x03fd, 0x03fd, 0x03fd, 0x03fd, 0x03fd, 0x03fd, 0x03fd,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      0x03fd, 0x03fd, 0x03fd, 0x03fd, 0x03fd, 0x03fd, 0x03fd, 0x03fd,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1 },
    /* 0400-0fff */
    { 0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd, 0x0ffd,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1 },
    /* 1000-1fff */
    { 0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd, 0x1ffd,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1 },
    /* 2000-3fff */
    { 0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd, 0x3ffd,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1 },
    /* 4000-7fff */
    { 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd,
      0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd,
      0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd,
      0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd, 0x7ffd },
    /* 8000-bfff */
    { 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd,
      0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd,
      0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd,
      0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd, 0xbffd },
    /* c000-cfff */
    { 0xcffd, 0xcffd, 0xcffd, 0xcffd, 0xcffd, 0xcffd, 0xcffd, 0xcffd,
          -1,     -1,     -1,     -1, 0xcffd, 0xcffd, 0xcffd, 0xcffd,
      0xcffd, 0xcffd, 0xcffd, 0xcffd, 0xcffd, 0xcffd, 0xcffd, 0xcffd,
          -1,     -1,     -1,     -1, 0xcffd, 0xcffd, 0xcffd, 0xcffd },
    /* d000-dfff */
    { 0xdffd, 0xdffd, 0xdffd, 0xdffd, 0xdffd, 0xdffd, 0xdffd, 0xdffd,
          -1,     -1,     -1,     -1, 0xdffd, 0xdffd, 0xdffd, 0xdffd,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1 },
    /* e000-efff */
    { 0xeffd, 0xeffd, 0xeffd, 0xeffd, 0xeffd, 0xeffd, 0xeffd, 0xeffd,
          -1,     -1,     -1,     -1, 0xeffd, 0xeffd, 0xeffd, 0xeffd,
      0xeffd, 0xeffd, 0xeffd, 0xeffd, 0xeffd, 0xeffd, 0xeffd, 0xeffd,
          -1,     -1,     -1,     -1, 0xeffd, 0xeffd, 0xeffd, 0xeffd },
    /* f000-fbff */
    { 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd,
          -1,     -1,     -1,     -1, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd,
      0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd,
          -1,     -1,     -1,     -1, 0xfbfd, 0xfbfd, 0xfbfd, 0xfbfd },
    /* fc00-feff */
    { 0xfefd, 0xfefd, 0xfefd, 0xfefd, 0xfefd, 0xfefd, 0xfefd, 0xfefd,
          -1,     -1,     -1,     -1, 0xfefd, 0xfefd, 0xfefd, 0xfefd,
      0xfefd, 0xfefd, 0xfefd, 0xfefd, 0xfefd, 0xfefd, 0xfefd, 0xfefd,
          -1,     -1,     -1,     -1, 0xfefd, 0xfefd, 0xfefd, 0xfefd },
    /* ff00-ffff */
    {     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
          -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1 } };

    for (i = 0; i < NUM_CONFIGS; i++) {
        int mstart[13] = { 0x00, 0x02, 0x04, 0x10, 0x20, 0x40, 0x80, 0xc0,
                           0xd0, 0xe0, 0xf0, 0xfc, 0xff };
        int mend[13]   = { 0x01, 0x03, 0x0f, 0x1f, 0x3f, 0x7f, 0xbf, 0xcf,
                           0xdf, 0xef, 0xfb, 0xfe, 0xff};
        for (j = 0; j < 13; j++) {
            for (k = mstart[j]; k <= mend[j]; k++) {
                mem_read_limit_tab[i][k] = limit_tab[j][i];
            }
        }
    }

    for (j = 0; j < NUM_CONFIGS; j++) {

        for (i = 0; i <= 0x100; i++) {
            mem_read_base_tab[j][i] = NULL;
        }

        mem_read_tab[j][0] = read_zero;
        mem_write_tab[j][0] = store_zero;
        mem_read_tab[j][1] = read_one;
        mem_write_tab[j][1] = store_one;
    }


    for (j = 0; j < 32; j += 16) {
        for (i = 0x02; i <= 0x3f; i++) {
            mem_read_tab[0+j][i] = read_ram;
            mem_read_tab[1+j][i] = read_ram;
            mem_read_tab[2+j][i] = read_ram;
            mem_read_tab[3+j][i] = read_ram;
            mem_read_tab[4+j][i] = read_ram;
            mem_read_tab[5+j][i] = read_ram;
            mem_read_tab[6+j][i] = read_ram;
            mem_read_tab[7+j][i] = read_ram;
            mem_read_tab[8+j][i] = read_lo;
            mem_read_tab[9+j][i] = read_lo;
            mem_read_tab[10+j][i] = read_lo;
            mem_read_tab[11+j][i] = read_lo;
            mem_read_tab[12+j][i] = read_lo;
            mem_read_tab[13+j][i] = read_lo;
            mem_read_tab[14+j][i] = read_lo;
            mem_read_tab[15+j][i] = read_lo;
            mem_write_tab[0+j][i] = store_ram;
            mem_write_tab[1+j][i] = store_ram;
            mem_write_tab[2+j][i] = store_ram;
            mem_write_tab[3+j][i] = store_ram;
            mem_write_tab[4+j][i] = store_ram;
            mem_write_tab[5+j][i] = store_ram;
            mem_write_tab[6+j][i] = store_ram;
            mem_write_tab[7+j][i] = store_ram;
            mem_write_tab[8+j][i] = store_lo;
            mem_write_tab[9+j][i] = store_lo;
            mem_write_tab[10+j][i] = store_lo;
            mem_write_tab[11+j][i] = store_lo;
            mem_write_tab[12+j][i] = store_lo;
            mem_write_tab[13+j][i] = store_lo;
            mem_write_tab[14+j][i] = store_lo;
            mem_write_tab[15+j][i] = store_lo;
            mem_read_base_tab[0+j][i] = ram + (i << 8);
            mem_read_base_tab[1+j][i] = ram + (i << 8);
            mem_read_base_tab[2+j][i] = ram + (i << 8);
            mem_read_base_tab[3+j][i] = ram + (i << 8);
            mem_read_base_tab[4+j][i] = ram + (i << 8);
            mem_read_base_tab[5+j][i] = ram + (i << 8);
            mem_read_base_tab[6+j][i] = ram + (i << 8);
            mem_read_base_tab[7+j][i] = ram + (i << 8);
            mem_read_base_tab[8+j][i] = NULL;
            mem_read_base_tab[9+j][i] = NULL;
            mem_read_base_tab[10+j][i] = NULL;
            mem_read_base_tab[11+j][i] = NULL;
            mem_read_base_tab[12+j][i] = NULL;
            mem_read_base_tab[13+j][i] = NULL;
            mem_read_base_tab[14+j][i] = NULL;
            mem_read_base_tab[15+j][i] = NULL;
        }

        for (i = 0x40; i <= 0x7f; i++) {
            mem_read_tab[0+j][i] = read_ram;
            mem_read_tab[1+j][i] = read_basic_lo;
            mem_read_tab[2+j][i] = read_ram;
            mem_read_tab[3+j][i] = read_basic_lo;
            mem_read_tab[4+j][i] = read_ram;
            mem_read_tab[5+j][i] = read_basic_lo;
            mem_read_tab[6+j][i] = read_ram;
            mem_read_tab[7+j][i] = read_basic_lo;
            mem_read_tab[8+j][i] = read_ram;
            mem_read_tab[9+j][i] = read_basic_lo;
            mem_read_tab[10+j][i] = read_ram;
            mem_read_tab[11+j][i] = read_basic_lo;
            mem_read_tab[12+j][i] = read_ram;
            mem_read_tab[13+j][i] = read_basic_lo;
            mem_read_tab[14+j][i] = read_ram;
            mem_read_tab[15+j][i] = read_basic_lo;
            mem_write_tab[0+j][i] = store_ram;
            mem_write_tab[1+j][i] = store_basic_lo;
            mem_write_tab[2+j][i] = store_ram;
            mem_write_tab[3+j][i] = store_basic_lo;
            mem_write_tab[4+j][i] = store_ram;
            mem_write_tab[5+j][i] = store_basic_lo;
            mem_write_tab[6+j][i] = store_ram;
            mem_write_tab[7+j][i] = store_basic_lo;
            mem_write_tab[8+j][i] = store_ram;
            mem_write_tab[9+j][i] = store_basic_lo;
            mem_write_tab[10+j][i] = store_ram;
            mem_write_tab[11+j][i] = store_basic_lo;
            mem_write_tab[12+j][i] = store_ram;
            mem_write_tab[13+j][i] = store_basic_lo;
            mem_write_tab[14+j][i] = store_ram;
            mem_write_tab[15+j][i] = store_basic_lo;
            mem_read_base_tab[0+j][i] = ram + (i << 8);
            mem_read_base_tab[1+j][i] = basic_rom + ((i & 0x3f) << 8);
            mem_read_base_tab[2+j][i] = ram + (i << 8);
            mem_read_base_tab[3+j][i] = basic_rom + ((i & 0x3f) << 8);
            mem_read_base_tab[4+j][i] = ram + (i << 8);
            mem_read_base_tab[5+j][i] = basic_rom + ((i & 0x3f) << 8);
            mem_read_base_tab[6+j][i] = ram + (i << 8);
            mem_read_base_tab[7+j][i] = basic_rom + ((i & 0x3f) << 8);
            mem_read_base_tab[8+j][i] = ram + 0x10000 + (i << 8);
            mem_read_base_tab[9+j][i] = basic_rom + ((i & 0x3f) << 8);
            mem_read_base_tab[10+j][i] = ram + 0x10000 + (i << 8);
            mem_read_base_tab[11+j][i] = basic_rom + ((i & 0x3f) << 8);
            mem_read_base_tab[12+j][i] = ram + 0x10000 + (i << 8);
            mem_read_base_tab[13+j][i] = basic_rom + ((i & 0x3f) << 8);
            mem_read_base_tab[14+j][i] = ram + 0x10000 + (i << 8);
            mem_read_base_tab[15+j][i] = basic_rom + ((i & 0x3f) << 8);
        }

        for (i = 0x80; i <= 0xbf; i++) {
            mem_read_tab[0+j][i] = read_ram;
            mem_read_tab[1+j][i] = read_ram;
            mem_read_tab[2+j][i] = read_basic_hi;
            mem_read_tab[3+j][i] = read_basic_hi;
            mem_read_tab[4+j][i] = read_ram;
            mem_read_tab[5+j][i] = read_ram;
            mem_read_tab[6+j][i] = read_basic_hi;
            mem_read_tab[7+j][i] = read_basic_hi;
            mem_read_tab[8+j][i] = read_ram;
            mem_read_tab[9+j][i] = read_ram;
            mem_read_tab[10+j][i] = read_basic_hi;
            mem_read_tab[11+j][i] = read_basic_hi;
            mem_read_tab[12+j][i] = read_ram;
            mem_read_tab[13+j][i] = read_ram;
            mem_read_tab[14+j][i] = read_basic_hi;
            mem_read_tab[15+j][i] = read_basic_hi;
            mem_write_tab[0+j][i] = store_ram;
            mem_write_tab[1+j][i] = store_ram;
            mem_write_tab[2+j][i] = store_basic_hi;
            mem_write_tab[3+j][i] = store_basic_hi;
            mem_write_tab[4+j][i] = store_ram;
            mem_write_tab[5+j][i] = store_ram;
            mem_write_tab[6+j][i] = store_basic_hi;
            mem_write_tab[7+j][i] = store_basic_hi;
            mem_write_tab[8+j][i] = store_ram;
            mem_write_tab[9+j][i] = store_ram;
            mem_write_tab[10+j][i] = store_basic_hi;
            mem_write_tab[11+j][i] = store_basic_hi;
            mem_write_tab[12+j][i] = store_ram;
            mem_write_tab[13+j][i] = store_ram;
            mem_write_tab[14+j][i] = store_basic_hi;
            mem_write_tab[15+j][i] = store_basic_hi;
            mem_read_base_tab[0+j][i] = ram + (i << 8);
            mem_read_base_tab[1+j][i] = ram + (i << 8);
            mem_read_base_tab[2+j][i] = basic_rom + 0x4000 + ((i & 0x3f) << 8);
            mem_read_base_tab[3+j][i] = basic_rom + 0x4000 + ((i & 0x3f) << 8);
            mem_read_base_tab[4+j][i] = ram + (i << 8);
            mem_read_base_tab[5+j][i] = ram + (i << 8);
            mem_read_base_tab[6+j][i] = basic_rom + 0x4000 + ((i & 0x3f) << 8);
            mem_read_base_tab[7+j][i] = basic_rom + 0x4000 + ((i & 0x3f) << 8);
            mem_read_base_tab[8+j][i] = ram + 0x10000 + (i << 8);
            mem_read_base_tab[9+j][i] = ram + 0x10000 + (i << 8);
            mem_read_base_tab[10+j][i] = basic_rom + 0x4000 + ((i & 0x3f) << 8);
            mem_read_base_tab[11+j][i] = basic_rom + 0x4000 + ((i & 0x3f) << 8);
            mem_read_base_tab[12+j][i] = ram + 0x10000 + (i << 8);
            mem_read_base_tab[13+j][i] = ram + 0x10000 + (i << 8);
            mem_read_base_tab[14+j][i] = basic_rom + 0x4000 + ((i & 0x3f) << 8);
            mem_read_base_tab[15+j][i] = basic_rom + 0x4000 + ((i & 0x3f) << 8);
        }

        for (i = 0xc0; i <= 0xcf; i++) {
            mem_read_tab[0+j][i] = read_ram;
            mem_read_tab[1+j][i] = read_ram;
            mem_read_tab[2+j][i] = read_ram;
            mem_read_tab[3+j][i] = read_ram;
            mem_read_tab[4+j][i] = read_editor;
            mem_read_tab[5+j][i] = read_editor;
            mem_read_tab[6+j][i] = read_editor;
            mem_read_tab[7+j][i] = read_editor;
            mem_read_tab[8+j][i] = read_top_shared;
            mem_read_tab[9+j][i] = read_top_shared;
            mem_read_tab[10+j][i] = read_top_shared;
            mem_read_tab[11+j][i] = read_top_shared;
            mem_read_tab[12+j][i] = read_editor;
            mem_read_tab[13+j][i] = read_editor;
            mem_read_tab[14+j][i] = read_editor;
            mem_read_tab[15+j][i] = read_editor;
            mem_write_tab[0+j][i] = store_ram;
            mem_write_tab[1+j][i] = store_ram;
            mem_write_tab[2+j][i] = store_ram;
            mem_write_tab[3+j][i] = store_ram;
            mem_write_tab[4+j][i] = store_editor;
            mem_write_tab[5+j][i] = store_editor;
            mem_write_tab[6+j][i] = store_editor;
            mem_write_tab[7+j][i] = store_editor;
            mem_write_tab[8+j][i] = store_top_shared;
            mem_write_tab[9+j][i] = store_top_shared;
            mem_write_tab[10+j][i] = store_top_shared;
            mem_write_tab[11+j][i] = store_top_shared;
            mem_write_tab[12+j][i] = store_editor;
            mem_write_tab[13+j][i] = store_editor;
            mem_write_tab[14+j][i] = store_editor;
            mem_write_tab[15+j][i] = store_editor;
            mem_read_base_tab[0+j][i] = ram + (i << 8);
            mem_read_base_tab[1+j][i] = ram + (i << 8);
            mem_read_base_tab[2+j][i] = ram + (i << 8);
            mem_read_base_tab[3+j][i] = ram + (i << 8);
            mem_read_base_tab[4+j][i] = basic_rom + 0x8000 + ((i & 0xf) << 8);
            mem_read_base_tab[5+j][i] = basic_rom + 0x8000 + ((i & 0xf) << 8);
            mem_read_base_tab[6+j][i] = basic_rom + 0x8000 + ((i & 0xf) << 8);
            mem_read_base_tab[7+j][i] = basic_rom + 0x8000 + ((i & 0xf) << 8);
            mem_read_base_tab[8+j][i] = NULL;
            mem_read_base_tab[9+j][i] = NULL;
            mem_read_base_tab[10+j][i] = NULL;
            mem_read_base_tab[11+j][i] = NULL;
            mem_read_base_tab[12+j][i] = basic_rom + 0x8000 + ((i & 0xf) << 8);
            mem_read_base_tab[13+j][i] = basic_rom + 0x8000 + ((i & 0xf) << 8);
            mem_read_base_tab[14+j][i] = basic_rom + 0x8000 + ((i & 0xf) << 8);
            mem_read_base_tab[15+j][i] = basic_rom + 0x8000 + ((i & 0xf) << 8);
        }
    }

    for (i = 0xd0; i <= 0xdf; i++) {
        mem_read_tab[0][i] = read_ram;
        mem_read_tab[1][i] = read_ram;
        mem_read_tab[2][i] = read_ram;
        mem_read_tab[3][i] = read_ram;
        mem_read_tab[4][i] = read_chargen;
        mem_read_tab[5][i] = read_chargen;
        mem_read_tab[6][i] = read_chargen;
        mem_read_tab[7][i] = read_chargen;
        mem_read_tab[8][i] = read_top_shared;
        mem_read_tab[9][i] = read_top_shared;
        mem_read_tab[10][i] = read_top_shared;
        mem_read_tab[11][i] = read_top_shared;
        mem_read_tab[12][i] = read_chargen;
        mem_read_tab[13][i] = read_chargen;
        mem_read_tab[14][i] = read_chargen;
        mem_read_tab[15][i] = read_chargen;
        mem_write_tab[0][i] = store_ram;
        mem_write_tab[1][i] = store_ram;
        mem_write_tab[2][i] = store_ram;
        mem_write_tab[3][i] = store_ram;
        mem_write_tab[4][i] = store_hi;
        mem_write_tab[5][i] = store_hi;
        mem_write_tab[6][i] = store_hi;
        mem_write_tab[7][i] = store_hi;
        mem_write_tab[8][i] = store_top_shared;
        mem_write_tab[9][i] = store_top_shared;
        mem_write_tab[10][i] = store_top_shared;
        mem_write_tab[11][i] = store_top_shared;
        mem_write_tab[12][i] = store_hi;
        mem_write_tab[13][i] = store_hi;
        mem_write_tab[14][i] = store_hi;
        mem_write_tab[15][i] = store_hi;
        mem_read_base_tab[0][i] = ram + (i << 8);
        mem_read_base_tab[1][i] = ram + (i << 8);
        mem_read_base_tab[2][i] = ram + (i << 8);
        mem_read_base_tab[3][i] = ram + (i << 8);
        mem_read_base_tab[4][i] = chargen_rom + ((i & 0xf) << 8);
        mem_read_base_tab[5][i] = chargen_rom + ((i & 0xf) << 8);
        mem_read_base_tab[6][i] = chargen_rom + ((i & 0xf) << 8);
        mem_read_base_tab[7][i] = chargen_rom + ((i & 0xf) << 8);
        mem_read_base_tab[8][i] = NULL;
        mem_read_base_tab[9][i] = NULL;
        mem_read_base_tab[10][i] = NULL;
        mem_read_base_tab[11][i] = NULL;
        mem_read_base_tab[12][i] = chargen_rom + ((i & 0xf) << 8);
        mem_read_base_tab[13][i] = chargen_rom + ((i & 0xf) << 8);
        mem_read_base_tab[14][i] = chargen_rom + ((i & 0xf) << 8);
        mem_read_base_tab[15][i] = chargen_rom + ((i & 0xf) << 8);
    }

    for (j = (NUM_CONFIGS / 2); j < NUM_CONFIGS; j++) {
        for (i = 0xd0; i <= 0xd3; i++) {
            mem_read_tab[j][i] = vic_read;
            mem_write_tab[j][i] = vic_store;
        }

        mem_read_tab[j][0xd4] = sid_read;
        mem_write_tab[j][0xd4] = sid_store;
        mem_read_tab[j][0xd5] = mmu_read;
        mem_write_tab[j][0xd5] = mmu_store;
        mem_read_tab[j][0xd6] = vdc_read;
        mem_write_tab[j][0xd6] = vdc_store;

        mem_read_tab[j][0xd7] = d7xx_read;    /* read_empty_io; */
        mem_write_tab[j][0xd7] = d7xx_store;  /* store_empty_io; */

        mem_read_tab[j][0xd8] = mem_read_tab[j][0xd9] = colorram_read;
        mem_read_tab[j][0xda] = mem_read_tab[j][0xdb] = colorram_read;
        mem_write_tab[j][0xd8] = mem_write_tab[j][0xd9] = colorram_store;
        mem_write_tab[j][0xda] = mem_write_tab[j][0xdb] = colorram_store;

        mem_read_tab[j][0xdc] = cia1_read;
        mem_write_tab[j][0xdc] = cia1_store;
        mem_read_tab[j][0xdd] = cia2_read;
        mem_write_tab[j][0xdd] = cia2_store;

        mem_read_tab[j][0xde] = io1_read;
        mem_write_tab[j][0xde] = io1_store;

        mem_read_tab[j][0xdf] = io2_read;
        mem_write_tab[j][0xdf] = io2_store;
    }

    for (j = 0; j < 32; j += 16) {
        for (i = 0xe0; i <= 0xfe; i++) {
            mem_read_tab[0+j][i] = read_ram;
            mem_read_tab[1+j][i] = read_ram;
            mem_read_tab[2+j][i] = read_ram;
            mem_read_tab[3+j][i] = read_ram;
            mem_read_tab[4+j][i] = read_hi;
            mem_read_tab[5+j][i] = read_hi;
            mem_read_tab[6+j][i] = read_hi;
            mem_read_tab[7+j][i] = read_hi;
            mem_read_tab[8+j][i] = read_top_shared;
            mem_read_tab[9+j][i] = read_top_shared;
            mem_read_tab[10+j][i] = read_top_shared;
            mem_read_tab[11+j][i] = read_top_shared;
            mem_read_tab[12+j][i] = read_hi;
            mem_read_tab[13+j][i] = read_hi;
            mem_read_tab[14+j][i] = read_hi;
            mem_read_tab[15+j][i] = read_hi;
            mem_write_tab[0+j][i] = store_ram;
            mem_write_tab[1+j][i] = store_ram;
            mem_write_tab[2+j][i] = store_ram;
            mem_write_tab[3+j][i] = store_ram;
            mem_write_tab[4+j][i] = store_hi;
            mem_write_tab[5+j][i] = store_hi;
            mem_write_tab[6+j][i] = store_hi;
            mem_write_tab[7+j][i] = store_hi;
            mem_write_tab[8+j][i] = store_top_shared;
            mem_write_tab[9+j][i] = store_top_shared;
            mem_write_tab[10+j][i] = store_top_shared;
            mem_write_tab[11+j][i] = store_top_shared;
            mem_write_tab[12+j][i] = store_hi;
            mem_write_tab[13+j][i] = store_hi;
            mem_write_tab[14+j][i] = store_hi;
            mem_write_tab[15+j][i] = store_hi;
            mem_read_base_tab[0+j][i] = ram + (i << 8);
            mem_read_base_tab[1+j][i] = ram + (i << 8);
            mem_read_base_tab[2+j][i] = ram + (i << 8);
            mem_read_base_tab[3+j][i] = ram + (i << 8);
            mem_read_base_tab[4+j][i] = kernal_rom + ((i & 0x1f) << 8);
            mem_read_base_tab[5+j][i] = kernal_rom + ((i & 0x1f) << 8);
            mem_read_base_tab[6+j][i] = kernal_rom + ((i & 0x1f) << 8);
            mem_read_base_tab[7+j][i] = kernal_rom + ((i & 0x1f) << 8);
            mem_read_base_tab[8+j][i] = NULL;
            mem_read_base_tab[9+j][i] = NULL;
            mem_read_base_tab[10+j][i] = NULL;
            mem_read_base_tab[11+j][i] = NULL;
            mem_read_base_tab[12+j][i] = kernal_rom + ((i & 0x1f) << 8);
            mem_read_base_tab[13+j][i] = kernal_rom + ((i & 0x1f) << 8);
            mem_read_base_tab[14+j][i] = kernal_rom + ((i & 0x1f) << 8);
            mem_read_base_tab[15+j][i] = kernal_rom + ((i & 0x1f) << 8);
        }
    }

    for (j = 0; j < NUM_CONFIGS; j++) {
        mem_read_tab[j][0xff] = mmu_ffxx_read;
        mem_write_tab[j][0xff] = mmu_ffxx_store;

        mem_read_tab[j][0x100] = mem_read_tab[j][0x0];
        mem_write_tab[j][0x100] = mem_write_tab[j][0x0];

        mem_read_base_tab[j][0x100] = NULL;
        mem_read_limit_tab[j][0x100] = -1;
    }

    mmu_reset();

    top_shared_limit = 0xffff;
    bottom_shared_limit = 0x0000;
    ram_bank = ram;
    page_zero = ram;
    page_one = ram + 0x100;

    _mem_read_tab_ptr = mem_read_tab[7];
    _mem_write_tab_ptr = mem_write_tab[7];
    _mem_read_base_tab_ptr = mem_read_base_tab[7];
    mem_read_limit_tab_ptr = mem_read_limit_tab[7];
}

#ifdef _MSC_VER
#pragma optimize("",on);
#endif

/* ------------------------------------------------------------------------- */

/* Initialize RAM for power-up.  */
void mem_powerup(void)
{
    int i;

    for (i = 0; i < 0x20000; i += 0x80) {
        memset(ram + i, 0x0, 0x40);
        memset(ram + i + 0x40, 0xff, 0x40);
    }
}

static int mem_kernal_checksum(void) 
{
    int i,id;
    WORD sum;

    /* Check Kernal ROM.  */
    for (i = 0, sum = 0; i < C128_KERNAL_ROM_SIZE; i++)
        sum += kernal_rom[i];

    id = rom_read(0xff80);

    log_message(c128_mem_log, "Kernal rev #%d.", id);
    if (id == 1
        && sum != C128_KERNAL_CHECKSUM_R01
        && sum != C128_KERNAL_CHECKSUM_R01SWE
        && sum != C128_KERNAL_CHECKSUM_R01GER)
        log_error(c128_mem_log, "Warning: Kernal image may be corrupted."
		" Sum: %d.", sum);
    return 0;
}

static int mem_load_kernal(void) 
{
    int trapfl;

    if (!rom_loaded)
        return 0;

    /* disable traps before loading the ROM */
    resources_get_value("VirtualDevices", (resource_value_t*) &trapfl);
    resources_set_value("VirtualDevices", (resource_value_t) 1);

    if(!IS_NULL(kernal_rom_name)) {
        /* Load Kernal ROM.  */
        if (sysfile_load(kernal_rom_name,
            kernal_rom, C128_KERNAL_ROM_SIZE,
            C128_KERNAL_ROM_SIZE) < 0) {
            log_error(c128_mem_log, "Couldn't load kernal ROM `%s'.", 
			  kernal_rom_name);
	    resources_set_value("VirtualDevices", (resource_value_t) trapfl);
            return -1;
	}
    }

    mem_kernal_checksum();

    resources_set_value("VirtualDevices", (resource_value_t) trapfl);

    return 0;
}

static int mem_basic_checksum(void) 
{
    int i,id;
    WORD sum;

    /* Check Basic ROM.  */
    for (i = 0, sum = 0; i < C128_BASIC_ROM_SIZE; i++)
        sum += basic_rom[i];

    if (sum != C128_BASIC_CHECKSUM_85 && sum != C128_BASIC_CHECKSUM_86)
        log_error(c128_mem_log,
                  "Warning: Unknown Basic image `%s'.  Sum: %d ($%04X).",
                  basic_rom_name, sum, sum);

    /* Check Editor ROM.  */
    for (i = C128_BASIC_ROM_SIZE, sum = 0;
         i < C128_BASIC_ROM_SIZE + C128_EDITOR_ROM_SIZE;
         i++)
        sum += basic_rom[i];

    id = rom_read(0xff80);
    if (id == 01
        && sum != C128_EDITOR_CHECKSUM_R01
        && sum != C128_EDITOR_CHECKSUM_R01SWE
        && sum != C128_EDITOR_CHECKSUM_R01GER) {
        log_error(c128_mem_log, "Warning: EDITOR image may be corrupted."
		" Sum: %d.", sum);
        log_error(c128_mem_log, "Check your Basic ROM.");
    }
    return 0;
}

static int mem_load_basic(void)
{
    if (!rom_loaded)
        return 0;

    if(!IS_NULL(basic_rom_name)) {
        /* Load Basic ROM.  */
        if (sysfile_load(basic_rom_name,
            basic_rom, C128_BASIC_ROM_SIZE + C128_EDITOR_ROM_SIZE,
            C128_BASIC_ROM_SIZE + C128_EDITOR_ROM_SIZE) < 0) {
            log_error(c128_mem_log, "Couldn't load basic ROM `%s'.",
                  basic_rom_name);
            return -1;
	}
    }
    return mem_basic_checksum();
}

static int mem_load_chargen(void)
{
    if (!rom_loaded)
        return 0;

    if (!IS_NULL(chargen_rom_name)) {
        /* Load chargen ROM.  */
        if (sysfile_load(chargen_rom_name,
            chargen_rom, C128_CHARGEN_ROM_SIZE,
            C128_CHARGEN_ROM_SIZE) < 0) {
            log_error(c128_mem_log, "Couldn't load character ROM `%s'.",
                  chargen_rom_name);
            return -1;
	}
    }
    return 0;
}

/* Load ROMs at startup.  This is half-stolen from the old `load_mem()' in
   `memory.c'.  */
int mem_load(void)
{
    if (c128_mem_log == LOG_ERR)
        c128_mem_log = log_open("C128MEM");

    mem_powerup();

    page_zero = ram;
    page_one = ram + 0x100;

    initialize_memory();

    rom_loaded = 1;

    if(mem_load_kernal() < 0)
	return -1;

    if(mem_load_basic() < 0)
	return -1;

    if(mem_load_chargen() < 0)
	return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */

/* Change the current video bank.  Call this routine only when the vbank
   has really changed.  */
void mem_set_vbank(int new_vbank)
{
    vbank = new_vbank;
    vic_ii_set_vbank(new_vbank);
}

void mem_toggle_watchpoints(int flag)
{
    /* FIXME: Still to do.  */
}

/* Set the tape sense status.  */
void mem_set_tape_sense(int sense)
{
    tape_sense = sense;
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

void mem_get_basic_text(ADDRESS * start, ADDRESS * end)
{
    if (start != NULL)
        *start = ram[0x2b] | (ram[0x2c] << 8);
    if (end != NULL)
        *end = ram[0x1210] | (ram[0x1211] << 8);
}

void mem_set_basic_text(ADDRESS start, ADDRESS end)
{
    ram[0x2b] = ram[0xac] = start & 0xff;
    ram[0x2c] = ram[0xad] = start >> 8;
    ram[0x1210] = end & 0xff;
    ram[0x1211] = end >> 8;
}

/* ------------------------------------------------------------------------- */

int mem_rom_trap_allowed(ADDRESS addr)
{
    return 1;
}

/* ------------------------------------------------------------------------- */

/* Banked memory access functions for the monitor */

/* FIXME: peek, cartridge support */

static void store_bank_io(ADDRESS addr, BYTE byte)
{
    switch (addr & 0xff00) {
      case 0xd000:
      case 0xd100:
      case 0xd200:
      case 0xd300:
        vic_store(addr, byte);
        break;
      case 0xd400:
        sid_store(addr, byte);
        break;
      case 0xd500:
        mmu_store(addr, byte);
        break;
      case 0xd600:
        vdc_store(addr, byte);
        break;
      case 0xd700:
        d7xx_store(addr, byte);
        break;
      case 0xd800:
      case 0xd900:
      case 0xda00:
      case 0xdb00:
        colorram_store(addr, byte);
        break;
      case 0xdc00:
        cia1_store(addr, byte);
        break;
      case 0xdd00:
        cia2_store(addr, byte);
        break;
      case 0xde00:
        io1_store(addr, byte);
        break;
      case 0xdf00:
        io2_store(addr, byte);
        break;
    }
    return;
}

static BYTE read_bank_io(ADDRESS addr)
{
    switch (addr & 0xff00) {
      case 0xd000:
      case 0xd100:
      case 0xd200:
      case 0xd300:
        return vic_read(addr);
      case 0xd400:
        return sid_read(addr);
      case 0xd500:
        return mmu_read(addr);
      case 0xd600:
        return vdc_read(addr);
      case 0xd700:
        return d7xx_read(addr);
      case 0xd800:
      case 0xd900:
      case 0xda00:
      case 0xdb00:
        return colorram_read(addr);
      case 0xdc00:
        return cia1_read(addr);
      case 0xdd00:
        return cia2_read(addr);
      case 0xde00:
        return io1_read(addr);
      case 0xdf00:
        return io2_read(addr);
    }
    return 0xff;
}

static BYTE peek_bank_io(ADDRESS addr)
{
    switch (addr & 0xff00) {
      case 0xd000:
      case 0xd100:
      case 0xd200:
      case 0xd300:
        return vic_peek(addr);
      case 0xd400:
        return sid_read(addr); /* FIXME */
      case 0xd500:
        return mmu_read(addr);
      case 0xd600:
        return vdc_read(addr); /* FIXME */
      case 0xd700:
        return d7xx_read(addr); /* FIXME */
      case 0xd800:
      case 0xd900:
      case 0xda00:
      case 0xdb00:
        return colorram_read(addr);
      case 0xdc00:
        return cia1_peek(addr);
      case 0xdd00:
        return cia2_peek(addr);
      case 0xde00:
        return io1_read(addr);  /* FIXME */
      case 0xdf00:
        return io2_read(addr);  /* FIXME */
    }
    return 0xff;
}

/* Exported banked memory access functions for the monitor.  */

static const char *banknames[] = {
    "default", "cpu", "ram", "rom", "io", "ram1", NULL
};

static int banknums[] = {
    1, 0, 1, 2, 3, 4
};

const char **mem_bank_list(void)
{
    return banknames;
}

int mem_bank_from_name(const char *name)
{
    int i = 0;

    while (banknames[i]) {
        if (!strcmp(name, banknames[i])) {
            return banknums[i];
        }
        i++;
    }
    return -1;
}

BYTE mem_bank_read(int bank, ADDRESS addr)
{
    switch (bank) {
      case 0:                   /* current */
        return mem_read(addr);
        break;
      case 4:                   /* ram1 */
        return ram[addr + 0x10000];
      case 3:                   /* io */
        if (addr >= 0xd000 && addr < 0xe000) {
            return read_bank_io(addr);
        }
      case 2:                   /* rom */
        if (addr <= 0x0FFF) {
            return read_bios(addr);
        }
        if (addr >= 0x4000 && addr <= 0xCFFF) {
            return basic_rom[addr - 0x4000];
        }
        if (addr >= 0xD000 && addr <= 0xDFFF) {
            return chargen_rom[addr & 0x0fff];
        }
        if (addr >= 0xE000 && addr <= 0xFFFF) {
            return kernal_rom[addr & 0x1fff];
        }
      case 1:                   /* ram */
        break;
    }
    return ram[addr];
}

BYTE mem_bank_peek(int bank, ADDRESS addr)
{
    switch (bank) {
      case 0:                   /* current */
        return mem_read(addr);  /* FIXME */
        break;
      case 3:                   /* io */
        if (addr >= 0xd000 && addr < 0xe000) {
            return peek_bank_io(addr);
        }
    }
    return mem_bank_read(bank, addr);
}

void mem_bank_write(int bank, ADDRESS addr, BYTE byte)
{
    switch (bank) {
      case 0:                   /* current */
        mem_store(addr, byte);
        return;
      case 4:                   /* ram1 */
        ram[addr + 0x10000] = byte;
        return;
      case 3:                   /* io */
        if (addr >= 0xd000 && addr < 0xe000) {
            store_bank_io(addr, byte);
            return;
        }
      case 2:                   /* rom */
        if (addr >= 0x4000 && addr <= 0xCFFF) {
            return;
        }
        if (addr >= 0xE000 && addr <= 0xffff) {
            return;
        }
      case 1:                   /* ram */
        break;
    }
    ram[addr] = byte;
}

void mem_get_screen_parameter(ADDRESS *base, BYTE *rows, BYTE *columns)
{
    *base = ((vic_peek(0xd018) & 0xf0) << 6)
            | ((~cia2_peek(0xdd00) & 0x03) << 14);
    *rows = 25;
    *columns = 40;
}

/* ------------------------------------------------------------------------- */

/* Snapshot.  */

static char snap_rom_module_name[] = "C128ROM";
#define SNAP_ROM_MAJOR 0
#define SNAP_ROM_MINOR 0

int mem_write_rom_snapshot_module(snapshot_t *s)
{
    snapshot_module_t *m;
    int trapfl;

    /* Main memory module.  */

    m = snapshot_module_create(s, snap_rom_module_name, 
                               SNAP_ROM_MAJOR, SNAP_ROM_MINOR);
    if (m == NULL)
        return -1;

    /* disable traps before saving the ROM */
    resources_get_value("VirtualDevices", (resource_value_t*) &trapfl);
    resources_set_value("VirtualDevices", (resource_value_t) 1);

    if (0
        || snapshot_module_write_byte_array(m, kernal_rom, 
                                            C128_KERNAL_ROM_SIZE) < 0
        || snapshot_module_write_byte_array(m, basic_rom, 
                                            C128_BASIC_ROM_SIZE) < 0
        || snapshot_module_write_byte_array(m, basic_rom + C128_BASIC_ROM_SIZE, 
                                            C128_EDITOR_ROM_SIZE) < 0
        || snapshot_module_write_byte_array(m, chargen_rom, 
                                            C128_CHARGEN_ROM_SIZE) < 0
	)
        goto fail;

    /* FIXME: save cartridge ROM (& RAM?) areas:
       first write out the configuration, i.e.
       - type of cartridge (banking scheme type)
       - state of cartridge (active/which bank, ...)
       then the ROM/RAM arrays:
       - cartridge ROM areas
       - cartridge RAM areas
    */

    /* enable traps again when necessary */
    resources_set_value("VirtualDevices", (resource_value_t) trapfl);

    if (snapshot_module_close(m) < 0)
        goto fail;

    return 0;

 fail:
    /* enable traps again when necessary */
    resources_set_value("VirtualDevices", (resource_value_t) trapfl);

    if (m != NULL)
        snapshot_module_close(m);
    return -1;
}

int mem_read_rom_snapshot_module(snapshot_t *s)
{
    BYTE major_version, minor_version;
    snapshot_module_t *m;
    int trapfl;

    /* Main memory module.  */

    m = snapshot_module_open(s, snap_rom_module_name,
                             &major_version, &minor_version);
    /* This module is optional.  */
    if (m == NULL)
        return 0;

    /* disable traps before loading the ROM */
    resources_get_value("VirtualDevices", (resource_value_t*) &trapfl);
    resources_set_value("VirtualDevices", (resource_value_t) 1);

    if (major_version > SNAP_ROM_MAJOR || minor_version > SNAP_ROM_MINOR) {
        log_error(c128_mem_log,
                  "MEM: Snapshot module version (%d.%d) newer than %d.%d.",
                  major_version, minor_version,
                  SNAP_ROM_MAJOR, SNAP_ROM_MINOR);
        goto fail;
    }

    if (0
        || snapshot_module_read_byte_array(m, kernal_rom, 
                                           C128_KERNAL_ROM_SIZE) < 0
        || snapshot_module_read_byte_array(m, basic_rom, 
                                           C128_BASIC_ROM_SIZE) < 0
        || snapshot_module_read_byte_array(m, basic_rom + C128_BASIC_ROM_SIZE, 
                                           C128_EDITOR_ROM_SIZE) < 0
        || snapshot_module_read_byte_array(m, chargen_rom, 
                                           C128_CHARGEN_ROM_SIZE) < 0
	)
        goto fail;

    log_warning(c128_mem_log,"Dumped Romset files and saved settings will "
                "represent\nthe state before loading the snapshot!");

    mem_basic_checksum();
    mem_kernal_checksum();

    /* enable traps again when necessary */
    resources_set_value("VirtualDevices", (resource_value_t) trapfl);

    /* to get all the checkmarks right */
    ui_update_menus();

    return 0;

 fail:

    /* enable traps again when necessary */
    resources_set_value("VirtualDevices", (resource_value_t) trapfl);

    if (m != NULL)
        snapshot_module_close(m);
    return -1;
}
static char snap_module_name[] = "C128MEM";
#define SNAP_MAJOR 0
#define SNAP_MINOR 0

int mem_write_snapshot_module(snapshot_t *s, int save_roms)
{
    snapshot_module_t *m;
    ADDRESS i;

    /* Main memory module.  */

    m = snapshot_module_create(s, snap_module_name, SNAP_MAJOR, SNAP_MINOR);
    if (m == NULL)
        return -1;

    /* Assuming no side-effects.  */
    for (i = 0; i < 11; i++) {
	if ( snapshot_module_write_byte(m, mmu_read(i)) < 0)
	    goto fail;
    }

    if (0
        || snapshot_module_write_byte_array(m, ram, C128_RAM_SIZE) < 0)
        goto fail;

    if (snapshot_module_close(m) < 0)
        goto fail;
    m = NULL;

    if (save_roms && mem_write_rom_snapshot_module(s) <0)
	goto fail;

    /* REU module: FIXME.  */

    /* IEEE 488 module.  */
    if (ieee488_enabled && tpi_write_snapshot_module(s) < 0)
        goto fail;

#ifdef HAVE_RS232
    /* ACIA module.  */
    if (acia_de_enabled && acia1_write_snapshot_module(s) < 0)
        goto fail;
#endif

    return 0;

fail:
    if (m != NULL)
        snapshot_module_close(m);
    return -1;
}

int mem_read_snapshot_module(snapshot_t *s)
{
    BYTE major_version, minor_version;
    snapshot_module_t *m;
    ADDRESS i;
    BYTE byte;

    /* Main memory module.  */

    m = snapshot_module_open(s, snap_module_name,
                             &major_version, &minor_version);
    if (m == NULL)
        return -1;

    if (major_version > SNAP_MAJOR || minor_version > SNAP_MINOR) {
        log_error(c128_mem_log,
                  "MEM: Snapshot module version (%d.%d) newer than %d.%d.",
                  major_version, minor_version,
                  SNAP_MAJOR, SNAP_MINOR);
        goto fail;
    }

    for (i = 0; i < 11; i++) {
	if (snapshot_module_read_byte(m, &byte) < 0)
	    goto fail;
        mmu_store(i, byte);	/* Assuming no side-effects */
    }

    if (0
        || snapshot_module_read_byte_array(m, ram, C128_RAM_SIZE) < 0)
        goto fail;

    /* pla_config_changed(); */

    if (snapshot_module_close(m) < 0)
        goto fail;
    m = NULL;

    if (mem_read_rom_snapshot_module(s) < 0)
	goto fail;

    /* REU module: FIXME.  */

    /* IEEE488 module.  */
    if (tpi_read_snapshot_module(s) < 0) {
        ieee488_enabled = 0;
    } else {
        ieee488_enabled = 1;
    }

#ifdef HAVE_RS232
    /* ACIA module.  */
    if (acia1_read_snapshot_module(s) < 0) {
        acia_de_enabled = 0;
    } else {
        acia_de_enabled = 1;
    }
#endif

    ui_update_menus();

    return 0;

fail:
    if (m != NULL)
        snapshot_module_close(m);
    return -1;
}

/* ------------------------------------------------------------------------- */

/* FIXME: is called from c64tpi.c to enable/disable C64 IEEE488 ROM module */
void mem_set_exrom(int active)
{
}

