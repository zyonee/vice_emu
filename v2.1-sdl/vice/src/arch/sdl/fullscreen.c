/*
 * fullscreen.c
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Andreas Boose <viceteam@t-online.de>
 *  Martin Pottendorfer <pottendo@utanet.at>
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
#include <string.h>
#include <SDL/SDL.h>

#include "fullscreen.h"
#include "fullscreenarch.h"
#include "lib.h"
#include "video.h"
#include "videoarch.h"

int fullscreen_is_enabled;

int fullscreen_available(void) 
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

void fullscreen_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

void fullscreen_suspend(int level)
{
fprintf(stderr,"%s: %i\n",__func__,level);
}

void fullscreen_resume(void)
{
fprintf(stderr,"%s\n",__func__);
}

void fullscreen_set_mouse_timeout(void)
{
fprintf(stderr,"%s\n",__func__);
}

void fullscreen_mode_callback(const char *device, void *callback)
{
fprintf(stderr,"%s: %s\n",__func__,device);
/*
#ifdef USE_XF86_VIDMODE_EXT
    if (strcmp(STR_VIDMODE, device) == 0)
        vidmode_mode_callback(callback);
#endif
*/
}

void fullscreen_menu_create(struct ui_menu_entry_s *menu)
{
fprintf(stderr,"%s\n",__func__);
}

void fullscreen_menu_shutdown(struct ui_menu_entry_s *menu)
{
fprintf(stderr,"%s\n",__func__);
}

int fullscreen_init(void)
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

int fullscreen_init_alloc_hooks(struct video_canvas_s *canvas)
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

void fullscreen_shutdown_alloc_hooks(struct video_canvas_s *canvas)
{
fprintf(stderr,"%s\n",__func__);
}

static int fullscreen_statusbar(struct video_canvas_s *canvas, int enable)
{
fprintf(stderr,"%s: %i\n",__func__,enable);
return 0;
}

static int fullscreen_enable(struct video_canvas_s *canvas, int enable)
{
fprintf(stderr,"%s: %i\n",__func__,enable);
    if (canvas->fullscreenconfig->device == NULL)
	return 0;

    fullscreen_is_enabled = canvas->fullscreenconfig->enable = enable;

    if(enable) {
        SDL_ShowCursor(SDL_DISABLE);
    } else {
        SDL_ShowCursor(SDL_ENABLE);
    }
    video_viewport_resize(canvas);
    return 0;
}

static int fullscreen_double_size(struct video_canvas_s *canvas,
                                  int double_size)
{
fprintf(stderr,"%s: %i\n",__func__,double_size);
    canvas->fullscreenconfig->double_size = double_size;
    return 0;
}

static int fullscreen_double_scan(struct video_canvas_s *canvas,
                                  int double_scan)
{
fprintf(stderr,"%s: %i\n",__func__,double_scan);
    canvas->fullscreenconfig->double_scan = double_scan;
    return 0;
}

static int fullscreen_device(struct video_canvas_s *canvas, const char *device)
{
fprintf(stderr,"%s: %s\n",__func__,device);
    if (strcmp("SDL", device) != 0)
        return -1;

    if (canvas->fullscreenconfig->device)
	lib_free(canvas->fullscreenconfig->device);
    
    canvas->fullscreenconfig->device = lib_stralloc(device);

    return 0;
}

static int fullscreen_mode_sdl(struct video_canvas_s *canvas, int mode)
{
fprintf(stderr,"%s: %i\n",__func__,mode);
    return 0;
}

void fullscreen_capability(cap_fullscreen_t *cap_fullscreen)
{
fprintf(stderr,"%s\n",__func__);
    cap_fullscreen->device_num = 0;
    cap_fullscreen->device_name[cap_fullscreen->device_num] = "SDL";
    cap_fullscreen->enable = fullscreen_enable;
    cap_fullscreen->statusbar = fullscreen_statusbar;
    cap_fullscreen->double_size = fullscreen_double_size;
    cap_fullscreen->double_scan = fullscreen_double_scan;
    cap_fullscreen->device = fullscreen_device;
    cap_fullscreen->mode[cap_fullscreen->device_num] = fullscreen_mode_sdl;
    cap_fullscreen->device_num += 1;
}

