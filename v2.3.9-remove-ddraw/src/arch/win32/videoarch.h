/*
 * video.h - Video implementation for Win32, using DirectDraw.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef VICE_VIDEOARCH_H
#define VICE_VIDEOARCH_H

/*  Nasty patch to avoid defining BYTE, WORD, DWORD more than once.  */

#undef BYTE
#undef WORD
#undef DWORD

#if defined(HAVE_D3D9_H) && !defined(RC_INVOKED)
#include <d3d9.h>
#endif

#include "types.h"
#include "video.h"

#define CANVAS_USES_TRIPLE_BUFFERING(c) 0

typedef struct video_canvas_s {
    char *title;
    int width, height;
    int mapped;
    int depth;
    float refreshrate; /* currently displayed refresh rate */
    BITMAPINFO bmp_info;
    BYTE *pixels;
    HWND hwnd;
    HWND render_hwnd;
    HWND client_hwnd;
#ifdef HAVE_D3D9_H
    LPDIRECT3DDEVICE9 d3ddev;
    LPDIRECT3DSURFACE9 d3dsurface;
    D3DPRESENT_PARAMETERS d3dpp;
    RECT *dest_rect_ptr;
    RECT dest_rect;
#endif
    int client_width;
    int client_height;
} video_canvas_t;

/* ------------------------------------------------------------------------ */

extern int video_create_triple_surface(struct video_canvas_s *canvas, int width, int height);

extern struct raster_s *video_canvas_for_hwnd(HWND hwnd);
extern int video_canvas_nr_for_hwnd(HWND hwnd);
extern void video_canvas_add(struct raster_s *raster);

extern void video_canvas_update(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h);

extern float video_refresh_rate(video_canvas_t *c);

extern int video_set_physical_colors(video_canvas_t *c, struct palette_s *palette, struct video_render_color_tables_s *color_tables);

#ifdef HAVE_D3D9_H
extern int video_dx9_enabled(void);
extern int video_dx9_available(void);
#endif


/* FIXME: ugly */
extern int fullscreen_enabled;
extern int dx_primary_surface_rendering;

#endif
