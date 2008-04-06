/*
 * plus4mem.c -- Plus4 memory handling.
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

#include "datasette.h"
#include "drive.h"
#include "drivecpu.h"
#include "iecdrive.h"
#include "log.h"
#include "maincpu.h"
#include "mon.h"
#include "plus4acia.h"
#include "plus4iec.h"
#include "plus4mem.h"
#include "plus4memlimit.h"
#include "plus4pio1.h"
#include "plus4pio2.h"
#include "resources.h"
#include "sysfile.h"
#include "ted-mem.h"
#include "types.h"
#include "utils.h"

static int hard_reset_flag=0;

/* Adjust this pointer when the MMU changes banks.  */
static BYTE **bank_base;
static int *bank_limit = NULL;
unsigned int old_reg_pc;

const char *mem_romset_resources_list[] = {
    NULL
};

/* ------------------------------------------------------------------------- */

/* Number of possible memory configurations.  */
#define NUM_CONFIGS     32

/* The Plus4 memory.  */
BYTE ram[PLUS4_RAM_SIZE];
BYTE basic_rom[PLUS4_BASIC_ROM_SIZE];
BYTE kernal_rom[PLUS4_KERNAL_ROM_SIZE];
BYTE extromlo1[PLUS4_BASIC_ROM_SIZE];
BYTE extromlo2[PLUS4_BASIC_ROM_SIZE];
BYTE extromlo3[PLUS4_BASIC_ROM_SIZE];
BYTE extromhi1[PLUS4_KERNAL_ROM_SIZE];
BYTE extromhi2[PLUS4_KERNAL_ROM_SIZE];
BYTE extromhi3[PLUS4_KERNAL_ROM_SIZE];

/* Flag: nonzero if the Kernal and BASIC ROMs have been loaded.  */
static int plus4_rom_loaded = 0;

/* Pointers to the currently used memory read and write tables.  */
read_func_ptr_t *_mem_read_tab_ptr;
store_func_ptr_t *_mem_write_tab_ptr;
BYTE **_mem_read_base_tab_ptr;
int *mem_read_limit_tab_ptr;

/* Memory read and write tables.  */
static store_func_ptr_t mem_write_tab[NUM_CONFIGS][0x101];
static read_func_ptr_t mem_read_tab[NUM_CONFIGS][0x101];
static BYTE *mem_read_base_tab[NUM_CONFIGS][0x101];
static int mem_read_limit_tab[NUM_CONFIGS][0x101];

static store_func_ptr_t mem_write_tab_watch[0x101];
static read_func_ptr_t mem_read_tab_watch[0x101];

/* Processor port.  */
static struct {
    BYTE dir, data, data_out;
} pport;

/* Current memory configuration.  */
static unsigned int mem_config;

/* Logging goes here.  */
static log_t plus4_mem_log = LOG_ERR;

/* Pointer to the IEC structure.  */
static iec_info_t *iec_info;

static iec_cpu_write_callback_t iec_cpu_write_callback[4] = {
    iec_cpu_write_conf0, iec_cpu_write_conf1,
    iec_cpu_write_conf2, iec_cpu_write_conf3
};


/* ------------------------------------------------------------------------- */

#define RAM0 ram + 0x0000
#define RAM4 ram + 0x4000
#define RAM8 ram + 0x8000
#define RAMC ram + 0xc000

static BYTE *chargen_tab[8][16] = {
    /* 0000-3fff, RAM selected  */
    {       RAM0,       RAM0,       RAM0,       RAM0,
            RAM0,       RAM0,       RAM0,       RAM0,
            RAM0,       RAM0,       RAM0,       RAM0,
            RAM0,       RAM0,       RAM0,       RAM0 },
    /* 4000-7fff, RAM selected  */
    {       RAM4,       RAM4,       RAM4,       RAM4,
            RAM4,       RAM4,       RAM4,       RAM4,
            RAM4,       RAM4,       RAM4,       RAM4,
            RAM4,       RAM4,       RAM4,       RAM4 },
    /* 8000-bfff, RAM selected  */
    {       RAM8,       RAM8,       RAM8,       RAM8,
            RAM8,       RAM8,       RAM8,       RAM8,
            RAM8,       RAM8,       RAM8,       RAM8,
            RAM8,       RAM8,       RAM8,       RAM8 },
    /* c000-ffff, RAM selected  */
    {       RAMC,       RAMC,       RAMC,       RAMC,
            RAMC,       RAMC,       RAMC,       RAMC,
            RAMC,       RAMC,       RAMC,       RAMC,
            RAMC,       RAMC,       RAMC,       RAMC },

    /* 0000-3fff, ROM selected  */
    {       RAM0,       RAM0,       RAM0,       RAM0,
            RAM0,       RAM0,       RAM0,       RAM0,
            RAM0,       RAM0,       RAM0,       RAM0,
            RAM0,       RAM0,       RAM0,       RAM0 },
    /* 4000-7fff, ROM selected  */
    {       RAM4,       RAM4,       RAM4,       RAM4,
            RAM4,       RAM4,       RAM4,       RAM4,
            RAM4,       RAM4,       RAM4,       RAM4,
            RAM4,       RAM4,       RAM4,       RAM4 },
    /* 8000-bfff, ROM selected  */
    {  basic_rom,  extromlo1,  extromlo2,  extromlo3,
       basic_rom,  extromlo1,  extromlo2,  extromlo3,
       basic_rom,  extromlo1,  extromlo2,  extromlo3,
       basic_rom,  extromlo1,  extromlo2,  extromlo3 },
    /* c000-ffff, ROM selected  */
    { kernal_rom, kernal_rom, kernal_rom, kernal_rom,
       extromhi1,  extromhi1,  extromhi1,  extromhi1,
       extromhi2,  extromhi2,  extromhi2,  extromhi2,
       extromhi3,  extromhi3,  extromhi3,  extromhi3 }
};

BYTE *mem_get_tedmem_base(unsigned int segment)
{
    return chargen_tab[segment][mem_config >> 1];
}

/* ------------------------------------------------------------------------- */

/* Tape motor status.  */
static BYTE old_port_data_out = 0xff;

/* Tape read input.  */
static BYTE tape_read = 0xff;

inline static void mem_proc_port_store(void)
{
    pport.data_out = (pport.data_out & ~pport.dir)
                     | (pport.data & pport.dir);

    iec_cpu_write_callback[iec_callback_index]((BYTE)~pport.data_out);

    if (((pport.dir & pport.data) & 0x08) != old_port_data_out) {
        old_port_data_out = (pport.dir & pport.data) & 0x08;
        datasette_set_motor(!old_port_data_out);
    }
}

inline static BYTE mem_proc_port_read(ADDRESS addr)
{
    BYTE tmp;

    if (addr == 0)
        return pport.dir;

    tmp = (pport.data | ~pport.dir) & (pport.data_out | 0x3f);

    tmp |= iec_cpu_read();

    if (tape_read)
        tmp &= ~0x10;

    return tmp;
}

void mem_proc_port_trigger_flux_change(unsigned int on)
{
   /*printf("FLUXCHANGE\n");*/
   tape_read = on; 
}

/* ------------------------------------------------------------------------- */

BYTE REGPARM1 read_zero(ADDRESS addr)
{
    addr &= 0xff;

    switch ((BYTE)addr) {
      case 0:
      case 1:
        return mem_proc_port_read(addr);
    }
    return ram[addr];
}

void REGPARM2 store_zero(ADDRESS addr, BYTE value)
{
    addr &= 0xff;

    switch ((BYTE)addr) {
      case 0:
        if (pport.dir != value) {
            pport.dir = value;
            mem_proc_port_store();
        }
        ram[addr] = value;
        break;
      case 1:
        if (pport.data != value) {
            pport.data = value;
            mem_proc_port_store();
        }
        ram[addr] = value;
        break;
      default:
        ram[addr] = value;
    }
}

/* ------------------------------------------------------------------------- */

static void mem_config_set(unsigned int config)
{
    mem_config = config;

    if (any_watchpoints(e_comp_space)) {
        _mem_read_tab_ptr = mem_read_tab_watch;
        _mem_write_tab_ptr = mem_write_tab_watch;
    } else {
        _mem_read_tab_ptr = mem_read_tab[mem_config];
        _mem_write_tab_ptr = mem_write_tab[mem_config];
    }

    _mem_read_base_tab_ptr = mem_read_base_tab[mem_config];
    mem_read_limit_tab_ptr = mem_read_limit_tab[mem_config];

    if (bank_limit != NULL) {
        *bank_base = _mem_read_base_tab_ptr[old_reg_pc >> 8];
        if (*bank_base != 0)
            *bank_base = _mem_read_base_tab_ptr[old_reg_pc >> 8]
                         - (old_reg_pc & 0xff00);
        *bank_limit = mem_read_limit_tab_ptr[old_reg_pc >> 8];
    }
}

void mem_config_ram_set(unsigned int config)
{
    mem_config_set((mem_config & ~0x01) | config);
}

void mem_config_rom_set(unsigned int config)
{
    mem_config_set((mem_config & ~0x1e) | config);
}

/* ------------------------------------------------------------------------- */

BYTE REGPARM1 read_watch(ADDRESS addr)
{
    mon_watch_push_load_addr(addr, e_comp_space);
    return mem_read_tab[mem_config][addr >> 8](addr);
}


void REGPARM2 store_watch(ADDRESS addr, BYTE value)
{
    mon_watch_push_store_addr(addr, e_comp_space);
    mem_write_tab[mem_config][addr >> 8](addr, value);
}

void mem_toggle_watchpoints(int flag)
{
    if (flag) {
        _mem_read_tab_ptr = mem_read_tab_watch;
        _mem_write_tab_ptr = mem_write_tab_watch;
    } else {
        _mem_read_tab_ptr = mem_read_tab[mem_config];
        _mem_write_tab_ptr = mem_write_tab[mem_config];
    }
}

/* ------------------------------------------------------------------------- */

BYTE REGPARM1 basic_read(ADDRESS addr)
{
    return basic_rom[addr & 0x3fff];
}

BYTE REGPARM1 kernal_read(ADDRESS addr)
{
    return kernal_rom[addr & 0x3fff];
}

void REGPARM2 kernal_store(ADDRESS addr, BYTE value)
{
    kernal_rom[addr & 0x3fff] = value;
}

static BYTE REGPARM1 extromlo1_read(ADDRESS addr)
{
    return extromlo1[addr & 0x3fff];
}

static BYTE REGPARM1 extromlo2_read(ADDRESS addr)
{
    return extromlo2[addr & 0x3fff];
}

static BYTE REGPARM1 extromlo3_read(ADDRESS addr)
{
    return extromlo3[addr & 0x3fff];
}

static BYTE REGPARM1 extromhi1_read(ADDRESS addr)
{
    return extromhi1[addr & 0x3fff];
}

static BYTE REGPARM1 extromhi2_read(ADDRESS addr)
{
    return extromhi2[addr & 0x3fff];
}

static BYTE REGPARM1 extromhi3_read(ADDRESS addr)
{
    return extromhi3[addr & 0x3fff];
}


BYTE REGPARM1 ram_read(ADDRESS addr)
{
    return ram[addr];
}

void REGPARM2 ram_store(ADDRESS addr, BYTE value)
{
    ram[addr] = value;
}

BYTE REGPARM1 rom_read(ADDRESS addr)
{
    switch (addr & 0xc000) {
      case 0x8000:
        switch ((mem_config >> 1) & 3) {
          case 0:
            return basic_read(addr);
          case 1:
            return extromlo1_read(addr);
          case 2:
            return extromlo2_read(addr);
          case 3:
            return extromlo3_read(addr);
        }
      case 0xc000:
        if ((addr & 0xff00) == 0xfc00) {
            return kernal_read(addr);
        } else {
            switch ((mem_config >> 3) & 3) {
              case 0:
                return kernal_read(addr);
              case 1:
                return extromhi1_read(addr);
              case 2:
                return extromhi2_read(addr);
              case 3:
                return extromhi3_read(addr);
            }
        }
    }

    return 0;
}

void REGPARM2 rom_store(ADDRESS addr, BYTE value)
{
    switch (addr & 0xc000) {
      case 0x8000:
        basic_rom[addr & 0x3fff] = value;
        break;
      case 0xc000:
        kernal_rom[addr & 0x3fff] = value;
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

static BYTE REGPARM1 fdxx_read(ADDRESS addr)
{
    if (addr >= 0xfd00 && addr <= 0xfd0f)
        return acia_read(addr);

    if (addr >= 0xfd10 && addr <= 0xfd1f)
        return pio1_read(addr);

    if (addr >= 0xfd30 && addr <= 0xfd3f)
        return pio2_read(addr);

    return 0;
}

static void REGPARM2 fdxx_store(ADDRESS addr, BYTE value)
{
    if (addr >= 0xfd00 && addr <= 0xfd0f) {
        acia_store(addr, value);
        return;
    }
    if (addr >= 0xfd10 && addr <= 0xfd1f) {
        pio1_store(addr, value);
        return;
    }
    if (addr >= 0xfd30 && addr <= 0xfd3f) {
        pio2_store(addr, value);
        return;
    }
    if (addr >= 0xfdd0 && addr <= 0xfddf) {
        mem_config_rom_set((addr & 0xf) << 1);
        return;
    }
}

static BYTE REGPARM1 ram_ffxx_read(ADDRESS addr)
{
    if ((addr >= 0xff20) && (addr != 0xff3e) && (addr != 0xff3f))
        return ram_read(addr);

    return ted_read(addr);
}

static void REGPARM2 ram_ffxx_store(ADDRESS addr, BYTE value)
{
    if (addr < 0xff20 || addr == 0xff3e || addr == 0xff3f) {
        ted_store(addr, value);
    } else {
        ram_store(addr, value);
    }
}

static BYTE REGPARM1 rom_ffxx_read(ADDRESS addr)
{
    if ((addr >= 0xff20) && (addr != 0xff3e) && (addr != 0xff3f))
        return rom_read(addr);

    return ted_read(addr);
}

static void REGPARM2 rom_ffxx_store(ADDRESS addr, BYTE value)
{
    if (addr < 0xff20 || addr == 0xff3e || addr == 0xff3f) {
        ted_store(addr, value);
    } else {
        ram_store(addr, value);
    }
}

/* ------------------------------------------------------------------------- */

static void set_write_hook(int config, int page, store_func_t *f)
{
     mem_write_tab[config][page] = f;
}

void mem_initialize_memory(void)
{
    int i, j;

    iec_info = iec_get_drive_port();

    mem_limit_init(mem_read_limit_tab);

    /* Default is RAM.  */
    for (i = 0; i <= 0x100; i++) {
        mem_read_tab_watch[i] = read_watch;
        mem_write_tab_watch[i] = store_watch;
    }

    for (i = 0; i < NUM_CONFIGS; i++) {
        set_write_hook(i, 0, store_zero);
        mem_read_tab[i][0] = read_zero;
        mem_read_base_tab[i][0] = ram;
        for (j = 1; j <= 0xfe; j++) {
            mem_read_tab[i][j] = ram_read;
            mem_read_base_tab[i][j] = ram + (j << 8);
#if 0
            if ((j & 0xc0) == (k << 6)) {
                switch (j & 0x3f) {
                  case 0x39:
                    mem_write_tab[i][j] = ted_mem_vbank_39xx_store;
                    break;
                  case 0x3f:
                    mem_write_tab[i][j] = ted_mem_vbank_3fxx_store;
                    break;
                  default:
                    mem_write_tab[i][j] = ted_mem_vbank_store;
                }
            } else {
#endif
                mem_write_tab[i][j] = ted_mem_vbank_store;
#if 0
            }
#endif
        }
        mem_read_tab[i][0xff] = ram_read;
        mem_read_base_tab[i][0xff] = ram + 0xff00;
        set_write_hook(i, 0xff, ram_store);
    }

    /* Setup BASIC ROM and extension ROMs at $8000-$BFFF.  */
    for (i = 0x80; i <= 0xbf; i++) {
        mem_read_tab[1][i] = basic_read;
        mem_read_base_tab[1][i] = basic_rom + ((i & 0x3f) << 8);
        mem_read_tab[3][i] = extromlo1_read;
        mem_read_base_tab[3][i] = extromlo1 + ((i & 0x3f) << 8);
        mem_read_tab[5][i] = extromlo2_read;
        mem_read_base_tab[5][i] = extromlo2 + ((i & 0x3f) << 8);
        mem_read_tab[7][i] = extromlo3_read;
        mem_read_base_tab[7][i] = extromlo3 + ((i & 0x3f) << 8);
        mem_read_tab[9][i] = basic_read;
        mem_read_base_tab[9][i] = basic_rom + ((i & 0x3f) << 8);
        mem_read_tab[11][i] = extromlo1_read;
        mem_read_base_tab[11][i] = extromlo1 + ((i & 0x3f) << 8);
        mem_read_tab[13][i] = extromlo2_read;
        mem_read_base_tab[13][i] = extromlo2 + ((i & 0x3f) << 8);
        mem_read_tab[15][i] = extromlo3_read;
        mem_read_base_tab[15][i] = extromlo3 + ((i & 0x3f) << 8);
        mem_read_tab[17][i] = basic_read;
        mem_read_base_tab[17][i] = basic_rom + ((i & 0x3f) << 8);
        mem_read_tab[19][i] = extromlo1_read;
        mem_read_base_tab[19][i] = extromlo1 + ((i & 0x3f) << 8);
        mem_read_tab[21][i] = extromlo2_read;
        mem_read_base_tab[21][i] = extromlo2 + ((i & 0x3f) << 8);
        mem_read_tab[23][i] = extromlo3_read;
        mem_read_base_tab[23][i] = extromlo3 + ((i & 0x3f) << 8);
        mem_read_tab[25][i] = basic_read;
        mem_read_base_tab[25][i] = basic_rom + ((i & 0x3f) << 8);
        mem_read_tab[27][i] = extromlo1_read;
        mem_read_base_tab[27][i] = extromlo1 + ((i & 0x3f) << 8);
        mem_read_tab[29][i] = extromlo2_read;
        mem_read_base_tab[29][i] = extromlo2 + ((i & 0x3f) << 8);
        mem_read_tab[31][i] = extromlo3_read;
        mem_read_base_tab[31][i] = extromlo3 + ((i & 0x3f) << 8);
    }

    /* Setup Kernal ROM and extension ROMs at $E000-$FFFF.  */
    for (i = 0xc0; i <= 0xff; i++) {
        mem_read_tab[1][i] = kernal_read;
        mem_read_base_tab[1][i] = kernal_rom + ((i & 0x3f) << 8);
        mem_read_tab[3][i] = kernal_read;
        mem_read_base_tab[3][i] = kernal_rom + ((i & 0x3f) << 8);
        mem_read_tab[5][i] = kernal_read;
        mem_read_base_tab[5][i] = kernal_rom + ((i & 0x3f) << 8);
        mem_read_tab[7][i] = kernal_read;
        mem_read_base_tab[7][i] = kernal_rom + ((i & 0x3f) << 8);
        mem_read_tab[9][i] = extromhi1_read;
        mem_read_base_tab[9][i] = extromhi1 + ((i & 0x3f) << 8);
        mem_read_tab[11][i] = extromhi1_read;
        mem_read_base_tab[11][i] = extromhi1 + ((i & 0x3f) << 8);
        mem_read_tab[13][i] = extromhi1_read;
        mem_read_base_tab[13][i] = extromhi1 + ((i & 0x3f) << 8);
        mem_read_tab[15][i] = extromhi1_read;
        mem_read_base_tab[15][i] = extromhi1 + ((i & 0x3f) << 8);
        mem_read_tab[17][i] = extromhi2_read;
        mem_read_base_tab[17][i] = extromhi2 + ((i & 0x3f) << 8);
        mem_read_tab[19][i] = extromhi2_read;
        mem_read_base_tab[19][i] = extromhi2 + ((i & 0x3f) << 8);
        mem_read_tab[21][i] = extromhi2_read;
        mem_read_base_tab[21][i] = extromhi2 + ((i & 0x3f) << 8);
        mem_read_tab[23][i] = extromhi2_read;
        mem_read_base_tab[23][i] = extromhi2 + ((i & 0x3f) << 8);
        mem_read_tab[25][i] = extromhi3_read;
        mem_read_base_tab[25][i] = extromhi3 + ((i & 0x3f) << 8);
        mem_read_tab[27][i] = extromhi3_read;
        mem_read_base_tab[27][i] = extromhi3 + ((i & 0x3f) << 8);
        mem_read_tab[29][i] = extromhi3_read;
        mem_read_base_tab[29][i] = extromhi3 + ((i & 0x3f) << 8);
        mem_read_tab[31][i] = extromhi3_read;
        mem_read_base_tab[31][i] = extromhi3 + ((i & 0x3f) << 8);
    }

    for (i = 0; i < NUM_CONFIGS; i += 2) {
        mem_read_tab[i + 1][0xfc] = kernal_read;
        mem_read_base_tab[i + 1][0xfc] = kernal_rom + ((0xfc & 0x3f) << 8);

        mem_read_tab[i + 0][0xfd] = fdxx_read;
        mem_write_tab[i + 0][0xfd] = fdxx_store;
        mem_read_base_tab[i + 0][0xfd] = NULL;
        mem_read_tab[i + 1][0xfd] = fdxx_read;
        mem_write_tab[i + 1][0xfd] = fdxx_store;
        mem_read_base_tab[i + 1][0xfd] = NULL;

        mem_read_tab[i + 0][0xff] = ram_ffxx_read;
        mem_write_tab[i + 0][0xff] = ram_ffxx_store;
        mem_read_base_tab[i + 0][0xff] = NULL;
        mem_read_tab[i + 1][0xff] = rom_ffxx_read;
        mem_write_tab[i + 1][0xff] = rom_ffxx_store;
        mem_read_base_tab[i + 1][0xff] = NULL;

        mem_read_tab[i + 0][0x100] = mem_read_tab[i + 0][0];
        mem_write_tab[i + 0][0x100] = mem_write_tab[i + 0][0];
        mem_read_base_tab[i + 0][0x100] = mem_read_base_tab[i + 0][0];
        mem_read_tab[i + 1][0x100] = mem_read_tab[i + 1][0];
        mem_write_tab[i + 1][0x100] = mem_write_tab[i + 1][0];
        mem_read_base_tab[i + 1][0x100] = mem_read_base_tab[i + 1][0];
    }
    if (hard_reset_flag) {
        hard_reset_flag=0;
        mem_config = 1;
    }
    _mem_read_tab_ptr = mem_read_tab[mem_config];
    _mem_write_tab_ptr = mem_write_tab[mem_config];
    _mem_read_base_tab_ptr = mem_read_base_tab[mem_config];
    mem_read_limit_tab_ptr = mem_read_limit_tab[mem_config];
}

/* ------------------------------------------------------------------------- */

/* Initialize RAM for power-up.  */
void mem_powerup(void)
{
    int i;

    for (i = 0; i < 0x10000; i += 0x80) {
        memset(ram + i, 0, 0x40);
        memset(ram + i + 0x40, 0xff, 0x40);
    }

    hard_reset_flag = 1;
}

int mem_load_kernal(const char *rom_name)
{
    int trapfl;

    if (!plus4_rom_loaded)
        return 0;

    /* Make sure serial code assumes there are no traps installed.  */
    /* serial_remove_traps(); */
    /* we also need the TAPE traps!!! therefore -> */
    /* disable traps before saving the ROM */
    resources_get_value("VirtualDevices", (resource_value_t*)&trapfl);
    resources_set_value("VirtualDevices", (resource_value_t)1);

    /* Load Kernal ROM.  */
    if (sysfile_load(rom_name,
        kernal_rom, PLUS4_KERNAL_ROM_SIZE, PLUS4_KERNAL_ROM_SIZE) < 0) {
        log_error(plus4_mem_log, "Couldn't load kernal ROM `%s'.",
                  rom_name);
        resources_set_value("VirtualDevices", (resource_value_t)trapfl);
        return -1;
    }

    resources_set_value("VirtualDevices", (resource_value_t)trapfl);

    return 0;
}

int mem_load_basic(const char *rom_name)
{
    if (!plus4_rom_loaded)
        return 0;

    /* Load Basic ROM.  */
    if (sysfile_load(rom_name,
        basic_rom, PLUS4_BASIC_ROM_SIZE, PLUS4_BASIC_ROM_SIZE) < 0) {
        log_error(plus4_mem_log,
                  "Couldn't load basic ROM `%s'.",
                  rom_name);
        return -1;
    }
    return 0;
}

int mem_load_3plus1lo(const char *rom_name)
{
    if (!plus4_rom_loaded)
        return 0;

    /* Load 3plus1 low ROM.  */
    if (sysfile_load(rom_name,
        extromlo1, PLUS4_BASIC_ROM_SIZE, PLUS4_BASIC_ROM_SIZE) < 0) {
        log_error(plus4_mem_log,
                  "Couldn't load 3plus1 low ROM `%s'.",
                  rom_name);
        return -1;
    }
    return 0;
}

int mem_load_3plus1hi(const char *rom_name)
{
    if (!plus4_rom_loaded)
        return 0;

    /* Load 3plus1 high ROM.  */
    if (sysfile_load(rom_name,
        extromhi1, PLUS4_KERNAL_ROM_SIZE, PLUS4_KERNAL_ROM_SIZE) < 0) {
        log_error(plus4_mem_log,
                  "Couldn't load 3plus1 high ROM `%s'.",
                  rom_name);
        return -1;
    }
    return 0;
}

int mem_load(void)
{

    char *rom_name = NULL;

    mem_powerup();

    if (plus4_mem_log == LOG_ERR)
        plus4_mem_log = log_open("PLUS4MEM");

    plus4_rom_loaded = 1;

    if (resources_get_value("KernalName", (resource_value_t)&rom_name) < 0)
        return -1;
    if (mem_load_kernal(rom_name) < 0)
        return -1;

    if (resources_get_value("BasicName", (resource_value_t)&rom_name) < 0)
        return -1;
    if (mem_load_basic(rom_name) < 0)
        return -1;

    if (resources_get_value("3plus1loName", (resource_value_t)&rom_name) < 0)
        return -1;
    if (mem_load_3plus1lo(rom_name) < 0)
        return -1;

    if (resources_get_value("3plus1hiName", (resource_value_t)&rom_name) < 0)
        return -1;
    if (mem_load_3plus1hi(rom_name) < 0)
        return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */

void mem_set_bank_pointer(BYTE **base, int *limit)
{
    bank_base = base;
    bank_limit = limit;
}

/* ------------------------------------------------------------------------- */

/* FIXME: this part needs to be checked.  */

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

/* ------------------------------------------------------------------------- */

int mem_rom_trap_allowed(ADDRESS addr)
{
    return addr >= 0x8000 && (mem_config & 0x1);
}

/* ------------------------------------------------------------------------- */

/* Exported banked memory access functions for the monitor.  */

static const char *banknames[] =
{
    "default", "cpu", "ram", "rom", "funcrom", "cart1rom", "cart2rom", NULL
};

static int banknums[] =
{
    1, 0, 1, 2, 3, 4, 5
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
      case 2:                   /* rom */
        if (addr >= 0x8000 && addr <= 0xbfff) {
            return basic_rom[addr & 0x3fff];
        }
        if (addr >= 0xc000 && addr <= 0xffff) {
            return kernal_rom[addr & 0x3fff];
        }
        break;
      case 3:                   /* funcrom */
        if (addr >= 0x8000 && addr <= 0xbfff) {
            return extromlo1[addr & 0x3fff];
        }
        if (addr >= 0xc000 && addr <= 0xffff) {
            return extromhi1[addr & 0x3fff];
        }
        break;
      case 4:                   /* cart1rom */
        if (addr >= 0x8000 && addr <= 0xbfff) {
            return extromlo2[addr & 0x3fff];
        }
        if (addr >= 0xc000 && addr <= 0xffff) {
            return extromhi2[addr & 0x3fff];
        }
        break;
      case 5:                   /* cart2rom */
        if (addr >= 0x8000 && addr <= 0xbfff) {
            return extromlo3[addr & 0x3fff];
        }
        if (addr >= 0xc000 && addr <= 0xffff) {
            return extromhi3[addr & 0x3fff];
        }
        break;
      case 1:                   /* ram */
        break;
    }
    return ram[addr];
}

BYTE mem_bank_peek(int bank, ADDRESS addr)
{
    return mem_bank_read(bank, addr);
}

void mem_bank_write(int bank, ADDRESS addr, BYTE byte)
{
    switch (bank) {
      case 0:                   /* current */
        mem_store(addr, byte);
        return;
      case 2:                   /* rom */
        if (addr >= 0x8000 && addr <= 0xbfff) {
            return;
        }
        if (addr >= 0xc000 && addr <= 0xffff) {
            return;
        }
        break;
      case 3:                   /* funcrom */
        if (addr >= 0x8000 && addr <= 0xbfff) {
            return;
        }
        if (addr >= 0xc000 && addr <= 0xffff) {
            return;
        }
        break;
      case 4:                   /* cart1rom */
        if (addr >= 0x8000 && addr <= 0xbfff) {
            return;
        }
        if (addr >= 0xc000 && addr <= 0xffff) {
            return;
        }
        break;
      case 5:                   /* cart2rom */
        if (addr >= 0x8000 && addr <= 0xbfff) {
            return;
        }
        if (addr >= 0xc000 && addr <= 0xffff) {
            return;
        }
        break;
      case 1:                   /* ram */
        break;
    }
    ram[addr] = byte;
}

mem_ioreg_list_t *mem_ioreg_list_get(void)
{
    mem_ioreg_list_t *mem_ioreg_list;

    mem_ioreg_list = (mem_ioreg_list_t *)xmalloc(sizeof(mem_ioreg_list_t) * 2);

    mem_ioreg_list[0].name = "ACIA";
    mem_ioreg_list[0].start = 0xfd00;
    mem_ioreg_list[0].end = 0xfd0f;
    mem_ioreg_list[0].next = &mem_ioreg_list[1];

    mem_ioreg_list[1].name = "TED";
    mem_ioreg_list[1].start = 0xff00;
    mem_ioreg_list[1].end = 0xff3f;
    mem_ioreg_list[1].next = NULL;

    return mem_ioreg_list;
}

void mem_get_screen_parameter(ADDRESS *base, BYTE *rows, BYTE *columns)
{
    *base = (ted_peek(0xff14) & 0xf8) << 8 | 0x400;
    *rows = 25;
    *columns = 40;
}

