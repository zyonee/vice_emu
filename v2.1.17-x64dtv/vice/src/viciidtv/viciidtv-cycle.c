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

#include "c64dtvdma.h"
#include "c64dtvblitter.h"
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
    static int drift_debug = 0;
    int ba_low = 0;

    /*VICII_DEBUG_CYCLE(("cycle 1: line %i, clk %i", vicii.raster_line, vicii.raster_cycle));*/

    if (drift_debug) {
        if (vicii.raster_cycle != VICIIDTV_RASTER_CYCLE(maincpu_clk)) {
            VICII_DEBUG_CYCLE(("cycle 1: line %i, clk %i != old_clk %i, rewind %i", vicii.raster_line, vicii.raster_cycle, VICIIDTV_RASTER_CYCLE(maincpu_clk), dtvrewind));
            vicii.raster_cycle = VICIIDTV_RASTER_CYCLE(maincpu_clk);
        }
    }

    /* Next cycle */
    vicii.raster_cycle++;

    /* Handle the "hole" on PAL systems at cycles 54-55 */
    if ((vicii.raster_cycle == 54) && (vicii.cycles_per_line == 63)) {
        vicii.raster_cycle = 56;
    }

    if (vicii.raster_cycle == 65) {
        vicii.raster_cycle = 0;
        vicii_raster_draw_alarm_handler(maincpu_clk, 0);
    }

    if (vicii.raster_cycle == 64) {
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

    /* Blitter/DMA on vblank */
    if ((vicii.raster_line == (vicii.screen_height - 1)) && (vicii.raster_cycle == 10)) {
        /* Scheduled Blitter */
        if (blitter_on_irq & 0x40) {
            c64dtvblitter_trigger_blitter();
        }
        /* Scheduled DMA */
        if (dma_on_irq & 0x40) {
            c64dtvdma_trigger_dma();
        }
    }

    switch (vicii.raster_cycle) {
        /*case 8:*/ /* VICII_FETCH_CYCLE (overscan) */
        case 11: /* VICII_FETCH_CYCLE */
            viciidtv_fetch_start();
            break;
        default:
            break;
    }

    if (vicii.prefetch_cycles) {
        ba_low = !vicii.colorfetch_disable;
    } else if (vicii.fetch_active) {
        if (vicii.fetch_mode != VICIIDTV_FETCH_NORMAL) {
            viciidtv_fetch_linear_a();
        }
        if (vicii.bad_line) {
            ba_low |= viciidtv_fetch_matrix();
        }
    }

    ba_low |= viciidtv_fetch_sprites(vicii.raster_cycle);

    return ba_low;
}

/* Memory access cycle 3:
    Graphic fetch/Counter B */
void viciidtv_cycle_3(void)
{
    if (vicii.prefetch_cycles) {
        vicii.prefetch_cycles--;
    }

    if ((!vicii.idle_state) && (vicii.raster_cycle >= 14) && (vicii.raster_cycle <= 53)) {
        viciidtv_fetch_graphics();
    }

    if (vicii.raster_cycle == 53) {
        viciidtv_fetch_stop();
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
