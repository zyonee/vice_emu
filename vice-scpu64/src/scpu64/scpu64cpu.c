/*
 * scpu64cpu.c - Emulation of the main 65816 processor.
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
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

#include "clkguard.h"
#include "main65816cpu.h"
#include "mem.h"
#include "scpu64mem.h"
#include "vicii.h"
#include "viciitypes.h"

/* ------------------------------------------------------------------------- */

/* MACHINE_STUFF should define/undef

 - NEED_REG_PC
 - TRACE

 The following are optional:

 - PAGE_ZERO
 - PAGE_ONE
 - STORE_IND
 - LOAD_IND
 - DMA_FUNC
 - DMA_ON_RESET

*/

/* ------------------------------------------------------------------------- */

static int fastmode = 1;
static int half_cycles;
int scpu64_emulation_mode;

int scpu64_get_half_cycle(void)
{
    if (fastmode) {
        return half_cycles;
    }
    return -1;
}

static void scpu64_clock_add(CLOCK *clock, int amount)
{
    if (fastmode) {
        half_cycles += amount;
        if (amount > 0) {
            while (half_cycles > 19) {
                clock[0]++;
                half_cycles -= 20;
            }
        } else {
            while (half_cycles < 0) {
                clock[0]--;
                half_cycles += 20;
            }
        }
    } else {
        clock[0] += amount;
    }
}

void scpu64_set_fastmode(int mode)
{
    fastmode = mode;
}

int scpu64_get_fastmode(void)
{
    return fastmode;
}

/* if half cycle is not 19, add extra half cycles to stretch */
void scpu64_clock_read_stretch(void)
{
    if (fastmode) {
        half_cycles = 19;
    }
}

/* add 1 full cycle for 2 cycle stretch if rmw,
   otherwise add half cycle for 1 cycle stretch */
void scpu64_clock_write_stretch(void)
{
    if (maincpu_rmw_flag == 0) {
        /* subtract a (half) cycle and use the read stretch to handle it */
        scpu64_clock_add(&maincpu_clk, -1);
        scpu64_clock_read_stretch();

        /* add previously subtracted (half) cycle */
        scpu64_clock_add(&maincpu_clk, 1);
    } else {
        if (vicii.fastmode != 19) {
            /* handle I/O access stretch for rmw */
            maincpu_clk++;
        }
    }
}

/* SCPU64 needs external reg_pc */
#define NEED_REG_PC

#define STORE(addr, value) \
    do { \
        DWORD tmpx1 = (addr); \
        BYTE tmpx2 = (value); \
        if (tmpx1 & ~0xffff) { \
            mem_store2(tmpx1, tmpx2); \
        } else { \
            (*_mem_write_tab_ptr[tmpx1 >> 8])((WORD)tmpx1, tmpx2); \
        } \
    } while (0)

#define LOAD(addr) \
    (((addr) & ~0xffff)?mem_read2(addr):(*_mem_read_tab_ptr[(addr) >> 8])((WORD)(addr)))

#define STORE_LONG(addr, value) store_long(addr, value)

static inline void store_long(DWORD addr, BYTE value)
{
    scpu64_clock_add(&maincpu_clk, 1);
    if (addr & ~0xffff) {
        mem_store2(addr, value);
    } else {
        (*_mem_write_tab_ptr[addr >> 8])((WORD)addr, value);
    }
}

#define LOAD_LONG(addr) load_long(addr)

static inline BYTE load_long(DWORD addr)
{
    BYTE tmp;

    if ((addr) & ~0xffff) {
        tmp = mem_read2(addr);
    } else {
        tmp = (*_mem_read_tab_ptr[(addr) >> 8])((WORD)addr);
    }
    scpu64_clock_add(&maincpu_clk, 1);
    return tmp;
}

#define EMULATION_MODE_CHANGED scpu64_emulation_mode = reg_emul

#define CLK_ADD(clock, amount) scpu64_clock_add(&clock, amount)

#define LOAD_INT_ADDR(addr)                        \
    if (scpu64_interrupt_reroute()) {              \
        reg_pc = LOAD_LONG(addr + 0xf80000);       \
        reg_pc |= LOAD_LONG(addr + 0xf80001) << 8; \
    } else {                                       \
        reg_pc = LOAD_LONG(addr);                  \
        reg_pc |= LOAD_LONG(addr + 1) << 8;        \
    }
/* ------------------------------------------------------------------------- */

#include "../main65816cpu.c"
