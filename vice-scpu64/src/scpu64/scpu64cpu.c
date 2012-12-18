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
static int buffer_finish_half;
static CLOCK buffer_finish;
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

static inline void wait_buffer(void)
{
    if (buffer_finish > maincpu_clk || (buffer_finish == maincpu_clk && buffer_finish_half > half_cycles)) {
        maincpu_clk = buffer_finish;
        half_cycles = buffer_finish_half;
    }
}

void scpu64_clock_read_stretch(void)
{
    if (fastmode) {
        wait_buffer();
        if (half_cycles > 13) {
            maincpu_clk++;
        }
        maincpu_clk++;
        half_cycles = 2; /* measured, don't change */
    }
}

void scpu64_clock_write_stretch_io_slow(void)
{
    if (fastmode) {
        wait_buffer();
//        buffer_finish = maincpu_clk + 2;
//        buffer_finish_half = 10;
        if (half_cycles > 12) {
            maincpu_clk++;
        }
        maincpu_clk++;
        maincpu_clk++;
        half_cycles = 7; /* measured, don't change */
    }
}

void scpu64_clock_write_stretch_io(void)
{
    if (fastmode) {
        wait_buffer();
        if (half_cycles > 13) {
            maincpu_clk++;
        }
        maincpu_clk++;
        half_cycles = 5; /* measured, don't change */
    }
}

void scpu64_clock_write_stretch(void)
{
    if (fastmode) {
        wait_buffer();
        buffer_finish = maincpu_clk + 1;
        buffer_finish_half = 3;
    }
}

static void clk_overflow_callback(CLOCK sub, void *unused_data)
{
    if (buffer_finish > sub) {
        buffer_finish -= sub;
    } else {
        buffer_finish = 0;
    }
}

#define CPU_ADDITIONAL_INIT() clk_guard_add_callback(maincpu_clk_guard, clk_overflow_callback, NULL)

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

#define CPU_ADDITIONAL_RESET() (buffer_finish = maincpu_clk, buffer_finish_half = 0)

#define FETCH_PARAM_DUMMY(addr) scpu64_clock_add(&maincpu_clk, 1)
#define LOAD_LONG_DUMMY(addr) scpu64_clock_add(&maincpu_clk, 1)

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
