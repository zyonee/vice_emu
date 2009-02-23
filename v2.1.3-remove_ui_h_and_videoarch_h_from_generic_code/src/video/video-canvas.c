/*
 * video-canvas.c
 *
 * Written by
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

#include <stdio.h>
#include <stdlib.h>

#include "lib.h"
#include "machine.h"
#include "types.h"
#include "video-canvas.h"
#include "video-color.h"
#include "video.h"
#include "raster.h"
#include "viewport.h"


/* Temporary! */
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif


void video_canvas_refresh_all(raster_t *canvas)
{
    viewport_t *viewport;
    geometry_t *geometry;

    if (console_mode || vsid_mode) {
        return;
    }

    viewport = canvas->viewport;
    geometry = canvas->geometry;

    video_canvas_refresh(canvas->canvas,
                 viewport->first_x
                 + geometry->extra_offscreen_border_left,
                 viewport->first_line,
                 viewport->x_offset,
                 viewport->y_offset,
                 MIN(canvas->canvas_width,
                     geometry->screen_size.width - viewport->first_x),
                 MIN(canvas->canvas_height,
                     viewport->last_line - viewport->first_line + 1),
                 canvas->videoconfig->doublesizex,
                 canvas->videoconfig->doublesizey);
}

void video_canvas_redraw_size(raster_t *canvas, unsigned int width,
                              unsigned int height)
{
    if (canvas->videoconfig->doublesizex)
        width /= 2;
    if (canvas->videoconfig->doublesizey)
        height /= 2;

    if (width != canvas->canvas_width
        || height != canvas->canvas_height) {
        canvas->canvas_width = width;
        canvas->canvas_height = height;
        video_viewport_resize(canvas->canvas,
            canvas->geometry,
            canvas->viewport,
            canvas->canvas_width,
            canvas->canvas_height,
            canvas->videoconfig->doublesizex,
            canvas->videoconfig->doublesizey);
    }
    video_canvas_refresh_all(canvas);
}

int video_canvas_palette_set(raster_t *canvas,
                             struct palette_s *palette)
{
    struct palette_s *old_palette;

    if (palette == NULL)
        return 0;

    old_palette = canvas->palette;

    if (video_canvas_set_palette(palette, canvas->videoconfig->color_tables.physical_colors) < 0)
        return -1;
    else {
        canvas->palette = palette;
    }

    if (old_palette != NULL)
        video_color_palette_free(old_palette);

    video_canvas_refresh_all(canvas);

    return 0;
}

