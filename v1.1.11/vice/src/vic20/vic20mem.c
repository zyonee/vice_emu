/*
 * vic20mem.c -- VIC20 memory handling.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
 *  Andr� Fachat (fachat@physik.tu-chemnitz.de)
 *
 * Multiple memory configuration support originally by
 *  Alexander Lehmann (alex@mathematik.th-darmstadt.de)
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

#include "vic20mem.h"

#include "cartridge.h"
#include "cmdline.h"
#include "emuid.h"
#include "interrupt.h"
#include "log.h"
#include "maincpu.h"
#include "mon.h"
#include "resources.h"
#include "stdlib.h"
#include "sysfile.h"
#include "ui.h"
#include "utils.h"
#include "vic.h"
#include "vic20ieeevia.h"
#include "vic20via.h"

#define IS_NULL(s)  (s == NULL || *s == '\0')

/* ------------------------------------------------------------------------- */

const char *mem_romset_resources_list[] = {
    "KernalName", "ChargenName", "BasicName", 
    "CartridgeFile2000", "CartridgeFile6000", 
    "CartridgeFileA000", "CartridgeFileB000",
    "DosName1541", "DosName1571", "DosName1581",
    "DosName1541ii",
    NULL
};

/* ------------------------------------------------------------------------- */

/* VIC20 memory-related resources.  */

#define VIC_BLK0 1
#define VIC_BLK1 2
#define VIC_BLK2 4
#define VIC_BLK3 8
#define VIC_BLK5 16
#define VIC_BLK_ALL (VIC_BLK0 | VIC_BLK1 | VIC_BLK2 | VIC_BLK3 | VIC_BLK5)

#define	VIC_ROM_BLK1A	1
#define	VIC_ROM_BLK1B	2
#define	VIC_ROM_BLK2A	4
#define	VIC_ROM_BLK2B	8
#define	VIC_ROM_BLK3A	16
#define	VIC_ROM_BLK3B	32
#define	VIC_ROM_BLK5A	64
#define	VIC_ROM_BLK5B	128

#define	VIC_ROM_BLK0A	0
#define	VIC_ROM_BLK0B	0

/*----------------------------------------------------------------------*/

static int mem_load_kernal(void);
static int mem_load_basic(void);
static int mem_load_chargen(void);

/*----------------------------------------------------------------------*/

/* Resource handling */

/* Name of the character ROM.  */
static char *chargen_rom_name;

/* Name of the BASIC ROM.  */
static char *basic_rom_name;

/* Name of the Kernal ROM.  */
static char *kernal_rom_name;

/* Flag: Do we enable the Emulator ID?  */
static int emu_id_enabled;

/* Flag: Do we enable the VIC-1112 IEEE488 interface?  */
static int ieee488_enabled;

/* which ROMs are loaded - bits are VIC_BLK* */
static int mem_rom_blocks;

/* Flag: Do we have RAM block `n'?  */
static int ram_block_0_enabled;
static int ram_block_1_enabled;
static int ram_block_2_enabled;
static int ram_block_3_enabled;
static int ram_block_5_enabled;

/* Old program counter.  Not used without MMU support.  */
unsigned int old_reg_pc;

static int set_chargen_rom_name(resource_value_t v)
{
    const char *name = (const char *) v;

    if (chargen_rom_name != NULL && name != NULL
	&& strcmp(name, chargen_rom_name) == 0)
	return 0;

    string_set(&chargen_rom_name, name);

    return mem_load_chargen();
}

static int set_kernal_rom_name(resource_value_t v)
{
    const char *name = (const char *) v;

    if (kernal_rom_name != NULL && name != NULL
	&& strcmp(name, kernal_rom_name) == 0)
	return 0;

    string_set(&kernal_rom_name, name);

    return mem_load_kernal();
}

static int set_basic_rom_name(resource_value_t v)
{
    const char *name = (const char *) v;

    if (basic_rom_name != NULL && name != NULL
	&& strcmp(name, basic_rom_name) == 0)
	return 0;

    string_set(&basic_rom_name, name);

    return mem_load_basic();
}

/* Ugly hack...  */
#define DEFINE_SET_BLOCK_FUNC(num)                                      \
    static int set_ram_block_##num##_enabled(resource_value_t v)        \
    {                                                                   \
        int value = (int) v;                                            \
                                                                        \
        ram_block_##num##_enabled = value;                              \
        if (value) {                                                    \
	    mem_rom_blocks &= (VIC_ROM_BLK##num##A | VIC_ROM_BLK##num##B); \
            return vic20_mem_enable_ram_block(num);                     \
        } else                                                          \
            return vic20_mem_disable_ram_block(num);                    \
    }

DEFINE_SET_BLOCK_FUNC(0)
DEFINE_SET_BLOCK_FUNC(1)
DEFINE_SET_BLOCK_FUNC(2)
DEFINE_SET_BLOCK_FUNC(3)
DEFINE_SET_BLOCK_FUNC(5)

static int set_emu_id_enabled(resource_value_t v)
{
    emu_id_enabled = (int) v;
    return 0;
}

static int set_ieee488_enabled(resource_value_t v)
{
    ieee488_enabled = (int) v;

    ui_update_menus();

    return 0;
}

/* Enable/disable the Emulator ID.  */
void mem_toggle_emu_id(int flag)
{
    emu_id_enabled = flag;
}

static resource_t resources[] =
{
    {"ChargenName", RES_STRING, (resource_value_t) "chargen",
     (resource_value_t *) & chargen_rom_name, set_chargen_rom_name},
    {"KernalName", RES_STRING, (resource_value_t) "kernal",
     (resource_value_t *) & kernal_rom_name, set_kernal_rom_name},
    {"BasicName", RES_STRING, (resource_value_t) "basic",
     (resource_value_t *) & basic_rom_name, set_basic_rom_name},
    {"RAMBlock0", RES_INTEGER, (resource_value_t) 1,
     (resource_value_t *) & ram_block_0_enabled, set_ram_block_0_enabled},
    {"RAMBlock1", RES_INTEGER, (resource_value_t) 1,
     (resource_value_t *) & ram_block_1_enabled, set_ram_block_1_enabled},
    {"RAMBlock2", RES_INTEGER, (resource_value_t) 1,
     (resource_value_t *) & ram_block_2_enabled, set_ram_block_2_enabled},
    {"RAMBlock3", RES_INTEGER, (resource_value_t) 1,
     (resource_value_t *) & ram_block_3_enabled, set_ram_block_3_enabled},
    {"RAMBlock5", RES_INTEGER, (resource_value_t) 1,
     (resource_value_t *) & ram_block_5_enabled, set_ram_block_5_enabled},
    {"EmuID", RES_INTEGER, (resource_value_t) 0,
     (resource_value_t *) & emu_id_enabled, set_emu_id_enabled},
    {"IEEE488", RES_INTEGER, (resource_value_t) 0,
     (resource_value_t *) & ieee488_enabled, set_ieee488_enabled},
    {NULL}
};

int vic20_mem_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------- */

/* This function parses the mem config string given as `-memory' and returns
 * the appropriate values or'ed together.
 *
 * basically we accept a comma separated list of options like the following:
 * ""   - no extension
 * none - same
 * all  - all blocks
 * 3k   - 3k space in block 0 (=3k extension cartridge)
 * 8k   - 1st 8k extension block
 * 16k  - 1st and 2nd 8 extension (= 16k extension cartridge)
 * 24k  - 1-3rd extension block (=8k and 16k extension cartridges)
 *
 * 0,1,2,3,5      - memory in respective block
 * 04,20,40,60,a0 - memory at respective address (same as block #s)
 *
 * example: xvic -memory ""
 *
 *            enables unexpanded computer
 *
 *          xvic -memory 60,a0
 *
 *            enables memory in blocks 3 and 5, which is the usual
 *            configuration for 16k rom modules
 *
 * 12/27/96 Alexander Lehmann (alex@mathematik.th-darmstadt.de)
 * Edited by Ettore to fit in the new command-line parsing.
 */

static int cmdline_memory(const char *param, void *extra_param)
{
    int memconf = 0;
    const char *memstring = param, *optend;
    char *opt = alloca(strlen(param) + 1);

    /* Default is all banks. */
    if (!memstring)
	memconf = VIC_BLK_ALL;
    else {
	/* Maybe we should use strtok for this? */
	while (*memstring) {
	    for (optend = memstring; *optend && *optend != ','; optend++);

	    strncpy(opt, memstring, optend - memstring);
	    opt[optend - memstring] = '\0';

	    if (strcmp(opt, "") == 0 || strcmp(opt, "none") == 0) {
		/* no extension */
	    } else if (strcmp(opt, "all") == 0) {
		memconf = VIC_BLK_ALL;
	    } else if (strcmp(opt, "3k") == 0) {
		memconf |= VIC_BLK0;
	    } else if (strcmp(opt, "8k") == 0) {
		memconf |= VIC_BLK1;
	    } else if (strcmp(opt, "16k") == 0) {
		memconf |= VIC_BLK1 | VIC_BLK2;
	    } else if (strcmp(opt, "24k") == 0) {
		memconf |= VIC_BLK1 | VIC_BLK2 | VIC_BLK3;;
	    } else if (strcmp(opt, "0") == 0 || strcmp(opt, "04") == 0) {
		memconf |= VIC_BLK0;
	    } else if (strcmp(opt, "1") == 0 || strcmp(opt, "20") == 0) {
		memconf |= VIC_BLK1;
	    } else if (strcmp(opt, "2") == 0 || strcmp(opt, "40") == 0) {
		memconf |= VIC_BLK2;
	    } else if (strcmp(opt, "3") == 0 || strcmp(opt, "60") == 0) {
		memconf |= VIC_BLK3;
	    } else if (strcmp(opt, "5") == 0 || strcmp(opt, "a0") == 0
		       || strcmp(opt, "A0") == 0) {
		memconf |= VIC_BLK5;
	    } else {
		/* FIXME: this is before log is initialized, right? */
		fprintf(stdout,
                        "Unsupported memory extension option: \"%s\".\n",
			opt);
		return -1;
	    }
	    memstring = optend;
	    if (*memstring)
		memstring++;	/* skip ',' */
	}
    }

    /* FIXME: this is before log is initialized, right? */
    printf("Extension memory enabled: ");
    if (memconf & VIC_BLK0) {
	set_ram_block_0_enabled((resource_value_t) 1);
	printf("blk0 ");
    } else {
	set_ram_block_0_enabled((resource_value_t) 0);
    }
    if (memconf & VIC_BLK1) {
	set_ram_block_1_enabled((resource_value_t) 1);
	printf("blk1 ");
    } else {
	set_ram_block_1_enabled((resource_value_t) 0);
    }
    if (memconf & VIC_BLK2) {
	set_ram_block_2_enabled((resource_value_t) 1);
	printf("blk2 ");
    } else {
	set_ram_block_2_enabled((resource_value_t) 0);
    }
    if (memconf & VIC_BLK3) {
	set_ram_block_3_enabled((resource_value_t) 1);
	printf("blk3 ");
    } else {
	set_ram_block_3_enabled((resource_value_t) 0);
    }
    if (memconf & VIC_BLK5) {
	set_ram_block_5_enabled((resource_value_t) 1);
	printf("blk5");
    } else {
	set_ram_block_5_enabled((resource_value_t) 0);
    }
    if (memconf == 0)
	printf("none");
    printf("\n");

    return 0;
}

/* VIC20 memory-related command-line options.  */
static cmdline_option_t cmdline_options[] =
{
    { "-kernal", SET_RESOURCE, 1, NULL, NULL, "KernalName", NULL,
      "<name>", "Specify name of Kernal ROM image" },
    { "-basic", SET_RESOURCE, 1, NULL, NULL, "BasicName", NULL,
      "<name>", "Specify name of BASIC ROM image" },
    { "-chargen", SET_RESOURCE, 1, NULL, NULL, "ChargenName", NULL,
      "<name>", "Specify name of character generator ROM image" },
    {"-memory", CALL_FUNCTION, 1, cmdline_memory, NULL, NULL, NULL,
     "<spec>", "Specify memory configuration"},
    {"-emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (resource_value_t) 1,
     NULL, "Enable emulator identification"},
    {"+emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (resource_value_t) 0,
     NULL, "Disable emulator identification"},
    {"-ieee488", SET_RESOURCE, 0, NULL, NULL, "IEEE488", (resource_value_t) 1,
     NULL, "Enable VIC-1112 IEEE488 interface"},
    {"+ieee488", SET_RESOURCE, 0, NULL, NULL, "IEEE488", (resource_value_t) 0,
     NULL, "Disable VIC-1112 IEEE488 interface"},
    {NULL}
};

int vic20_mem_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/* The VIC20 memory. */
BYTE ram[VIC20_RAM_SIZE];
int ram_size = VIC20_RAM_SIZE;
BYTE rom[VIC20_BASIC_ROM_SIZE + VIC20_KERNAL_ROM_SIZE];
#define kernal_rom (rom + VIC20_BASIC_ROM_SIZE)
#define basic_rom (rom)

BYTE cartrom[0x10000];

/* The second 0x400 handles a possible segfault by a wraparound of the
   chargen by setting it to $8c00.  FIXME: This does not cause the exact
   behavior to be emulated though!  */
BYTE chargen_rom[0x400 + VIC20_CHARGEN_ROM_SIZE + 0x400];

/* Memory read and write tables.  */
read_func_ptr_t _mem_read_tab[0x101];
store_func_ptr_t _mem_write_tab[0x101];
BYTE *_mem_read_base_tab[0x101];
int mem_read_limit_tab[0x101];

/* These ones are used when watchpoints are turned on.  */
read_func_ptr_t _mem_read_tab_watch[0x101];
store_func_ptr_t _mem_write_tab_watch[0x101];
read_func_ptr_t _mem_read_tab_nowatch[0x101];
store_func_ptr_t _mem_write_tab_nowatch[0x101];

read_func_ptr_t *_mem_read_tab_ptr;
store_func_ptr_t *_mem_write_tab_ptr;
BYTE **_mem_read_base_tab_ptr;
int *mem_read_limit_tab_ptr;

/* Flag: nonzero if the Kernal and BASIC ROMs have been loaded.  */
static int rom_loaded = 0;

static log_t vic20_mem_log = LOG_ERR;

/* ------------------------------------------------------------------------- */

static void REGPARM2 store_wrap(ADDRESS addr, BYTE value)
{
    ram[addr & (VIC20_RAM_SIZE - 1)] = value;
    chargen_rom[addr & 0x3ff] = value;
}

/* ------------------------------------------------------------------------- */

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
    return chargen_rom[0x400 + (addr & 0xfff)];
}

BYTE REGPARM1 read_zero(ADDRESS addr)
{
    return ram[addr & 0xff];
}

void REGPARM2 store_zero(ADDRESS addr, BYTE value)
{
    ram[addr & 0xff] = value;
}

static BYTE REGPARM1 read_ram(ADDRESS addr)
{
    return ram[addr];
}

static void REGPARM2 store_ram(ADDRESS addr, BYTE value)
{
    ram[addr & (VIC20_RAM_SIZE - 1)] = value;
}

static BYTE REGPARM1 read_cartrom(ADDRESS addr)
{
    return cartrom[addr & 0xffff];
}

BYTE REGPARM1 read_rom(ADDRESS addr)
{
    switch (addr & 0xf000) {
      case 0x8000:
        return read_chargen(addr);
      case 0xc000:
      case 0xd000:
        return read_basic(addr);
      case 0xe000:
      case 0xf000:
        return read_kernal(addr);
    }

    return 0;
}

void REGPARM2 store_rom(ADDRESS addr, BYTE value)
{
    switch (addr & 0xf000) {
      case 0x8000:
 	chargen_rom[0x400 + (addr & 0x0fff)] = value;
        break;
      case 0xc000:
      case 0xd000:
        basic_rom[addr & 0x1fff] = value;
        break;
      case 0xe000:
      case 0xf000:
        kernal_rom[addr & 0x1fff] = value;
        break;
    }
}

void REGPARM2 store_via(ADDRESS addr, BYTE value)
{
    if (addr & 0x10)		/* $911x (VIA2) */
	store_via2(addr, value);
    if (addr & 0x20)		/* $912x (VIA1) */
	store_via1(addr, value);
}

BYTE REGPARM1 read_via(ADDRESS addr)
{
    BYTE ret = 0xff;

    if (addr & 0x10)		/* $911x (VIA2) */
	ret &= read_via2(addr);
    if (addr & 0x20)		/* $912x (VIA1) */
	ret &= read_via1(addr);

    return ret;
}

static BYTE REGPARM1 read_emuid(ADDRESS addr)
{
    addr &= 0xff;
    if (addr >= 0xa0) {
	return emulator_id[addr - 0xa0];
    }
    return 0xff;
}

static void REGPARM2 store_emuid(ADDRESS addr, BYTE value)
{
    addr &= 0xff;
    if (addr == 0xff) {
	emulator_id[addr - 0xa0] ^= 0xff;
    }
    return;
}

/*-------------------------------------------------------------------*/

static BYTE REGPARM1 read_io3(ADDRESS addr)
{
    if (emu_id_enabled && (addr & 0xff00) == 0x9f00) 
	return read_emuid(addr);
    return 0xff;
}

static void REGPARM2 store_io3(ADDRESS addr, BYTE value)
{
    if (emu_id_enabled && (addr & 0xff00) == 0x9f00) 
	store_emuid(addr, value);
    return;
}

static BYTE REGPARM1 read_io2(ADDRESS addr)
{
    if (ieee488_enabled) {
	if (addr & 0x10) {
	    return read_ieeevia2(addr);
	} else {
	    return read_ieeevia1(addr);
	}
    }
    return 0xff;
}

static void REGPARM2 store_io2(ADDRESS addr, BYTE value)
{
    if (ieee488_enabled) {
	if (addr & 0x10) {
	    store_ieeevia2(addr, value);
	} else {
	    store_ieeevia1(addr, value);
	}
    }
    return;
}

/*-------------------------------------------------------------------*/

static BYTE REGPARM1 read_dummy(ADDRESS addr)
{
    return 0xff;
}

static void REGPARM2 store_dummy(ADDRESS addr, BYTE value)
{
    return;
}


/* Watchpoint functions */


static BYTE REGPARM1 read_watch(ADDRESS addr)
{
    mon_watch_push_load_addr(addr, e_comp_space);
    return _mem_read_tab_nowatch[addr >> 8] (addr);
}

static void REGPARM2 store_watch(ADDRESS addr, BYTE value)
{
    mon_watch_push_store_addr(addr, e_comp_space);
    _mem_write_tab_nowatch[addr >> 8] (addr, value);
}

/* ------------------------------------------------------------------------- */

/* Generic memory access.  */

void REGPARM2 mem_store(ADDRESS addr, BYTE value)
{
    _mem_write_tab_ptr[addr >> 8] (addr, value);
}

BYTE REGPARM1 mem_read(ADDRESS addr)
{
    return _mem_read_tab_ptr[addr >> 8] (addr);
}

/* ------------------------------------------------------------------------- */

static void set_mem(int start_page, int end_page,
		    read_func_ptr_t read_func,
		    store_func_ptr_t store_func,
		    BYTE * read_base, int base_mask)
{
    int i;

    if (read_base != NULL) {
	for (i = start_page; i <= end_page; i++) {
	    _mem_read_tab_nowatch[i] = read_func;
	    _mem_write_tab_nowatch[i] = store_func;
	    _mem_read_base_tab[i] = read_base + ((i << 8) & base_mask);
	    mem_read_limit_tab[i] = (end_page << 8) + 0xfd;
	}
    } else {
	for (i = start_page; i <= end_page; i++) {
	    _mem_read_tab_nowatch[i] = read_func;
	    _mem_write_tab_nowatch[i] = store_func;
	    _mem_read_base_tab[i] = NULL;
	    mem_read_limit_tab[i] = -1;
	}
    }
}

void mem_set_bank_pointer(BYTE **base, int *limit)
{
    /* We do not need MMU support.  */
}

int vic20_mem_enable_rom_block(int num)
{
    if (num == 1 || num == 2 || num == 3 || num == 5) {
	set_mem(num * 0x20, num * 0x20 + 0x1f,
		read_cartrom, store_dummy,
		cartrom, 0xffff);
	return 0;
    } else
	return -1;
}

int vic20_mem_enable_ram_block(int num)
{
    if (num == 0) {
	set_mem(0x04, 0x0f,
		read_ram, store_ram,
		ram, 0xffff);
	return 0;
    } else if (num > 0 && num != 4 && num <= 5) {
	set_mem(num * 0x20, num * 0x20 + 0x1f,
		read_ram, store_ram,
		ram, 0xffff);
	return 0;
    } else
	return -1;
}

int vic20_mem_disable_ram_block(int num)
{
    if (num == 0) {
	set_mem(0x04, 0x0f,
		read_dummy, store_dummy,
		ram, 0xffff);
	return 0;
    } else if (num > 0 && num != 4 && num <= 5) {
	set_mem(num * 0x20, num * 0x20 + 0x1f,
		read_dummy, store_dummy,
		ram, 0xffff);
	return 0;
    } else
	return -1;
}

void initialize_memory(void)
{
    int i;


    /* Setup low standard RAM at $0000-$0300. */
    set_mem(0x00, 0x03,
	    read_ram, store_ram,
	    ram, 0xffff);

    /* Setup more low RAM at $1000-$1FFF.  */
    set_mem(0x10, 0x1b,
	    read_ram, store_ram,
	    ram, 0xffff);
    set_mem(0x1c, 0x1f,
	    read_ram, store_wrap,
	    ram, 0xffff);

    /* Setup RAM at $0400-$0FFF.  */
    if (ram_block_0_enabled)
	vic20_mem_enable_ram_block(0);
    else
	vic20_mem_disable_ram_block(0);

    /* Setup RAM or cartridge ROM at $2000-$3FFF.  */
    if (mem_rom_blocks & (VIC_ROM_BLK1A | VIC_ROM_BLK1B)) {
        vic20_mem_enable_rom_block(1);
    } else 
    if (ram_block_1_enabled) {
	vic20_mem_enable_ram_block(1);
    } else {
	vic20_mem_disable_ram_block(1);
    }

    /* Setup RAM or cartridge ROM at $4000-$5FFF.  */
    if (mem_rom_blocks & (VIC_ROM_BLK2A | VIC_ROM_BLK2B)) {
        vic20_mem_enable_rom_block(2);
    } else 
    if (ram_block_2_enabled) {
	vic20_mem_enable_ram_block(2);
    } else {
	vic20_mem_disable_ram_block(2);
    }

    /* Setup RAM or cartridge ROM at $6000-$7FFF.  */
    if (mem_rom_blocks & (VIC_ROM_BLK3A | VIC_ROM_BLK3B)) {
        vic20_mem_enable_rom_block(3);
    } else 
    if (ram_block_3_enabled) {
	vic20_mem_enable_ram_block(3);
    } else {
	vic20_mem_disable_ram_block(3);
    }

    /* Setup RAM or cartridge ROM at $A000-$BFFF.  */
    if (mem_rom_blocks & (VIC_ROM_BLK5A | VIC_ROM_BLK5B)) {
        vic20_mem_enable_rom_block(5);
    } else 
    if (ram_block_5_enabled) {
	vic20_mem_enable_ram_block(5);
    } else {
	vic20_mem_disable_ram_block(5);
    }

    /* Setup character generator ROM at $8000-$8FFF. */
    set_mem(0x80, 0x8f,
	    read_chargen, store_dummy,
	    chargen_rom + 0x400, 0x0fff);

    /* Setup VIC-I at $9000-$90FF. */
    set_mem(0x90, 0x90,
	    read_vic, store_vic,
	    NULL, 0);

    /* Setup VIAs at $9100-$93FF. */
    set_mem(0x91, 0x93,
	    read_via, store_via,
	    NULL, 0);

    /* Setup color memory at $9400-$97FF.
       Warning: we use a kludge here.  Instead of mapping the color memory
       separately, we map it directly in the corresponding RAM address
       space. */
    set_mem(0x94, 0x97,
	    read_ram, store_ram,
	    ram, 0xffff);

    /* Setup I/O2 at the expansion port */
    set_mem(0x98, 0x9b,
	    read_io2, store_io2,
	    NULL, 0);

    /* Setup I/O3 at the expansion port (includes emulator ID) */
    set_mem(0x9c, 0x9f,
	    read_io3, store_io3,
	    NULL, 0);

    /* Setup BASIC ROM at $C000-$DFFF. */
    set_mem(0xc0, 0xdf,
	    read_basic, store_dummy,
	    basic_rom, 0x1fff);

    /* Setup Kernal ROM at $E000-$FFFF. */
    set_mem(0xe0, 0xff,
	    read_kernal, store_dummy,
	    kernal_rom, 0x1fff);

    _mem_read_tab_nowatch[0x100] = _mem_read_tab_nowatch[0];
    _mem_write_tab_nowatch[0x100] = _mem_write_tab_nowatch[0];
    _mem_read_base_tab[0x100] = _mem_read_base_tab[0];
    mem_read_limit_tab[0x100] = -1;

    _mem_read_base_tab_ptr = _mem_read_base_tab;
    mem_read_limit_tab_ptr = mem_read_limit_tab;

    for (i = 0; i <= 0x100; i++) {
	_mem_read_tab_watch[i] = read_watch;
	_mem_write_tab_watch[i] = store_watch;
    }

    mem_toggle_watchpoints(0);
}

void mem_toggle_watchpoints(int flag)
{
    if (flag) {
	_mem_read_tab_ptr = _mem_read_tab_watch;
	_mem_write_tab_ptr = _mem_write_tab_watch;
    } else {
	_mem_read_tab_ptr = _mem_read_tab_nowatch;
	_mem_write_tab_ptr = _mem_write_tab_nowatch;
    }
}

/* ------------------------------------------------------------------------- */

/* Initialize RAM for power-up.  */
void mem_powerup(void)
{
    int i;

    for (i = 0; i < VIC20_RAM_SIZE; i += 0x80) {
	memset(ram + i, 0, 0x40);
	memset(ram + i + 0x40, 0xff, 0x40);
    }
}

static int mem_kernal_checksum(void)
{
    int i;
    WORD sum;

    /* Check Kernal ROM.  */
    for (i = 0, sum = 0; i < VIC20_KERNAL_ROM_SIZE; i++)
	sum += kernal_rom[i];

    if (sum != VIC20_KERNAL_CHECKSUM) {
	log_error(vic20_mem_log,
                  "Warning: Unknown Kernal image.  Sum: %d ($%04X).",
                  sum, sum);
    }
    return 0;
}

static int mem_load_kernal(void) 
{
    int trapfl;

    if(!rom_loaded) return 0;

    /* disable traps before saving the ROM */
    resources_get_value("VirtualDevices", (resource_value_t*) &trapfl);
    resources_set_value("VirtualDevices", (resource_value_t) 1);

    if(!IS_NULL(kernal_rom_name)) {
        /* Load Kernal ROM. */
        if (sysfile_load(kernal_rom_name,
            kernal_rom, VIC20_KERNAL_ROM_SIZE,
            VIC20_KERNAL_ROM_SIZE) < 0) {
	    log_error(vic20_mem_log, "Couldn't load kernal ROM.");
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
    int i;
    WORD sum;

    /* Check Basic ROM. */
    for (i = 0, sum = 0; i < VIC20_BASIC_ROM_SIZE; i++)
	sum += basic_rom[i];

    if (sum != VIC20_BASIC_CHECKSUM)
	log_error(vic20_mem_log,
                  "Warning: Unknown Basic image.  Sum: %d ($%04X).",
                  sum, sum);
    return 0;
}

static int mem_load_basic(void) 
{
    if(!rom_loaded) return 0;

    if(!IS_NULL(basic_rom_name)) {
        /* Load Basic ROM. */
        if (sysfile_load(basic_rom_name,
            basic_rom, VIC20_BASIC_ROM_SIZE,
            VIC20_BASIC_ROM_SIZE) < 0) {
	    log_error(vic20_mem_log, "Couldn't load basic ROM.");
	    return -1;
	}
    }
    return mem_basic_checksum();
}

static int mem_load_chargen(void) 
{
    if(!rom_loaded) return 0;

    if(!IS_NULL(chargen_rom_name)) {
        /* Load chargen ROM. */
        if (sysfile_load(chargen_rom_name,
            chargen_rom + 0x400, VIC20_CHARGEN_ROM_SIZE,
            VIC20_CHARGEN_ROM_SIZE) < 0) {
	    log_error(vic20_mem_log, "Couldn't load character ROM.");
	    return -1;
	}
    }
    return 0;
}

/* Load ROMs at startup.  This is half-stolen from the old `load_mem()' in
   `memory.c'. */
int mem_load(void)
{
    if (vic20_mem_log == LOG_ERR)
        vic20_mem_log = log_open("VIC20MEM");

    mem_powerup();

    rom_loaded = 1;

    if (mem_load_kernal() < 0) 
	return -1;

    if (mem_load_basic() < 0) 
	return -1;

    if( mem_load_chargen() < 0) 
	return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */

void mem_attach_cartridge(int type, BYTE * rawcart)
{
    switch(type) {
      case CARTRIDGE_VIC20_4KB_2000:
        log_message(vic20_mem_log, "CART: attaching 4KB cartridge at $2000.");
        memcpy(cartrom + 0x2000, rawcart, 0x2000);
	mem_rom_blocks |= VIC_ROM_BLK1A;
	set_ram_block_1_enabled((resource_value_t) 0);
	break;
      case CARTRIDGE_VIC20_8KB_2000:
        log_message(vic20_mem_log, "CART: attaching 8KB cartridge at $2000.");
        memcpy(cartrom + 0x2000, rawcart, 0x2000);
        mem_rom_blocks |= VIC_ROM_BLK1A | VIC_ROM_BLK1B;
	set_ram_block_1_enabled((resource_value_t) 0);
        break;
      case CARTRIDGE_VIC20_16KB_2000:
        log_message(vic20_mem_log, "CART: attaching 16KB cartridge at $2000.");
        memcpy(cartrom + 0x2000, rawcart, 0x2000);
        memcpy(cartrom + 0xA000, rawcart + 0x2000, 0x2000);
        mem_rom_blocks |= VIC_ROM_BLK1A | VIC_ROM_BLK1B
			| VIC_ROM_BLK5A | VIC_ROM_BLK5B;
	set_ram_block_1_enabled((resource_value_t) 0);
	set_ram_block_5_enabled((resource_value_t) 0);
        break;

      case CARTRIDGE_VIC20_4KB_4000:
        log_message(vic20_mem_log, "CART: attaching 4KB cartridge at $4000.");
        memcpy(cartrom + 0x4000, rawcart, 0x2000);
	mem_rom_blocks |= VIC_ROM_BLK2A;
	set_ram_block_2_enabled((resource_value_t) 0);
	break;
      case CARTRIDGE_VIC20_8KB_4000:
        log_message(vic20_mem_log, "CART: attaching 8KB cartridge at $4000.");
        memcpy(cartrom + 0x4000, rawcart, 0x2000);
        mem_rom_blocks |= VIC_ROM_BLK2A | VIC_ROM_BLK2B;
	set_ram_block_2_enabled((resource_value_t) 0);
        break;
      case CARTRIDGE_VIC20_16KB_4000:
        log_message(vic20_mem_log, "CART: attaching 16KB cartridge at $4000.");
        memcpy(cartrom + 0x4000, rawcart, 0x2000);
        memcpy(cartrom + 0xA000, rawcart + 0x2000, 0x2000);
        mem_rom_blocks |= VIC_ROM_BLK2A | VIC_ROM_BLK2B
			| VIC_ROM_BLK5A | VIC_ROM_BLK5B;
	set_ram_block_2_enabled((resource_value_t) 0);
	set_ram_block_5_enabled((resource_value_t) 0);
        break;

      case CARTRIDGE_VIC20_4KB_6000:
        log_message(vic20_mem_log, "CART: attaching 4KB cartridge at $6000.");
        memcpy(cartrom + 0x6000, rawcart, 0x2000);
        mem_rom_blocks |= VIC_ROM_BLK3A;
	set_ram_block_3_enabled((resource_value_t) 0);
        break;
      case CARTRIDGE_VIC20_8KB_6000:
        log_message(vic20_mem_log, "CART: attaching 8KB cartridge at $6000.");
        memcpy(cartrom + 0x6000, rawcart, 0x2000);
        mem_rom_blocks |= VIC_ROM_BLK3A | VIC_ROM_BLK3B;
	set_ram_block_3_enabled((resource_value_t) 0);
        break;
      case CARTRIDGE_VIC20_16KB_6000:
        log_message(vic20_mem_log, "CART: attaching 16KB cartridge at $6000.");
        memcpy(cartrom + 0x6000, rawcart, 0x2000);
        memcpy(cartrom + 0xA000, rawcart + 0x2000, 0x2000);
        mem_rom_blocks |= VIC_ROM_BLK3A | VIC_ROM_BLK3B
			| VIC_ROM_BLK5A | VIC_ROM_BLK5B;
	set_ram_block_3_enabled((resource_value_t) 0);
	set_ram_block_5_enabled((resource_value_t) 0);
        break;

      case CARTRIDGE_VIC20_4KB_A000:
        log_message(vic20_mem_log, "CART: attaching 4KB cartridge at $A000.");
        memcpy(cartrom + 0xa000, rawcart, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK5A;
	set_ram_block_5_enabled((resource_value_t) 0);
        break;
      case CARTRIDGE_VIC20_8KB_A000:
        log_message(vic20_mem_log, "CART: attaching 8KB cartridge at $A000.");
        memcpy(cartrom + 0xA000, rawcart, 0x2000);
        mem_rom_blocks |= VIC_ROM_BLK5A | VIC_ROM_BLK5B;
	set_ram_block_5_enabled((resource_value_t) 0);
        break;

      case CARTRIDGE_VIC20_4KB_B000:
        log_message(vic20_mem_log, "CART: attaching 4KB cartridge at $B000.");
        memcpy(cartrom + 0xB000, rawcart, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK5B;
	set_ram_block_5_enabled((resource_value_t) 0);
        break;
      default:
        log_error(vic20_mem_log, "Unknown Cartridge Type!");
        return;
    }

    initialize_memory();
    return;
}

void mem_detach_cartridge(int type)
{
    switch(type) {
      case CARTRIDGE_VIC20_16KB_2000:
        mem_rom_blocks &= ~(VIC_ROM_BLK5A | VIC_ROM_BLK5B);
      case CARTRIDGE_VIC20_8KB_2000:
      case CARTRIDGE_VIC20_4KB_2000:
        mem_rom_blocks &= ~(VIC_ROM_BLK1A | VIC_ROM_BLK1B);
        break;
      case CARTRIDGE_VIC20_16KB_4000:
        mem_rom_blocks &= ~(VIC_ROM_BLK5A | VIC_ROM_BLK5B);
      case CARTRIDGE_VIC20_8KB_4000:
      case CARTRIDGE_VIC20_4KB_4000:
        mem_rom_blocks &= ~(VIC_ROM_BLK2A | VIC_ROM_BLK2B);
	break;
      case CARTRIDGE_VIC20_16KB_6000:
        mem_rom_blocks &= ~(VIC_ROM_BLK5A | VIC_ROM_BLK5B);
      case CARTRIDGE_VIC20_8KB_6000:
      case CARTRIDGE_VIC20_4KB_6000:
        mem_rom_blocks &= ~(VIC_ROM_BLK3A | VIC_ROM_BLK3B);
        break;
      case CARTRIDGE_VIC20_8KB_A000:
      case CARTRIDGE_VIC20_4KB_A000:
      case CARTRIDGE_VIC20_4KB_B000:
        mem_rom_blocks &= ~(VIC_ROM_BLK5A | VIC_ROM_BLK5B);
        break;
      default:
        return;
    }

    initialize_memory();
    return;
}

/* ------------------------------------------------------------------------- */

/* FIXME: this part needs to be checked. */

void mem_get_basic_text(ADDRESS * start, ADDRESS * end)
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

/* ------------------------------------------------------------------------- */

int mem_rom_trap_allowed(ADDRESS addr)
{
    return addr >= 0xe000;
}

/* ------------------------------------------------------------------------- */

/* Banked memory access functions for the monitor */

/* Exported banked memory access functions for the monitor */

/* FIXME: peek */

static const char *banknames[] = {
    "default", "cpu", NULL
};

const int banknums[] = {
    0, 0
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
      case 0:			/* current */
        return mem_read(addr);
        break;
    }
    return 0xff;
}

BYTE mem_bank_peek(int bank, ADDRESS addr)
{
    switch (bank) {
      case 0:			/* current */
        return mem_read(addr);	/* FIXME */
        break;
    }
    return mem_bank_read(bank, addr);
}

void mem_bank_write(int bank, ADDRESS addr, BYTE byte)
{
    switch (bank) {
      case 0:			/* current */
        mem_store(addr, byte);
        return;
    }
}

/************************************************************************/

/*
 * VIC20 memory dump contains the available memory at the moment
 */
#define VIC20MEM_DUMP_VER_MAJOR   1
#define VIC20MEM_DUMP_VER_MINOR   0
#define SNAP_MEM_MODULE_NAME      "VIC20MEM"

/*
 * UBYTE        CONFIG          Bit 0: 1 = expansion block 0 RAM enabled
 *				    1: 1 = expansion block 1 RAM enabled
 *				    2: 1 = expansion block 2 RAM enabled
 *				    3: 1 = expansion block 3 RAM enabled
 *				    5: 1 = expansion block 5 RAM enabled
 *
 * ARRAY	RAM0		1k RAM $0000-$03ff
 * ARRAY	RAM1		4k RAM $1000-$1fff
 * ARRAY	COLORRAM	2k Video RAM
 * ARRAY	BLK0		3k RAM $0400-$0fff (if blk 0 RAM enabled)
 * ARRAY	BLK1		8k RAM $2000-$3fff (if blk 1 RAM enabled)
 * ARRAY	BLK2		8k RAM $4000-$5fff (if blk 2 RAM enabled)
 * ARRAY	BLK3		8k RAM $6000-$7fff (if blk 3 RAM enabled)
 * ARRAY	BLK5		8k RAM $A000-$Bfff (if blk 5 RAM enabled)
 *
 */


static int mem_write_ram_snapshot_module(snapshot_t *p)
{
    snapshot_module_t *m;
    BYTE config;

    config = (ram_block_0_enabled ? 1 : 0)
		| (ram_block_1_enabled ? 2 : 0)
		| (ram_block_2_enabled ? 4 : 0)
		| (ram_block_3_enabled ? 8 : 0)
		| (ram_block_5_enabled ? 32 : 0) ;

    m = snapshot_module_create(p, SNAP_MEM_MODULE_NAME,
                               VIC20MEM_DUMP_VER_MAJOR, VIC20MEM_DUMP_VER_MINOR);
    if (m == NULL)
        return -1;

    snapshot_module_write_byte(m, config);

    snapshot_module_write_byte_array(m, ram, 0x0400);
    snapshot_module_write_byte_array(m, ram + 0x1000, 0x1000);
    snapshot_module_write_byte_array(m, ram + 0x9400, 0x0800);

    if (config & 1) {
        snapshot_module_write_byte_array(m, ram + 0x0400, 0x0c00);
    }
    if (config & 2) {
        snapshot_module_write_byte_array(m, ram + 0x2000, 0x2000);
    }
    if (config & 4) {
        snapshot_module_write_byte_array(m, ram + 0x4000, 0x2000);
    }
    if (config & 8) {
        snapshot_module_write_byte_array(m, ram + 0x6000, 0x2000);
    }
    if (config & 32) {
        snapshot_module_write_byte_array(m, ram + 0xA000, 0x2000);
    }

    snapshot_module_close(m);

    return 0;
}

static int mem_read_ram_snapshot_module(snapshot_t *p)
{
    BYTE vmajor, vminor;
    snapshot_module_t *m;
    BYTE config;

    m = snapshot_module_open(p, SNAP_MEM_MODULE_NAME, &vmajor, &vminor);
    if (m == NULL)
        return -1;

    if (vmajor != VIC20MEM_DUMP_VER_MAJOR) {
        snapshot_module_close(m);
        return -1;
    }

    snapshot_module_read_byte(m, &config);

    snapshot_module_read_byte_array(m, ram, 0x0400);
    snapshot_module_read_byte_array(m, ram + 0x1000, 0x1000);
    /* setup wraparound copy of chargen */
    memcpy(chargen_rom, ram + 0x1c00, 0x400);

    snapshot_module_read_byte_array(m, ram + 0x9400, 0x0800);

    set_ram_block_0_enabled((resource_value_t)(config & 1));
    if (config & 1) {
        snapshot_module_read_byte_array(m, ram + 0x0400, 0x0c00);
    }
    set_ram_block_1_enabled((resource_value_t)(config & 2));
    if (config & 2) {
        snapshot_module_read_byte_array(m, ram + 0x2000, 0x2000);
    }
    set_ram_block_2_enabled((resource_value_t)(config & 4));
    if (config & 4) {
        snapshot_module_read_byte_array(m, ram + 0x4000, 0x2000);
    }
    set_ram_block_3_enabled((resource_value_t)(config & 8));
    if (config & 8) {
        snapshot_module_read_byte_array(m, ram + 0x6000, 0x2000);
    }
    set_ram_block_5_enabled((resource_value_t)(config & 32));
    if (config & 32) {
        snapshot_module_read_byte_array(m, ram + 0xA000, 0x2000);
    }

    snapshot_module_close(m);

    initialize_memory();

    return 0;
}

/*
 * VIC20 ROM dump
 */
#define VIC20ROM_DUMP_VER_MAJOR   1
#define VIC20ROM_DUMP_VER_MINOR   1
#define SNAP_ROM_MODULE_NAME      "VIC20ROM"

/*
 * UBYTE        CONFIG          Bit 0: 1 = ROM block $2*** enabled
 *				    1: 1 = ROM block $3*** enabled
 *				    2: 1 = ROM block $4*** enabled
 *				    3: 1 = ROM block $5*** enabled
 *				    4: 1 = ROM block $6*** enabled
 *				    5: 1 = ROM block $7*** enabled
 *				    6: 1 = ROM block $A*** enabled
 *				    7: 1 = ROM block $B*** enabled
 *
 * ARRAY	KERNAL		8k KERNAL ROM $e000-$ffff
 * ARRAY	BASIC		8k BASIC ROM $c000-$dfff
 * ARRAY	CHARGEN		4k CHARGEN ROM 
 * ARRAY	BLK1A		4k ROM $2*** (if CONFIG & 1)
 * ARRAY	BLK1B		4k ROM $3*** (if CONFIG & 2)
 * ARRAY	BLK3A		4k ROM $6*** (if CONFIG & 16)
 * ARRAY	BLK3B		4k ROM $7*** (if CONFIG & 32) 
 * ARRAY	BLK5A		4k ROM $A*** (if CONFIG & 64)
 * ARRAY	BLK5B		4k ROM $B*** (if CONFIG & 128) 
 *
 * ARRAY	BLK2A		4k ROM $4*** (if CONFIG & 4) 
 * ARRAY	BLK2B		4k ROM $5*** (if CONFIG & 8) 
 */


static int mem_write_rom_snapshot_module(snapshot_t *p, int save_roms)
{
    snapshot_module_t *m;
    BYTE config;
    int trapfl;
    
    if (!save_roms)
        return 0;

    m = snapshot_module_create(p, SNAP_ROM_MODULE_NAME,
                          VIC20MEM_DUMP_VER_MAJOR, VIC20MEM_DUMP_VER_MINOR);
    if (m == NULL)
        return -1;

    /* disable traps before saving the ROM */
    resources_get_value("VirtualDevices", (resource_value_t*) &trapfl);
    resources_set_value("VirtualDevices", (resource_value_t) 1);

    config = mem_rom_blocks;

    snapshot_module_write_byte(m, config);

    /* save kernal */
    snapshot_module_write_byte_array(m, rom + 0x2000, 0x2000);
    /* save basic */
    snapshot_module_write_byte_array(m, rom + 0x0000, 0x2000);

    snapshot_module_write_byte_array(m, chargen_rom + 0x400, 0x1000);

    if (config & 1) {
        snapshot_module_write_byte_array(m, cartrom + 0x2000, 0x1000);
    }
    if (config & 2) {
        snapshot_module_write_byte_array(m, cartrom + 0x3000, 0x1000);
    }
    if (config & 16) {
        snapshot_module_write_byte_array(m, cartrom + 0x6000, 0x1000);
    }
    if (config & 32) {
        snapshot_module_write_byte_array(m, cartrom + 0x7000, 0x1000);
    }
    if (config & 64) {
        snapshot_module_write_byte_array(m, cartrom + 0xA000, 0x1000);
    }
    if (config & 128) {
        snapshot_module_write_byte_array(m, cartrom + 0xB000, 0x1000);
    }
    if (config & 4) {
        snapshot_module_write_byte_array(m, cartrom + 0x4000, 0x1000);
    }
    if (config & 8) {
        snapshot_module_write_byte_array(m, cartrom + 0x5000, 0x1000);
    }

    /* enable traps again when necessary */
    resources_set_value("VirtualDevices", (resource_value_t) trapfl);

    snapshot_module_close(m);

    return 0;
}

static int mem_read_rom_snapshot_module(snapshot_t *p)
{
    BYTE vmajor, vminor;
    snapshot_module_t *m;
    BYTE config;
    int trapfl;

    m = snapshot_module_open(p, SNAP_ROM_MODULE_NAME, &vmajor, &vminor);
    if (m == NULL)
        return 0;	/* optional */

    if (vmajor != VIC20ROM_DUMP_VER_MAJOR) {
        snapshot_module_close(m);
        return -1;
    }

    /* disable traps before loading the ROM */
    resources_get_value("VirtualDevices", (resource_value_t*) &trapfl);
    resources_set_value("VirtualDevices", (resource_value_t) 1);

    snapshot_module_read_byte(m, &config);

    /* read kernal */
    snapshot_module_read_byte_array(m, rom + 0x2000, 0x2000);
    /* read basic */
    snapshot_module_read_byte_array(m, rom + 0x0000, 0x2000);

    snapshot_module_read_byte_array(m, chargen_rom + 0x400, 0x1000);

    mem_rom_blocks = 0;

    if (config & 1) {
        snapshot_module_read_byte_array(m, cartrom + 0x2000, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK1A;
    }
    if (config & 2) {
        snapshot_module_read_byte_array(m, cartrom + 0x3000, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK1B;
    }
    if (config & 16) {
        snapshot_module_read_byte_array(m, cartrom + 0x6000, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK3A;
    }
    if (config & 32) {
        snapshot_module_read_byte_array(m, cartrom + 0x7000, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK3B;
    }
    if (config & 64) {
        snapshot_module_read_byte_array(m, cartrom + 0xA000, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK5A;
    }
    if (config & 128) {
        snapshot_module_read_byte_array(m, cartrom + 0xB000, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK5B;
    }
    if (config & 4) {
        snapshot_module_read_byte_array(m, cartrom + 0x4000, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK2A;
    }
    if (config & 8) {
        snapshot_module_read_byte_array(m, cartrom + 0x5000, 0x1000);
        mem_rom_blocks |= VIC_ROM_BLK2B;
    }

    mem_kernal_checksum();
    mem_basic_checksum();

    log_warning(vic20_mem_log,"Dumped Romset files and saved settings will "
                "represent\nthe state before loading the snapshot!");

    /* enable traps again when necessary */
    resources_set_value("VirtualDevices", (resource_value_t) trapfl);

    snapshot_module_close(m);

    initialize_memory();

    return 0;
}

int mem_write_snapshot_module(snapshot_t *m, int save_roms)
{
    if (mem_write_ram_snapshot_module(m) < 0
	|| mem_write_rom_snapshot_module(m, save_roms) < 0 )
	return -1;
    return 0;
}

int mem_read_snapshot_module(snapshot_t *m)
{
    if (mem_read_ram_snapshot_module(m) < 0
	|| mem_read_rom_snapshot_module(m) < 0 )
	return -1;
    return 0;
}

