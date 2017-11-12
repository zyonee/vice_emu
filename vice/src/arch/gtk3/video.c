/*
 * video.c - Native GTK3 UI video stuff.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Michael C. Martin <mcmartin@gmail.com>
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "debug_gtk3.h"
#include "not_implemented.h"

#include "cmdline.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "palette.h"
#include "raster.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "videoarch.h"
#include "cairo_renderer.h"
#include "opengl_renderer.h"

/* To test the OpenGL renderer, change the assigned value here to &vice_opengl_backend */
/* WARNING: OpenGL backend isn't ready for prime-time yet */
/* TODO: Make the rendering process transparent enough that this can be selected and altered as-needed */
vice_renderer_backend_t *vice_renderer_backend = &vice_cairo_backend;

#define VICE_DEBUG_NATIVE_GTK3

/** \brief  Log for Gtk3-native video messages
 */
static log_t    gtk3video_log = LOG_ERR;

/** \brief  Keep aspect ratio when resizing */
static int keepaspect = 0;

/** \brief  Use true aspect ratio */
static int trueaspect = 0;

/** \brief  Display depth in bits (8, 15, 16, 24, 32) */
static int display_depth = 24;


/** \brief  Set KeepAspectRatio resource (bool)
 *
 * \param[in]   val     new value
 * \param[in]   param   extra parameter (unused)
 *
 * \return 0
 */
static int set_keepaspect(int val, void *param)
{
    keepaspect = val ? 1 : 0;
    ui_trigger_resize();
    return 0;
}


/** \brief  Set TrueAspectRatio resource (bool)
 *
 * \param[in]   val     new value
 * \param[in]   param   extra parameter (unused)
 *
 * \return 0
 */
static int set_trueaspect(int val, void *param)
{
    trueaspect = val ? 1 : 0;
    ui_trigger_resize();
    return 0;
}


static int set_display_depth(int val, void *param)
{
    if (val != 0 && val != 8 && val != 15 && val != 16 && val != 24 && val != 32) {
        return -1;
    }
    display_depth = val;
    return 0;
}

/** \brief  Command line options related to generic video output
 */
static const cmdline_option_t cmdline_options[] = {
    { "-trueaspect", SET_RESOURCE, 0,
      NULL, NULL, "TrueAspectRatio", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable true aspect ratio" },
    { "+trueaspect", SET_RESOURCE, 0,
      NULL, NULL, "TrueAspectRatio", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable true aspect ratio" },
    { "-keepaspect", SET_RESOURCE, 0,
      NULL, NULL, "KeepAspectRatio", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Keep aspect ratio when scaling" },
    { "+keepaspect", SET_RESOURCE, 0,
      NULL, NULL, "KeepAspectRatio", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Do not keep aspect ratio when scaling (freescale)" },
    CMDLINE_LIST_END
};


/** \brief  Integer/boolean resources related to video output
 */
static const resource_int_t resources_int[] = {
    { "KeepAspectRatio", 1, RES_EVENT_NO, NULL,
      &keepaspect, set_keepaspect, NULL },
    { "TrueAspectRatio", 1, RES_EVENT_NO, NULL,
      &trueaspect, set_trueaspect, NULL },
    { "DisplayDepth", 0, RES_EVENT_NO, NULL,
      &display_depth, set_display_depth, NULL },
    RESOURCE_INT_LIST_END
};

/** \brief  Initialize video canvas
 */
void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    /* copy/paste from gnomevideo.c */
    canvas->video_draw_buffer_callback = NULL;
#ifdef HAVE_FULLSCREEN
    if (machine_class != VICE_MACHINE_VSID) {
        canvas->fullscreenconfig = lib_calloc(1, sizeof(fullscreenconfig_t));
        fullscreen_init_alloc_hooks(canvas);
    }
#endif
}


/** \brief  Initialize command line options for generic video resouces
 *
 * \return  0 on success, < 0 on failure
 */
int video_arch_cmdline_options_init(void)
{
    if (machine_class != VICE_MACHINE_VSID) {
        return cmdline_register_options(cmdline_options);
    }
    return 0;
}


/** \brief  Initialize video-related resources
 *
 * \return  0 on success, < on failure
 */
int video_arch_resources_init(void)
{
    if (machine_class != VICE_MACHINE_VSID) {
        return resources_register_int(resources_int);
    }
    return 0;
}


void video_arch_resources_shutdown(void)
{
}


char video_canvas_can_resize(video_canvas_t *canvas)
{
    return 1;
}

/* FIXME: temporary hack */
extern void ui_set_toplevel_widget(GtkWidget *win, GtkWidget *status);

video_canvas_t *video_canvas_create(video_canvas_t *canvas,
                                    unsigned int *width, unsigned int *height,
                                    int mapped)
{
    VICE_GTK3_FUNC_ENTERED();
    canvas->initialized = 0;
    canvas->created = 0;
    canvas->renderer_context = NULL;
    ui_create_toplevel_window(canvas);
    if (width && height) {
        vice_renderer_backend->update_context(canvas, *width, *height);
    }
    ui_display_toplevel_window(canvas);

    canvas->created = 1;
    canvas->initialized = 1;
    return canvas;
}

void video_canvas_destroy(struct video_canvas_s *canvas)
{
    if (canvas != NULL) {
#ifdef HAVE_FULLSCREEN
        fullscreen_shutdown_alloc_hooks(canvas);
        if (canvas->fullscreenconfig != NULL) {
            lib_free(canvas->fullscreenconfig);
        }
#endif
        vice_renderer_backend->destroy_context(canvas);
    }
}

void video_canvas_refresh(struct video_canvas_s *canvas,
                          unsigned int xs, unsigned int ys,
                          unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h)
{
    unsigned int backing_surface_w, backing_surface_h;
    if (console_mode || video_disabled_mode || !canvas) {
        return;
    }

    xi *= canvas->videoconfig->scalex;
    w *= canvas->videoconfig->scalex;

    yi *= canvas->videoconfig->scaley;
    h *= canvas->videoconfig->scaley;

    if (vice_renderer_backend->get_backbuffer_info(canvas, &backing_surface_w, &backing_surface_h, NULL)) {
        /* Backbuffer isn't actually ready yet, nothing to refresh */
        return;
    }

    if (((xi + w) > backing_surface_w) || ((yi+h) > backing_surface_h)) {
        /* Trying to draw outside canvas? */
        fprintf(stderr, "Attempt to draw outside canvas!\nXI%u YI%u W%u H%u CW%u CH%u\n", xi, yi, w, h, backing_surface_w, backing_surface_h);
        return;
    }

    /* fprintf(stderr, "Xsc%d Ysc%d XS%u YS%u XI%u YI%u W%u H%u CW%u CH%u\n", canvas->videoconfig->scalex, canvas->videoconfig->scaley, xs, ys, xi, yi, w, h, backing_surface_w, backing_surface_h); */

    vice_renderer_backend->refresh_rect(canvas, xs, ys, xi, yi, w, h);
}

void video_canvas_resize(struct video_canvas_s *canvas, char resize_canvas)
{
    if (!canvas || !canvas->drawing_area) {
        return;
    } else {
        int new_width = canvas->draw_buffer->canvas_physical_width;
        int new_height = canvas->draw_buffer->canvas_physical_height;

        if (new_width <= 0 || new_height <= 0) {
            /* Ignore impossible dimensions, but complain about it */
            fprintf(stderr, "%s:%d: warning: function %s called with impossible dimensions\n", __FILE__, __LINE__, __func__);
            return;
        }

        vice_renderer_backend->update_context(canvas, new_width, new_height);

        /* Set the palette */
        if (video_canvas_set_palette(canvas, canvas->palette) < 0) {
            fprintf(stderr, "Setting palette for this mode failed. (Try 16/24/32 bpp.)");
            exit(-1);
        }
    }
}

void video_canvas_adjust_aspect_ratio(struct video_canvas_s *canvas)
{
    int width = canvas->draw_buffer->canvas_physical_width;
    int height = canvas->draw_buffer->canvas_physical_height;
    if (keepaspect && trueaspect) {
        width *= canvas->geometry->pixel_aspect_ratio;
    }

    /* Finally alter our minimum size so the GUI may respect the new minima/maxima */
    gtk_widget_set_size_request(canvas->drawing_area, width, height);
}

int video_canvas_set_palette(struct video_canvas_s *canvas,
                             struct palette_s *palette)
{
    if (!canvas || !palette) {
        return 0; /* No palette, nothing to do */
    }
    canvas->palette = palette;
    vice_renderer_backend->set_palette(canvas);
    return 0;
}

int video_init(void)
{
    if (gtk3video_log == LOG_ERR) {
        gtk3video_log = log_open("Gtk3Video");
    }
    return 0;
}


void video_shutdown(void)
{
    /* It's a no-op */
}
