/*
 * ted-mem.c - Memory interface for the TED emulation.
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#include "interrupt.h"
#include "keyboard.h"
#include "maincpu.h"
#include "plus4mem.h"
#include "plus4tia1.h"
#include "ted-resources.h"
#include "ted-mem.h"
#include "ted-sound.h"
#include "ted-timer.h"
#include "ted.h"
#include "tedtypes.h"
#include "types.h"


/* ---------------------------------------------------------------------*/
/*
extern void (*ted_irq_handler)(int irq, int state, CLOCK clk);

#define ted_set_irq(irq, state)         \
    if (ted_irq_handler != NULL) {      \
        ted_irq_handler(irq, state, 0); \
    } else {                            \
        maincpu_set_irq(irq, state);    \
    }
*/

#define ted_set_irq(irq, state) maincpu_set_irq(irq, state)

/* ---------------------------------------------------------------------*/

/* Unused bits in TED registers: these are always 1 when read.  */
static int unused_bits_in_registers[64] =
{
    0x00 /* $FF00 */ , 0x00 /* $FF01 */ , 0x00 /* $FF02 */ , 0x00 /* $FF03 */ ,
    0x00 /* $FF04 */ , 0x00 /* $FF05 */ , 0x00 /* $FF06 */ , 0x00 /* $FF07 */ ,
    0x00 /* $FF08 */ , 0x00 /* $FF09 */ , 0xa0 /* $FF0A */ , 0x00 /* $FF0B */ ,
    0xfc /* $FF0C */ , 0x00 /* $FF0D */ , 0x00 /* $FF0E */ , 0x00 /* $FF0F */ ,
    0x00 /* $FF10 */ , 0x00 /* $FF11 */ , 0x00 /* $FF12 */ , 0x00 /* $FF13 */ ,
    0x00 /* $FF14 */ , 0x80 /* $FF15 */ , 0x80 /* $FF16 */ , 0x80 /* $FF17 */ ,
    0x80 /* $FF18 */ , 0x80 /* $FF19 */ , 0x00 /* $FF1A */ , 0x00 /* $FF1B */ ,
    0x00 /* $FF1C */ , 0x00 /* $FF1D */ , 0x00 /* $FF1E */ , 0x00 /* $FF1F */ ,
    0x00 /* $FF20 */ , 0x00 /* $FF21 */ , 0x00 /* $FF22 */ , 0x00 /* $FF23 */ ,
    0x00 /* $FF24 */ , 0x00 /* $FF25 */ , 0x00 /* $FF26 */ , 0x00 /* $FF27 */ ,
    0x00 /* $FF28 */ , 0x00 /* $FF29 */ , 0x00 /* $FF2A */ , 0x00 /* $FF2B */ ,
    0x00 /* $FF2C */ , 0x00 /* $FF2D */ , 0x00 /* $FF2E */ , 0x00 /* $FF2F */ ,
    0x00 /* $FF30 */ , 0x00 /* $FF31 */ , 0x00 /* $FF32 */ , 0x00 /* $FF33 */ ,
    0x00 /* $FF34 */ , 0x00 /* $FF35 */ , 0x00 /* $FF36 */ , 0x00 /* $FF37 */ ,
    0x00 /* $FF38 */ , 0x00 /* $FF39 */ , 0x00 /* $FF3A */ , 0x00 /* $FF3B */ ,
    0x00 /* $FF3C */ , 0x00 /* $FF3D */ , 0x00 /* $FF3E */ , 0x00 /* $FF3F */
};


inline void REGPARM2 ted_local_store_vbank(ADDRESS addr, BYTE value)
{
    /* This can only cause "aesthetical" errors, so let's save some time if
       the current frame will not be visible.  */
    if (!ted.raster.skip_frame) {
        int f;

        /* Argh... this is a dirty kludge!  We should probably find a cleaner
           solution.  */
        do {
            CLOCK mclk;

            /* WARNING: Assumes `rmw_flag' is 0 or 1.  */
            mclk = clk - rmw_flag - 1;
            f = 0;

            if (mclk >= ted.fetch_clk) {
                /* If the fetch starts here, the sprite fetch routine should
                   get the new value, not the old one.  */
                if (mclk == ted.fetch_clk) {
                    ram[addr] = value;
                }
                ted_raster_fetch_alarm_handler (clk - ted.fetch_clk);
                f = 1;
                /* WARNING: Assumes `rmw_flag' is 0 or 1.  */
                mclk = clk - rmw_flag - 1;
            }

            if (mclk >= ted.draw_clk) {
                ted_raster_draw_alarm_handler(0);
                f = 1;
            }
        } while (f);
    }

    ram[addr] = value;
}

/* Encapsulate inlined function for other modules */
void REGPARM2 ted_mem_vbank_store(ADDRESS addr, BYTE value)
{
    ted_local_store_vbank(addr, value);
}

#if 0
/* As `store_vbank()', but for the $3900...$39FF address range.  */
void REGPARM2 ted_mem_vbank_39xx_store(ADDRESS addr, BYTE value)
{
    ted_local_store_vbank(addr, value);

    if (ted.idle_data_location == IDLE_39FF && (addr & 0x3fff) == 0x39ff)
        raster_add_int_change_foreground
            (&ted.raster,
            TED_RASTER_CHAR(TED_RASTER_CYCLE(clk)),
            &ted.idle_data,
            value);
}

/* As `store_vbank()', but for the $3F00...$3FFF address range.  */
void REGPARM2 ted_mem_vbank_3fxx_store(ADDRESS addr, BYTE value)
{
    ted_local_store_vbank (addr, value);

    if (ted.idle_data_location == IDLE_3FFF && (addr & 0x3fff) == 0x3fff)
        raster_add_int_change_foreground
            (&ted.raster,
            TED_RASTER_CHAR(TED_RASTER_CYCLE(clk)),
            &ted.idle_data,
            value);
}
#endif

inline static void check_bad_line_state_change(BYTE value, int cycle, int line)
{
    int was_bad_line, now_bad_line;

    /* Check whether bad line state has changed.  */
    was_bad_line = (ted.allow_bad_lines
                    && (ted.raster.ysmooth == (line & 7)));
    now_bad_line = (ted.allow_bad_lines
                    && ((value & 7) == (line & 7)));

    if (was_bad_line && !now_bad_line) {

        /* Bad line becomes good.  */
        ted.bad_line = 0;

        /* By changing the values in the registers, one can make the TED 
           switch from idle to display state, but not from display to
           idle state.  So we are always in display state if this
           happens.  This is only true if the value changes in some
           cycle > 0, though; otherwise, the line never becomes bad.  */
        if (cycle > 0) {
            ted.raster.draw_idle_state = ted.idle_state = 0;
            ted.idle_data_location = IDLE_NONE;
            if (cycle > TED_FETCH_CYCLE + 2
                && !ted.ycounter_reset_checked) {
                ted.raster.ycounter = 0;
                ted.ycounter_reset_checked = 1;
            }
        }

    } else if (!was_bad_line && now_bad_line) {
        if (cycle >= TED_FETCH_CYCLE
            && cycle <= TED_FETCH_CYCLE + TED_SCREEN_TEXTCOLS + 3) {
            int pos;            /* Value of line counter when this happens.  */
            int inc;            /* Total increment for the line counter.  */
            int num_chars;      /* Total number of characters to fetch.  */
            int num_0xff_fetches; /* Number of 0xff fetches to do.  */

            ted.bad_line = 1;

            if (cycle <= TED_FETCH_CYCLE + 2)
                ted.raster.ycounter = 0;

            ted.ycounter_reset_checked = 1;

            num_chars = (TED_SCREEN_TEXTCOLS
                         - (cycle - (TED_FETCH_CYCLE + 3)));

            if (num_chars <= TED_SCREEN_TEXTCOLS) {
                /* Matrix fetches starts immediately, but the TED needs
                   at least 3 cycles to become the bus master.  Before
                   this happens, it fetches 0xff.  */
                num_0xff_fetches = 3;

                /* If we were in idle state before creating the bad
                   line, the counters have not been incremented.  */
                if (ted.idle_state) {
                    pos = 0;
                    inc = num_chars;
                    if (inc < 0)
                        inc = 0;
                } else {
                    pos = cycle - (TED_FETCH_CYCLE + 3);
                    if (pos > TED_SCREEN_TEXTCOLS - 1)
                        pos = TED_SCREEN_TEXTCOLS - 1;
                    inc = TED_SCREEN_TEXTCOLS;
                }
            } else {
                pos = 0;
                num_chars = inc = TED_SCREEN_TEXTCOLS;
                num_0xff_fetches = cycle - TED_FETCH_CYCLE;
            }

            /* This is normally done at cycle `TED_FETCH_CYCLE + 2'.  */
            ted.mem_counter = ted.memptr;

            /* Force the DMA.  */
            /* Note that `ted.cbuf' is loaded from the value of
               the next opcode as the VIC-II is not the bus master yet.  */
            if (num_chars <= num_0xff_fetches) {
                memset(ted.vbuf + pos, 0xff, num_chars);
                memset(ted.cbuf + pos, ram[reg_pc] & 0xf,
                       num_chars);
            } else {
                memset(ted.vbuf + pos, 0xff, num_0xff_fetches);
                memset(ted.cbuf + pos, ram[reg_pc] & 0xf,
                       num_0xff_fetches);
                ted_fetch_matrix(pos + num_0xff_fetches,
                                 num_chars - num_0xff_fetches);
                ted_fetch_color(pos + num_0xff_fetches,
                                num_chars - num_0xff_fetches);
            }

            /* Set the value by which `ted.mem_counter' is incremented on
               this line.  */
            ted.mem_counter_inc = inc;

            /* Take over the bus until the memory fetch is done.  */
            clk = (TED_LINE_START_CLK(clk) + TED_FETCH_CYCLE
                   + TED_SCREEN_TEXTCOLS + 3);

            /* Remember we have done a DMA.  */
            ted.memory_fetch_done = 2;

            /* As we are on a bad line, switch to display state.  */
            ted.idle_state = 0;

            /* Try to display things correctly.  This is not exact,
               but should be OK for most cases (FIXME?).  */
            if (inc == TED_SCREEN_TEXTCOLS) {
                ted.raster.draw_idle_state = 0;
                ted.idle_data_location = IDLE_NONE;
            }
        } else if (cycle <= TED_FETCH_CYCLE + TED_SCREEN_TEXTCOLS + 6) {
            /* Bad line has been generated after fetch interval, but
               before `ted.raster.ycounter' is incremented.  */

            ted.bad_line = 1;

            /* If in idle state, counter is not incremented.  */
            if (ted.idle_state)
                ted.mem_counter_inc = 0;

            /* We are not in idle state anymore.  */
            /* This is not 100% correct, but should be OK for most cases.
               (FIXME?)  */
            ted.raster.draw_idle_state = ted.idle_state = 0;
            ted.idle_data_location = IDLE_NONE;

        } else {
            /* Line is now bad, so we must switch to display state.
               Anyway, we cannot do it here as the `ycounter' handling
               must happen in as in idle state.  */
            ted.force_display_state = 1;
        }
    }
}

inline static void ted06_store(BYTE value)
{
    int cycle;
    int line;

    cycle = TED_RASTER_CYCLE(clk);
    line = TED_RASTER_Y(clk);

    TED_DEBUG_REGISTER(("\tControl register: $%02X\n", value));
    TED_DEBUG_REGISTER(("$D011 tricks at cycle %d, line $%04X, "
                       "value $%02X\n", cycle, line, value));

    /* This is the funniest part... handle bad line tricks.  */

    if (line == ted.first_dma_line && (value & 0x10) != 0)
        ted.allow_bad_lines = 1;

    if (ted.raster.ysmooth != (value & 7)
        && line >= ted.first_dma_line
        && line <= ted.last_dma_line)
        check_bad_line_state_change(value, cycle, line);

    ted.raster.ysmooth = value & 0x7;

    /* Check for 24 <-> 25 line mode switch.  */
    if ((value ^ ted.regs[0x06]) & 8) {
        if (value & 0x8) {
            /* 24 -> 25 row mode switch.  */

            ted.raster.display_ystart = ted.row_25_start_line;
            ted.raster.display_ystop = ted.row_25_stop_line;

            if (line == ted.row_24_stop_line && cycle > 0) {
                /* If on the first line of the 24-line border, we
                   still see the 25-line (lowmost) border because the
                   border flip flop has already been turned on.  */
                ted.raster.blank_enabled = 1;
            } else {
                if (!ted.raster.blank && line == ted.row_24_start_line
                    && cycle > 0) {
                    /* A 24 -> 25 switch somewhere on the first line of
                       the 24-row mode is enough to disable screen
                       blanking.  */
                    ted.raster.blank_enabled = 0;
                }
            }
            TED_DEBUG_REGISTER(("\t25 line mode enabled\n"));
        } else {
            /* 25 -> 24 row mode switch.  */

            ted.raster.display_ystart = ted.row_24_start_line;
            ted.raster.display_ystop = ted.row_24_stop_line;

            /* If on the last line of the 25-line border, we still see the
               24-line (upmost) border because the border flip flop has
               already been turned off.  */
            if (!ted.raster.blank
                && line == ted.row_25_start_line
                && cycle > 0)
                ted.raster.blank_enabled = 0;
            else if (line == ted.row_25_stop_line && cycle > 0)
                ted.raster.blank_enabled = 1;

            TED_DEBUG_REGISTER(("\t24 line mode enabled\n"));
        }
    }

    ted.raster.blank = !(value & 0x10);        /* `DEN' bit.  */

    ted.regs[0x06] = value;

    /* FIXME: save time.  */
    ted_update_video_mode(cycle);
}

inline static void ted07_store(BYTE value)
{
    raster_t *raster;
    int cycle;

    TED_DEBUG_REGISTER(("\tControl register: $%02X\n", value));

    raster = &ted.raster;
    cycle = TED_RASTER_CYCLE(clk);

    /* FIXME: Line-based emulation!  */
    if ((value & 7) != (ted.regs[0x07] & 7)) {
#if 1
        if (raster->skip_frame || TED_RASTER_CHAR(cycle) <= 1)
            raster->xsmooth = value & 0x7;
        else
            raster_add_int_change_next_line(raster,
                                            &raster->xsmooth,
                                            value & 0x7);
#else
        raster_add_int_change_foreground(raster,
                                         TED_RASTER_CHAR(cycle),
                                         &raster->xsmooth,
                                         value & 7);
#endif
    }

    /* Bit 4 (CSEL) selects 38/40 column mode.  */
    if ((value & 0x8) != (ted.regs[0x07] & 0x8)) {
        if (value & 0x8) {
            /* 40 column mode.  */
            if (cycle <= 17)
                raster->display_xstart = TED_40COL_START_PIXEL;
            else
                raster_add_int_change_next_line(raster,
                                                &raster->display_xstart,
                                                TED_40COL_START_PIXEL);
            if (cycle <= 56)
                raster->display_xstop = TED_40COL_STOP_PIXEL;
            else
                raster_add_int_change_next_line(raster,
                                                &raster->display_xstop,
                                                TED_40COL_STOP_PIXEL);
            TED_DEBUG_REGISTER(("\t40 column mode enabled\n"));

            /* If CSEL changes from 0 to 1 at cycle 17, the border is
               not turned off and this line is blank.  */
            if (cycle == 17 && !(ted.regs[0x07] & 0x8))
                raster->blank_this_line = 1;
        } else {
            /* 38 column mode.  */
            if (cycle <= 17)
                raster->display_xstart = TED_38COL_START_PIXEL;
            else
                raster_add_int_change_next_line(raster,
                                                &raster->display_xstart,
                                                TED_38COL_START_PIXEL);
            if (cycle <= 56)
                raster->display_xstop = TED_38COL_STOP_PIXEL;
            else
                raster_add_int_change_next_line(raster,
                                                &raster->display_xstop,
                                                TED_38COL_STOP_PIXEL);
            TED_DEBUG_REGISTER(("\t38 column mode enabled\n"));

            /* If CSEL changes from 1 to 0 at cycle 56, the lateral
               border is open.  */
            if (cycle == 56 && (ted.regs[0x07] & 0x8)
                && (!raster->blank_enabled || raster->open_left_border))
                raster->open_right_border = 1;
        }
    }

    ted.regs[0x07] = value;

    ted_update_video_mode(cycle);
}

inline static void ted08_store(BYTE value)
{
    BYTE val = 0xff;
    BYTE msk = tia_kbd;
    BYTE m;
    int i;

    for (m = 0x1, i = 0; i < 8; m <<= 1, i++)
        if (!(msk & m))
           val &= ~keyarr[i];

    if (!(value & 4)) {
        val = val & ~(joystick_value[1] & 15);
        if (joystick_value[1] & 16)
            val = val & ~64;
    }

    if (!(value & 2)) {
        val = val & ~(joystick_value[2] & 15);
        if (joystick_value[2] & 16)
            val = val & ~128;
    }

    ted.kbdval = val;
}

inline static void ted09_store(BYTE value)
{
    if (rmw_flag) { /* (emulates the Read-Modify-Write bug) */
        ted.irq_status = 0;
    } else {
        ted.irq_status &= ~((value & 0x5e) | 0x80);
        if (ted.irq_status & ted.regs[0x0a])
            ted.irq_status |= 0x80;
    }

    /* Update the IRQ line accordingly...
       The external TED IRQ line is an AND of the internal collision and
       ted.raster IRQ lines.  */
    if (ted.irq_status & 0x80) {
        ted_set_irq(I_RASTER, 1);
    } else {
        ted_set_irq(I_RASTER, 0);
    }

    TED_DEBUG_REGISTER(("\tIRQ flag register: $%02X\n", ted.irq_status));
}

inline static void ted0a_store(BYTE value)
{
    int new_irq_line;

    new_irq_line = ((ted.raster_irq_line & 0xff) | ((value & 1) << 8));
    ted_set_raster_irq(new_irq_line);

    ted.regs[0x0a] = value & 0x5f;

    if (ted.regs[0x0a] & ted.irq_status) {
        ted.irq_status |= 0x80;
        ted_set_irq(I_RASTER, 1);
    } else {
        ted.irq_status &= 0x7f;
        ted_set_irq(I_RASTER, 0);
    }

    TED_DEBUG_REGISTER(("\tIRQ mask register: $%02X\n", ted.regs[addr]));
}

inline static void ted0b_store(BYTE value)
{
    unsigned int line;
    unsigned int old_raster_irq_line;

    /* FIXME: Not accurate as bit #8 is missing.  */
    value = (value - ted.offset) & 255;

    TED_DEBUG_REGISTER (("\tRaster compare register: $%02X\n", value));

    if (value == ted.regs[0x0b])
        return;

    line = TED_RASTER_Y(clk);
    ted.regs[0x0b] = value;

    TED_DEBUG_REGISTER(("\tRaster interrupt line set to $%04X\n",
                          ted.raster_irq_line));

    old_raster_irq_line = ted.raster_irq_line;
    ted_set_raster_irq((ted.raster_irq_line & 0x100) | value);

    /* Check whether we should activate the IRQ line now.  */
    if (ted.regs[0x0a] & 0x2) {
        int trigger_irq;

        trigger_irq = 0;

        if (rmw_flag) {
            if (TED_RASTER_CYCLE (clk) == 0) {
                unsigned int previous_line = TED_PREVIOUS_LINE(line);

                if (previous_line != old_raster_irq_line
                    && ((old_raster_irq_line & 0x100)
                    == (previous_line & 0x100)))
                    trigger_irq = 1;
            } else {
                if (line != old_raster_irq_line
                    && (old_raster_irq_line & 0x100) == (line & 0x100))
                    trigger_irq = 1;
            }
        }

        if (ted.raster_irq_line == line && line != old_raster_irq_line)
            trigger_irq = 1;

        if (trigger_irq) {
            ted.irq_status |= 0x82;
            ted_set_irq (I_RASTER, 1);
        }
    }
}

inline static void ted0c0d_store(ADDRESS addr, BYTE value)
{
    int pos;

    if (ted.regs[addr] == value)
        return;

    if (addr & 1)
        pos = (ted.crsrpos & 0x300) | value;
    else
        pos = (ted.crsrpos & 0xff) | ((value & 3) << 8);

#if 0
    raster_add_int_change_background(&ted.raster,
                                     TED_RASTER_CHAR(TED_RASTER_CYCLE(clk)),
                                     &ted.crsrpos,
                                     pos);
#else
    ted.crsrpos = pos;
#endif
    ted.regs[addr] = value;
}

inline static void ted12_store(BYTE value)
{
    value &= 0x3c;

    if (ted.regs[0x12] == value)
        return;

    ted.regs[0x12] = value;
    ted_update_memory_ptrs(TED_RASTER_CYCLE(clk));
}

inline static void ted13_store(BYTE value)
{
    if ((ted.regs[0x13] & 0xfe) == (value & 0xfe))
        return;

    if ((ted.regs[0x13] & 2) ^ (value & 2)) {
        if (value & 2)
            log_debug("Slow mode");
        else
            log_debug("Fast mode");
    }

    ted.regs[0x13] = (ted.regs[0x13] & 0x01) | (value & 0xfe);
    ted_update_memory_ptrs(TED_RASTER_CYCLE(clk));
}

inline static void ted14_store(BYTE value)
{
    if (ted.regs[0x14] == value)
        return;

    ted.regs[0x14] = value;
    ted_update_memory_ptrs(TED_RASTER_CYCLE(clk));
}

inline static void ted15_store(BYTE value)
{
    int x_pos;

    value &= 0x7f;

    TED_DEBUG_REGISTER(("\tBackground #0 color register: $%02X\n",
                          value));

    if (ted.regs[0x15] == value)
        return;

    if (!ted.force_black_overscan_background_color)
        raster_add_int_change_background
            (&ted.raster,
            TED_RASTER_X(TED_RASTER_CYCLE(clk)),
            &ted.raster.overscan_background_color,
            value);

    x_pos = TED_RASTER_X(TED_RASTER_CYCLE(clk));
    raster_add_int_change_background(&ted.raster,
                                     x_pos,
                                     &ted.raster.background_color,
                                     value);
    ted.regs[0x15] = value;
}

inline static void ted161718_store(ADDRESS addr, BYTE value)
{
    int char_num;

    value &= 0x7f;

    TED_DEBUG_REGISTER(("\tBackground color #%d register: $%02X\n",
                          addr - 0x15, value));

    if (ted.regs[addr] == value)
        return;

    char_num = TED_RASTER_CHAR(TED_RASTER_CYCLE(clk));

    raster_add_int_change_foreground(&ted.raster,
                                     char_num,
                                     &ted.ext_background_color[addr - 0x16],
                                     value);

    ted.regs[addr] = value;
}

inline static void ted19_store(BYTE value)
{
    TED_DEBUG_REGISTER(("\tBorder color register: $%02X\n", value));

    value &= 0x7f;

    if (ted.regs[0x19] != value) {
        ted.regs[0x19] = value;
        raster_add_int_change_border(&ted.raster,
                                     TED_RASTER_X(TED_RASTER_CYCLE(clk)), 
                                     &ted.raster.border_color,
                                     value);
    }
}

inline static void ted3e_store(void)
{
    ted.regs[0x13] |= 0x01;
    mem_config_ram_set(1);
}

inline static void ted3f_store(void)
{
    ted.regs[0x13] &= 0xfe;
    mem_config_ram_set(0);
}

/* Store a value in a TED register.  */
void REGPARM2 ted_store(ADDRESS addr, BYTE value)
{
    addr &= 0x3f;

    /* WARNING: assumes `rmw_flag' is 0 or 1.  */
    ted_handle_pending_alarms(rmw_flag + 1);

    /* This is necessary as we must be sure that the previous line has been
       updated and `current_line' is actually set to the current Y position of
       the raster.  Otherwise we might mix the changes for this line with the
       changes for the previous one.  */
    if (clk >= ted.draw_clk)
        ted_raster_draw_alarm_handler(clk - ted.draw_clk);

    switch (addr) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
        ted_timer_store(addr, value);
        break;
      case 0x06:
        ted06_store(value);
        break;
      case 0x07:
        ted07_store(value);
        break;
      case 0x08:
        ted08_store(value);
        break;
      case 0x09:
        ted09_store(value);
        break;
      case 0x0a:
        ted0a_store(value);
        break;
      case 0x0b:
        ted0b_store(value);
        break;
      case 0x0c:
      case 0x0d:
        ted0c0d_store(addr, value);
        break;
      case 0x0e:
      case 0x0f:
      case 0x10:
      case 0x11:
        ted_sound_store(addr, value);
        break;
      case 0x12:
        ted12_store(value);
        ted_sound_store(addr, value);
        break;
      case 0x13:
        ted13_store(value);
        break;
      case 0x14:
        ted14_store(value);
        break;
      case 0x15:
        ted15_store(value);
        break;
      case 0x16:
      case 0x17:
      case 0x18:
        ted161718_store(addr, value);
        break;
      case 0x19:
        ted19_store(value);
        break;
      case 0x3e:
        ted3e_store();
        break;
      case 0x3f:
        ted3f_store();
        break;
    }
}


inline static unsigned int read_raster_y(void)
{
    int raster_y;

    raster_y = TED_RASTER_Y(clk);

    /* Line 0 is 62 cycles long, while line (SCREEN_HEIGHT - 1) is 64
       cycles long.  As a result, the counter is incremented one
       cycle later on line 0.  */
    if (raster_y == 0 && TED_RASTER_CYCLE(clk) == 0)
        raster_y = ted.screen_height - 1;

    return raster_y;
}

inline static BYTE ted08_read(void)
{
    return ted.kbdval;
}

inline static BYTE ted09_read(void)
{
    if (TED_RASTER_Y(clk) == ted.raster_irq_line
        && (ted.regs[0x0a] & 0x2))
        /* As int_raster() is called 2 cycles later than it should be to
           emulate the 6510 internal IRQ delay, `ted.irq_status' might not
           have bit 0 set as it should.  */
        return ted.irq_status | 0x23;
    else
        return ted.irq_status | 0x22;
}

inline static BYTE ted0a_read(void)
{
    return (ted.regs[0x0a] & 0x5f) | 0xa0;

}

inline static BYTE ted12_read(void)
{
    return ted.regs[0x12] | 0xc0;
}

inline static BYTE ted1a1b_read(ADDRESS addr)
{
    if (addr == 0x1a)
        return (ted.mem_counter & 0x100) >> 8;
    else
        return ted.mem_counter & 0xff;
}

inline static BYTE ted1c1d_read(ADDRESS addr)
{
    unsigned int tmp = (ted.screen_height + read_raster_y()
                       - ted.offset) % ted.screen_height;

    if (addr == 0x1c)
        return (tmp & 0x100) >> 8;
    else
        return tmp & 0xff;
}

inline static BYTE ted1e_read(void)
{
    int xpos;

    xpos = TED_RASTER_X(clk % ted.cycles_per_line);
    if (xpos < 0)
        xpos = ted.cycles_per_line * 8 + xpos;

    xpos = xpos / 2 + 2;

    return (BYTE)xpos;
}

inline static BYTE ted1f_read(void)
{
    return 0x80 | ((ted.cursor_phase & 0x0f) << 3) | ted.raster.ycounter;
}

/* Read a value from a TED register.  */
BYTE REGPARM1 ted_read(ADDRESS addr)
{
    addr &= 0x3f;

    /* Serve all pending events.  */
    ted_handle_pending_alarms(0);

    switch (addr) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
        return ted_timer_read(addr);
      case 0x08: 
        return ted08_read();
      case 0x09:
        return ted09_read();
      case 0x0a:
        return ted0a_read();
      case 0x0e:
      case 0x0f:
      case 0x10:
      case 0x11:
        return ted_sound_read(addr);
      case 0x12:
        return ted_sound_read(addr) | ted12_read();
      case 0x1a:
      case 0x1b:
        return ted1a1b_read(addr);
      case 0x1c:
      case 0x1d:
        return ted1c1d_read(addr);
      case 0x1e:
        return ted1e_read();
      case 0x1f:
        return ted1f_read();
    }

    return ted.regs[addr] | unused_bits_in_registers[addr];
}

BYTE REGPARM1 ted_peek(ADDRESS addr)
{
    addr &= 0x3f;

    switch (addr) {
      case 0x08:
        return ted08_read();
      case 0x09:
        return ted09_read();
      case 0x0e:
      case 0x0f:
      case 0x10:
      case 0x11:
        return ted_sound_read(addr);
      case 0x12:
        return ted_sound_read(addr) | ted12_read();
      case 0x1a:
      case 0x1b:
        return ted1a1b_read(addr);
      case 0x1c:
      case 0x1d:
        return ted1c1d_read(addr);
      case 0x1e:
        return ted1e_read();
      case 0x1f:
        return ted1f_read();
      default:
        return ted.regs[addr] | unused_bits_in_registers[addr];
    }
}

