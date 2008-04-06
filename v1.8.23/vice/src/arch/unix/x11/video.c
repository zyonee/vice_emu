/*
 * video.c - X11 graphics routines.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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


/*** MITSHM-code rewritten by Dirk Farin <farin@ti.uni-mannheim.de>. **

     This is how the MITSHM initialization now works:

       Three variables are used to enable/disable the usage of MITSHM:
       - 'try_mitshm' is set to true by default to specify that
         MITSHM shall be used if possible. If the user sets this
         variable to false MITSHM will be disabled.
       - 'use_mitshm' will be set in video_init() after some quick
         tests if the X11 server supports MITSHM.
       - Every framebuffer structure has a new field named 'using_mitshm'
         that is set to true if MITSHM is used for this buffer.
         Note that it is possible that one buffer is using MITSHM
         while some other buffer is not.

       Detecting if MITSHM usage is possible is now done using a
       minimum of intelligence (only XShmQueryExtension() is checked
       in video_init() ). Then the allocation process is executed
       and the X11 error in case of failure is catched. If an error
       occured the allocation process is reversed and non-MITSHM
       XImages are used instead.
*/

#include "vice.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

#include "color.h"
#include "cmdline.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "types.h"
#include "ui.h"
#include "uicolor.h"
#include "utils.h"
#include "video.h"
#include "videoarch.h"
#ifdef USE_XF86_EXTENSIONS
#include "fullscreen.h"
#endif
#ifdef USE_GNOMEUI
#include <gdk/gdkx.h>
#endif
#ifdef HAVE_XVIDEO
#include "renderxv.h"
#include "raster/raster.h"
#include "video/video-resources.h"
extern DWORD yuv_table[128];
#endif

/* ------------------------------------------------------------------------- */

/* Flag: Do we call `XSync()' after blitting?  */
int _video_use_xsync;

/* Flag: Do we try to use the MIT shared memory extensions?  */
static int try_mitshm;

static int set_use_xsync(resource_value_t v, void *param)
{
    _video_use_xsync = (int)v;
    return 0;
}

static int set_try_mitshm(resource_value_t v, void *param)
{
    try_mitshm = (int)v;
    return 0;
}

static int set_use_xvideo(resource_value_t v, void *param)
{
    use_xvideo = (int)v;
    return 0;
}

static unsigned int fourcc;
static int set_fourcc(resource_value_t v, void *param)
{
    if (v && strlen((char *)v) == 4) {
        memcpy(&fourcc, (char *)v, 4);
    }
    else {
        fourcc = 0;
    }
    
    return 0;
}

/* Video-related resources.  */
static resource_t resources[] = {
    { "UseXSync", RES_INTEGER, (resource_value_t)1,
      (resource_value_t *)&_video_use_xsync, set_use_xsync, NULL },
      /* turn MITSHM on by default */
    { "MITSHM", RES_INTEGER, (resource_value_t)1,
      (resource_value_t *)&try_mitshm, set_try_mitshm, NULL },
    { "XVIDEO", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&use_xvideo, set_use_xvideo, NULL },
    { "FOURCC", RES_STRING, (resource_value_t)"",
      (resource_value_t *)&fourcc, set_fourcc, NULL },
    { NULL }
};

int video_arch_init_resources(void)
{

    return resources_register(resources);
}

/* ------------------------------------------------------------------------- */

/* Video-related command-line options.  */
static cmdline_option_t cmdline_options[] = {
    { "-xsync", SET_RESOURCE, 0, NULL, NULL,
      "UseXSync", (resource_value_t)1,
      NULL, N_("Call `XSync()' after updating the emulation window") },
    { "+xsync", SET_RESOURCE, 0, NULL, NULL,
      "UseXSync", (resource_value_t)0,
      NULL, N_("Do not call `XSync()' after updating the emulation window") },
    { "-mitshm", SET_RESOURCE, 0, NULL, NULL,
      "MITSHM", (resource_value_t) 0,
      NULL, N_("Never use shared memory (slower)") },
#ifdef HAVE_XVIDEO
    { "-xvideo", SET_RESOURCE, 0, NULL, NULL,
      "XVIDEO", (resource_value_t) 1,
      NULL, N_("Use XVideo Extension (hardware scaling)") },
    { "+xvideo", SET_RESOURCE, 0, NULL, NULL,
      "XVIDEO", (resource_value_t) 0,
      NULL, N_("Use software rendering") },
    { "-fourcc", SET_RESOURCE, 1, NULL, NULL, "FOURCC", NULL,
      "<fourcc>", N_("Request YUV FOURCC format") },
#endif
    { NULL }
};

int video_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

static GC _video_gc;
static void (*_refresh_func)();

/* This is set to 1 if the Shared Memory Extensions can actually be used. */
int use_mitshm = 0;

/* This is set to 1 if the XVideo Extension is used. */
int use_xvideo = 0;

/* The RootWindow of our screen. */
/* static Window root_window; */

/* Logging goes here.  */
static log_t video_log = LOG_ERR;

/* ------------------------------------------------------------------------- */

static void (*_convert_func) (BYTE *draw_buffer,
                              unsigned int draw_buffer_line_size,
                              video_canvas_t *canvas,
                              int x, int y, int w, int h);
static BYTE shade_table[256];

void video_convert_color_table(unsigned int i, BYTE *pixel_return, BYTE *data,
                               unsigned int dither, long col,
                               video_canvas_t *c)
{
    *pixel_return = (BYTE)i;

    switch (c->depth) {
      case 8:
        video_render_setphysicalcolor(&c->videoconfig, i,
                                      (DWORD)(*(BYTE *)data), 8);
        break;
      case 16:
        video_render_setphysicalcolor(&c->videoconfig, i, (DWORD)(col), 16);
        break;
      case 32:
        video_render_setphysicalcolor(&c->videoconfig, i, (DWORD)(col), 32);
        break;
      default:
        video_render_setphysicalcolor(&c->videoconfig, i, (DWORD)(col),
                                      c->depth);
    }

    if (c->depth == 1)
        shade_table[i] = dither;
}

/* This doesn't usually happen, but if it does, this is a great speedup
   comparing the general convert_8toall() -routine. */
static void convert_8to8(BYTE *draw_buffer,
                         unsigned int draw_buffer_line_size,
                         video_canvas_t *canvas,
                         int sx, int sy, int w, int h)
{
    video_render_main(&canvas->videoconfig, draw_buffer,
                      canvas->x_image->data,
                      w, h, sx, sy, sx, sy, draw_buffer_line_size,
                      canvas->x_image->bytes_per_line, 8);

}

static void convert_8to16(BYTE *draw_buffer,
                          unsigned int draw_buffer_line_size,
                          video_canvas_t *canvas,
                          int sx, int sy, int w, int h)
{
    video_render_main(&canvas->videoconfig, draw_buffer,
                      canvas->x_image->data,
                      w, h, sx, sy, sx, sy, draw_buffer_line_size,
                      canvas->x_image->bytes_per_line, 16);
}

static void convert_8to32(BYTE *draw_buffer,
                          unsigned int draw_buffer_line_size,
                          video_canvas_t *canvas,
                          int sx, int sy, int w, int h)
{
    video_render_main(&canvas->videoconfig, draw_buffer,
                      canvas->x_image->data,
                      w, h, sx, sy, sx, sy, draw_buffer_line_size,
                      canvas->x_image->bytes_per_line, 32);
}

#define SRCPTR(x, y) \
        (draw_buffer + (y) * draw_buffer_line_size + (x))
#define DESTPTR(i, x, y, t) \
        ((t *)((BYTE *)(i)->x_image->data + \
               (i)->x_image->bytes_per_line * (y)) + (x))

/* Use dither on 1bit display. This is slow but who cares... */
BYTE dither_table[4][4] = {
    { 0, 8, 2, 10 },
    { 12, 4, 14, 6 },
    { 3, 11, 1, 9 },
    { 15, 7, 13, 5 }
};

static void convert_8to1_dither(BYTE *draw_buffer,
                                unsigned int draw_buffer_line_size,
                                video_canvas_t *canvas,
                                int sx, int sy, int w, int h)
{
    BYTE *src, *dither;
    int x, y;
    for (y = 0; y < h; y++) {
        src = SRCPTR(sx, sy + y);
        dither = dither_table[(sy + y) % 4];
        for (x = 0; x < w; x++) {
            /* XXX: trusts that real_pixel[0, 1] == black, white */
            XPutPixel(canvas->x_image, sx + x, sy + y,
                      canvas->videoconfig.physical_colors[shade_table[src[x]]
                      > dither[(sx + x) % 4]]);
        }
    }
}

/* And this is inefficient... */
static void convert_8toall(BYTE *draw_buffer,
                           unsigned int draw_buffer_line_size,
                           video_canvas_t *canvas,
                           int sx, int sy, int w, int h)
{
    BYTE *src;
    int x, y;
    for (y = 0; y < h; y++) {
        src = SRCPTR(sx, sy + y);
        for (x = 0; x < w; x++)
            XPutPixel(canvas->x_image, sx + x, sy + y,
                      canvas->videoconfig.physical_colors[src[x]]);
    }
}


int video_convert_func(video_canvas_t *canvas, unsigned int width,
                       unsigned int height)
{
    if (use_xvideo) {
        return 0;
    }

    switch (canvas->depth) {
      case 1:
        _convert_func = convert_8to1_dither;
        break;
      case 8:
        _convert_func = convert_8to8;
        break;
      case 16:
        _convert_func = convert_8to16;
        break;
      case 32:
        _convert_func = convert_8to32;
        break;
      default:
        _convert_func = convert_8toall;
    }
    return 0;
}

extern GC video_get_gc(void *not_used);

int video_init(void)
{
    XGCValues gc_values;
    Display *display;

    _video_gc = video_get_gc(&gc_values);
    display = ui_get_display_ptr();

    if (video_log == LOG_ERR)
        video_log = log_open("Video");

    color_init();

#ifdef USE_MITSHM
    if (!try_mitshm) {
        use_mitshm = 0;
    } else {
        /* This checks if the server has MITSHM extensions available
           If try_mitshm is true and we are on a different machine,
           frame_buffer_alloc will fall back to non shared memory calls. */
        int major_version, minor_version, pixmap_flag;

        /* Check whether the server supports the Shared Memory Extension. */
        if (!XShmQueryVersion(display, &major_version, &minor_version,
                              &pixmap_flag)) {
            log_warning(video_log,
                        _("The MITSHM extension is not supported "
                        "on this display."));
            use_mitshm = 0;
        } else {
            DEBUG_MITSHM((_("MITSHM extensions version %d.%d detected."),
                          major_version, minor_version));
            use_mitshm = 1;
        }
    }

#else
    use_mitshm = 0;
#endif

    video_init_arch();

    return 0;
}

#ifdef USE_MITSHM
int mitshm_failed = 0; /* will be set to true if XShmAttach() failed */
int shmmajor;          /* major number of MITSHM error codes */

/* Catch XShmAttach()-failure. */
int shmhandler(Display* display,XErrorEvent* err)
{
    if (err->request_code == shmmajor &&
        err->minor_code == X_ShmAttach)
      mitshm_failed=1;

    return 0;
}
#endif

/* Free an allocated frame buffer. */
static void video_arch_frame_buffer_free(video_canvas_t *canvas)
{
    Display *display;

    if (canvas == NULL)
        return;

#ifdef HAVE_XVIDEO
    if (use_xvideo) {
        XShmSegmentInfo* shminfo = use_mitshm ? &canvas->xshm_info : NULL;

        display = ui_get_display_ptr();
	destroy_yuv_image(display, canvas->xv_image, shminfo);
	return;
    }
#endif

#ifdef USE_XF86_EXTENSIONS
    if (fullscreen_is_enabled)
        return;
#endif

    display = ui_get_display_ptr();

#ifdef USE_MITSHM
    if (canvas->using_mitshm) {
        XShmDetach(display, &(canvas->xshm_info));
        if (canvas->x_image)
            XDestroyImage(canvas->x_image);
        if (shmdt(canvas->xshm_info.shmaddr))
            log_error(video_log, _("Cannot release shared memory!"));
    } 
#ifndef USE_GNOMEUI
    else if (canvas->x_image)
        XDestroyImage(canvas->x_image);
#endif
#else
#ifndef USE_GNOMEUI
    if (canvas->x_image)
        XDestroyImage(canvas->x_image);
#endif
#endif
}

void video_register_raster(struct raster_s *raster)
{
#ifdef USE_XF86_DGA2_EXTENSIONS
    fullscreen_set_raster(raster);
#endif
}

/* ------------------------------------------------------------------------- */
/* Create a canvas.  In the X11 implementation, this is just (guess what?) a
   window. */
video_canvas_t *video_canvas_create(const char *win_name, unsigned int *width,
                                    unsigned int *height, int mapped,
                                    void_t exposure_handler,
                                    const struct palette_s *palette,
                                    BYTE *pixel_return)
{
    video_canvas_t *canvas;
    ui_window_t w;
    XGCValues gc_values;

    canvas = (video_canvas_t *)xmalloc(sizeof(video_canvas_t));
    memset(canvas, 0, sizeof(video_canvas_t));

    canvas->depth = ui_get_display_depth();
    canvas->video_draw_buffer_callback = NULL;

#ifdef USE_XF86_DGA2_EXTENSIONS
    canvas->video_draw_buffer_callback = 
	xmalloc(sizeof(struct video_draw_buffer_callback_s));
    canvas->video_draw_buffer_callback->draw_buffer_alloc = 
	fs_draw_buffer_alloc;
    canvas->video_draw_buffer_callback->draw_buffer_free = 
	fs_draw_buffer_free;
    canvas->video_draw_buffer_callback->draw_buffer_clear = 
	fs_draw_buffer_clear;
#endif

#ifdef HAVE_XVIDEO
    /* Request specified video format. */
    canvas->xv_format.id = fourcc;
#endif
    if (video_arch_frame_buffer_alloc(canvas, *width, *height) < 0) {
        free(canvas);
        return NULL;
    }

    video_render_initconfig(&canvas->videoconfig);

    w = ui_open_canvas_window(canvas, win_name, *width, *height, 1,
                              (canvas_redraw_t)exposure_handler, palette,
                              pixel_return);

    if (!_video_gc)
        _video_gc = video_get_gc(&gc_values);

    if (!w) {
        free(canvas);
        return (video_canvas_t *)NULL;
    }

    canvas->emuwindow = w;
    canvas->width = *width;
    canvas->height = *height;
    ui_finish_canvas(canvas);

    uicolor_init_video_colors();
    video_add_handlers(w);
    if (console_mode || vsid_mode)
        return canvas;

#ifdef USE_XF86_DGA2_EXTENSIONS
    fullscreen_set_palette(canvas, (struct palette_s *) palette, pixel_return);
#endif
    return canvas;
}

void video_canvas_destroy(video_canvas_t *c)
{
        /* FIXME: Just a dummy so far */
}


int video_canvas_set_palette(video_canvas_t *c, const struct palette_s *palette,
                             BYTE *pixel_return)
{
    int res;
    
    res = uicolor_set_palette(c, palette, pixel_return);
#ifdef USE_XF86_DGA2_EXTENSIONS
    if (res != -1)
	fullscreen_set_palette(c, palette, pixel_return);
#endif
    return res;
}

/* Change the size of the canvas. */
void video_canvas_resize(video_canvas_t *canvas, unsigned int width,
                         unsigned int height)
{
    if (console_mode || vsid_mode) {
        return;
    }

    video_arch_frame_buffer_free(canvas);
    video_arch_frame_buffer_alloc(canvas, width, height);

#ifdef USE_XF86_DGA2_EXTENSIONS
    /* printf("%s: w = %d, h = %d\n", __FUNCTION__, width, height); */
    if (fullscreen_is_enabled)
	return;
    fullscreen_resize(width, height);
#endif

    ui_resize_canvas_window(canvas->emuwindow, width, height);
    canvas->width = width;
    canvas->height = height;
    ui_finish_canvas(canvas);
}

void enable_text(void)
{
}

void disable_text(void)
{
}

/* ------------------------------------------------------------------------- */

void video_refresh_func(void (*rfunc)(void))
{
    _refresh_func = rfunc;
}

/* ------------------------------------------------------------------------- */

/* Refresh a canvas.  */
void video_canvas_refresh(video_canvas_t *canvas,
                          BYTE *draw_buffer,
                          unsigned int draw_buffer_line_size,
                          unsigned int xs, unsigned int ys,
                          unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h)
{
    Display *display;
    /*printf("XS%i YS%i XI%i YI%i W%i H%i\n",xs, ys, xi, yi, w, h);*/

#ifdef HAVE_XVIDEO
    if (use_xvideo) {
        raster_t *raster = raster_get_raster_from_canvas(canvas);

        int doublesize = canvas->videoconfig.doublesizex
	  && canvas->videoconfig.doublesizey;

        XShmSegmentInfo* shminfo = use_mitshm ? &canvas->xshm_info : NULL;
        Window root;
	int x, y;
	unsigned int dest_w, dest_h, border_width, depth;
        int xmin, wmax, ymin, hmax;

	if (!raster) {
	    log_error(video_log, "video_canvas_refresh called with raster == NULL");
	    return;
	}

	xmin = raster->geometry.extra_offscreen_border_left;
	wmax = raster->geometry.screen_size.width;
	ymin = raster->geometry.first_displayed_line;
	hmax = raster->geometry.last_displayed_line
	  - raster->geometry.first_displayed_line + 1;

	if (canvas->videoconfig.doublesizex) {
	  xs /= 2;
	  w /= 2;
	}
	if (canvas->videoconfig.doublesizey) {
	  ys /= 2;
	  h /= 2;
	}

	display = ui_get_display_ptr();

	/* FIXME: raster.c passes off-screen areas! */
	if (xs < xmin
	    || xs + w > xmin + wmax
	    || ys < ymin
	    || ys + h > ymin + hmax
	    )
	{
            log_error(video_log, "Off-screen area passed to video_canvas_refresh: x=%i, y=%i, w=%i, h=%i",
		      xs - xmin,
		      ys - ymin,
		      w, h);
	    xs = xmin;
	    w = wmax;
	    ys = ymin;
	    h = hmax;
	}

	render_yuv_image(doublesize,
			 canvas->videoconfig.doublescan,
			 video_resources.pal_mode,
			 video_resources.pal_scanlineshade*1024/1000,
			 canvas->xv_format,
			 canvas->xv_image,
			 draw_buffer, draw_buffer_line_size,
			 yuv_table,
			 xs, ys, w, h,
			 xs - xmin,
			 ys - ymin);

	XGetGeometry(display,
#ifdef USE_GNOMEUI
		     GDK_WINDOW_XWINDOW(canvas->drawable),
#else
		     canvas->drawable,
#endif
		     &root, &x, &y,
		     &dest_w, &dest_h, &border_width, &depth);

	/* Xv does subpixel scaling. Since coordinates are in integers we
	   refresh the entire image to get it right. */
	display_yuv_image(display, canvas->xv_port,
#ifdef USE_GNOMEUI
			  GDK_WINDOW_XWINDOW(canvas->drawable), GDK_GC_XGC(_video_gc),
#else
			  canvas->drawable, _video_gc,
#endif
			  canvas->xv_image, shminfo,
			  0, 0,
			  canvas->videoconfig.doublesizex ? wmax*2 : wmax,
			  canvas->videoconfig.doublesizey ? hmax*2 : hmax,
			  dest_w, dest_h);

	if (_video_use_xsync)
	    XSync(display, False);

	return;
    }
#endif
#ifdef USE_XF86_DGA2_EXTENSIONS
    if (fullscreen_is_enabled) {
        fullscreen_refresh_func(draw_buffer, draw_buffer_line_size, 
				xs, ys, xi, yi, w, h);
        return;
    }

#endif
    if (_convert_func)
        _convert_func(draw_buffer, draw_buffer_line_size, canvas,
                      xs, ys, w, h);

    /* This could be optimized away.  */
    display = ui_get_display_ptr();

    _refresh_func(display, canvas->drawable, _video_gc,
                  canvas->x_image, xs, ys, xi, yi, w, h, False,
                  NULL, canvas);
    if (_video_use_xsync)
        XSync(display, False);
}
