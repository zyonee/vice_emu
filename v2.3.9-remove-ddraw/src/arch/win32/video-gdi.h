/*
 * video-ddraw.h - Video implementation for Win32, using DirectDraw.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#ifndef VICE_VIDEO_DDRAW_H
#define VICE_VIDEO_DDRAW_H

/* DDraw functions */
struct raster_s;
struct palette_s;
struct video_canvas_s;

extern struct video_canvas_s *video_canvas_create_gdi(struct raster_s *raster, unsigned int *width, unsigned int *height);
extern void video_canvas_refresh_gdi(struct raster_s *raster, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h);
extern void video_canvas_update_gdi(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h);

#endif
