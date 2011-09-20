/*
 * video-dx9.h - Video implementation for Win32, using Direct3D 9.
 *
 * Written by
 *  Fabrizio Gennari <fabrizio.ge@tiscali.it>
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

#ifdef HAVE_D3D9_H

#include <d3d9.h>

struct raster_s;
struct video_canvas_s;

/* DX9 functions */
extern int video_setup_dx9(void);
extern void video_shutdown_dx9(void);
extern int video_device_create_dx9(struct raster_s *raster, int fullscreen);
extern struct video_canvas_s *video_canvas_create_dx9(struct raster_s *raster, unsigned int *width, unsigned int *height);
extern void video_device_release_dx9(struct video_canvas_s *canvas);
extern HRESULT video_canvas_reset_dx9(struct video_canvas_s *canvas);
extern int video_canvas_refresh_dx9(struct raster_s *raster, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h);
extern void video_canvas_update_dx9(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h);

extern int video_set_palette(struct video_canvas_s *c);

extern LPDIRECT3D9 d3d;

#endif /* HAVE_D3D9_H */
