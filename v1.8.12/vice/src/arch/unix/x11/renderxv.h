/*
 * renderxv.c - XVideo rendering.
 *
 * Written by
 *  Dag Lem <resid@nimrod.no>
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

#ifndef _RENDERXV_H
#define _RENDERXV_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

#include "video/video-resources.h"

typedef void (*xv_render_function_t)(int pal_mode,
				     XvImage* image,
				     unsigned char* src,
				     int src_pitch,
				     unsigned int* src_color,
				     int src_x, int src_y,
				     unsigned int src_w, unsigned int src_h,
				     int dest_x, int dest_y);

typedef struct {
  char* format;
  xv_render_function_t render_function;
} xv_render_t;

int find_yuv_port(Display* display, XvPortID* port, int* format,
		  xv_render_t* render);

XvImage* create_yuv_image(Display* display, XvPortID port, int format,
			  int width, int height, XShmSegmentInfo* shminfo);

void destroy_yuv_image(Display* display, XvImage* image,
		       XShmSegmentInfo* shminfo);

void display_yuv_image(Display* display, XvPortID port, Drawable d, GC gc,
		       XvImage* image,
		       XShmSegmentInfo* shminfo,
		       int src_x, int src_y,
		       unsigned int src_w, unsigned int src_h,
		       unsigned int dest_w, unsigned int dest_h);

#endif /* _RENDERXV_H */
