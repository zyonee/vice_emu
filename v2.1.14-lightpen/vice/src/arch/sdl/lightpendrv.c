/*
 * lightpendrv.c - Lightpen driver for SDL UI.
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
#include "types.h"

#include <SDL/SDL.h>

#include "lightpen.h"
#include "lightpendrv.h"
#include "videoarch.h"

/* ------------------------------------------------------------------ */
/* static functions */


/* ------------------------------------------------------------------ */
/* External interface */

void sdl_lightpen_update(void)
{
    int x, y;
    Uint8 buttons;

    if (!lightpen_enabled) {
        return;
    }

    if (SDL_GetAppState() & SDL_APPMOUSEFOCUS) {
        buttons = SDL_GetMouseState(&x, &y);
    } else {
        x = y = -1;
        buttons = 0;
    }

#ifdef SDL_DEBUG
fprintf(stderr,"%s: x = %i, y = %i, buttons = %02x\n",__func__, x, y, buttons);
#endif

    if (x >= 0) {
        /* TODO hwscale, resized, ... */
        if (sdl_active_canvas->videoconfig->doublesizex) {
            x /= 2;
        }

        if (sdl_active_canvas->videoconfig->doublesizey) {
            y /= 2;
        }
    }

    lightpen_update(sdl_active_canvas_num, x, y, (int)buttons);
}
