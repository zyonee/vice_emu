/*
 * video-dx9.c - Video using DirectX9 for Win32
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

#include "fullscrn.h"
#include "log.h"
#include "ui.h"
#include "video.h"
#include "videoarch.h"


typedef IDirect3D9    *(WINAPI* Direct3DCreate9_t)  (UINT SDKVersion);
static Direct3DCreate9_t  DynDirect3DCreate9;
static HINSTANCE d3d9dll = NULL;

static LPDIRECT3D9 d3d;


int video_setup_dx9(void)
{
    d3d9dll = LoadLibrary("d3d9.dll");
    if (d3d9dll == NULL)
        return -1;
    DynDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(d3d9dll, "Direct3DCreate9");
    if (!DynDirect3DCreate9)
        return -1;
    return 0;
}


int video_device_create_dx9(video_canvas_t *canvas, int fullscreen)
{
    HRESULT ddresult;
    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
    d3dpp.Flags = 0;
    d3dpp.EnableAutoDepthStencil = FALSE;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    if (fullscreen) {
        int width, height, bitdepth, refreshrate;
        GetCurrentModeParameters(&width, &height, &bitdepth, &refreshrate);
        d3dpp.Windowed = FALSE;
        d3dpp.BackBufferWidth = width;
        d3dpp.BackBufferHeight = height;
    } else {
        d3dpp.Windowed = TRUE;
        d3dpp.BackBufferWidth = canvas->width;
        d3dpp.BackBufferHeight = canvas->height;
    }

    ddresult = IDirect3D9_CreateDevice(
                    d3d,D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                    canvas->render_hwnd,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                    &d3dpp, &canvas->d3ddev);

    ddresult += IDirect3DDevice9_CreateOffscreenPlainSurface(
                    canvas->d3ddev,
                    canvas->width, canvas->height, D3DFMT_X8R8G8B8,
                    D3DPOOL_DEFAULT, &canvas->d3dsurface, NULL);

	log_debug("video_device_create returns %d", ddresult);

    return ddresult;
}


video_canvas_t *video_canvas_create_dx9(video_canvas_t *canvas, 
                                        unsigned int *width,
                                        unsigned int *height, int mapped)
{
    canvas->depth = 32;

    d3d = DynDirect3DCreate9(D3D_SDK_VERSION);

    if  (video_device_create_dx9(canvas, 0) != 0)
        return NULL;

    if (video_set_physical_colors(canvas) < 0)
        return NULL;

    video_canvas_add(canvas);

    log_debug("Device created");

    return canvas;
}


void video_device_release_dx9(video_canvas_t *canvas)
{
    HRESULT ddresult;

    ddresult = IDirect3DSurface9_Release(canvas->d3dsurface);
    ddresult = IDirect3DDevice9_Release(canvas->d3ddev);
}


void video_canvas_resize_dx9(video_canvas_t *canvas, unsigned int width,
                         unsigned int height)
{
    HRESULT ddresult;
    LPDIRECT3DSURFACE9 d3dbackbuffer = NULL;
    LPDIRECT3DSWAPCHAIN9 d3dsc;
    D3DPRESENT_PARAMETERS d3dpp;


    log_debug ("Begin resize DX9 device");
    ddresult = IDirect3DSurface9_Release(canvas->d3dsurface);
    ddresult = IDirect3DDevice9_GetSwapChain(canvas->d3ddev, 0, &d3dsc);
    ddresult = IDirect3DSwapChain9_GetPresentParameters(d3dsc, &d3dpp);
    ddresult = IDirect3DSwapChain9_Release(d3dsc);

    d3dpp.BackBufferWidth = width;
    d3dpp.BackBufferHeight = height;

    ddresult = IDirect3DDevice9_TestCooperativeLevel(canvas->d3ddev);
    ddresult = IDirect3DDevice9_Reset(canvas->d3ddev, &d3dpp);
    
    ddresult = IDirect3DDevice9_CreateOffscreenPlainSurface(
                    canvas->d3ddev, width, height,
                    D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
                    &canvas->d3dsurface, NULL);

    log_debug ("End resize DX9 device");
}


void video_canvas_refresh_dx9(video_canvas_t *canvas,
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

	//log_debug("video_canvas_refresh: %d/%d -> %d/%d (%d/%d)",xs,ys,xi,yi,w,h);

    ddresult = IDirect3DDevice9_BeginScene(canvas->d3ddev);
	//log_debug("BeginScene: %d", ddresult);
    ddresult = IDirect3DDevice9_GetBackBuffer(
                    canvas->d3ddev, 0, 0,
                    D3DBACKBUFFER_TYPE_MONO, &d3dbackbuffer);
	//log_debug("GetBackBuffer: %d", ddresult);
    ddresult = IDirect3DSurface9_LockRect(
                    canvas->d3dsurface, &lockedrect, NULL, 0);

	//log_debug("LockRect: %d", ddresult);
    video_canvas_render(canvas, lockedrect.pBits,
                        w, h, xs, ys, xi, yi,
                        lockedrect.Pitch, 32);

    ddresult = IDirect3DSurface9_UnlockRect(canvas->d3dsurface);
	//log_debug("UnlockRect: %d", ddresult);
    ddresult = IDirect3DDevice9_StretchRect(
                    canvas->d3ddev, canvas->d3dsurface,
                    NULL, d3dbackbuffer, NULL, D3DTEXF_LINEAR);
	//log_debug("StretchRect: %d", ddresult);
    ddresult = IDirect3DSurface9_Release(d3dbackbuffer);
	//log_debug("Release: %d", ddresult);
    ddresult = IDirect3DDevice9_EndScene(canvas->d3ddev);
	//log_debug("EndScene: %d", ddresult);
    ddresult = IDirect3DDevice9_Present(canvas->d3ddev, NULL, NULL, NULL, NULL);
	//log_debug("Present: %d", ddresult);
}
