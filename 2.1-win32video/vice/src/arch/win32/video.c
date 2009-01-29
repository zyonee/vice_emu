/*
 * video.c - Video implementation for Win32, using DirectDraw.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include <d3d9.h>

#include "cmdline.h"
#include "intl.h"
#include "lib.h"
#include "log.h"
#include "fullscrn.h"
#include "fullscreen.h"
#include "palette.h"
#include "res.h"
#include "resources.h"
#include "statusbar.h"
#include "translate.h"
#include "types.h"
#include "ui.h"
#include "video.h"
#include "videoarch.h"
#include "viewport.h"
#include "vsyncapi.h"

static int video_number_of_canvases;
static video_canvas_t *video_canvases[2];

static LPDIRECT3D9 d3d;


/* ------------------------------------------------------------------------ */
/* Video-related command-line options.  */

static const cmdline_option_t cmdline_options[] = {
    { NULL }
};


int video_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}


/* ------------------------------------------------------------------------ */
/* Video-related resources.  */
static const resource_int_t resources_int[] = {
    { NULL }
};

int video_arch_resources_init(void)
{
    return resources_register_int(resources_int);
}

void video_arch_resources_shutdown(void)
{
}


int video_init(void)
{
    return 0;
}


void video_shutdown(void)
{
}

int video_set_physical_colors(video_canvas_t *c)
{
    unsigned int i;
    COLORREF oldcolor = (COLORREF)0;
    DWORD mask = 0;
    int rshift = 0;
    int rbits = 0;
    int gshift = 0;
    int gbits = 0;
    int bshift = 0;
    int bbits = 0;
    DWORD rmask = 0;
    DWORD gmask = 0;
    DWORD bmask = 0;

    rshift = 16;
    rmask = 0xff;
    rbits = 0;

    gshift = 8;
    gmask = 0xff;
    gbits = 0;

    bshift = 0;
    bmask = 0xff;
    bbits = 0;
    if (c->depth > 8) {
        for (i = 0; i < 256; i++) {
            video_render_setrawrgb(i,
                ((i & (rmask << rbits)) >> rbits) << rshift,
                ((i & (gmask << gbits)) >> gbits) << gshift,
                ((i & (bmask << bbits)) >> bbits) << bshift);
        }
        video_render_initraw();
    }

    for (i = 0; i < c->palette->num_entries; i++) {
        DWORD p;

        p = (((c->palette->entries[i].red&(rmask << rbits)) >> rbits)
                << rshift) +
                (((c->palette->entries[i].green&(gmask << gbits)) >> gbits)
                << gshift) +
                (((c->palette->entries[i].blue&(bmask << bbits)) >> bbits)
                << bshift);
        video_render_setphysicalcolor(c->videoconfig, i, p, c->depth);
    }
    return 0;
}


void video_canvas_update(HWND hwnd, HDC hdc, int xclient, int yclient, int w,
                         int h)
{
}

video_canvas_t *video_canvas_for_hwnd(HWND hwnd)
{
    int i;

    for (i = 0; i < video_number_of_canvases; i++) {
        if (video_canvases[i]->hwnd == hwnd) {
            return video_canvases[i];
        }
    }

    return NULL;
}


void video_canvas_resize(video_canvas_t *canvas, unsigned int width,
                         unsigned int height)
{
    HRESULT ddresult;
    LPDIRECT3DSURFACE9 d3dbackbuffer = NULL;
    LPDIRECT3DSWAPCHAIN9 d3dsc;
    D3DPRESENT_PARAMETERS d3dpp;

    if (canvas->videoconfig->doublesizex)
        width *= 2;

    if (canvas->videoconfig->doublesizey)
        height *= 2;

    canvas->width = width;
    canvas->height = height;

    canvas->client_width = width;
    canvas->client_height = height;

    ui_resize_canvas_window(canvas);

    ddresult = IDirect3DSurface9_Release(canvas->d3dsurface);
    ddresult = IDirect3DDevice9_GetSwapChain(canvas->d3ddev, 0, &d3dsc);
    ddresult = IDirect3DSwapChain9_GetPresentParameters(d3dsc, &d3dpp);
    ddresult = IDirect3DSwapChain9_Release(d3dsc);

    ddresult = IDirect3DDevice9_GetBackBuffer(canvas->d3ddev, 0, 0, D3DBACKBUFFER_TYPE_MONO, &d3dbackbuffer);
    if (d3dbackbuffer)
        ddresult = IDirect3DSurface9_Release(d3dbackbuffer);


    d3dpp.BackBufferWidth = width;
    d3dpp.BackBufferHeight = height;

    ddresult = IDirect3DDevice9_TestCooperativeLevel(canvas->d3ddev);
    ddresult = IDirect3DDevice9_Reset(canvas->d3ddev, &d3dpp);
    
    ddresult = IDirect3DDevice9_CreateOffscreenPlainSurface(canvas->d3ddev, width, height, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &canvas->d3dsurface, NULL);
}


void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    canvas->video_draw_buffer_callback = NULL;
}


void video_canvas_refresh(video_canvas_t *canvas,
                          unsigned int xs, unsigned int ys,
                          unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h)
{
    HRESULT ddresult;
    LPDIRECT3DSURFACE9 d3dbackbuffer = NULL;
    D3DLOCKED_RECT lockedrect;

    if (canvas->videoconfig->doublesizex) {
        xi *= 2;
        w *= 2;
    }

    if (canvas->videoconfig->doublesizey) {
        yi *= 2;
        h *= 2;
    }

    ddresult = IDirect3DDevice9_BeginScene(canvas->d3ddev);
    ddresult = IDirect3DDevice9_GetBackBuffer(
                    canvas->d3ddev, 0, 0,
                    D3DBACKBUFFER_TYPE_MONO, &d3dbackbuffer);
    ddresult = IDirect3DSurface9_LockRect(
                    canvas->d3dsurface, &lockedrect, NULL, 0);

    video_canvas_render(canvas, lockedrect.pBits,
                        w, h, xs, ys, xi, yi,
                        lockedrect.Pitch, 32);

    ddresult = IDirect3DSurface9_UnlockRect(canvas->d3dsurface);
    ddresult = IDirect3DDevice9_StretchRect(
                    canvas->d3ddev, canvas->d3dsurface,
                    NULL, d3dbackbuffer, NULL, D3DTEXF_NONE);
    ddresult = IDirect3DSurface9_Release(d3dbackbuffer);
    ddresult = IDirect3DDevice9_EndScene(canvas->d3ddev);
    ddresult = IDirect3DDevice9_Present(canvas->d3ddev, NULL, NULL, NULL, NULL);
}


int video_canvas_set_palette(struct video_canvas_s *canvas, palette_t *p)
{
    canvas->palette = p;
    video_set_physical_colors(canvas);
    return 0;
}

video_canvas_t *video_canvas_create(video_canvas_t *canvas, unsigned int *width,
                                    unsigned int *height, int mapped)
{
    HRESULT ddresult;
    D3DPRESENT_PARAMETERS d3dpp;

    ui_open_canvas_window(canvas);

    d3d = Direct3DCreate9(D3D_SDK_VERSION);

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.hDeviceWindow = canvas->render_hwnd;
    d3dpp.BackBufferWidth = *width;
    d3dpp.BackBufferHeight = *height;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
    d3dpp.Flags = 0;
    d3dpp.EnableAutoDepthStencil = FALSE;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    ddresult = IDirect3D9_CreateDevice(
                    d3d,D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                    canvas->render_hwnd,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                    &d3dpp, &canvas->d3ddev);

    ddresult = IDirect3DDevice9_CreateOffscreenPlainSurface(
                    canvas->d3ddev,
                    *width, *height, D3DFMT_X8R8G8B8,
                    D3DPOOL_DEFAULT, &canvas->d3dsurface, NULL);

    if (video_set_physical_colors(canvas) < 0)
        return NULL;

    video_canvases[video_number_of_canvases++] = canvas;

    canvas->depth = 32;

    return canvas;
}


void video_canvas_destroy(video_canvas_t *canvas)
{
    if (canvas != NULL) {
        if (canvas->hwnd !=0) {
            DestroyWindow(canvas->hwnd);
        }
        if (canvas->title != NULL) {
            lib_free(canvas->title);
        }
        video_canvas_shutdown(canvas);
    }
}
