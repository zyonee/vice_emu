/*
 * vic-cycle.c - Cycle based VIC-I emulation.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Daniel Kahlin <daniel@kahlin.net>
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

#include "raster.h"
#include "vic-cycle.h"
#include "vic.h"

/* ------------------------------------------------------------------------- */

/* Open vertical flipflop */
static inline void vic_cycle_open_v(void)
{
    vic.area = 1;
    vic.raster.display_ystart = vic.raster_line;
    vic.raster.geometry->gfx_position.y = vic.raster_line - vic.first_displayed_line;
    vic.raster.display_ystop = vic.screen_height - 1;
}

/* Close vertical flipflop */
static inline void vic_cycle_close_v(void)
{
    vic.area = 2;
    vic.raster.display_ystop = vic.raster_line;
}

/* Open horizontal flipflop */
static inline void vic_cycle_open_h(void)
{
    int xstart, xstop;

    xstart = MIN((unsigned int)(vic.raster_cycle * 4), vic.screen_width);
    xstop = xstart + vic.text_cols * 8;

    if (xstop >= (int)vic.screen_width) {
        xstop = vic.screen_width - 1;
        /* FIXME: SCREEN-MIXUP not handled */
    }

    xstart *= VIC_PIXEL_WIDTH;
    xstop *= VIC_PIXEL_WIDTH;

    vic.raster.display_xstart = xstart;
    vic.raster.geometry->gfx_position.x = xstart;
    vic.raster.display_xstop = xstop;

    if (xstop != xstart) {
        vic.raster.blank_this_line = 0;
        vic.fetch_state = VIC_FETCH_MATRIX;
        vic.buf_offset = 0;
    }
}

/* Close horizontal flipflop */
static inline void vic_cycle_close_h(void)
{
    vic.fetch_state = VIC_FETCH_DONE;
}

/* Handle end of line */
static inline void vic_cycle_end_of_line(void)
{
    vic.raster_cycle = 0;
    vic_raster_draw_handler();
    vic.raster_line++;
    vic.fetch_state = VIC_FETCH_IDLE;
    vic.raster.blank_this_line = 1;
}

/* Handle end of frame */
static inline void vic_cycle_end_of_frame(void)
{
    /* Close h-flipflop on end of frame */
    if (vic.area == 1) {
        vic_cycle_close_v();
    }

    vic.raster_line = 0;
    vic.area = 0;
    vic.raster.display_ystart = -1;
    vic.raster.display_ystop = -1;
}

/* Latch number of columns */
static inline void vic_cycle_latch_columns(void)
{
    int new_text_cols = MIN(vic.regs[2] & 0x7f, (int)vic.max_text_cols);
    vic.text_cols = new_text_cols;
    vic.raster.geometry->gfx_size.width = new_text_cols * 8 * VIC_PIXEL_WIDTH;
    vic.raster.geometry->text_size.width = new_text_cols;
}

/* Latch number of rows */
static inline void vic_cycle_latch_rows(void)
{
    int new_text_lines = MIN((vic.regs[3] & 0x7e) >> 1, (int)vic.max_text_cols);
    vic.text_lines = new_text_lines;
    vic.raster.geometry->gfx_size.height = new_text_lines * 8;
    vic.raster.geometry->text_size.height = new_text_lines;
}

/* ------------------------------------------------------------------------- */

void vic_cycle(void)
{
    if (vic.area == 0) {
        /* Check for vertical flipflop */
        /* TODO 1 cycle too early? */
        if (vic.regs[1] == (vic.raster_line >> 1)) {
            vic_cycle_open_v();
        }
    }

    if (vic.area == 1) {
        /* Check for horizontal flipflop */
        if (vic.fetch_state == VIC_FETCH_IDLE && (vic.regs[0] & 0x7f) == vic.raster_cycle) {
            vic_cycle_open_h();
        }
    }

    /* Latch number of rows */
    if ((vic.raster_line == 0) && (vic.raster_cycle == 2)) {
        vic_cycle_latch_rows();
    }

    /* Latch number of columns */
    if (vic.raster_cycle == 1) {
        vic_cycle_latch_columns();
    }


    /* TODO:
        - handle VIC fetches (to a line buffer)
        - update last read V-bus
    */


    /* Next cycle */
    vic.raster_cycle++;
    if (vic.raster_cycle == vic.cycles_per_line) {
        vic_cycle_end_of_line();
        if (vic.raster_line == vic.screen_height) {
            vic_cycle_end_of_frame();
        }
    }
}

