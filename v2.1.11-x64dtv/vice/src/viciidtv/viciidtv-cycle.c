/*
 * viciidtv-cycle.c - VIC-II DTV emulation subcycle handling.
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
#include "maindtvcpu.h"
#include "types.h"
#include "viciidtv-fetch.h"
#include "viciidtv-irq.h"
#include "viciidtv.h"
#include "viciidtvtypes.h"

extern BYTE dtvrewind;

/* Memory access cycle 1:
    Character fetch/Counter A
   Memory access cycle 2:
    Color/sprite fetch/Blitter/DMA
  Returns 1 if access on cycle 2 was done (BA low) */
int viciidtv_cycle_1_2(void)
{
    static int drift_debug = 1;
    int ba_low = 0;

    /*VICII_DEBUG_CYCLE(("cycle 1: line %i, clk %i", vicii.raster_line, vicii.raster_cycle));*/

    if (drift_debug) {
        if (vicii.raster_cycle != VICIIDTV_RASTER_CYCLE(maincpu_clk)) {
            VICII_DEBUG_CYCLE(("cycle 1: line %i, clk %i != old_clk %i, rewind %i", vicii.raster_line, vicii.raster_cycle, VICIIDTV_RASTER_CYCLE(maincpu_clk), dtvrewind));
            vicii.raster_cycle = VICIIDTV_RASTER_CYCLE(maincpu_clk);
        }
    }

    switch (vicii.raster_cycle) {
        /*case 8:*/ /* VICII_FETCH_CYCLE (overscan) */
        case 11: /* VICII_FETCH_CYCLE */
            viciidtv_fetch_start();
            break;
        case 56: /* sprite 0 trigger */
        case 58: /* sprite 1 trigger */
        case 59: /* sprite 0 pointer */
        case 60: /* sprite 0 data */
        case 61: /* sprite 1 pointer */
        case 62: /* sprite 1 data */
        case 63: /* sprite 2 pointer */
        case 64: /* sprite 2 data */
        case 0: /* sprite 3 pointer */
        case 1: /* sprite 3 data */
        case 2: /* sprite 4 pointer */
        case 3: /* sprite 4 data */
        case 4: /* sprite 5 pointer */
        case 5: /* sprite 5 data */
        case 6: /* sprite 6 pointer */
        case 7: /* sprite 6 data */
        case 8: /* sprite 7 pointer */
        case 9: /* sprite 7 data */
        default:
            break;
    }

    if ((vicii.fetch_mode != VICIIDTV_FETCH_NORMAL)
        && (!vicii.idle_state)) {
        viciidtv_fetch_linear_a();
    }

    if (vicii.bad_line) {
        ba_low |= viciidtv_fetch_matrix();
    }

    return ba_low;
}

/* Memory access cycle 3:
    Graphic fetch/Counter B */
void viciidtv_cycle_3(void)
{
    if ((!vicii.idle_state) 
       && (vicii.raster_cycle >= 11)
       && (vicii.raster_cycle <= 53)) {
        viciidtv_fetch_graphics();
    }

    if (vicii.raster_cycle == 53) {
        viciidtv_fetch_stop();
    }

    /* Next cycle */
    vicii.raster_cycle++;

    /* Handle the "hole" on PAL systems at cycles 54-55 */
    if ((vicii.raster_cycle == 54) && (vicii.cycles_per_line == 63)) {
        vicii.raster_cycle = 56;
    }

    if (vicii.raster_cycle == 65) {
        vicii_raster_draw_alarm_handler(maincpu_clk, 0);
        vicii.raster_cycle = 0;
        vicii.raster_line++;
        if (vicii.raster_line == vicii.screen_height) {
            vicii.raster_line = 0;
        }
    }

    if (vicii.raster_line == vicii.raster_irq_line) {
        if (vicii.raster_cycle == vicii.raster_irq_offset) {
            vicii_irq_alarm_handler(maincpu_clk, 0);
        }
    }
}

/* Badline (emulation) enabled */
int viciidtv_badline_enabled(void)
{
    return !vicii.badline_disable;
}

/* Steal cycles from CPU  */
void viciidtv_steal_cycles(void)
{
    int ba_low;

    maincpu_ba_low_flag = 0;

    /*VICII_DEBUG_CYCLE(("steal cycles: line %i, clk %i", vicii.raster_line, vicii.raster_cycle));*/

    do {
        maincpu_clk++;
        ba_low = viciidtv_cycle_1_2();
        viciidtv_cycle_3();
    } while (ba_low);
}
