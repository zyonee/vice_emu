/*
 * vicii-cycle.c - Cycle based VIC-II emulation.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#include "6510core.h"
#include "alarm.h"
#include "debug.h"
#include "interrupt.h"
#include "log.h"
#include "mainc64cpu.h"
#include "types.h"
#include "vicii-chip-model.h"
#include "vicii-cycle.h"
#include "vicii-draw-cycle.h"
#include "vicii-fetch.h"
#include "vicii-irq.h"
#include "vicii.h"
#include "viciitypes.h"

static inline void check_badline(void)
{
    /* Check badline condition (line range and "allow bad lines" handled outside */
    if ((vicii.raster_line & 7) == vicii.ysmooth) {
        vicii.bad_line = 1;
        vicii.idle_state = 0;
    } else {
        vicii.bad_line = 0;
    }

    if (vicii.bad_line && !vicii.fetch_active && (vicii.raster_cycle >= VICII_PAL_CYCLE(12)) && (vicii.raster_cycle <=  VICII_PAL_CYCLE(54))) {
        /* Start a fetch */
        vicii.fetch_active = 1;
        /* Hold BA low for 3 cycles before actual fetch */
        vicii.prefetch_cycles = 3;
    } else if (vicii.fetch_active && !vicii.bad_line) {
        /* Cancel a fetch */
        /* FIXME this is not a clean way, try to get rid of fetch_active */
        vicii.fetch_active = 0;
        vicii.prefetch_cycles = 0;
    }
}

static inline void check_sprite_display(void)
{
    int i, b;
    int enable = vicii.regs[0x15];

    for (i = 0, b = 1; i < VICII_NUM_SPRITES; i++, b <<= 1) {
        int y = vicii.regs[i*2 + 1]; 	 
        vicii.sprite[i].mc = vicii.sprite[i].mcbase;

        if (vicii.sprite[i].dma) {
            if ( (enable & b) && (y == (vicii.raster_line & 0xff)) ) {
                    vicii.sprite_display_bits |= b;
            }
        } else {
            vicii.sprite_display_bits &= ~b;
        }
    }
}

static inline void sprite_mcbase_update(void)
{
    int i;

    for (i = 0; i < VICII_NUM_SPRITES; i++) {
        if (vicii.sprite[i].exp_flop) {
            vicii.sprite[i].mcbase = vicii.sprite[i].mc;
            if (vicii.sprite[i].mcbase == 63) {
                vicii.sprite[i].dma = 0;
            }
        }
    }
}

static inline void check_exp(void)
{
    int i, b;
    int y_exp = vicii.regs[0x17];

    for (i = 0, b = 1; i < VICII_NUM_SPRITES; i++, b <<= 1) {
        if (vicii.sprite[i].dma && (y_exp & b)) {
            vicii.sprite[i].exp_flop ^= 1;
        }
    }
}

/* Enable DMA for sprite i.  */
static inline void turn_sprite_dma_on(unsigned int i, int y_exp)
{
    vicii.sprite[i].dma = 1;
    vicii.sprite[i].mcbase = 0;
    vicii.sprite[i].exp_flop = 1;
}

static inline void check_sprite_dma(void)
{
    int i, b;
    int enable = vicii.regs[0x15];
    int y_exp = vicii.regs[0x17];

    for (i = 0, b = 1; i < VICII_NUM_SPRITES; i++, b <<= 1) {
        int y = vicii.regs[i*2 + 1];

        if ((enable & b) && (y == (vicii.raster_line & 0xff)) && !vicii.sprite[i].dma) {
            turn_sprite_dma_on(i, y_exp & b);
        }
    }
}

static inline BYTE cycle_phi1_fetch(unsigned int cycle_flags)
{
    BYTE data;
    int s;

    if (is_fetch_g(cycle_flags)) {
        if (!vicii.idle_state) {
            data = vicii_fetch_graphics();
        } else {
            data = vicii_fetch_idle_gfx();
        }
        return data;
    }
 
    if (is_sprite_ptr_dma0(cycle_flags)) {
        s = get_sprite_num(cycle_flags);
        data = vicii_fetch_sprite_pointer(s);
        return data;
    }
    if (is_sprite_dma1_dma2(cycle_flags)) {
        s = get_sprite_num(cycle_flags);
        data = vicii_fetch_sprite_dma_1(s);
        return data;
    }

    if (is_refresh(cycle_flags)) {
        data = vicii_fetch_refresh();
        return data;
    }

    data = vicii_fetch_idle();

    return data;
}

static inline void check_vborder(int line)
{
    int rsel = vicii.regs[0x11] & 0x08;

    if ((line == (rsel ? VICII_25ROW_START_LINE : VICII_24ROW_START_LINE)) && (vicii.regs[0x11] & 0x10)) {
        vicii.vborder = 0;
    } else if (line == (rsel ? VICII_25ROW_STOP_LINE : VICII_24ROW_STOP_LINE)) {
        vicii.vborder = 1;
    }
}

static inline void check_hborder(int cycle)
{
    int csel = vicii.regs[0x16] & 0x08;

    if ( cycle == (csel ? VICII_40COL_START_CYCLE : VICII_38COL_START_CYCLE) ) {
        check_vborder(vicii.raster_line);
        if (vicii.vborder == 0) {
            vicii.main_border = 0;
        }
    } else if ( cycle == (csel ? VICII_40COL_STOP_CYCLE : VICII_38COL_STOP_CYCLE) ) {
        vicii.main_border = 1;
    }
}

static inline void vicii_cycle_start_of_frame(void)
{
    vicii.start_of_frame = 0;
    vicii.raster_line = 0;
    vicii.refresh_counter = 0xff;
    vicii.allow_bad_lines = 0;
    vicii.vcbase = 0;
    vicii.vc = 0;
    vicii.light_pen.triggered = 0;
}

static inline void vicii_cycle_end_of_line(void)
{
    vicii_raster_draw_handler();
    if (vicii.raster_line == vicii.screen_height-1) {
        vicii.start_of_frame = 1;
    }
}

static inline void vicii_cycle_start_of_line(void)
{
    /* Check DEN bit on first cycle of the line following the first DMA line  */
    if ((vicii.raster_line == VICII_FIRST_DMA_LINE) && !vicii.allow_bad_lines && (vicii.regs[0x11] & 0x10)) {
        vicii.allow_bad_lines = 1;
    }

    /* Disallow bad lines after the last possible one has passed */
    if (vicii.raster_line == VICII_LAST_DMA_LINE) {
        vicii.allow_bad_lines = 0;
    }

    vicii.bad_line = 0;
}


static inline void next_vicii_cycle(void)
{
    /* Next cycle */
    vicii.raster_cycle++;

    if (vicii.raster_cycle == 63) {
        vicii.raster_cycle = 0;
    }
}

int vicii_cycle(void)
{
    int ba_low = 0;
    int can_sprite_sprite, can_sprite_background;

    /*VICII_DEBUG_CYCLE(("cycle: line %i, clk %i", vicii.raster_line, vicii.raster_cycle));*/

    /* perform phi2 fetch after the cpu has executed */
    vicii_fetch_sprites(vicii.cycle_flags);

    /*
     *
     * End of Phi2
     *
     ******/

    /* Next cycle */
    next_vicii_cycle();
    vicii.cycle_flags = vicii.cycle_table[vicii.raster_cycle];

    /******
     *
     * Start of Phi1
     *
     */

    /* Phi1 fetch */
    vicii.last_read_phi1 = cycle_phi1_fetch(vicii.cycle_flags);

    /* Check horizontal border flag */
    check_hborder(vicii.raster_cycle);

    can_sprite_sprite = (vicii.sprite_sprite_collisions == 0);
    can_sprite_background = (vicii.sprite_background_collisions == 0);

    /* Draw one cycle of pixels */
    vicii_draw_cycle();

    /* Trigger collision IRQs */
    if (can_sprite_sprite && vicii.sprite_sprite_collisions) {
        vicii_irq_sscoll_set();
    }
    if (can_sprite_background && vicii.sprite_background_collisions) {
        vicii_irq_sbcoll_set();
    }

    /* Stop fetch */
    if (vicii.raster_cycle == VICII_PAL_CYCLE(55)) {
        vicii.fetch_active = 0;
        vicii.prefetch_cycles = 0;
    }

    /*
     *
     * End of Phi1
     *
     ******/

    /******
     *
     * Start of Phi2
     *
     */

    /* Handle end of line/start of new line */
    if (vicii.raster_cycle == VICII_PAL_CYCLE(1)) {
        vicii_cycle_end_of_line();
        vicii_cycle_start_of_line();
    }

    if (vicii.start_of_frame) {
        if (vicii.raster_cycle == VICII_PAL_CYCLE(2)) {
            vicii_cycle_start_of_frame();
        }
    } else {
        if (vicii.raster_cycle == VICII_PAL_CYCLE(1)) {
            vicii.raster_line++;
        }
    }

    /*
     * Trigger a raster IRQ if the raster comparison goes from
     * non-match to match.
     */
    if ((vicii.raster_line == vicii.raster_irq_line)) {
        if (!vicii.raster_irq_triggered) {
            vicii_irq_raster_trigger();
            vicii.raster_irq_triggered = 1;
        }
    } else {
        vicii.raster_irq_triggered = 0;
    }

    /* Check DEN bit on first DMA line */
    if ((vicii.raster_line == VICII_FIRST_DMA_LINE) && !vicii.allow_bad_lines) {
        vicii.allow_bad_lines = (vicii.regs[0x11] & 0x10) ? 1 : 0; 
    }

    /* Check vertical border flag */
    if (vicii.raster_cycle == VICII_PAL_CYCLE(1)) {
        check_vborder(vicii.raster_line);
    }

    /* Check sprite DMA */
    if (vicii.raster_cycle == VICII_PAL_CYCLE(55)
        || vicii.raster_cycle == VICII_PAL_CYCLE(56) ) {
        check_sprite_dma();
    }
    /* Check sprite expansion flags */
    if (vicii.raster_cycle == VICII_PAL_CYCLE(56)) {
        check_exp();
    }

    /* Check sprite display */
    if (vicii.raster_cycle == VICII_PAL_CYCLE(58)) {
        check_sprite_display();
    }

    /* Update sprite mcbase */
    if (vicii.raster_cycle == VICII_PAL_CYCLE(16)) {
        sprite_mcbase_update();
    }

    /* Check badline condition, trigger fetches */
    if (vicii.allow_bad_lines) {
        check_badline();
    }

    if (vicii.raster_cycle == VICII_PAL_CYCLE(14)) {
        vicii.vc = vicii.vcbase;
        vicii.vmli = 0;
        if (vicii.bad_line) {
            vicii.rc = 0;
        }
    }

    if (vicii.raster_cycle == VICII_PAL_CYCLE(58)) {
        /* `rc' makes the chip go to idle state when it reaches the 
           maximum value.  */
        if (vicii.rc == 7) {
            vicii.idle_state = 1;
            vicii.vcbase = vicii.vc;
        }
        if (!vicii.idle_state || vicii.bad_line) {
            vicii.rc = (vicii.rc + 1) & 0x7;
            vicii.idle_state = 0;
        }
    }

    /* Matrix fetch */
    if (vicii.fetch_active && (vicii.raster_cycle >= VICII_PAL_CYCLE(15)) ) {
#ifdef DEBUG
        if (debug.maincpu_traceflg) {
            log_debug("DMA at cycle %d   %d", vicii.raster_cycle, maincpu_clk);
        }
#endif
        ba_low = 1;
        vicii_fetch_matrix();
    }

    if (vicii.prefetch_cycles) {
        ba_low = 1;
        vicii.prefetch_cycles--;
    }

    /* Check BA for Sprite Phi2 fetch */
    ba_low |= vicii_check_sprite_ba(vicii.cycle_flags);

    /* clear internal bus (may get set by a VIC-II read or write) */
    vicii.last_bus_phi2 = 0xff;

    return ba_low;
}

/* Steal cycles from CPU  */
void vicii_steal_cycles(void)
{
    int ba_low;
    interrupt_cpu_status_t *cs = maincpu_int_status;

    maincpu_ba_low_flag = 0;

    /*VICII_DEBUG_CYCLE(("steal cycles: line %i, clk %i", vicii.raster_line, vicii.raster_cycle));*/

    do {
        maincpu_clk++;
        ba_low = vicii_cycle();
    } while (ba_low);

    while (maincpu_clk >= alarm_context_next_pending_clk(maincpu_alarm_context)) {
        alarm_context_dispatch(maincpu_alarm_context, maincpu_clk);
    }

    /* SEI */
    if (OPINFO_NUMBER(*cs->last_opcode_info_ptr) == 0x78) {
        /* do not update interrupt delay counters */
        return;
    }

    /* CLI */
    if (OPINFO_NUMBER(*cs->last_opcode_info_ptr) == 0x58) {
        /* this is a hacky way of signaling CLI() that it
           shouldn't delay the interrupt */
        OPINFO_SET_ENABLES_IRQ(*cs->last_opcode_info_ptr, 1);
    }

    if (cs->irq_delay_cycles == 0 && cs->irq_clk < maincpu_clk) {
        cs->irq_delay_cycles++;
    }
    if (cs->nmi_delay_cycles == 0 && cs->nmi_clk < maincpu_clk) {
        cs->nmi_delay_cycles++;
    }
}
