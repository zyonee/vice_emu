/*
 * x11video.c - Simple Xaw-based graphical user interface.  It uses widgets
 * from the Free Widget Foundation and Robert W. McMullen.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
 *
 * Support for multiple visuals and depths by
 *  Teemu Rantanen <tvr@cs.hut.fi>
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "log.h"
#include "utils.h"
#include "videoarch.h"
#include "video.h"

static log_t x11video_log = LOG_ERR;

video_canvas_t *dangling_canvas = NULL;

void video_init_arch(void)
{
    if (x11video_log == LOG_ERR)
        x11video_log = log_open("X11Video");
}

/* Allocate a frame buffer. */
int video_frame_buffer_alloc(video_frame_buffer_t **ip, unsigned int width,
                             unsigned int height)
{
    int sizeofpixel = sizeof(PIXEL);
    int depth;
    Display *display;
    video_frame_buffer_t *i;
#ifdef USE_MITSHM
    int (*olderrorhandler)(Display*,XErrorEvent*);
    int dummy;
#endif

    i = (video_frame_buffer_t *)xmalloc(sizeof(video_frame_buffer_t));
    *ip = i;

#ifdef USE_MITSHM
    i->using_mitshm = use_mitshm;
#endif

    depth = ui_get_display_depth();
    display = ui_get_display_ptr();

    if (sizeof(PIXEL2) != sizeof(PIXEL) * 2 ||
	sizeof(PIXEL4) != sizeof(PIXEL) * 4) {
	log_error(x11video_log, "PIXEL2 / PIXEL4 typedefs have wrong size.");
	return -1;
    }
    /* Round up to 32-bit boundary. */
    width = (width + 3) & ~0x3;

#if VIDEO_DISPLAY_DEPTH == 0
    /* sizeof(PIXEL) is not always what we are using. I guess this should
       be checked from the XImage but I'm lazy... */
    if (depth > 8)
	sizeofpixel *= 2;
    if (depth > 16)
	sizeofpixel *= 2;
#endif

#ifdef USE_MITSHM
tryagain:
    if (i->using_mitshm) {
	DEBUG_MITSHM(("frame_buffer_alloc(): allocating XImage with MITSHM, "
		      "%d x %d pixels...", width, height));
	i->x_image = XShmCreateImage(display, visual, depth, ZPixmap,
				   NULL, &(i->xshm_info), width, height);
	if (!i->x_image) {
	    log_warning(x11video_log,
                        _("Cannot allocate XImage with XShm; falling back to non MITSHM extension mode."));
	    i->using_mitshm=0;
	    goto tryagain;
	}
	DEBUG_MITSHM(("Done."));
        DEBUG_MITSHM(("frame_buffer_alloc(): shmgetting %ld bytes...",
                      (long) i->x_image->bytes_per_line * i->x_image->height));
	i->xshm_info.shmid = shmget(IPC_PRIVATE, i->x_image->bytes_per_line *
				    i->x_image->height, IPC_CREAT | 0604);
	if (i->xshm_info.shmid == -1) {
	    log_warning(x11video_log,
                        _("Cannot get shared memory; falling back to non MITSHM extension mode."));
	    XDestroyImage(i->x_image);
	    i->using_mitshm=0;
	    goto tryagain;
	}
	DEBUG_MITSHM(("Done, id = 0x%x.", i->xshm_info.shmid));
        DEBUG_MITSHM(("frame_buffer_alloc(): getting address... "));
	i->xshm_info.shmaddr = shmat(i->xshm_info.shmid, 0, 0);
        i->x_image->data = i->xshm_info.shmaddr;
        if (i->xshm_info.shmaddr == (char *) -1) {
	    log_warning(x11video_log,
                       _("Cannot get shared memory address; falling back to non MITSHM extension mode."));
	    shmctl(i->xshm_info.shmid,IPC_RMID,0);
	    XDestroyImage(i->x_image);
	    i->using_mitshm=0;
	    goto tryagain;
	}
	DEBUG_MITSHM(("0x%lx OK.", (unsigned long) i->xshm_info.shmaddr));
	i->xshm_info.readOnly = True;
	mitshm_failed = 0;

	XQueryExtension(display,"MIT-SHM",&shmmajor,&dummy,&dummy);
	olderrorhandler = XSetErrorHandler(shmhandler);

	if (!XShmAttach(display, &(i->xshm_info))) {
	    log_warning(x11video_log,
                        _("Cannot attach shared memory; falling back to non MITSHM extension mode."));
	    shmdt(i->xshm_info.shmaddr);
	    shmctl(i->xshm_info.shmid,IPC_RMID,0);
	    XDestroyImage(i->x_image);
	    i->using_mitshm=0;
	    goto tryagain;
	}

	/* Wait for XShmAttach to fail or to succede. */
	XSync(display,False);
	XSetErrorHandler(olderrorhandler);

	/* Mark memory segment for automatic deletion. */
	shmctl(i->xshm_info.shmid, IPC_RMID, 0);

	if (mitshm_failed) {
	    log_warning(x11video_log,
                        _("Cannot attach shared memory; falling back to non MITSHM extension mode."));
	    shmdt(i->xshm_info.shmaddr);
	    XDestroyImage(i->x_image);
	    i->using_mitshm=0;
	    goto tryagain;
	}

	DEBUG_MITSHM((_("MITSHM initialization succeed.\n")));
        video_refresh_func((void (*)(void))XShmPutImage);
    } else
#endif
    {				/* !i->using_mitshm */
	PIXEL *data = (PIXEL *)xmalloc(width * height * sizeofpixel);

	if (!data)
	    return -1;
	i->x_image = XCreateImage(display, visual, depth, ZPixmap, 0,
				  (char *) data, width, height, 32, 0);
	if (!i->x_image)
	    return -1;

        video_refresh_func((void (*)(void))XPutImage);
    }

#ifdef USE_MITSHM
    log_message(x11video_log, _("Successfully initialized%s shared memory."),
                (i->using_mitshm) ? _(", using") : _(" without"));

    if (!(i->using_mitshm))
	log_warning(x11video_log, _("Performance will be poor."));
#else
    log_message(x11video_log,
                _("Successfully initialized without shared memory."));
#endif 

    if (video_convert_func(i, depth, width, height) < 0)
        return -1;

#ifdef USE_XF86_DGA2_EXTENSIONS
    fullscreen_set_framebuffer(i);
#endif 

    if (dangling_canvas != NULL)
        i->canvas = dangling_canvas;
    else
        i->canvas = NULL;

    return 0;
}

GC video_get_gc(XGCValues *gc_values)
{
    Display *display;

    display = ui_get_display_ptr();

    return XCreateGC(display, XtWindow(_ui_top_level), 0, gc_values);
}

void video_add_handlers(ui_window_t w)
{
    XtAddEventHandler(w, (EnterWindowMask | LeaveWindowMask | KeyReleaseMask
			  | KeyPressMask), True,
		      (XtEventHandler) kbd_event_handler, NULL);
    
    /* FIXME: ...REALLY ugly... */
    XtAddEventHandler(XtParent(w), (EnterWindowMask | LeaveWindowMask
				  | KeyReleaseMask | KeyPressMask), True,
		      (XtEventHandler) kbd_event_handler, NULL);
}

/* Make the canvas visible. */
void video_canvas_map(video_canvas_t *s)
{
    Display *display;

    display = ui_get_display_ptr();

    XMapWindow(display, s->drawable);
    XFlush(display);
}

/* Make the canvas not visible. */
void video_canvas_unmap(video_canvas_t *s)
{
    Display *display;

    display = ui_get_display_ptr();

    XUnmapWindow(display, s->drawable);
    XFlush(display);
}

void ui_finish_canvas(video_canvas_t *c)
{
    c->drawable = XtWindow(c->emuwindow);
}


