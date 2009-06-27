/*
 * vsyncarch.c - End-of-frame handling for SDL
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Dag Lem <resid@nimrod.no>
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

#include "joy.h"
#include "kbdbuf.h"
#include "raster.h"
#include "ui.h"
#include "uistatusbar.h"
#include "videoarch.h"
#include "vkbd.h"
#include "vsyncapi.h"

#include <SDL/SDL.h>

/* ------------------------------------------------------------------------- */

/* SDL_Delay & GetTicks have 1ms resolution, while VICE needs 1us */
#define VICE_SDL_TICKS_SCALE 1000

/* Number of timer units per second. */
signed long vsyncarch_frequency(void)
{
    /* Milliseconds resolution. */
    return 1000*VICE_SDL_TICKS_SCALE;
}

/* Get time in timer units. */
unsigned long vsyncarch_gettime(void)
{
    return SDL_GetTicks()*VICE_SDL_TICKS_SCALE;
}

void vsyncarch_init(void)
{
#ifdef SDL_DEBUG
fprintf(stderr,"%s\n",__func__);
#endif
}

/* Display speed (percentage) and frame rate (frames per second). */
void vsyncarch_display_speed(double speed, double frame_rate, int warp_enabled)
{
    ui_display_speed((float)speed, (float)frame_rate, warp_enabled);
}

/* Sleep a number of timer units. */
void vsyncarch_sleep(signed long delay)
{
/*fprintf(stderr,"%s: %i\n",__func__,delay);*/
    SDL_Delay(delay/VICE_SDL_TICKS_SCALE);
}

void vsyncarch_presync(void)
{
    if (sdl_vkbd_state & SDL_VKBD_ACTIVE) {
        while (sdl_vkbd_process(ui_dispatch_events()));
#ifdef HAVE_SDL_NUMJOYSTICKS
        sdl_vkbd_process(sdljoy_autorepeat());
#endif
    } else {
        ui_dispatch_events();
    }

    if ((sdl_vkbd_state & SDL_VKBD_REPAINT) || (uistatusbar_state & UISTATUSBAR_REPAINT)) {
        raster_force_repaint(sdl_active_canvas->parent_raster);
        sdl_vkbd_state &= ~SDL_VKBD_REPAINT;
        uistatusbar_state &= ~UISTATUSBAR_REPAINT;
    }

    kbdbuf_flush();
}

void vsyncarch_postsync(void)
{
}

