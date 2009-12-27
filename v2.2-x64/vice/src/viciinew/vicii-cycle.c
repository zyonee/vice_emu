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

int vicii_cycle(void)
{
    int ba_low = 0;

    /*VICII_DEBUG_CYCLE(("cycle: line %i, clk %i", vicii.raster_line, vicii.raster_cycle));*/

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

    if (vicii.raster_cycle == 65) {
        vicii.raster_cycle = 0;
        vicii_raster_draw_alarm_handler(maincpu_clk, 0);
        vicii.raster_line++;
        if (vicii.raster_line == vicii.screen_height) {
            vicii.raster_line = 0;
        }
        if (vicii.raster_line == vicii.raster_irq_line) {
            vicii_irq_alarm_handler(maincpu_clk, 0);
        }
    }

    switch (vicii.raster_cycle) {
        case 11: /* VICII_FETCH_CYCLE */
            vicii_fetch_start();
            break;
        default:
            break;
    }

    /* Matrix fetch */
    if (vicii.prefetch_cycles) {
        ba_low = 1;
    } else if (vicii.fetch_active) {
        if (vicii.bad_line) {
            ba_low |= vicii_fetch_matrix();
        }
    }

    /* Sprite fetch */
    ba_low |= vicii_fetch_sprites(vicii.raster_cycle);

    /* Graphics fetch */
    if (vicii.prefetch_cycles) {
        vicii.prefetch_cycles--;
    }

    if ((!vicii.idle_state) && (vicii.raster_cycle >= 14) && (vicii.raster_cycle <= 53)) {
        vicii_fetch_graphics();
    }

    if (vicii.raster_cycle == 53) {
        vicii_fetch_stop();
    }

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
