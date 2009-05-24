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
#include "viciidtv.h"
#include "viciidtvtypes.h"

extern BYTE dtvrewind;

/* Memory access cycle 1:
    Character fetch/Counter A */
void viciidtv_cycle_1(void)
{
    static int drift_debug = 1;
    /*VICII_DEBUG_CYCLE(("cycle 1: line %i, clk %i, draw %i", vicii.raster_line, vicii.raster_cycle, vicii.draw_cycle));*/

    if (drift_debug) {
        if (vicii.raster_cycle != VICIIDTV_RASTER_CYCLE(maincpu_clk)) {
            VICII_DEBUG_CYCLE(("cycle 1: line %i, clk %i != old_clk %i, rewind %i", vicii.raster_line, vicii.raster_cycle, VICIIDTV_RASTER_CYCLE(maincpu_clk), dtvrewind));
            vicii.raster_cycle = VICIIDTV_RASTER_CYCLE(maincpu_clk);
        }
    }
}

/* Memory access cycle 2:
    Color/sprite fetch/Blitter/DMA
  Returns 1 if access was done (BA low) */
int viciidtv_cycle_2(void)
{
    return 0;
}

/* Memory access cycle 3:
    Graphic fetch/Counter B */
void viciidtv_cycle_3(void)
{
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

    if (vicii.regs[0x1a] & 1) {
        if (vicii.raster_line == vicii.raster_irq_line) {
            if (vicii.raster_cycle == vicii.raster_irq_offset) {
                vicii_irq_alarm_handler(maincpu_clk, 0);
            }
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

    VICII_DEBUG_CYCLE(("steal cycles: line %i, clk %i", vicii.raster_line, vicii.raster_cycle));

    do {
        maincpu_clk++;
        viciidtv_cycle_1();
        ba_low = viciidtv_cycle_2();
        viciidtv_cycle_3();
    } while (ba_low);
}
