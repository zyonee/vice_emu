/*
 * vicii-cycle.c - Cycle based VIC-II emulation.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#include "log.h"
#include "mainc64cpu.h"
#include "types.h"
#include "vicii-cycle.h"
#include "vicii-fetch.h"
#include "vicii-irq.h"
#include "vicii.h"
#include "viciitypes.h"

static inline void check_badline(void)
{
    raster_t *raster;
    raster = &vicii.raster;

    if ((vicii.raster_line & 7) == (unsigned int)raster->ysmooth
        && vicii.allow_bad_lines
        && vicii.raster_line >= vicii.first_dma_line
        && vicii.raster_line <= vicii.last_dma_line) {
        vicii.bad_line = 1;
        vicii.idle_state = 0;
    } else {
        vicii.bad_line = 0;
    }

    if (vicii.bad_line && !vicii.fetch_active && (vicii.raster_cycle >= 11) && (vicii.raster_cycle <= 53)) {
        /*VICII_DEBUG_CYCLE(("fetch start: line %i, clk %i, memptr %04x, counter %04x, y %i", vicii.raster_line, vicii.raster_cycle, vicii.memptr, vicii.mem_counter, vicii.raster.ycounter));*/
        vicii.fetch_active = 1;
 
        raster->draw_idle_state = 0;
        vicii.force_display_state = 1;
        
        vicii.idle_data_location = IDLE_NONE;
        vicii.ycounter_reset_checked = 1;
        vicii.memory_fetch_done = 2;
                
        vicii.prefetch_cycles = 3;
    }
}

static inline BYTE cycle_phi1_fetch(unsigned int cycle)
{
    BYTE data;

    switch (cycle) {
        /* Sprite pointers */
        case 58:
        case 60:
        case 62:
        case 64:
        case 1:
        case 3:
        case 5:
        case 7:
            data = vicii_fetch_sprite_pointer((cycle >= 58) ? ((cycle - 58) >> 1) : (4 + (cycle >> 1)));
            break;

        /* Sprite DMA */
        case 59:
        case 61:
        case 63:
        case 0:
        case 2:
        case 4:
        case 6:
        case 8:
            data = vicii_fetch_sprite_dma_1((cycle >= 59) ? ((cycle - 59) >> 1) : (3 + (cycle >> 1)));
            break;

        /* Refresh */
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            data = vicii_fetch_refresh();
            break;

        /* Idle */
        case 54:
        case 55:
        case 56:
        case 57:
            data = vicii_fetch_idle();
            break;

        /* Graphics fetch */
        default: /* 14 .. 53 */
            if (!vicii.idle_state) {
                data = vicii_fetch_graphics();
            } else {
                data = vicii_fetch_idle_gfx();
            }
            break;
    }

    return data;
}

int vicii_cycle(void)
{
    int ba_low = 0;

    /*VICII_DEBUG_CYCLE(("cycle: line %i, clk %i", vicii.raster_line, vicii.raster_cycle));*/

    /* Phi1 fetch */
    vicii.last_read_phi1 = cycle_phi1_fetch(vicii.raster_cycle);

    /* Stop fetch */
    if (vicii.raster_cycle == 53) {
        vicii.fetch_active = 0;
        vicii.buf_offset = 0;
        vicii.gbuf_offset = 0;
    }

    /* Next cycle */
    vicii.raster_cycle++;

    /* Handle the "hole" on PAL systems at cycles 54-55 */
    if ((vicii.raster_cycle == 54) && (vicii.cycles_per_line == 63)) {
        vicii.raster_cycle = 56;
    }
    /* Handle the "hole" on NTSC-old systems at cycle 54 */
    if ((vicii.raster_cycle == 54) && (vicii.cycles_per_line == 64)) {
        vicii.raster_cycle = 55;
    }

    /* Handle end of line */
    if (vicii.raster_cycle == 65) {
        vicii.raster_cycle = 0;
        vicii_raster_draw_alarm_handler(maincpu_clk, 0);
        vicii.raster_line++;
        if (vicii.raster_line == vicii.screen_height) {
            vicii.raster_line = 0;
            vicii.refresh_counter = 0xff;
        }
        if (vicii.raster_line == vicii.raster_irq_line) {
            vicii_irq_alarm_handler(maincpu_clk, 0);
        }
        vicii.buf_offset = 0;
        vicii.gbuf_offset = 0;
        vicii.bad_line = 0;
    }

    /* Check badline condition, trigger fetches */
    check_badline();

    if (vicii.raster_cycle == 13) {
        vicii.mem_counter = vicii.memptr;
        if (vicii.bad_line) {
            vicii.raster.ycounter = 0;
        }
    }

    if (vicii.raster_cycle == 57) {
        /* `ycounter' makes the chip go to idle state when it reaches the 
           maximum value.  */
        if (vicii.raster.ycounter == 7) { 
            vicii.idle_state = 1; 
            vicii.memptr = vicii.mem_counter; 
        } 
        if (!vicii.idle_state || vicii.bad_line) { 
            vicii.raster.ycounter = (vicii.raster.ycounter + 1) & 0x7; 
            vicii.idle_state = 0; 
        } 
    }

    /* Matrix fetch */
    if (vicii.fetch_active && (vicii.raster_cycle >= 14)) {
        ba_low = 1;
        vicii_fetch_matrix();
    }

    if (vicii.prefetch_cycles) {
        ba_low = 1;
        vicii.prefetch_cycles--;
    }

    /* Sprite Phi2 fetch */
    ba_low |= vicii_fetch_sprites(vicii.raster_cycle);

    return ba_low;
}

/* Steal cycles from CPU  */
void vicii_steal_cycles(void)
{
    int ba_low;

    maincpu_ba_low_flag = 0;

    /*VICII_DEBUG_CYCLE(("steal cycles: line %i, clk %i", vicii.raster_line, vicii.raster_cycle));*/

    do {
        maincpu_clk++;
        ba_low = vicii_cycle();
    } while (ba_low);
}
