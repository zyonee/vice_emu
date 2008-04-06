/*
 * c128mmu.c
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

#include "c128mem.h"
#include "c128mmu.h"
#include "c64cart.h"
#include "cmdline.h"
#include "functionrom.h"
#include "interrupt.h"
#include "keyboard.h"
#include "log.h"
#include "maincpu.h"
#include "mem.h"
#include "resources.h"
#include "reu.h"
#include "types.h"
#include "vdc.h"
#include "vicii.h"
#include "z80.h"
#include "z80mem.h"

/* #define MMU_DEBUG */

/* MMU register.  */
static BYTE mmu[11];

/* Memory configuration.  */
static int chargen_in;
static int basic_lo_in;
static int basic_hi_in;
static int kernal_in;
static int editor_in;
static int io_in;

/* State of the 40/80 column key.  */
static int mmu_column4080_key = 1;

static int mmu_config64;

/* Logging goes here.  */
static log_t mmu_log = LOG_ERR;

/* ------------------------------------------------------------------------- */

static int set_column4080_key(resource_value_t v, void *param)
{
    mmu_column4080_key = (int)v;

#ifdef HAS_SINGLE_CANVAS
    vdc_set_canvas_refresh(mmu_column4080_key ? 0 : 1);
    vic_ii_set_canvas_refresh(mmu_column4080_key ? 1 : 0);
#endif
    return 0;
}

static resource_t resources[] = {
    { "40/80ColumnKey", RES_INTEGER, (resource_value_t)1,
      (resource_value_t *)&mmu_column4080_key,
      set_column4080_key, NULL },
    { NULL }
};

int mmu_resources_init(void)
{
    return resources_register(resources);
}

static cmdline_option_t cmdline_options[] = {
    { "-40col", SET_RESOURCE, 0, NULL, NULL, "40/80ColumnKey",
      (resource_value_t) 1,
      NULL, "Activate 40 column mode" },
    { "-80col", SET_RESOURCE, 0, NULL, NULL, "40/80ColumnKey",
      (resource_value_t) 0,
      NULL, "Activate 80 comumn mode" },
    { NULL }
};

int mmu_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

static void mmu_toggle_column4080_key(void)
{
    mmu_column4080_key = !mmu_column4080_key;
    resources_set_value("40/80ColumnKey",
                        (resource_value_t)mmu_column4080_key);
    log_message(mmu_log, "40/80 column key %s.",
                (mmu_column4080_key) ? "released" : "pressed");
}

static void mmu_switch_cpu(int value)
{
    if (value) {
#ifdef MMU_DEBUG
        log_message(mmu_log, "Switching to 8502 CPU.");
#endif
        z80_trigger_dma();
    } else {
#ifdef MMU_DEBUG
        log_message(mmu_log, "Switching to Z80 CPU.");
#endif
        maincpu_trigger_dma();
    }
}

static void mmu_set_ram_bank(BYTE value)
{
    /* (We handle only 128K here.)  */
    ram_bank = ram + (((long) value & 0x40) << 10);
#ifdef MMU_DEBUG
    log_message(mmu_log, "Set RAM bank %i.", (value & 0x40) ? 1 : 0);
#endif
}

static void mmu_update_config(void)
{
#ifdef MMU_DEBUG
    log_message(mmu_log, "MMU5 %02x, MMU0 %02x, MMUC %02x\n",
                mmu[5] & 0x40, mmu[0], mmu_config64);
#endif

    if (mmu[5] & 0x40) {
        mem_update_config(0x80 + mmu_config64);
    } else {
        mem_update_config(((mmu[0] & 0x2) ? 0 : 1)
                          | ((mmu[0] & 0x0c) >> 1)
                          | ((mmu[0] & 0x30) >> 1)
                          | ((mmu[0] & 0x40) ? 32 : 0)
                          | ((mmu[0] & 0x1) ? 0 : 64));
        z80mem_update_config((((mmu[0] & 0x1)) ? 0 : 1)
                          | ((mmu[0] & 0x40) ? 2 : 0)
                          | ((mmu[0] & 0x80) ? 4 : 0));
    }
}

void mmu_set_config64(int config)
{
    mmu_config64 = config;
    mmu_update_config();
}

/* ------------------------------------------------------------------------- */

BYTE REGPARM1 mmu_read(ADDRESS addr)
{
    vic_ii_handle_pending_alarms_external(0);

    addr &= 0xff;

#ifdef MMU_DEBUG
    log_message(mmu_log, "MMU READ $%x.", addr);
#endif

    if (addr < 0xb) {
        if (addr == 5) {
            /* 0x80 = 40/80 key released.  */
            return (mmu[5] & 0x0f) | (mmu_column4080_key ? 0x80 : 0)
                   | ((export.game ^ 1) << 4) | ((export.exrom ^ 1) << 5);
        } else {
            return mmu[addr];
        }
    } else {
        return 0xf;
    }
}


void REGPARM2 mmu_store(ADDRESS address, BYTE value)
{
    vic_ii_handle_pending_alarms_external(maincpu_num_write_cycles());

    address &= 0xf;

#ifdef MMU_DEBUG
    log_message(mmu_log, "MMU STORE $%x <- #$%x.", address, value);
#endif

    if (address < 0xb) {
        BYTE oldvalue;

        oldvalue = mmu[address];
        mmu[address] = value;

        switch (address) {
          case 0:
            /* Configuration register (CR).  */
            io_in = !(value & 0x1);
            basic_lo_in = !(value & 0x2);
            basic_hi_in = !(value & 0xc);
            kernal_in = chargen_in = editor_in = !(value & 0x30);
            mmu_set_ram_bank(value);
#ifdef MMU_DEBUG
            log_message(mmu_log,
                        "IO: %s BASLO: %s BASHI: %s KERNAL %s FUNCLO %s.",
                        io_in ? "on" : "off", basic_lo_in ? "on" : "off",
                        basic_hi_in ? "on" : "off", kernal_in ? "on" : "off",
                        ((value & 0xc) == 0x4) ? "on" : "off");
#endif
            break;
          case 5:
            value = (value & 0x7f) | 0x30;
            if ((value & 1) ^ (oldvalue & 1))
                mmu_switch_cpu(value & 1);
            break;
          case 6:
            /* RAM configuration register (RCR).  */
            mem_set_ram_config(value);
            break;
          case 7:
          case 8:
          case 9:
          case 10:
            page_zero = (ram + (mmu[0x8] & 0x1 ? 0x10000 : 0x00000)
                         + (mmu[0x7] << 8));
            page_one = (ram + (mmu[0xa] & 0x1 ? 0x10000 : 0x00000)
                        + (mmu[0x9] << 8));
#ifdef MMU_DEBUG
            log_message(mmu_log, "PAGE ZERO %05x PAGE ONE %05x",
                (mmu[0x8] & 0x1 ? 0x10000 : 0x00000) + (mmu[0x7] << 8),
                (mmu[0xa] & 0x1 ? 0x10000 : 0x00000) + (mmu[0x9] << 8));
#endif
            break;
        }

        mmu_update_config();
    }
}

/* $FF00 - $FFFF: RAM, Kernal or internal function ROM, with MMU at
   $FF00 - $FF04.  */
BYTE REGPARM1 mmu_ffxx_read(ADDRESS addr)
{
    if (addr >= 0xff00 && addr <= 0xff04)
        return mmu[addr & 0xf];

    if ((mmu[0] & 0x30) == 0x00)
        return kernal_read(addr);
    if ((mmu[0] & 0x30) == 0x10)
        return internal_function_rom_read(addr);
 
    return top_shared_read(addr);
}

BYTE REGPARM1 mmu_ffxx_read_z80(ADDRESS addr)
{
    if (addr >= 0xff00 && addr <= 0xff04)
        return mmu[addr & 0xf];

    return top_shared_read(addr);
}

void REGPARM2 mmu_ffxx_store(ADDRESS addr, BYTE value)
{
    if (addr == 0xff00) {
        mmu_store(0, value);
        /* FIXME? [SRT] does reu_dma(-1) work here, or should
        it be deferred until later? */
        reu_dma(-1);
    } else {
        if (addr <= 0xff04)
            mmu_store(0, mmu[addr & 0xf]);
        else
            top_shared_store(addr, value);
    }
}

/* ------------------------------------------------------------------------- */

void mmu_init(void)
{
    if (mmu_log == LOG_ERR)
        mmu_log = log_open("MMU");

    set_column4080_key((resource_value_t)mmu_column4080_key, NULL);

    mmu[5] = 0;
}

void mmu_reset(void)
{
    ADDRESS i;

    for (i = 0; i < 0xb; i++)
        mmu[i] = 0;

    kbd_register_column4080_key(mmu_toggle_column4080_key);
}

