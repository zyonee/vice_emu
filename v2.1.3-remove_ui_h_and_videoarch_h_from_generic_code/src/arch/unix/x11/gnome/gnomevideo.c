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
#include "palette.h"
#include "raster.h"
#include "types.h"
#include "videoarch.h"
#include "video.h"
#include "machine.h"
#include "lib.h"

#include "ui.h"
#include "uiarch.h"
#include "uicolor.h"
#ifdef HAVE_OPENGL_SYNC
#include "openGL_sync.h"
#endif

static log_t gnomevideo_log = LOG_ERR;

int video_arch_resources_init(void)
{
#ifdef HAVE_OPENGL_SYNC
    openGL_register_resources();
#endif
    return 0;
}

void video_arch_resources_shutdown(void)
{
}

int video_init(void)
{
    if (gnomevideo_log == LOG_ERR)
        gnomevideo_log = log_open("GnomeVideo");

    return 0;
}

void video_shutdown(void)
{
}

int video_init_cmdline_options(void)
{
    return 0;
}

struct video_canvas_s * video_arch_canvas_init(void)
{
    struct video_canvas_s * canvas = (video_canvas_t *)lib_calloc(1, sizeof(video_canvas_t));
    canvas->video_draw_buffer_callback = NULL;

#ifdef HAVE_FULLSCREEN
    canvas->fullscreenconfig
        = (fullscreenconfig_t *)lib_calloc(1, sizeof(fullscreenconfig_t));
    fullscreen_init_alloc_hooks(canvas);
#endif

    return canvas;
}


unsigned char video_canvas_create(raster_t *canvas, unsigned int *width,
                                    unsigned int *height, char *title,
                                    int mapped)
{
    int res;

    canvas->canvas->gdk_image = NULL;
#ifdef HAVE_HWSCALE
    canvas->canvas->hwscale_image = NULL;
#endif

    res = ui_open_canvas_window(canvas, title,
				*width, *height, 1);
    if (res < 0) {
        return 0;
    }

#ifdef HAVE_OPENGL_SYNC
    openGL_sync_init(canvas);
#endif

    return 1;
}

void video_canvas_destroy(video_canvas_t *canvas)
{
#ifdef HAVE_FULLSCREEN
    if (canvas != NULL) {
        fullscreen_shutdown_alloc_hooks(canvas);
        lib_free(canvas->fullscreenconfig);
    }
#endif
    if (canvas->gdk_image != NULL)
        g_object_unref(canvas->gdk_image);
#ifdef HAVE_HWSCALE
    lib_free(canvas->hwscale_image);
#endif
}

static unsigned int endian_swap(unsigned int color, unsigned int bpp, unsigned int swap) {
    if (! swap)
        return color;

    if (bpp == 8)
        return color;

    if (bpp == 16)
        return ((color >> 8) & 0x00ff)
             | ((color << 8) & 0xff00);

    if (bpp == 24)
        return color; /* 24 bpp output special case at renderer level, always writes out with LSB order regardless of host CPU, handled elsewhere */

    if (bpp == 32)
        return ((color >> 24) & 0x000000ff)
             | ((color >>  8) & 0x0000ff00)
             | ((color <<  8) & 0x00ff0000)
             | ((color << 24) & 0xff000000);
    
    /* err? */
    return color;
}

/* set it, update if we know the endianness required by the image */
int video_canvas_set_palette(struct palette_s *palette, DWORD *physical_colors)
{
    unsigned int i, rs, gs, bs, rb, gb, bb, bpp, swap;

    /* Hwscaled colours are expected in GL_RGB order. 24 bpp renderers are
     * special, they always seem to expect color order to be logically ABGR,
     * which they write out in RGB memory order. (Glorious, eh?) */
    /*if (c->videoconfig->hwscale) {*/
        bpp = 24;
        rb = 8;
        gb = 8;
        bb = 8;
        rs = 0;
        gs = 8;
        bs = 16;
        swap = 0;
    /*} else {
        GdkVisual *vis = c->gdk_image->visual;
        bpp = vis->depth;
        rb = vis->red_prec;
        gb = vis->green_prec;
        bb = vis->blue_prec;
        rs = vis->red_shift;
        gs = vis->green_shift;
        bs = vis->blue_shift;
#ifdef WORDS_BIGENDIAN
        swap = vis->byte_order == GDK_LSB_FIRST;
#else
        swap = vis->byte_order == GDK_MSB_FIRST;
#endif
        /* 24 bpp modes do not really work with the existing
         * arrangement as they have been written to assume the A component is
         * in the 32-bit longword bits 24-31. If any arch needs 24 bpp, that
         * code must be specially written for it. 
    }*/

    for (i = 0; i < palette->num_entries; i++) {
        palette_entry_t color = palette->entries[i];
        /* scale 256 color palette for Gdk terms, then shift to precision,
         * then move component where it needs to be. */
        DWORD color_pixel = 
            color.red         |
            color.green <<  8 |
            color.blue  << 16;
        video_render_setphysicalcolor(physical_colors, i, color_pixel,
                                      24);
    }
    
    for (i = 0; i < 256; i ++) {
        video_render_setrawrgb(i, 
            i      ,
            i <<  8,
            i << 16
        );
    }
    
    video_render_initraw();

    return 0;
}

/* Change the size of the canvas. */
void video_canvas_resize(video_canvas_t *canvas, unsigned int width,
                         unsigned int height,
                         int doublesizex, int doublesizey)
{
    if (console_mode || vsid_mode)
        return;

    if (doublesizex)
        width *= 2;

    if (doublesizey)
        height *= 2;

    if (canvas->gdk_image != NULL)
        g_object_unref(canvas->gdk_image);
    canvas->gdk_image = gdk_image_new(GDK_IMAGE_FASTEST, gtk_widget_get_visual(canvas->emuwindow), width, height);
#ifdef HAVE_HWSCALE
    lib_free(canvas->hwscale_image);
    canvas->hwscale_image = lib_malloc(canvas->gdk_image->width * canvas->gdk_image->height * 3);
#endif

    ui_resize_canvas_window(canvas, width, height);
    video_canvas_redraw_size(canvas, width, height);
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
void video_canvas_refresh(raster_t *canvas,
                          unsigned int xs, unsigned int ys,
                          unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h,
                          int doublesizex, int doublesizey)
{
#if 0
    log_debug("XS%i YS%i XI%i YI%i W%i H%i PS%i", xs, ys, xi, yi, w, h,
              canvas->draw_buffer->draw_buffer_width);
#endif

    if (console_mode || vsid_mode || canvas->canvas->gdk_image == NULL)
        return;

    if (doublesizex) {
        xi *= 2;
        w *= 2;
    }

    if (doublesizey) {
        yi *= 2;
        h *= 2;
    }

#ifdef HAVE_FULLSCREEN
    if (canvas->video_fullscreen_refresh_func) {
        canvas->video_fullscreen_refresh_func(canvas, xs, ys, xi, yi, w, h);
        return;
    }
#endif

    if (xi + w > canvas->canvas->gdk_image->width || yi + h > canvas->canvas->gdk_image->height) {
        log_debug("Attempt to draw outside canvas!\n"
                  "XI%i YI%i W%i H%i CW%i CH%i\n",
                  xi, yi, w, h, canvas->canvas->gdk_image->width, canvas->canvas->gdk_image->height);
        exit(-1);
    }

    video_render_main(canvas->videoconfig->rendermode,
                      canvas->draw_buffer->draw_buffer,
                      canvas->canvas->hwscale_image,
                      w, h, xs, ys, xi, yi,
                      canvas->draw_buffer->draw_buffer_width,
                      canvas->canvas->gdk_image->width * 3,
                      24,
                      canvas->viewport,
                      &canvas->videoconfig->color_tables,
                      canvas->videoconfig->doublescan,
                      canvas->videoconfig->scale2x);
    gtk_widget_queue_draw(canvas->canvas->emuwindow);
}

void video_canvas_refresh_all(raster_t *canvas)
{
    gtk_widget_queue_draw(canvas->canvas->emuwindow);
}

