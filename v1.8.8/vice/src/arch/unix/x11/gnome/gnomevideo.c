/*
 * ui.c - Gnome based graphical user interface.  
 *
 * Written by
 *  Martin Pottendorfer <Martin.Pottendorfer@alcatel.at>
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

#include <gdk/gdkx.h>
#include <string.h>

#include "log.h"
#include "videoarch.h"
#include "video.h"

#include "ui.h"
#include "uiarch.h"
#include "utils.h"
#ifdef HAVE_XVIDEO
#include "renderxv.h"
#endif

static log_t gnomevideo_log = LOG_ERR;

void video_init_arch(void)
{
    if (gnomevideo_log == LOG_ERR)
        gnomevideo_log = log_open("GnomeVideo");
}
extern GtkWidget *canvas;
extern GdkGC *app_gc;

inline void GDK_PUTIMAGE(Display *d, GdkPixmap *drawable, GdkGC *gc,
			 GdkImage *image, int src_x, int src_y,
			 int dest_x, int dest_y,
			 unsigned int width, unsigned int height, int b,
			 void *dummy, video_canvas_t *c)
{
  gdk_draw_image(drawable, gc, c->gdk_image, src_x, src_y,
		 dest_x, dest_y, width, height);
  gdk_window_clear_area(c->emuwindow->window, dest_x, dest_y, width, height);

  gdk_flush();
}

int video_arch_frame_buffer_alloc(video_canvas_t *canvas, unsigned int width,
                                  unsigned int height)
{
    int sizeofpixel = sizeof(BYTE);
    GdkImageType typ;

    if (!use_xvideo) {
        /* FIXME!!! */
        width *= 2;
	height *= 2;

	/* Round up to 32-bit boundary. */
	width = (width + 3) & ~0x3;
    }

    /* sizeof(PIXEL) is not always what we are using. I guess this should
       be checked from the XImage but I'm lazy... */
    if (canvas->depth > 8)
	sizeofpixel *= 2;
    if (canvas->depth > 16)
	sizeofpixel *= 2;

#ifdef HAVE_XVIDEO
    if (use_xvideo) {
        Display *display = ui_get_display_ptr();
        XShmSegmentInfo* shminfo = use_mitshm ? &canvas->xshm_info : NULL;

	if (!find_yuv_port(display, &canvas->xv_port, &canvas->xv_format,
			   &canvas->xv_render)) {
	  return -1;
	}

	if (!(canvas->xv_image = create_yuv_image(display, canvas->xv_port, canvas->xv_format, width, height, shminfo))) {
	  return -1;
	}

	log_message(gnomevideo_log,
		    _("Successfully initialized using X Video."));

	return 0;
    }
#endif
    typ = GDK_IMAGE_FASTEST;
    canvas->gdk_image = gdk_image_new(typ, visual, width, height);
    canvas->x_image = GDK_IMAGE_XIMAGE(canvas->gdk_image);
    if (!canvas->x_image)
	return -1;

    if (canvas->emuwindow != NULL)
        ui_finish_canvas(canvas);
 
    video_refresh_func((void (*)(void))GDK_PUTIMAGE);

    if (video_convert_func(canvas, width, height) < 0)
        return -1;

    log_message(gnomevideo_log,
                _("Successfully initialized video."));

#ifdef USE_XF86_DGA2_EXTENSIONS
    fullscreen_set_framebuffer(i);
#endif 
    return 0;
}

GC video_get_gc(void *not_used)
{
    return (GC) app_gc;
}

void video_add_handlers(ui_window_t w) 
{
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

void ui_finish_canvas(video_canvas_t *c)
{
    int depth;

    if (use_xvideo) {
        return;
    }

    depth = ui_get_display_depth();

    if (c->drawable)
	gdk_pixmap_unref(c->drawable);    
    
    c->drawable = gdk_pixmap_new(c->emuwindow->window, 
				 c->width, c->height, depth);
    gdk_window_set_back_pixmap(c->emuwindow->window, 
			       c->drawable, 0);
}

