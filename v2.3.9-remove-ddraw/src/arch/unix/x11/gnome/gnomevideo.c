/*
 * ui.c - Gnome based graphical user interface.  
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
 *  Heavily derived from Oliver Schaertel <orschaer@forwiss.uni-erlangen.de>
 *  GTK+ port.
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

#include <string.h>
#include <stdlib.h>

#include "fullscreenarch.h"
#include "log.h"
#include "raster.h"
#include "resources.h"
#include "types.h"
#include "videoarch.h"
#include "video.h"
#include "video-render.h"
#include "machine.h"
#include "lib.h"

#include "ui.h"
#include "uiarch.h"
#include "uicolor.h"

#ifdef HAVE_OPENGL_SYNC
#include "openGL_sync.h"
#endif

static log_t gnomevideo_log = LOG_ERR;

/* FIXME: a resize event should be triggered
          when any of these two is changed
*/
static int keepaspect, trueaspect;
static int set_keepaspect(int val, void *param)
{
    keepaspect = val;
    return 0;
}

static int set_trueaspect(int val, void *param)
{
    trueaspect = val;
    return 0;
}

static const resource_int_t resources_int[] = {
    { "KeepAspectRatio", 1, RES_EVENT_NO, NULL,
      &keepaspect, set_keepaspect, NULL },
    { "TrueAspectRatio", 1, RES_EVENT_NO, NULL,
      &trueaspect, set_trueaspect, NULL },
    { NULL }
};

int video_arch_resources_init(void)
{
#ifdef HAVE_OPENGL_SYNC
    openGL_register_resources();
#endif
    return resources_register_int(resources_int);
}

void video_arch_resources_shutdown(void)
{
}

int video_init(void)
{
    if (gnomevideo_log == LOG_ERR) {
        gnomevideo_log = log_open("GnomeVideo");
    }

    return 0;
}

void video_shutdown(void)
{
}

int video_init_cmdline_options(void)
{
    return 0;
}

void video_arch_canvas_init(video_canvas_t **canvas)
{
    *canvas = malloc(sizeof(video_canvas_t));
    (*canvas)->video_draw_buffer_callback = NULL;

#ifdef HAVE_FULLSCREEN
    (*canvas)->fullscreenconfig = lib_calloc(1, sizeof(fullscreenconfig_t));
    fullscreen_init_alloc_hooks(*canvas);
#endif
}


video_canvas_t *video_canvas_create(raster_t *raster, unsigned int *width, unsigned int *height, int mapped)
{
    int res;

    raster->canvas->gdk_image = NULL;
#ifdef HAVE_HWSCALE
    raster->canvas->hwscale_image = NULL;
#endif

    res = ui_open_canvas_window(raster, raster->viewport->title, *width, *height, 1);
    if (res < 0) {
        return NULL;
    }

#ifdef HAVE_OPENGL_SYNC
    openGL_sync_init(raster->canvas);
#endif

    return raster->canvas;
}

void video_canvas_destroy(video_canvas_t *canvas)
{
#ifdef HAVE_FULLSCREEN
    if (canvas != NULL) {
        fullscreen_shutdown_alloc_hooks(canvas);
        lib_free(canvas->fullscreenconfig);
    }
#endif
    if (canvas->gdk_image != NULL) {
        g_object_unref(canvas->gdk_image);
    }

#ifdef HAVE_HWSCALE
    lib_free(canvas->hwscale_image);
#endif
}

/* set it, update if we know the endianness required by the image */
int video_canvas_set_palette(raster_t *raster)
{
    return uicolor_set_palette(raster);
}

/* Change the size of the canvas. */
void video_canvas_resize(raster_t *raster, unsigned int width, unsigned int height)
{
    if (console_mode || vsid_mode) {
        return;
    }

    if (raster->videoconfig->doublesizex) {
        width *= (raster->videoconfig->doublesizex + 1);
    }

    if (raster->videoconfig->doublesizey) {
        height *= (raster->videoconfig->doublesizey + 1);
    }

    if (raster->canvas->gdk_image != NULL) {
        g_object_unref(raster->canvas->gdk_image);
    }
    raster->canvas->gdk_image = gdk_image_new(GDK_IMAGE_FASTEST, gtk_widget_get_visual(raster->canvas->emuwindow), width, height);

#ifdef HAVE_HWSCALE
    lib_free(raster->canvas->hwscale_image);
    raster->canvas->hwscale_image = lib_malloc(canvas->gdk_image->width * canvas->gdk_image->height * 4);
#endif

    if (video_canvas_set_palette(raster) < 0) {
        log_debug("Setting palette for this mode failed. (Try 16/24/32 bpp.)");
        exit(-1);
    }

    ui_resize_canvas_window(raster, width, height);
    raster_redraw_size(raster, width, height);
}

/* Make the canvas visible. */
void video_canvas_map(video_canvas_t *s)
{
    fprintf(stderr, "**Function `canvas_map' not implemented.\n");
}

/* Make the canvas not visible. */
void video_canvas_unmap(video_canvas_t *s)
{
    fprintf(stderr, "**Function `canvas_unmap' not implemented.\n");
}

/* Refresh a canvas.  */
void video_canvas_refresh(raster_t *raster, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
#if 0
    log_debug("XS%i YS%i XI%i YI%i W%i H%i PS%i", xs, ys, xi, yi, w, h, canvas->draw_buffer->draw_buffer_width);
#endif

    if (console_mode || vsid_mode) {
        return;
    }

    if (raster->videoconfig->doublesizex) {
        xi *= (raster->videoconfig->doublesizex + 1);
        w *= (raster->videoconfig->doublesizex + 1);
    }

    if (raster->videoconfig->doublesizey) {
        yi *= (raster->videoconfig->doublesizey + 1);
        h *= (raster->videoconfig->doublesizey + 1);
    }

#ifdef HAVE_FULLSCREEN
    if (raster->canvas->video_fullscreen_refresh_func) {
        raster->canvas->video_fullscreen_refresh_func(raster->canvas, xs, ys, xi, yi, w, h);
        return;
    }
#endif

    if (xi + w > raster->canvas->gdk_image->width || yi + h > raster->canvas->gdk_image->height) {
#ifdef DEBUG	
        log_debug("Attempt to draw outside canvas!\nXI%i YI%i W%i H%i CW%i CH%i\n", xi, yi, w, h, raster->canvas->gdk_image->width, raster->canvas->gdk_image->height);
#endif
	return;
    }

#ifdef VIDEO_SCALE_SOURCE
    if (canvas->videoconfig->doublesizex) {
        xs /= (canvas->videoconfig->doublesizex + 1);
    }
    if (canvas->videoconfig->doublesizey) {
        ys /= (canvas->videoconfig->doublesizey + 1);
    }
#endif
#ifdef HAVE_HWSCALE
    if (raster->videoconfig->hwscale) {
        video_render_main(raster->canvas->videoconfig, raster->draw_buffer->draw_buffer,
                      raster->canvas->hwscale_image, w, h, xs, ys, xi, yi,
                      raster->draw_buffer->draw_buffer_width, canvas->gdk_image->width * 4, 32,
                      raster->viewport);

video_canvas_render(raster->canvas, raster->canvas->hwscale_image, w, h, xs, ys, xi, yi, canvas->gdk_image->width * 4, 32);
        gtk_widget_queue_draw(raster->canvas->emuwindow);
    } else
#endif
    {
        video_render_main(raster->videoconfig, raster->draw_buffer->draw_buffer,
                      raster->canvas->gdk_image->mem, w, h, xs, ys, xi, yi,
                      raster->draw_buffer->draw_buffer_width, raster->canvas->gdk_image->bpl,
                      raster->canvas->gdk_image->bits_per_pixel, raster->viewport);
        gtk_widget_queue_draw_area(raster->canvas->emuwindow, xi, yi, w, h);
    }
}
