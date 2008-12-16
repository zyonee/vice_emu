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

#include "cmdline.h"
#include "fullscreen.h"
#include "fullscreenarch.h"
#include "lib.h"
#include "log.h"
#include "palette.h"
#include "resources.h"
#include "translate.h"
#include "videoarch.h"
#include "vkbd.h"

static log_t sdlvideo_log = LOG_ERR;

static int sdl_bitdepth;

#define MAX_CANVAS_NUM 2           
static int sdl_num_screens = 0;
static int sdl_active_canvas_num = 0;
static video_canvas_t *sdl_canvaslist[MAX_CANVAS_NUM];
video_canvas_t *sdl_active_canvas = NULL;

/* ------------------------------------------------------------------------- */

/* Video-related resources.  */

static int set_sdl_bitdepth(int d, void *param)
{
    if ((d == 0) || (d == 8) || (d == 15) || (d == 16) || (d == 32)) {
        if (sdl_bitdepth == d) {
            return 0;
        }
        sdl_bitdepth = d;
        /* update */
        return 0;
    }
    return -1;
}

static const resource_string_t resources_string[] = {
    { NULL }
};

static const resource_int_t resources_int[] = {
    { "SDLBitdepth", 0, RES_EVENT_NO, NULL,
      &sdl_bitdepth, set_sdl_bitdepth, NULL },
    { NULL }
};

int video_arch_resources_init(void)
{
fprintf(stderr,"%s\n",__func__);
    if (resources_register_string(resources_string) < 0)
        return -1;

    return resources_register_int(resources_int);
}

void video_arch_resources_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

/* ------------------------------------------------------------------------- */

/* Video-related command-line options.  */
static const cmdline_option_t cmdline_options[] = {
    { "-sdlbitdepth", SET_RESOURCE, 1, NULL, NULL, "SDLBitdepth", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<bpp>", "Set bitdepth (0 = current, 8, 15, 16, 24, 32)" },
    { NULL }
};

int video_init_cmdline_options(void)
{
fprintf(stderr,"%s\n",__func__);
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

int video_init(void)
{
    sdlvideo_log = log_open("SDLVideo");
	return 0;
}

void video_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
    sdl_active_canvas = NULL;
}

/* ------------------------------------------------------------------------- */

video_canvas_t *video_canvas_create(video_canvas_t *canvas, 
		unsigned int *width, unsigned int *height, int mapped)
{
    SDL_Surface *new_screen;
    unsigned int new_width, new_height;
    int flags;

    flags = SDL_SWSURFACE;

    new_width = *width;
    new_height = *height;

    if (fullscreen_is_enabled) {
        flags = SDL_FULLSCREEN | SDL_HWSURFACE;
    }

    if (canvas->videoconfig->doublesizex) {
        new_width *= 2;
    }

    if (canvas->videoconfig->doublesizey) {
        new_height *= 2;
    }

/*fprintf(stderr,"%s: %ix%i,%i (%08x)\n",__func__,new_width,new_height,mapped,(unsigned int)canvas);*/

    if (canvas == sdl_active_canvas) {
        new_screen = SDL_SetVideoMode(new_width, new_height, sdl_bitdepth, flags);
    } else {
        if (canvas->screen) {
            SDL_FreeSurface(canvas->screen);
        }
        new_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, new_width, new_height, sdl_bitdepth, 0, 0, 0, 0);
    }

    if (!new_screen) {
        log_error(sdlvideo_log, "SDL_SetVideoMode failed!");
        return NULL;
    }
    sdl_bitdepth = new_screen->format->BitsPerPixel;

    canvas->depth = sdl_bitdepth;
    canvas->width = new_width;
    canvas->height = new_height;
    canvas->screen = new_screen;

    log_message(sdlvideo_log, "%ix%i %ibpp %s", new_width, new_height, sdl_bitdepth, fullscreen_is_enabled?"(fullscreen)":"");

    video_canvas_set_palette(canvas, canvas->palette);

    return canvas;
}

void video_canvas_refresh(struct video_canvas_s *canvas,
                          unsigned int xs, unsigned int ys,
                          unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h)
{
    if (sdl_vkbd_state) {
        sdl_vkbd_draw();
    }

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

void video_canvas_resize(struct video_canvas_s *canvas,
                         unsigned int width, unsigned int height)
{
fprintf(stderr,"%s: %i,%i (%08x)\n",__func__,width,height,(unsigned int)canvas);
    video_canvas_create(canvas, &width, &height, 0);
}

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
fprintf(stderr,"%s: (%08x)\n",__func__,(unsigned int)canvas);

    if (sdl_num_screens == MAX_CANVAS_NUM) {
        log_error(sdlvideo_log,"Too many canvases!");
        exit(-1);
    }

	canvas->video_draw_buffer_callback=NULL;

    canvas->fullscreenconfig
        = (fullscreenconfig_t *)lib_calloc(1, sizeof(fullscreenconfig_t));
    fullscreen_init_alloc_hooks(canvas);

    if (sdl_active_canvas_num == sdl_num_screens) {
        sdl_active_canvas = canvas;
    }

    sdl_canvaslist[sdl_num_screens++] = canvas;

    canvas->screen = NULL;
}

void video_canvas_destroy(struct video_canvas_s *canvas)
{
    int i;
fprintf(stderr,"%s: (%08x)\n",__func__,(unsigned int)canvas);

    for (i=0; i<sdl_num_screens; ++i) {
        if ((sdl_canvaslist[i] == canvas) && (i != sdl_active_canvas_num)) {
            SDL_FreeSurface(sdl_canvaslist[i]->screen);
            sdl_canvaslist[i]->screen = NULL;
        }
    }
}

void video_add_handlers(void)
{
fprintf(stderr,"%s\n",__func__);
}

