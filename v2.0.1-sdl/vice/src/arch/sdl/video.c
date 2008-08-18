/*
 * video.c - SDL video
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

#include <stdio.h>
#include <SDL/SDL.h>

#include "log.h"
#include "palette.h"
#include "videoarch.h"

static log_t sdlvideo_log = LOG_ERR;

int video_init(void)
{
    sdlvideo_log = log_open("SDLVideo");
	return 0;
}

int video_init_cmdline_options(void)
{
	return 0;
}

void video_shutdown(void)
{
}

video_canvas_t *video_canvas_create(video_canvas_t *canvas, 
		unsigned int *width, unsigned int *height, int mapped)
{
    SDL_Surface *new_screen;
    unsigned int new_width, new_height;


    new_width = *width;
    new_height = *height;
 
    if (canvas->videoconfig->doublesizex)
        new_width *= 2;
 
    if (canvas->videoconfig->doublesizey)
        new_height *= 2;

/*fprintf(stderr,"%s: %ix%i,%i (%08x)\n",__func__,new_width,new_height,mapped,(unsigned int)canvas);*/
 
    new_screen = SDL_SetVideoMode(new_width, new_height, 0, SDL_SWSURFACE);
    if(!new_screen) {
        log_error(sdlvideo_log, "SDL_SetVideoMode failed!");
        return NULL;
    }

    canvas->depth = new_screen->format->BitsPerPixel;
    canvas->width = new_width;
    canvas->height = new_height;
    canvas->screen = new_screen;

    log_message(sdlvideo_log, "%ix%i %ibpp", new_width, new_height, canvas->depth);

    video_canvas_set_palette(canvas, canvas->palette);

    return canvas;
}

void video_canvas_destroy(struct video_canvas_s *canvas)
{
fprintf(stderr,"%s: (%08x)\n",__func__,(unsigned int)canvas);
}

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
fprintf(stderr,"%s: (%08x)\n",__func__,(unsigned int)canvas);
	canvas->video_draw_buffer_callback=NULL;
}

void video_canvas_refresh(struct video_canvas_s *canvas,
                          unsigned int xs, unsigned int ys,
                          unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h)
{
    if (canvas->videoconfig->doublesizex) {
        xi *= 2;
        w *= 2;
    }

    if (canvas->videoconfig->doublesizey) {
        yi *= 2;
        h *= 2;
    }

    if (SDL_MUSTLOCK(canvas->screen)) {
        if (SDL_LockSurface(canvas->screen) < 0) {
            return;
        }
    }

    video_canvas_render(canvas, (BYTE *)canvas->screen->pixels,
                        w, h, xs, ys, xi, yi,
                        canvas->screen->pitch,
                        canvas->screen->format->BitsPerPixel);

    if (SDL_MUSTLOCK(canvas->screen)) {
        SDL_UnlockSurface(canvas->screen);
    }
    SDL_UpdateRect(canvas->screen, xi, yi, w, h);
}

int video_canvas_set_palette(struct video_canvas_s *canvas,
                             struct palette_s *palette)
{
    unsigned int i, col;
    SDL_PixelFormat *fmt;
    SDL_Color colors[256];

/*fprintf(stderr,"%s: (%08x)\n",__func__,(unsigned int)canvas);*/

    canvas->palette = palette;

    fmt = canvas->screen->format;

    for (i = 0; i < palette->num_entries; i++) {
        if (canvas->depth == 8) {
            colors[i].r = palette->entries[i].red;
            colors[i].b = palette->entries[i].blue;
            colors[i].g = palette->entries[i].green;
            col = i;
        } else {
            col = SDL_MapRGB(fmt, palette->entries[i].red, palette->entries[i].green, palette->entries[i].blue);
        }
        video_render_setphysicalcolor(canvas->videoconfig, i, col, canvas->depth);
    }

    if (canvas->depth == 8) {
        SDL_SetColors(canvas->screen, colors, 0, palette->num_entries);
    } else {
        for (i = 0; i < 256; i++) {
            video_render_setrawrgb(i, SDL_MapRGB(fmt, i, 0, 0), SDL_MapRGB(fmt, 0, i, 0), SDL_MapRGB(fmt, 0, 0, i));
        }
        video_render_initraw();
    }

    return 0;
}

int video_arch_resources_init(void)
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

void video_canvas_resize(struct video_canvas_s *canvas,
                                unsigned int width, unsigned int height)
{
fprintf(stderr,"%s: %i,%i (%08x)\n",__func__,width,height,(unsigned int)canvas);
}

void video_arch_resources_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

void video_add_handlers(void)
{
fprintf(stderr,"%s\n",__func__);
}

void fullscreen_capability(void)
{
fprintf(stderr,"%s\n",__func__);
}

