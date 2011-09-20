/*
 * video.c - Common video functions for Win32
 *
 * Written by
 *  Tibor Biczo <crown@matavnet.hu>
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "cmdline.h"
#include "fullscrn.h"
#include "lib.h"
#include "log.h"
#include "palette.h"
#include "res.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "video.h"
#include "video-dx9.h"
#include "video-gdi.h"
#include "videoarch.h"

#include "viewport.h"
#include "raster.h"

static int video_number_of_canvases;
static raster_t *video_canvases[2];
#ifdef HAVE_D3D9_H
static int dx9_available;
#endif

/* ------------------------------------------------------------------------ */
/* Video-related resources.  */

/* Flag: are we in fullscreen mode?  */
int fullscreen_enabled;
int dx_primary_surface_rendering;
int dx9_disable;

static int set_dx_primary_surface_rendering(int val, void *param)
{
    dx_primary_surface_rendering = val;

#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        int i;
        for (i = 0; i < video_number_of_canvases; i++) {
            video_canvas_reset_dx9(video_canvases[i]->canvas);
        }
    }
#endif

    return 0;
}

#ifdef HAVE_D3D9_H
static int set_dx9_disable(int val, void *param)
{
    int i;
    int old_dx9_disable, old_num_of_canvases;
    int old_width[2], old_height[2];

    if (!dx9_available) {
        return 0;
    }

    old_dx9_disable = dx9_disable;
    old_num_of_canvases = video_number_of_canvases;

    if (old_dx9_disable != val) {
        for (i = 0; i < video_number_of_canvases; i++) {       
            old_width[i] = video_canvases[i]->canvas->width;
            old_height[i] = video_canvases[i]->canvas->height;
            if (old_dx9_disable) {
                /* Anything to do here?? */
            } else {
                video_device_release_dx9(video_canvases[i]->canvas);
            }
        }

        dx9_disable = val;
        video_number_of_canvases = 0;

        for (i = 0; i < old_num_of_canvases; i++) {
            if (old_dx9_disable) {
                video_canvas_create_dx9(video_canvases[i], &old_width[i], &old_height[i]);
            } else {
                video_canvas_create_gdi(video_canvases[i], &old_width[i], &old_height[i]);
            }
            ui_canvas_child_window(video_canvases[i]->canvas, old_dx9_disable);
            ui_resize_canvas_window(video_canvases[i]->canvas, video_canvases[i]->geometry->pixel_aspect_ratio);
        }
        
        fullscreen_getmodes();
    }

    return 0;
}
#endif

static const resource_int_t resources_int[] = {
    { "DXPrimarySurfaceRendering", 0, RES_EVENT_NO, NULL,
      &dx_primary_surface_rendering, set_dx_primary_surface_rendering, NULL },
#ifdef HAVE_D3D9_H
    { "DX9Disable", 0, RES_EVENT_NO, NULL,
      &dx9_disable, set_dx9_disable, NULL },
#endif
    { NULL }
};

int video_arch_resources_init(void)
{
    return resources_register_int(resources_int);
}

void video_arch_resources_shutdown(void)
{
}

/* ------------------------------------------------------------------------ */

/* Video-related command-line options.  */

static const cmdline_option_t cmdline_options[] = {
    { "-fullscreen", SET_RESOURCE, 0,
      NULL, NULL, "FullScreenEnabled", (resource_value_t) 1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_START_VICE_FULLSCREEN_MODE,
      NULL, NULL },
#ifdef HAVE_D3D9_H
    { "-dx9disable", SET_RESOURCE, 0,
      NULL, NULL, "DX9Disable", (resource_value_t) 1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_DISABLE_DX9,
      NULL, NULL },
#endif
    { NULL }
};

int video_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}


/* ------------------------------------------------------------------------ */

/* Initialization.  */
int video_init(void)
{
    return 0;
}

void video_shutdown(void)
{
#ifdef HAVE_D3D9_H
    video_shutdown_dx9();
#endif
}

void video_arch_canvas_init(video_canvas_t **canvas)
{
    *canvas = lib_calloc(1, sizeof(video_canvas_t));
#ifdef HAVE_D3D9_H
    if (video_setup_dx9() >= 0) {
        dx9_available = 1;
    } else {
        dx9_available = 0;
    }
#endif
}

#ifdef HAVE_D3D9_H
int video_dx9_enabled(void)
{
    return (dx9_available && !dx9_disable);
}

int video_dx9_available(void)
{
    return dx9_available;
}
#endif

/* ------------------------------------------------------------------------ */

raster_t *video_canvas_for_hwnd(HWND hwnd)
{
    int i;

    for (i = 0; i < video_number_of_canvases; i++) {
        if (video_canvases[i]->canvas->hwnd == hwnd) {
            return video_canvases[i];
        }
    }

    return NULL;
}

int video_canvas_nr_for_hwnd(HWND hwnd)
{
    int i;

    for (i = 0; i < video_number_of_canvases; i++) {
        if (video_canvases[i]->canvas->hwnd == hwnd) {
            return i;
        }
    }

    return 0;
}

void video_canvas_add(raster_t *raster)
{
    video_canvases[video_number_of_canvases++] = raster;
}

video_canvas_t *video_canvas_create(raster_t *raster, unsigned int *width, unsigned int *height, int mapped)
{
    int enable =
#ifdef HAVE_D3D9_H
        video_dx9_enabled()
#else
        0
#endif
    ;

    fullscreen_transition = 1;

    raster->canvas->title = lib_stralloc(raster->viewport->title);
    raster->canvas->width = *width;
    raster->canvas->height = *height;

    if (raster->videoconfig->doublesizex) {
        raster->canvas->width *= 2;
    }

    if (raster->videoconfig->doublesizey) {
        raster->canvas->height *= 2;
    }

    ui_open_canvas_window(raster->canvas, raster->geometry->pixel_aspect_ratio);
    ui_canvas_child_window(raster->canvas, enable);

#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        video_canvas_t *canvas_temp = video_canvas_create_dx9(raster, width, height);
        if (canvas_temp == 0) {
            log_debug("video: Falling back to DirectDraw canvas!");
            dx9_available = 0;
            ui_canvas_child_window(raster->canvas, 0);
        } else {
            return raster->canvas;
        }
    }
#endif
    return video_canvas_create_gdi(raster, width, height);
}

void video_canvas_destroy(video_canvas_t *canvas)
{
#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        video_device_release_dx9(canvas);
    }
#endif

    if (canvas != NULL) {
        if (canvas->hwnd !=0) {
            DestroyWindow(canvas->hwnd);
        }
        lib_free(canvas->title);
        canvas->title = NULL;
    }
}

int video_canvas_set_palette(raster_t *raster)
{
    video_set_physical_colors(raster->canvas, raster->palette, &raster->videoconfig->color_tables);
    return 0;
}

int video_set_physical_colors(video_canvas_t *c, palette_t *palette, video_render_color_tables_t *color_tables)
{
    unsigned int i;
    int rshift;
    int rbits;
    int gshift;
    int gbits;
    int bshift;
    int bbits;
    DWORD rmask;
    DWORD gmask;
    DWORD bmask;

    /* Use hard coded D3DFMT_X8R8G8B8 format, driver does conversion */
    rshift = 16;
    rmask = 0xff;
    rbits = 0;

    gshift = 8;
    gmask = 0xff;
    gbits = 0;

    bshift = 0;
    bmask = 0xff;
    bbits = 0;

    if (c->depth > 8) {
        for (i = 0; i < 256; i++) {
            video_render_setrawrgb(i, ((i & (rmask << rbits)) >> rbits) << rshift, ((i & (gmask << gbits)) >> gbits) << gshift, ((i & (bmask << bbits)) >> bbits) << bshift);
        }
        video_render_initraw();
    }

    for (i = 0; i < palette->num_entries; i++) {
        DWORD p = (((palette->entries[i].red&(rmask << rbits)) >> rbits) << rshift) +
                  (((palette->entries[i].green&(gmask << gbits)) >> gbits) << gshift) +
                  (((palette->entries[i].blue&(bmask << bbits)) >> bbits) << bshift);
        video_render_setphysicalcolor(color_tables, i, p, c->depth);
    }
    return 0;
}

/* Change the size of `s' to `width' * `height' pixels.  */
void video_canvas_resize(raster_t *raster, unsigned int width, unsigned int height)
{
    int device;
    int fullscreen_width;
    int fullscreen_height;
    int bitdepth;
    int refreshrate;

    if (raster->videoconfig->doublesizex) {
        width *= 2;
    }

    if (raster->videoconfig->doublesizey) {
        height *= 2;
    }

    raster->canvas->width = width;
    raster->canvas->height = height;
    if (IsFullscreenEnabled()) {
        GetCurrentModeParameters(&device, &fullscreen_width, &fullscreen_height, &bitdepth, &refreshrate);
    } else {
        raster->canvas->client_width = width;
        raster->canvas->client_height = height;
        ui_resize_canvas_window(raster->canvas, raster->geometry->pixel_aspect_ratio);
    }

#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        video_canvas_reset_dx9(raster->canvas);
    }
#endif
    raster->canvas->bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    raster->canvas->bmp_info.bmiHeader.biWidth = width;
    raster->canvas->bmp_info.bmiHeader.biHeight = -(LONG)height;
    raster->canvas->bmp_info.bmiHeader.biPlanes = 1;
    raster->canvas->bmp_info.bmiHeader.biBitCount = 24;
    raster->canvas->bmp_info.bmiHeader.biCompression = BI_RGB;
    raster->canvas->bmp_info.bmiHeader.biSizeImage = 3 * width * height;
    raster->canvas->pixels = lib_malloc(raster->canvas->bmp_info.bmiHeader.biSizeImage);
#ifdef HAVE_D3D9_H
    if(!video_dx9_enabled())
#endif
    {
        raster->canvas->depth = 24;
    }
}


/* Raster code has updated display */
void video_canvas_refresh(raster_t *raster, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        video_canvas_refresh_dx9(raster, xs, ys, xi, yi, w, h);
    } else
#endif
    {
        video_canvas_refresh_gdi(raster, xs, ys, xi, yi, w, h);
    }
}

/* Window got a WM_PAINT and needs a refresh */
void video_canvas_update(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h)
{
#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        video_canvas_update_dx9(hwnd, hdc, xclient, yclient, w, h);
    } else
#endif
    {
        video_canvas_update_gdi(hwnd, hdc, xclient, yclient, w, h);
    }
}
