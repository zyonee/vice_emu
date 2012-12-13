/*
 * scpu64mem.c -- SCPU64 memory handling.
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include "alarm.h"
#include "scpu64.h"
#include "scpu64-resources.h"
#include "c64cart.h"
#include "c64cia.h"
#include "scpu64mem.h"
#include "scpu64meminit.h"
#include "scpu64memlimit.h"
#include "c64pla.h"
#include "c64cartmem.h"
#include "cartio.h"
#include "cartridge.h"
#include "cia.h"
#include "clkguard.h"
#include "machine.h"
#include "maincpu.h"
#include "mem.h"
#include "monitor.h"
#include "ram.h"
#include "reu.h"
#include "sid.h"
#include "tpi.h"
#include "vicii-mem.h"
#include "vicii-phi1.h"
#include "vicii.h"
#include "scpu64cpu.h"

/* Machine class (moved from c64.c to distinguish between x64 and x64sc) */
int machine_class = VICE_MACHINE_C64;

/* C64 memory-related resources.  */

/* ------------------------------------------------------------------------- */

/* Number of possible memory configurations.  */
#define NUM_CONFIGS     64

/* Number of possible video banks (16K each).  */
#define NUM_VBANKS      4

/* The C64 memory.  */
BYTE mem_ram[SCPU64_RAM_SIZE];
BYTE mem_sram[SCPU64_SRAM_SIZE];
BYTE mem_simm_ram[SCPU64_SIMM_SIZE];
BYTE scpu64memrom_scpu64_rom[SCPU64_SCPU64_ROM_SIZE];

#ifdef USE_EMBEDDED
#include "c64chargen.h"
#else
BYTE mem_chargen_rom[SCPU64_CHARGEN_ROM_SIZE];
#endif

/* Internal color memory.  */
static BYTE mem_color_ram[0x400];
BYTE *mem_color_ram_cpu, *mem_color_ram_vicii;

/* Pointer to the chargen ROM.  */
BYTE *mem_chargen_rom_ptr;

/* Pointers to the currently used memory read and write tables.  */
read_func_ptr_t *_mem_read_tab_ptr;
store_func_ptr_t *_mem_write_tab_ptr;
static BYTE **_mem_read_base_tab_ptr;
static DWORD *mem_read_limit_tab_ptr;

/* Memory read and write tables.  */
static store_func_ptr_t mem_write_tab[NUM_VBANKS][NUM_CONFIGS][0x101];
static read_func_ptr_t mem_read_tab[NUM_CONFIGS][0x101];
static BYTE *mem_read_base_tab[NUM_CONFIGS][0x101];
static DWORD mem_read_limit_tab[NUM_CONFIGS][0x101];

static store_func_ptr_t mem_write_tab_watch[0x101];
static read_func_ptr_t mem_read_tab_watch[0x101];

/* Current video bank (0, 1, 2 or 3).  */
static int vbank;

/* Current memory configuration.  */
static int mem_config;

/* Tape sense status: 1 = some button pressed, 0 = no buttons pressed.  */
static int tape_sense = 0;

/* Current watchpoint state. 1 = watchpoints active, 0 = no watchpoints */
static int watchpoints_active;


static int reg_sw_1mhz;     /* 1MHz physical switch */
static int reg_sw_jiffy = 1;/* Jiffy physical switch */
static int reg_soft_1mhz;   /* 1MHz software enabled */
static int reg_sys_1mhz;    /* 1MHz system enabled */
static int reg_hwenable;    /* hardware enabled */
static int reg_dosext;      /* dos extension enable */
static int reg_optim;       /* optimization mode */
static int reg_bootmap = 1; /* boot map */
static int reg_simm;        /* simm configuration */

/* ------------------------------------------------------------------------- */

static BYTE read_watch(WORD addr)
{
    monitor_watch_push_load_addr(addr, e_comp_space);
    return mem_read_tab[mem_config][addr >> 8](addr);
}

static void store_watch(WORD addr, BYTE value)
{
    monitor_watch_push_store_addr(addr, e_comp_space);
    mem_write_tab[vbank][mem_config][addr >> 8](addr, value);
}

void mem_toggle_watchpoints(int flag, void *context)
{
    if (flag) {
        _mem_read_tab_ptr = mem_read_tab_watch;
        _mem_write_tab_ptr = mem_write_tab_watch;
    } else {
        _mem_read_tab_ptr = mem_read_tab[mem_config];
        _mem_write_tab_ptr = mem_write_tab[vbank][mem_config];
    }
    watchpoints_active = flag;
}

/* ------------------------------------------------------------------------- */

void scpu64_mem_init(void)
{
}

void mem_pla_config_changed(void)
{
    mem_config = ((pport.data & 0x7) | (export.exrom << 3) | (export.game << 4) | (reg_bootmap << 5));

    c64pla_config_changed(tape_sense, 1, 0x17);

    if (watchpoints_active) {
        _mem_read_tab_ptr = mem_read_tab_watch;
        _mem_write_tab_ptr = mem_write_tab_watch;
    } else {
        _mem_read_tab_ptr = mem_read_tab[mem_config];
        _mem_write_tab_ptr = mem_write_tab[vbank][mem_config];
    }

    _mem_read_base_tab_ptr = mem_read_base_tab[mem_config];
    mem_read_limit_tab_ptr = mem_read_limit_tab[mem_config];

    maincpu_resync_limits();
}

BYTE zero_read(WORD addr)
{
    addr &= 0xff;

    switch ((BYTE)addr) {
        case 0:
            return pport.dir_read;
        case 1:
            return pport.data_read;
    }

    return mem_ram[addr & 0xff];
}
void zero_store(WORD addr, BYTE value)
{
    addr &= 0xff;

    mem_sram[addr] = value;

    switch ((BYTE)addr) {
        case 0:
            if (vbank == 0) {
                vicii_mem_vbank_store((WORD)0, vicii_read_phi1_lowlevel());
            } else {
                mem_ram[0] = vicii_read_phi1_lowlevel();
                machine_handle_pending_alarms(maincpu_rmw_flag + 1);
            }
            pport.dir = value;
            break;
        case 1:
            if (vbank == 0) {
                vicii_mem_vbank_store((WORD)1, vicii_read_phi1_lowlevel());
            } else {
                mem_ram[1] = vicii_read_phi1_lowlevel();
                machine_handle_pending_alarms(maincpu_rmw_flag + 1);
            }

            if (pport.data != value) {
                pport.data = value;
                mem_pla_config_changed();
            }
            break;
        default:
            if (vbank == 0) {
                vicii_mem_vbank_store(addr, value);
            } else {
                mem_ram[addr] = value;
            }
    }
}

/* ------------------------------------------------------------------------- */

BYTE chargen_read(WORD addr)
{
    return mem_chargen_rom[addr & 0xfff];
}

void chargen_store(WORD addr, BYTE value)
{
    mem_chargen_rom[addr & 0xfff] = value;
}

BYTE ram_read(WORD addr)
{
    return mem_sram[addr];
}

void ram_store(WORD addr, BYTE value)
{
    mem_sram[addr] = value;
    mem_ram[addr] = value;
}

void ram_hi_store(WORD addr, BYTE value)
{
    mem_sram[addr] = value;

    if (vbank == 3) {
        vicii_mem_vbank_3fxx_store(addr, value);
    } else {
        mem_ram[addr] = value;
    }

    if (addr == 0xff00) {
        reu_dma(-1);
    }
}

BYTE scpu64_kernal_read(WORD addr)
{
    return reg_hwenable ? mem_sram[0x8000 + addr] : mem_sram[0x10000 + addr];
}

BYTE scpu64_basic_read(WORD addr)
{
    return mem_sram[0x10000 + addr];
}

BYTE scpu64memrom_scpu64_read(WORD addr)
{
    return scpu64memrom_scpu64_rom[addr];
}

/* ------------------------------------------------------------------------- */

/* Generic memory access.  */

void mem_store(WORD addr, BYTE value)
{
    _mem_write_tab_ptr[addr >> 8](addr, value);
}

BYTE mem_read(WORD addr)
{
    return _mem_read_tab_ptr[addr >> 8](addr);
}

void mem_store2(DWORD addr, BYTE value)
{
    switch (addr & 0xfe0000) {
    case 0xf60000:
        if (SCPU64_SIMM_SIZE > 0 && addr < SCPU64_SIMM_SIZE + 0xf60000) {
            mem_simm_ram[addr & 0x1ffff] = value;
        } 
    case 0xf80000:
    case 0xfa0000:
    case 0xfc0000:
    case 0xfe0000:
        return;
    case 0x000000:
        if (addr < SCPU64_RAM_SIZE) {
            mem_ram[addr] = value;
        }
        mem_sram[addr] = value;
        return;
    default:
        if (SCPU64_SIMM_SIZE > 0 && addr < SCPU64_SIMM_SIZE) {
            mem_simm_ram[addr] = value;
        }
    }
}

BYTE mem_read2(DWORD addr)
{
    switch (addr & 0xfe0000) {
    case 0xf60000:
        if (SCPU64_SIMM_SIZE > 0) {
            return mem_simm_ram[addr & (SCPU64_SIMM_SIZE-1) & 0x1ffff];
        }
        break;
    case 0xf80000:
    case 0xfa0000:
    case 0xfc0000:
    case 0xfe0000:
        return scpu64memrom_scpu64_rom[addr & (SCPU64_SCPU64_ROM_SIZE-1) & 0x7ffff];
    case 0x000000:
        return mem_sram[addr];
    default:
        if (SCPU64_SIMM_SIZE > 0 && addr < SCPU64_SIMM_SIZE) {
            return mem_simm_ram[addr];
        }
        break;
    }
    return addr >> 16;
}

void mem_store_without_ultimax(WORD addr, BYTE value)
{
    store_func_ptr_t *write_tab_ptr;

    write_tab_ptr = mem_write_tab[vbank][mem_config & 7];

    write_tab_ptr[addr >> 8](addr, value);
}

BYTE mem_read_without_ultimax(WORD addr)
{
    read_func_ptr_t *read_tab_ptr;

    read_tab_ptr = mem_read_tab[mem_config & 7];

    return read_tab_ptr[addr >> 8](addr);
}

void mem_store_without_romlh(WORD addr, BYTE value)
{
    store_func_ptr_t *write_tab_ptr;

    write_tab_ptr = mem_write_tab[vbank][0];

    write_tab_ptr[addr >> 8](addr, value);
}

/* ------------------------------------------------------------------------- */
static BYTE scpu64_hardware_read(WORD addr)
{
    BYTE value = 0x00;

    switch (addr) {
    case 0xd0b0:
        value = 0x40;
        break;
    case 0xd0b1:
        break;
    case 0xd0b2:       /* bit 7 - hwreg enabled (1)/disabled (0) */
                       /* bit 6 - system 1 MHz enabled (1)/disabled (0) */
        value = (reg_hwenable ? 0x80 : 0x00) | (reg_sys_1mhz ? 0x40 : 0x00);
        break;
    case 0xd0b3:
    case 0xd0b4:
        value = reg_optim & 0xc0;
        break;
    case 0xd0b5:      /* bit 7 - Jiffy (1)/No jiffy (0) switch */
                      /* bit 6 - 1 MHz (1)/20 MHz (0) switch */
        value = (reg_sw_jiffy ? 0x80 : 0x00) | (reg_sw_1mhz ? 0x40 : 0x00);
        break;
    case 0xd0b6:      /* bit 7 - Emulation mode (1)/Native (0) */
        value = 0x80;
        break;
    case 0xd0b7:
        break;
    case 0xd0b9:      /* same as 0xd0b8 */
    case 0xd0b8:      /* bit 7 - software 1 MHz enabled (1)/disabled (0) */
                      /* bit 6 - 1 MHz (1)/20 MHz (2) switch+software+system */
        value = (reg_soft_1mhz ? 0x80 : 0x00) | ((reg_soft_1mhz || reg_sw_1mhz || reg_sys_1mhz) ? 0x40 : 0x00);
        break;
    case 0xd0ba:
        break;
    case 0xd0bb:
        break;
    case 0xd0bc:
    case 0xd0bd:
    case 0xd0be:
    case 0xd0bf:
        value = reg_dosext ? 0x80 : 0x00;
        break;
    default:
        value = 0xff;
        break;
    }
    return value | (reg_optim & 7);
}

void scpu64_hardware_store(WORD addr, BYTE value)
{
    switch (addr) {
    case 0xd071:
        break;
    case 0xd072: /* System 1MHz enable */
        reg_sys_1mhz = 1; 
        scpu64_set_fastmode(!(reg_sys_1mhz | reg_soft_1mhz | reg_sw_1mhz));
        break;
    case 0xd073: /* System 1MHz disable */
        reg_sys_1mhz = 0; 
        scpu64_set_fastmode(!(reg_sys_1mhz | reg_soft_1mhz | reg_sw_1mhz));
        break;
    case 0xd074: /* Optimization modes */
    case 0xd075:
    case 0xd076:
    case 0xd077:
        if (reg_hwenable) {
            reg_optim = (addr << 6);
        }
        break;
    case 0xd078: /* SIMM configuration */
        if (reg_hwenable) {
            reg_simm = value;
        }
        break;
    case 0xd07a: /* Software 1MHz enable */
        reg_soft_1mhz = 1; 
        scpu64_set_fastmode(!(reg_sys_1mhz | reg_soft_1mhz | reg_sw_1mhz));
        break;
    case 0xd079: /* same as 0xd07b */
    case 0xd07b: /* Software 1MHz disable */
        reg_soft_1mhz = 0;
        scpu64_set_fastmode(!(reg_sys_1mhz | reg_soft_1mhz | reg_sw_1mhz));
        break;
    case 0xd07c:
        break;
    case 0xd07e: /* hwreg enable */
        reg_hwenable = 1;
        break;
    case 0xd07d: /* same as 0xd07d */
    case 0xd07f: /* hwreg disable */
        reg_hwenable = 0;
        break;
    case 0xd0b0:
    case 0xd0b1:
        break;
    case 0xd0b2: /* hwenable and set system 1 MHz */
        if (reg_hwenable) {
            reg_sys_1mhz = !!(value & 0x40);
            reg_hwenable = value >> 7;
            scpu64_set_fastmode(!(reg_sys_1mhz | reg_soft_1mhz | reg_sw_1mhz));
        }
        break;
    case 0xd0b3: /* set optim mode */
        if (reg_hwenable) {
            reg_optim = (reg_optim & 0x38) | (value & 0xc7);
        }
        break;
    case 0xd0b4: /* set optim mode */
        if (reg_hwenable) {
            reg_optim = (reg_optim & 0x3f) | (value & 0xc0);
        }
        break;
    case 0xd0b5:
        break;
    case 0xd0b6: /* disable bootmap */
        if (reg_hwenable && reg_bootmap) {
            reg_bootmap = 0;
            mem_pla_config_changed();
        }
        break;
    case 0xd0b7: /* enable bootmap */
        if (reg_hwenable && !reg_bootmap) {
            reg_bootmap = 1;
            mem_pla_config_changed();
        }
        break;
    case 0xd0b8: /* set software 1 MHz */
        if (reg_hwenable) {
            reg_soft_1mhz = value >> 7;
            scpu64_set_fastmode(!(reg_sys_1mhz | reg_soft_1mhz | reg_sw_1mhz));
        }
        break;
    case 0xd0b9:
    case 0xd0ba:
    case 0xd0bb:
        break;
    case 0xd0bc: /* set dos extension */
        if (reg_hwenable) {
            reg_dosext = value >> 7;
        }
        break;
    case 0xd0be: /* dos extension enable */
        if (reg_hwenable) {
            reg_dosext = 1;
        }
        break;
    case 0xd0bd: /* same as 0xd0bf */
    case 0xd0bf: /* dos extension disable */
        reg_dosext = 0;
        break;
    default:
        break;
    }
}

static void colorram_store(WORD addr, BYTE value)
{
    mem_sram[0x10000 + addr] = value;
    mem_color_ram[addr & 0x3ff] = value & 0xf;
}

static BYTE colorram_read(WORD addr)
{
    return mem_sram[0x10000 + addr];
}

static BYTE scpu64_d200_read(WORD addr)
{
    return mem_sram[0x10000 + addr];
}

static void scpu64_d200_store(WORD addr, BYTE value)
{
    if (reg_hwenable) {
        mem_sram[0x10000 + addr] = value;
    }
}

static BYTE scpu64_d300_read(WORD addr)
{
    return mem_sram[0x10000 + addr];
}

static void scpu64_d300_store(WORD addr, BYTE value)
{
    if (reg_hwenable) {
        mem_sram[0x10000 + addr] = value;
    }
}
/* ------------------------------------------------------------------------- */

void scpu64io_colorram_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    colorram_store(addr, value);
}

BYTE scpu64io_colorram_read(WORD addr)
{
    scpu64_clock_read_stretch();
    return colorram_read(addr);
}

BYTE scpu64io_d000_read(WORD addr)
{
    BYTE value;

    if ((addr & 0xfff0) == 0xd0b0) {
        value = scpu64_hardware_read(addr);
    } else {
        value = c64io_d000_read(addr);
    }
    scpu64_clock_read_stretch();
    return value;
}

static BYTE scpu64io_d000_peek(WORD addr)
{
    if ((addr & 0xfff0) == 0xd0b0) {
        return scpu64_hardware_read(addr);
    } else {
        return c64io_d000_peek(addr);
    }
}

void scpu64io_d000_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    if ((addr >= 0xd071 && addr < 0xd080) || (addr >= 0xd0b0 && addr < 0xd0c0)) {
        scpu64_hardware_store(addr, value);
    } else {
        c64io_d000_store(addr, value);
    }
}

BYTE scpu64io_d100_read(WORD addr)
{
    BYTE temp_value = c64io_d100_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64io_d100_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    c64io_d100_store(addr, value);
}

BYTE scpu64io_d200_read(WORD addr)
{
    BYTE temp_value = scpu64_d200_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64io_d200_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    scpu64_d200_store(addr, value);
}

BYTE scpu64io_d300_read(WORD addr)
{
    BYTE temp_value = scpu64_d300_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64io_d300_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    scpu64_d300_store(addr, value);
}

BYTE scpu64io_d400_read(WORD addr)
{
    BYTE temp_value = c64io_d400_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64io_d400_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    c64io_d400_store(addr, value);
}

BYTE scpu64io_d500_read(WORD addr)
{
    BYTE temp_value = c64io_d500_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64io_d500_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    c64io_d500_store(addr, value);
}

BYTE scpu64io_d600_read(WORD addr)
{
    BYTE temp_value = c64io_d600_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64io_d600_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    c64io_d600_store(addr, value);
}

BYTE scpu64io_d700_read(WORD addr)
{
    BYTE temp_value = c64io_d700_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64io_d700_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    c64io_d700_store(addr, value);
}

BYTE scpu64_cia1_read(WORD addr)
{
    BYTE temp_value = cia1_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64_cia1_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    cia1_store(addr, value);
}

BYTE scpu64_cia2_read(WORD addr)
{
    BYTE temp_value = cia2_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64_cia2_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    cia2_store(addr, value);
}

BYTE scpu64io_de00_read(WORD addr)
{
    BYTE temp_value = c64io_de00_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64io_de00_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    c64io_de00_store(addr, value);
}

BYTE scpu64io_df00_read(WORD addr)
{
    BYTE temp_value = c64io_df00_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

void scpu64io_df00_store(WORD addr, BYTE value)
{
    scpu64_clock_write_stretch();
    c64io_df00_store(addr, value);
}

BYTE scpu64_roml_read(WORD addr)
{
    BYTE temp_value = roml_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

BYTE scpu64_romh_read(WORD addr)
{
    BYTE temp_value = romh_read(addr);
    scpu64_clock_read_stretch();
    return temp_value;
}

static void scpu64_mem_vbank_store(WORD addr, BYTE value) {
    mem_sram[addr] = value;
    vicii_mem_vbank_store(addr, value);
}

static void scpu64_mem_vbank_39xx_store(WORD addr, BYTE value) {
    mem_sram[addr] = value;
    vicii_mem_vbank_39xx_store(addr, value);
}

static void scpu64_mem_vbank_3fxx_store(WORD addr, BYTE value) {
    mem_sram[addr] = value;
    vicii_mem_vbank_3fxx_store(addr, value);
}

/* ------------------------------------------------------------------------- */

void mem_set_write_hook(int config, int page, store_func_t *f)
{
    int i;

    for (i = 0; i < NUM_VBANKS; i++) {
        mem_write_tab[i][config][page] = f;
    }
}

void mem_read_tab_set(unsigned int base, unsigned int index, read_func_ptr_t read_func)
{
    mem_read_tab[base][index] = read_func;
}

void mem_read_base_set(unsigned int base, unsigned int index, BYTE *mem_ptr)
{
    mem_read_base_tab[base][index] = mem_ptr;
}

void mem_initialize_memory(void)
{
    int i, j, k;

    mem_chargen_rom_ptr = mem_chargen_rom;
    mem_color_ram_cpu = mem_color_ram;
    mem_color_ram_vicii = mem_color_ram;

    mem_limit_init(mem_read_limit_tab);

    /* Default is RAM.  */
    for (i = 0; i <= 0x100; i++) {
        mem_read_tab_watch[i] = read_watch;
        mem_write_tab_watch[i] = store_watch;
    }

    for (i = 0; i < NUM_CONFIGS; i++) {
        mem_set_write_hook(i, 0, zero_store);
        mem_read_tab[i][0] = zero_read;
        mem_read_base_tab[i][0] = mem_sram;
        for (j = 1; j <= 0xfe; j++) {
            mem_read_tab[i][j] = ram_read;
            mem_read_base_tab[i][j] = mem_sram;
            for (k = 0; k < NUM_VBANKS; k++) {
                if ((j & 0xc0) == (k << 6)) {
                    switch (j & 0x3f) {
                        case 0x39:
                            mem_write_tab[k][i][j] = scpu64_mem_vbank_39xx_store;
                            break;
                        case 0x3f:
                            mem_write_tab[k][i][j] = scpu64_mem_vbank_3fxx_store;
                            break;
                        default:
                            mem_write_tab[k][i][j] = scpu64_mem_vbank_store;
                    }
                } else {
                    mem_write_tab[k][i][j] = ram_store;
                }
            }
        }
        mem_read_tab[i][0xff] = ram_read;
        mem_read_base_tab[i][0xff] = mem_sram;

        /* vbank access is handled within `ram_hi_store()'.  */
        mem_set_write_hook(i, 0xff, ram_hi_store);
    }

    /* Setup character generator ROM at $D000-$DFFF (memory configs 1, 2, 3, 9, 10, 11...).  */
    for (i = 0xd0; i <= 0xdf; i++) {
        for (j = 0; j < 64; j++) {
            if ((j & 7) == 1 || (j & 7) == 2 || (j & 7) == 3) {
                mem_read_tab[j][i] = chargen_read;
                mem_read_base_tab[j][i] = mem_chargen_rom - 0xd000;
            }
        }
    }

    scpu64meminit(0);

    for (i = 0; i < NUM_CONFIGS; i++) {
        mem_read_tab[i][0x100] = mem_read_tab[i][0];
        for (j = 0; j < NUM_VBANKS; j++) {
            mem_write_tab[j][i][0x100] = mem_write_tab[j][i][0];
        }
        mem_read_base_tab[i][0x100] = mem_read_base_tab[i][0];
    }

    _mem_read_tab_ptr = mem_read_tab[7];
    _mem_write_tab_ptr = mem_write_tab[vbank][7];
    _mem_read_base_tab_ptr = mem_read_base_tab[7];
    mem_read_limit_tab_ptr = mem_read_limit_tab[7];

    vicii_set_chargen_addr_options(0x7000, 0x1000);

    c64pla_pport_reset();
    export.exrom = 0;
    export.game = 0;
    reg_bootmap = 1;

    /* Setup initial memory configuration.  */
    mem_pla_config_changed();
    cartridge_init_config();
}

void mem_mmu_translate(unsigned int addr, BYTE **base, int *start, int *limit)
{
    BYTE *p;
    DWORD limits;

    if (addr & ~0xffff) {
        if (addr >= 0xf80000) {
            *base = scpu64memrom_scpu64_rom + (addr & 0xff0000 & (SCPU64_SCPU64_ROM_SIZE-1));
            *limit = 0xfffd;
            *start = 0x0000;
        } else {
            *base = NULL;
            *limit = 0;
            *start = 0;
        }
    } else {
        p = _mem_read_base_tab_ptr[addr >> 8];
        if (p != NULL && addr > 1) {
            *base = p;
            limits = mem_read_limit_tab_ptr[addr >> 8];
            *limit = limits & 0xffff;
            *start = limits >> 16;
        } else if (scpu64_get_fastmode()) {
            *base = NULL;
            *limit = 0;
            *start = 0;
        } else {
            cartridge_mmu_translate(addr, base, start, limit);
        }
    }
}

/* ------------------------------------------------------------------------- */

/* Initialize RAM for power-up.  */
void mem_powerup(void)
{
    ram_init(mem_ram, SCPU64_RAM_SIZE);
    ram_init(mem_sram, SCPU64_SRAM_SIZE);
    ram_init(mem_simm_ram, SCPU64_SIMM_SIZE);
    cartridge_ram_init();  /* Clean cartridge ram too */
}

/* ------------------------------------------------------------------------- */

/* Change the current video bank.  Call this routine only when the vbank
   has really changed.  */
void mem_set_vbank(int new_vbank)
{
    vbank = new_vbank;

    /* Do not override watchpoints on vbank switches.  */
    if (_mem_write_tab_ptr != mem_write_tab_watch) {
        _mem_write_tab_ptr = mem_write_tab[new_vbank][mem_config];
    }

    vicii_set_vbank(new_vbank);
}

/* Set the tape sense status.  */
void mem_set_tape_sense(int sense)
{
    tape_sense = sense;
    mem_pla_config_changed();
}

/* ------------------------------------------------------------------------- */

/* FIXME: this part needs to be checked.  */

void mem_get_basic_text(WORD *start, WORD *end)
{
    if (start != NULL) {
        *start = mem_sram[0x2b] | (mem_sram[0x2c] << 8);
    }
    if (end != NULL) {
        *end = mem_sram[0x2d] | (mem_sram[0x2e] << 8);
    }
}

void mem_set_basic_text(WORD start, WORD end)
{
    mem_sram[0x2b] = mem_sram[0xac] = start & 0xff;
    mem_sram[0x2c] = mem_sram[0xad] = start >> 8;
    mem_sram[0x2d] = mem_sram[0x2f] = mem_sram[0x31] = mem_sram[0xae] = end & 0xff;
    mem_sram[0x2e] = mem_sram[0x30] = mem_sram[0x32] = mem_sram[0xaf] = end >> 8;
}

void mem_inject(DWORD addr, BYTE value)
{
    /* could be made to handle various internal expansions in some sane way */
    mem_sram[addr & 0xffff] = value;
}

/* ------------------------------------------------------------------------- */

int mem_rom_trap_allowed(WORD addr)
{
    if (addr >= 0xe000) {
        switch (mem_config) {
            case 2:
            case 3:
            case 6:
            case 7:
            case 10:
            case 11:
            case 14:
            case 15:
            case 26:
            case 27:
            case 30:
            case 31:
                return 1;
            default:
                return 0;
        }
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

/* Banked memory access functions for the monitor.  */

void store_bank_io(WORD addr, BYTE byte)
{
    switch (addr & 0xff00) {
        case 0xd000:
            if ((addr >= 0xd071 && addr < 0xd080) || (addr >= 0xd0b0 && addr < 0xd0c0)) {
                scpu64_hardware_store(addr, byte);
            } else {
                c64io_d000_store(addr, byte);
            }
            break;
        case 0xd100:
            c64io_d100_store(addr, byte);
            break;
        case 0xd200:
            scpu64_d200_store(addr, byte);
            break;
        case 0xd300:
            scpu64_d300_store(addr, byte);
            break;
        case 0xd400:
            c64io_d400_store(addr, byte);
            break;
        case 0xd500:
            c64io_d500_store(addr, byte);
            break;
        case 0xd600:
            c64io_d600_store(addr, byte);
            break;
        case 0xd700:
            c64io_d700_store(addr, byte);
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
            c64io_de00_store(addr, byte);
            break;
        case 0xdf00:
            c64io_df00_store(addr, byte);
            break;
    }
    return;
}

BYTE read_bank_io(WORD addr)
{
    switch (addr & 0xff00) {
        case 0xd000:
            if ((addr & 0xfff0) == 0xd0b0) {
                return scpu64_hardware_read(addr);
            }
            return c64io_d000_read(addr);
        case 0xd100:
            return c64io_d100_read(addr);
        case 0xd200:
            return scpu64_d200_read(addr);
        case 0xd300:
            return scpu64_d300_read(addr);
        case 0xd400:
            return c64io_d400_read(addr);
        case 0xd500:
            return c64io_d500_read(addr);
        case 0xd600:
            return c64io_d600_read(addr);
        case 0xd700:
            return c64io_d700_read(addr);
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
            return c64io_de00_read(addr);
        case 0xdf00:
            return c64io_df00_read(addr);
    }
    return 0xff;
}

static BYTE peek_bank_io(WORD addr)
{
    switch (addr & 0xff00) {
        case 0xd000:
            return scpu64io_d000_peek(addr);
        case 0xd100:
            return c64io_d100_peek(addr);
        case 0xd200:
            return scpu64_d200_read(addr);
        case 0xd300:
            return scpu64_d300_read(addr);
        case 0xd400:
            return c64io_d400_peek(addr);
        case 0xd500:
            return c64io_d500_peek(addr);
        case 0xd600:
            return c64io_d600_peek(addr);
        case 0xd700:
            return c64io_d700_peek(addr);
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
            return c64io_de00_peek(addr);
        case 0xdf00:
            return c64io_df00_peek(addr);
    }
    return 0xff;
}

/* ------------------------------------------------------------------------- */

int scpu64_hwenable(void)
{
    return reg_hwenable && !reg_bootmap;
}

/* ------------------------------------------------------------------------- */

/* Exported banked memory access functions for the monitor.  */

static const char *banknames[] = {
    "default",
    "cpu",
    "ram",
    "rom",
    "io",
    "cart",
    "rom00", "rom01",
    "ram00", "ram01",
    NULL
};

static const int banknums[] = 
{ 
    1, 0, 1, 2, 3, 4,
    5, 6, 7, 8
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

/* read memory with side-effects */
BYTE mem_bank_read(int bank, WORD addr, void *context)
{
    if ((bank >= 5) && (bank <= 6)) {
        return scpu64memrom_scpu64_rom[((bank - 5) << 16) + addr]; /* ram00..ff */
    }
    if ((bank >= 7) && (bank <= 8)) {
        return mem_sram[((bank - 7) << 16) + addr]; /* ram00..ff */
    }

    switch (bank) {
        case 0:                   /* current */
            return mem_read(addr);
        case 3:                   /* io */
            if (addr >= 0xd000 && addr < 0xe000) {
                return read_bank_io(addr);
            }
        case 4:                   /* cart */
            return cartridge_peek_mem(addr);
        case 2:                   /* rom */
            if (addr >= 0xa000 && addr <= 0xbfff) {
                return scpu64_basic_read(addr);
            }
            if (addr >= 0xd000 && addr <= 0xdfff) {
                return mem_chargen_rom[addr & 0x0fff];
            }
            if (addr >= 0xe000) {
                return scpu64_kernal_read(addr);
            }
        case 1:                   /* ram */
            break;
    }
    return mem_sram[addr];
}

/* read memory without side-effects */
BYTE mem_bank_peek(int bank, WORD addr, void *context)
{
    if ((bank >= 5) && (bank <= 8)) {
        return mem_bank_read(bank, addr, context); /* ram00..ff */
    }
    switch (bank) {
        case 0:                   /* current */
            /* we must check for which bank is currently active, and only use peek_bank_io
               when needed to avoid side effects */
            if (scpu64meminit_io_config[mem_config]) {
                if ((addr >= 0xd000) && (addr < 0xe000)) {
                    return peek_bank_io(addr);
                }
            }
            return mem_read(addr);
            break;
        case 3:                   /* io */
            if ((addr >= 0xd000) && (addr < 0xe000)) {
                return peek_bank_io(addr);
            }
            break;
        case 4:                   /* cart */
            return cartridge_peek_mem(addr);
    }
    return mem_bank_read(bank, addr, context);
}

void mem_bank_write(int bank, WORD addr, BYTE byte, void *context)
{
    if ((bank >= 5) && (bank <= 6)) {
        scpu64memrom_scpu64_rom[((bank - 5) << 16) + addr] = byte; /* rom00..01 */
        return;
    }
    if ((bank >= 7) && (bank <= 8)) {
        mem_sram[((bank - 7) << 16) + addr] = byte; /* ram00..ff */
        return;
    }
    switch (bank) {
        case 0:                   /* current */
            mem_store(addr, byte);
            return;
        case 3:                   /* io */
            if (addr >= 0xd000 && addr < 0xe000) {
                store_bank_io(addr, byte);
                return;
            }
        case 2:                   /* rom */
            if (addr >= 0xa000 && addr <= 0xbfff) {
                return;
            }
            if (addr >= 0xd000 && addr <= 0xdfff) {
                return;
            }
            if (addr >= 0xe000) {
                return;
            }
        case 1:                   /* ram */
            break;
    }
    mem_sram[addr] = byte;
}

static int mem_dump_io(WORD addr)
{
    if ((addr >= 0xdc00) && (addr <= 0xdc3f)) {
        return ciacore_dump(machine_context.cia1);
    } else if ((addr >= 0xdd00) && (addr <= 0xdd3f)) {
        return ciacore_dump(machine_context.cia2);
    }
    return -1;
}

mem_ioreg_list_t *mem_ioreg_list_get(void *context)
{
    mem_ioreg_list_t *mem_ioreg_list = NULL;

    mon_ioreg_add_list(&mem_ioreg_list, "CIA1", 0xdc00, 0xdc0f, mem_dump_io);
    mon_ioreg_add_list(&mem_ioreg_list, "CIA2", 0xdd00, 0xdd0f, mem_dump_io);

    io_source_ioreg_add_list(&mem_ioreg_list);

    return mem_ioreg_list;
}

void mem_get_screen_parameter(WORD *base, BYTE *rows, BYTE *columns, int *bank)
{
    *base = ((vicii_peek(0xd018) & 0xf0) << 6) | ((~cia2_peek(0xdd00) & 0x03) << 14);
    *rows = 25;
    *columns = 40;
    *bank = 0;
}

/* ------------------------------------------------------------------------- */

void mem_color_ram_to_snapshot(BYTE *color_ram)
{
    memcpy(color_ram, mem_color_ram, 0x400);
}

void mem_color_ram_from_snapshot(BYTE *color_ram)
{
    memcpy(mem_color_ram, color_ram, 0x400);
}
